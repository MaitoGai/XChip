#include <XChip/Emulator.h>
#include <XChip/Fonts.h>
#include <XChip/Interfaces/iRender.h>
#include <XChip/Interfaces/iInput.h>
#include <XChip/Interfaces/iSound.h>
#include <XChip/Instructions.h>
#include <XChip/Utility/Log.h>
#include <XChip/Utility/Alloc.h>

namespace xchip {

Emulator::Emulator() noexcept
{
	using namespace utility::literals;
	_instrTimer.SetTargetTime(358_hz);
	_frameTimer.SetTargetTime(60_hz);
	utility::LOG("Creating Emulator object...");
}


Emulator::~Emulator()
{
	if(_initialized) 
		this->Dispose();

	utility::LOG("Destroying Emulator object...");
}


bool Emulator::Initialize(UniqueRender render, UniqueInput input, UniqueSound sound) noexcept
{
	if (_initialized) 
		this->Dispose();

	const auto scope = utility::make_scope_exit([this]() {
		if (!this->_initialized)
			this->Dispose();
	});

	// Default Chip8 Mode
	if (!_manager.SetMemory(0xFFFF)
		|| !_manager.SetRegisters(0x10)
		|| !_manager.SetStack(0x10)
		|| !_manager.SetGfx(64 * 32))
	{
		return false;
	}

	_manager.SetFont(fonts::chip8_default_font, 80);

	if (!InitMedia(render.release(), input.release(), sound.release()))
		return false;

	// little trick:
	// use the last  ( sizeof(bool*) + sizeof(uint8_t) ) bytes in chip's memory to store a pointer
	// to our exit flag and the 0xFF value to say that the pointer is available
	// ( normaly 5 bytes in x86 and 9 bytes in x64).
	const auto flagOffset = (_manager.GetMemorySize() - sizeof(bool*)) - 1;
	_manager.InsertAddress(&_exitf, flagOffset);
	_manager.InsertValue(uint8_t(0xFF), flagOffset - sizeof(uint8_t));
	_exitf = false;
	_initialized = true;
	return true;
}



void Emulator::Dispose() noexcept
{
	_manager.Dispose();
	_instrf = false;
	_drawf = false;
	_exitf = true;
	_initialized = false;
}




void Emulator::HaltForNextFlag() const
{
	if (!_instrf && !_drawf)
	{
		const auto instrRemain = _instrTimer.GetRemain();
		const auto frameRemain = _frameTimer.GetRemain();
		utility::Timer::Halt((instrRemain < frameRemain) ? instrRemain : frameRemain);
	}
}



void Emulator::UpdateTimers()
{
	using namespace utility::literals;

	if (!_instrf && _instrTimer.Finished())
	{
		_instrf = true;
		_instrTimer.Start();
	}

	if (!_drawf && _frameTimer.Finished())
	{
		_drawf = true;
		_frameTimer.Start();
	}

	
	static utility::Timer chip8Delay( 60_hz );

	if (chip8Delay.Finished())
	{
		auto& delayTimer = _manager.GetCpu().delayTimer;
		if (delayTimer)
			--delayTimer;

		chip8Delay.Start();
	}
}



void Emulator::UpdateSystems()
{
	_manager.GetRender()->UpdateEvents();
	_manager.GetInput()->UpdateKeys();
	this->UpdateTimers();
}




void Emulator::ExecuteInstr()
{
	Cpu& chip = _manager.GetCpu();
	chip.opcode = (chip.memory[chip.pc] << 8) | chip.memory[chip.pc+1];
	chip.pc += 2;
	instructions::instrTable[(chip.opcode & 0xf000) >> 12](&chip);
	_instrf = false;
}





void Emulator::Draw()
{
	_manager.GetRender()->DrawBuffer();
	_drawf = false;
}





void Emulator::Reset()
{
	if(_manager.GetSound()->IsPlaying())
		_manager.GetSound()->Stop();

	_manager.CleanGfx();
	_manager.CleanStack();
	_manager.CleanRegisters();
	_manager.SetPC(0);
	_manager.SetSP(0);
}




inline 
bool Emulator::InitMedia(iRender* rend, iInput* input, iSound* sound)
{
	_manager.SetRender(rend);
	_manager.SetInput(input);
	_manager.SetSound(sound);
	return InitRender() && InitInput() && InitSound();
}


bool Emulator::InitRender()
{
	auto rend = _manager.GetRender();

	if (!rend)  {
		utility::LOGerr("Cannot Initialize iRender: nullptr");
		return false;
	} 
	else if (rend->IsInitialized()) {
		return true;
	} 
	else if (!rend->Initialize(64, 32)) {
		return false;
	}

	rend->SetBuffer(_manager.GetGfx());
	rend->SetWinCloseCallback(&_exitf, [](const void* exitf) {*(bool*)exitf = true; });
	return true;
}




bool Emulator::InitInput()
{
	auto input = _manager.GetInput();

	if (!input) {
		utility::LOGerr("ERROR: Cannot Initialize iInput: nullptr");
		return false;
	}
	else if (input->IsInitialized()) {
		return true;
	}
	else if (!input->Initialize()) {
		return false;
	}

	input->SetEscapeKeyCallback(&_exitf, [](const void* exitf) { *(bool*)exitf = true; });
	input->SetResetKeyCallback(this, [](const void* _this) {
		((Emulator*)_this)->Reset();
	});


	input->SetWaitKeyCallback(this, [](const void* emu)
	{
		auto *emulator = (Emulator*) emu;
		do
		{
			emulator->UpdateSystems();
			emulator->HaltForNextFlag();

			if (emulator->GetExitFlag()) 
				return false;

			if (emulator->GetDrawFlag())
				emulator->Draw();

		} while (! emulator->GetInstrFlag());

		return true;
	});

	return true;
}



bool Emulator::InitSound()
{
	auto sound = _manager.GetSound();
	if (!sound)  {
		utility::LOGerr("Cannot Initialize iSound: nullptr");
		return false;
	}
	else if (sound->IsInitialized()) {
		return true;
	}
	else if (!sound->Initialize()) {
		return false;
	}

	sound->SetCountdownFreq(60);
	return true;
}












}
