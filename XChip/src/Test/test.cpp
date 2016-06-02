/*

XChip - A chip8 lib and emulator.
Copyright (C) 2016  Rafael Moura

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

#include <csignal>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#endif
#include <XChip/Plugins/iPlugin.h>

int main(int argc, char **argv)
{
	using xchip::PluginLoader;
	using xchip::PluginDeleter;

	// need to make sure that the functions that the 
	// dynamic library used from Utility / Core are
	// linked into the caller binary. Cuz 
	// Utility and Core are not compiled with -fPIC
	// so they can't be linked to a shared library.
	// but if needed, the -fPIC flag can be added easily.

	// removing these lines causes segmentation fault
	//volatile auto x=&xchip::utility::LogError;
	//volatile auto y=&xchip::utility::Log;

#ifdef _WIN32
	auto handle = LoadLibrary("XChipSFMLInput.dll");
	
	auto loadFun = (PluginLoader)GetProcAddress(handle, XCHIP_LOAD_PLUGIN_SYM);

	auto freeFun = (PluginDeleter)GetProcAddress(handle, XCHIP_FREE_PLUGIN_SYM);


	auto plugin = loadFun();
	std::cout << plugin->GetPluginName() << std::endl;
	std::cout << plugin->GetPluginVersion() << std::endl;

	freeFun(plugin);
	FreeLibrary(handle);

#elif defined(__linux__) || defined(__APPLE__)
	auto handle = dlopen("./XChipSFMLInput.so", RTLD_LAZY);

	auto loadFun = (LoadPtr)dlsym(handle, "XCHIP_LoadPlugin");

	auto plugin = loadFun();

	std::cout << plugin->GetPluginName() << std::endl;
	std::cout << plugin->GetPluginVersion() << std::endl;

	auto freeFun = (FreePtr)dlsym(handle, "XCHIP_FreePlugin");

	freeFun(plugin);

	dlclose(handle);
#endif
	return EXIT_SUCCESS;
}












