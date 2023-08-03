@echo off
setlocal enabledelayedexpansion

set root="%~dp0..\"
pushd %root%

rem Visual Studio Solution Files
del /q /f /s "*.sln"
del /q /f /s "include\*.vcxproj"
del /q /f /s "src\*.vcxproj"
del /q /f /s "src\*.vcxproj.filters"
del /q /f /s "external\fpng\*.vcxproj"
del /q /f /s "external\fpng\*.vcxproj.filters"
del /q /f /s "external\imgui\imgui.vcxproj"
del /q /f /s "external\imgui\imgui.vcxproj.filters"
del /q /f /s "external\glew\glew.vcxproj"
del /q /f /s "external\glew\glew.vcxproj.filters"
del /q /f /s "tests\test_native\*.vcxproj"
del /q /f /s "tests\test_native\*.vcxproj.user"

rem Visual Studio Solution Folders and Binaries
for %%G in (
    .pytest_cache
    .vs 
    bin 
    bin-int 
    external\fpng\bin 
    external\fpng\bin-int 
    external\imgui\bin 
    external\imgui\bin-int 
    external\glew\bin
    external\glew\bin-int
    tests\bin
    tests\bin-int
) do (
    set "target=%%G"
    rmdir /s /q !target! >nul 2>&1 && (echo Removed folder - %root%!target!) || (echo Could Not Find %root%!target!)
)

echo %cmdcmdline%|find /i """%~f0""">nul && (
    rem Batchfile was double clicked
    pause
) || (
    rem Batchfile was executed from the console
)

popd
endlocal