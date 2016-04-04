#ifndef _XCHIP_CPU_MANAGER_H_
#define _XCHIP_CPU_MANAGER_H_
#include <cstddef>
#include "Cpu.h"



namespace xchip {

class CpuManager
{
public:
	CpuManager() noexcept;
	~CpuManager();
	CpuManager(const CpuManager&) = delete;
	CpuManager& operator=(const CpuManager&) = delete;

	void Dispose() noexcept;

	size_t GetMemorySize() const;
	size_t GetRegistersSize() const;
	size_t GetStackSize() const;
	size_t GetGfxSize() const;

	const iRender* GetRender() const { return _cpu.render; }
	const iInput* GetInput() const { return _cpu.input; }
	const iSound* GetSound() const { return _cpu.sound; }
	const uint8_t* GetMemory() const { return _cpu.memory; }
	const uint8_t* GetRegisters() const { return _cpu.registers; }
	const uint16_t* GetStack() const { return _cpu.stack; }
	const uint32_t* GetGfx() const { return _cpu.gfx; }
	const Cpu& GetCpu() const { return _cpu; }

	Cpu& GetCpu() { return _cpu; }
	iRender* GetRender() { return _cpu.render; }
	iInput* GetInput() { return _cpu.input; }
	iSound* GetSound() { return _cpu.sound; }
	
	void SetRender(iRender* render);
	void SetInput(iInput* input);
	void SetSound(iSound* sound);

	bool SetMemory(const size_t size);
	bool SetRegisters(const size_t size);
	bool SetStack(const size_t size);
	bool SetGfx(const size_t size);
	void SetFont(const uint8_t* font, const size_t size);
	bool LoadRom(const char* file);

	
	iRender* SwapRender(iRender* render);
	iInput* SwapInput(iInput* input);
	iSound* SwapSound(iSound* sound);


	void CleanMemory();
	void CleanRegisters();
	void CleanStack();
	void CleanGfx();
private:
	Cpu _cpu;

};















} // xchip namespace

#endif
