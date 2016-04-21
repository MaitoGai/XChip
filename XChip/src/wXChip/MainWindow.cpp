/* wXChip
   (c) 2016
*/

#include <sstream>
#include <wXChip/MainWindow.h>
#include <wXChip/SaveList.h>
#include<thread>
#ifdef __APPLE__
#include <dispatch/dispatch.h>
#endif
#ifdef _WIN32
#include<windows.h>
#endif

#if defined(__APPLE__) || defined(__linux__)
#include <dirent.h>
#elif defined(_WIN32)
#include <wXChip/dirent.h>
#endif


enum { ID_Chip = 1, ID_LISTBOX = 2, ID_STARTROM = 3, ID_LOADROM = 4, ID_TEXT = 5, ID_EMUSET, ID_TIMER1};

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
EVT_MENU(ID_Chip,   MainWindow::OnChip)
EVT_MENU(wxID_EXIT,  MainWindow::OnExit)
EVT_MENU(ID_EMUSET,  MainWindow::LoadSettings)
EVT_CLOSE(MainWindow::OnWindowClose)
EVT_MENU(wxID_ABOUT, MainWindow::OnAbout)
EVT_MOTION(MainWindow::OnMouseOver)
EVT_BUTTON(ID_STARTROM, MainWindow::OnStartRom)
EVT_BUTTON(ID_LOADROM, MainWindow::OnChip)
EVT_BUTTON(ID_EMUSET, MainWindow::LoadSettings)
EVT_TIMER(ID_TIMER1, MainWindow::OnTimer)
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(wXChip);


void RunEmulator::init()
{
	using xchip::Emulator;
	using xchip::SdlRender;
	using xchip::SdlInput;
	using xchip::SdlSound;
	using xchip::UniqueRender;
	using xchip::UniqueInput;
	using xchip::UniqueSound;
	using xchip::utility::make_unique;
	

	UniqueRender render;
	UniqueInput input;
	UniqueSound sound;
	
	try {
		render = make_unique<SdlRender>();
		input = make_unique<SdlInput>();
		sound = make_unique<SdlSound>();
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return;
	}
	
	if (!emu.Initialize(std::move(render), std::move(input), std::move(sound)))
	{
		return;
	}
	
	
	emu.GetSound()->SetCountdownFreq(35.88f);
	
	// you can also:
	
	// emu.GetInput()->  whatever
	// emu.GetRender()-> whatever
	
	
	
	// remember to not use the old unique_ptr
	///render->DrawBuffer(); // ERROR we've moved it to the emulator
	
	// but we can get it back if we want
	render = emu.SwapRender(nullptr);
	// get our render back and set emulators render to nullptr
	// note that setting some emulator media interface to null
	// will set ExitFlag on.
	
	
	// lets check our default game color
	auto color = render->GetColorFilter();
	std::cout << "Default Color Filter: " << color << std::endl;
	
	// lets set our game color RED
	color = { 255, 0, 0 };
	
	if(!render->SetColorFilter(color))
	{
		std::cout << "could not set new color filter" << std::endl;
	}
	else
	{
		color = render->GetColorFilter();
		std::cout << "New Color Filter: " << color << std::endl;
	}
	
	// don't forget to put the render back!!
	// but because we have seted it to null before
	// now we need to clean flags
	emu.SetRender(std::move(render));
	emu.CleanFlags();
	
	
	// ok, now you may want to set some
	// emulator settings
	// lets show the default FPS and CPU Frequency
	std::cout << "Default FPS: " << emu.GetFps() << std::endl;
	std::cout << "Default CPU Freq: " << emu.GetCpuFreq() << std::endl;
	
	emu.SetFps(120); // I want 120 fps
	emu.SetCpuFreq(485); // I want 485 instructions per second
	
	std::cout << "New FPS: " << emu.GetFps() << std::endl;
	std::cout << "New CPU Freq: " << emu.GetCpuFreq() << std::endl;
	closing = false;
	
}


void RunEmulator::stop()
{

}

void RunEmulator::update() {
		emu.UpdateSystems(); // update window events / input events / timers / flags
		emu.HaltForNextFlag(); // sleep until instrFlag or drawFlag is TRUE
	
	if (emu.GetInstrFlag()) // if instrFLag is true, is time to execute one instruction
		emu.ExecuteInstr();
	if (emu.GetDrawFlag()) // if drawFlag is true, is time to the frame
		emu.Draw();
}

bool RunEmulator::load(const std::string &text) {
	return emu.LoadRom(text);
}

bool wXChip::OnInit()
{
	render_loop_on = false;
	using xchip::utility::make_unique;
	try {
		std::string fps_val, cpu_freq;
		const std::string file = getDirectory(fps_val, cpu_freq);
		auto frame = make_unique<MainWindow>( "wXChip ", wxPoint(50, 50), wxSize(800, 600) );
		
		if(file != "nolist")
			frame->LoadList(file, fps_val, cpu_freq);
	
		main = frame.get();
		
		frame->Show( true );
		
		frame.release();
	} catch(std::exception &e) {
		xchip::utility::LOGerr(e.what());
		return false;
	}
	activateRenderLoop(true);
	return true;
}

void wXChip::activateRenderLoop(bool on)
{
	if(on && !render_loop_on)
	{
		Connect( wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(wXChip::onIdle) );
		render_loop_on = true;
	}
	else if(!on && render_loop_on)
	{
		Disconnect( wxEVT_IDLE, wxIdleEventHandler(wXChip::onIdle) );
		render_loop_on = false;
	}
}
void wXChip::onIdle(wxIdleEvent& evt)
{
	if(render_loop_on)
	{

		main->UpdateEmulator();
		evt.RequestMore(); // render continuously, not only once on idle
	}
}



MainWindow::MainWindow(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame(NULL, wxID_ANY, title, pos, size, wxCAPTION | wxSYSTEM_MENU | wxMINIMIZE_BOX | wxCLOSE_BOX), _timer(this, ID_TIMER1)
{
	using xchip::utility::make_unique;
	running = false;
	auto menuFile = make_unique<wxMenu>();
	menuFile->Append(ID_Chip, "&Load Roms...\tCtrl-L", 
                         "Load Roms");
    
	menuFile->Append(ID_EMUSET, "&Settings\tCtrl-S", "Settings");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	auto menuHelp = make_unique<wxMenu>();
	menuHelp->Append(wxID_ABOUT);

	auto menuBar = make_unique<wxMenuBar>();
	menuBar->Append( menuFile.release(), "&File" );
	menuBar->Append( menuHelp.release(), "&Help" );
	SetMenuBar( menuBar.release() );

	CreateStatusBar();
	SetStatusText( "Welcome to wXChip" );
	
	CreateControls();
	
	SetMinSize(GetSize());
	SetMaxSize(GetSize());
	
	emulator = nullptr;
}

void MainWindow::CreateControls()
{
	using xchip::utility::make_unique;
	wxArrayString strings;
	_panel = make_unique<wxPanel>(this, wxID_ANY);
	_text = make_unique<wxStaticText>(_panel.get(), ID_TEXT, _T("Chip8 Roms"), wxPoint(10,10), wxSize(100,25));
	_listBox = make_unique<wxListBox>(_panel.get(), ID_LISTBOX, wxPoint(10, 35), wxSize(620, 360), strings, wxLB_SINGLE);
	_listBox->Connect(wxEVT_LEFT_DCLICK, wxMouseEventHandler(MainWindow::OnLDown), NULL, this);
	_startRom = make_unique<wxButton>(_panel.get(), ID_STARTROM, _T("Start Rom"), wxPoint(10, 400), wxSize(100,25));
	_settings = make_unique<wxButton>(_panel.get(), ID_LOADROM, _T("Load Roms"), wxPoint(120, 400), wxSize(100,25));
	_emulatorSettings = make_unique<wxButton>(_panel.get(), ID_EMUSET, _T("Settings"), wxPoint(230, 400), wxSize(100,25));
	_settingsWin = make_unique<SettingsWindow>("wXChip - Settings", wxPoint(150, 150), wxSize(430, 220));
}


void MainWindow::OnLDown(wxMouseEvent& event)
{
	auto m_lbox = static_cast<wxListBox*>(event.GetEventObject());

	// Get the item index
	int item = m_lbox->HitTest(event.GetPosition());
    
	if ( item != wxNOT_FOUND ) 
	{
		wxString str = m_lbox->GetString(item);
		std::ostringstream stream;
		stream << _filePath << "/" << str.c_str();
		std::string fullname = stream.str();
		std::cout << "Start Rom At Path: " << fullname << "\n";
		//_timer.Stop();
		StartProgram(fullname);
	}
}

void MainWindow::LaunchRom()
{
	// Get the item index
	int item = _listBox->GetSelection();
	
	if (item != wxNOT_FOUND )
	{
		const wxString str = _listBox->GetString(item);
		std::ostringstream stream;
		stream << _filePath << "/" << str.c_str();
		const std::string fullname = stream.str();
		std::cout << "Start Rom At Path: " << fullname << "\n";
		//if(running != true)
//		_timer.Stop();
		StartProgram(fullname);
	}
}

void MainWindow::OnStartRom(wxCommandEvent &event)
{
	std::cout << "Starting Rom...\n";
	LaunchRom();
    // start application
}

void MainWindow::LoadSettings(wxCommandEvent &event)
{
	_settingsWin->Show(true);
}

void MainWindow::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void MainWindow::OnAbout(wxCommandEvent& event)
{
	wxMessageBox( "wXChip - Chip8 Emulator",
                     "About wXChip", wxOK | wxICON_INFORMATION );
}

void MainWindow::OnMouseOver(wxMouseEvent &event)
{
    
}

void MainWindow::OnSize(wxSizeEvent& event)
{
    
}


void MainWindow::OnWindowClose(wxCloseEvent &event)
{

	wxGetApp().activateRenderLoop(false);
	event.Skip(); // don't stop event, we still want window to close
	running = false;
	delete emulator;
	Destroy();
	// Cleanup here}
}

void MainWindow::LoadList(const std::string &text, const std::string &fps, std::string &cpu_freq)
{

	saveDirectory(text, fps, cpu_freq);

	if(text == "nopath") return;
	
	wxArrayString strings;

	_listBox->Clear();

	DIR *dir = opendir(text.c_str());

	if(dir == NULL)
	{
		std::cerr << "Error could not open directory.\n";
		return;
	}
    
	dirent *e;

	while((e = readdir(dir)))
	{
		if(e->d_type == DT_REG)
		{
			wxString w(e->d_name);
			strings.Add(w);
		}
	}
    
	closedir(dir);
	
	
	if(!strings.IsEmpty())
	{
		_listBox->InsertItems(strings, 0);
		_filePath = text;
		_settingsWin->setRomPath(text, fps, cpu_freq);

	}
	else
 	{
		_settingsWin->setRomPath("", fps, cpu_freq);

	}
}







void MainWindow::OnChip(wxCommandEvent& event)
{
	wxDirDialog dlg(NULL, "Choose input directory", "",
                        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

	if (dlg.ShowModal() == wxID_CANCEL)
		return;


	wxString value = dlg.GetPath();
	std::string fps = _settingsWin->FPS();
	std::string cpu = _settingsWin->CPUFreq();
	LoadList(std::string(value.c_str()), fps, cpu);
}



void MainWindow::StartProgram(const std::string &rom)
{
	if(current_rom == "")
 	{
		current_rom = rom;
	}
	else if(current_rom == rom) return;
	
	
	if(emulator == nullptr) {
		emulator = new RunEmulator();
		emulator->init();
		emulator->load(rom);
		running = true;
	} else {
		emulator->init();
		emulator->load(rom);
		running = true;
	}

}

void MainWindow::UpdateEmulator() {
	
	if(running == true) {
		if(!emulator->emu.GetExitFlag() && emulator->closing == false)
		{
			emulator->update();
		}
		else
			running = false;
	}
}

void MainWindow::OnTimer(wxTimerEvent &te)
{
	/*
	if(emu->GetExitFlag()) {
		emu->Dispose();
		_timer.Stop();
		return;
	}
	if(closing == true) return;
	emu->UpdateSystems(); // update window events / input events / timers / flags
	emu->HaltForNextFlag(); // sleep until instrFlag or drawFlag is TRUE
	
	if (emu->GetInstrFlag()) // if instrFLag is true, is time to execute one instruction
		emu->ExecuteInstr();
	if (emu->GetDrawFlag()) // if drawFlag is true, is time to the frame
		emu->Draw();
	 */
}
