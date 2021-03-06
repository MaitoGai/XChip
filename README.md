# XChip
XChip is a Chip8/SuperChip emulator/interpreter designed to be flexible.
You can easily use the project's API to modify the emulator.
The XChip uses 3 plugins: iRender, iInput, iSound.
You can create your own plugins from those interfaces 
and use it with the emulator as shared libraries.



## Build:
Use CMake to build on Linux and Mac.
And use the Visual Studio project to build on Windows. 




#### Some Chip8 games running:
![ScreenShot](https://github.com/dhustkoder/XChip/blob/master/Tools/Images/BRIX.png?raw=true "BRIX.ch8")
![ScreenShot](https://github.com/dhustkoder/XChip/blob/master/Tools/Images/UFO.png?raw=true "UFO.ch8")

####Some SuperChip games running:
![ScreenShot](https://github.com/dhustkoder/XChip/blob/master/Tools/Images/CAR.png?raw=true "CAR.ch8")
![ScreenShot](https://github.com/dhustkoder/XChip/blob/master/Tools/Images/BLINKY.png?raw=true "BLINKY.ch8")

You can use command line arguments to set: CPU Frequency, Sound Frequency, Draw Color, Background Color, Window Size, enable fullscreen...
We're working on a wxWidgets GUI to easily set those parameters and call your games.
Its available for Mac OSX, Linux, Windows

![ScreenShot](https://github.com/dhustkoder/XChip/blob/master/Tools/Images/WXChip.png?raw=true "WXChip")


If you would like to contribute, please follow 
[Dolphin guideline] (https://github.com/dolphin-emu/dolphin/blob/master/Contributing.md)
