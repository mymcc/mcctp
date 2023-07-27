@echo off
setlocal

set root="%~dp0..\"
pushd %root%

rem Visual Studio Solution Files
del /q /f "*.sln"

rem explicit for now
del /q /f /s "src\*.vcxproj"
del /q /f /s "src\*.vcxproj.user"
del /q /f /s "src\*.vcxproj.filters"

rem explicit for now
del /q /f /s "lib\*.vcxproj"
del /q /f /s "lib\*.vcxproj.user"
del /q /f /s "lib\*.vcxproj.filters"

del /q /f  "external\imgui\*.vcxproj"
del /q /f  "external\imgui\*.vcxproj.user"
del /q /f  "external\imgui\*.vcxproj.filters"


rem Application Binaries (TODO)
rmdir /s /q "bin"
rmdir /s /q "bin-int"

echo %cmdcmdline%|find /i """%~f0""">nul && (
    REM Batchfile was double clicked
    pause
) || (
    REM Batchfile was executed from the console
)

popd
endlocal

