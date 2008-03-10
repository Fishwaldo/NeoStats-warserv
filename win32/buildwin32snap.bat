@echo off
IF EXIST win32\WarServ-Setup*.exe del win32\WarServ-Setup*.exe
"C:\Program Files\Pantaray\QSetup\Composer.exe" win32\WarServ.qsp /Compile /Exit
move win32\WarServ-Setup.exe win32\WarServ-Setup-3-0-a4-%1.exe
