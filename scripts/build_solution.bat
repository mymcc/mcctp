@echo off
setlocal

set vs_version=vs2022

set root="%~dp0..\"
pushd %root%

if "%1" == "" ( 
    echo No argument given ... the default will be used ^(Visual Studio 2022^)
)
if not "%1" == "" (
    echo Solution files will be built for %1
    set vs_version=%1
)

if not exist external\imgui\premake5.lua (
    copy external\premake\external\imgui\imgui.lua external\imgui
    rename external\imgui\imgui.lua premake5.lua
)

echo Building Visual Studio Solution files for %vs_version% ...

external\premake\premake5.exe %vs_version% | for /f "tokens=* delims=" %%i in ('more') do @(
    echo %%i
)

set error_level=%ERRORLEVEL%

if not "%error_level%" == "0" (
    echo System Error Code: %error_level%
)

echo %cmdcmdline%|find /i """%~f0""">nul && (
    REM Batchfile was double clicked
    pause
) || (
    REM Batchfile was executed from the console
)

popd
endlocal