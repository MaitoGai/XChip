/*

WXChip - chip8 emulator using XChip library and a wxWidgets gui.
Copyright (C) 2016  Jared Bruni, Rafael Moura.


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/gpl-3.0.html.

*/

#ifndef WXCHIP_MAINWINDOW_H_
#define WXCHIP_MAINWINDOW_H_


#include <string>
#include <wx/frame.h>
#include <Utix/Process.h>
#include <WXChip/SettingsWindow.h>

class MainWindow final : public wxFrame
{
#ifdef _WIN32
	constexpr static const char* const default_emuapp_relative_path = "bin\\EmuApp";
#elif defined(__linux__) || defined(__APPLE__)
	constexpr static const char* const default_emuapp_relative_path = "bin/EmuApp";
#else
	#error CANNOT PICK EMUAPP_RELATIVE_PATH
#endif

public:
	enum 
	{ 
		ID_MENU_BAR_LOAD_ROM, ID_ROMS_TEXT, ID_LISTBOX, 
		ID_BUTTON_LOAD_ROM, ID_BUTTON_SELECT_DIR, ID_BUTTON_SETTINGS
	};

	MainWindow(const wxString& title, const wxPoint& pos, const wxSize& size);
	~MainWindow();


private:
	void CreateControls();
	void CreateMenuBar();
	void StartEmulator();
	void StopEmulator();
	void ComputeEmuAppPath();


	void OnExit(wxCommandEvent& ev);
	void OnClose(wxCloseEvent& ev);
	void OnAbout(wxCommandEvent& ev);
	void OnListBoxDoubleClick(wxMouseEvent& ev);
	void OnListBoxMouseUp(wxMouseEvent& ev);
	void OnButtonLoadRom(wxCommandEvent& ev);
	void OnButtonSettings(wxCommandEvent& ev);
	void OnButtonSelectDir(wxCommandEvent& ev);
	void OnMenuBarLoadRom(wxCommandEvent& ev);

	wxString m_emuAppPath;
	wxString m_romPath;
	utix::Process m_process;

	std::unique_ptr<wxPanel> m_panel;
	std::unique_ptr<wxStaticText> m_romsTxt;
	std::unique_ptr<wxListBox> m_listBox;
	std::unique_ptr<wxButton> m_buttonLoadRom;
	std::unique_ptr<wxButton> m_buttonSelectDir;
	std::unique_ptr<wxButton> m_buttonSettings;
	std::unique_ptr<SettingsWindow> m_settingsWin;
	wxDECLARE_EVENT_TABLE();
};





#endif // WXCHIP_MAINWINDOW_H_
