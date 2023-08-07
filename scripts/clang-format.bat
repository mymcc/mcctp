@echo off
setlocal enabledelayedexpansion

set root="%~dp0..\"
pushd %root%

:: Set the path to clang-format
set clang_format=".\external\clang\clang-format-10_windows-amd64.exe"

:: Check if clang-format exists at the specified path
if not exist %clang_format% (
    echo clang-format not found at %clang_format%
    exit /b 1
)

set format_dirs="tests\" "src\" "include\"

:: Loop through subdirectories and run clang-format on each .cpp and .h file
for %%d in (%format_dirs%) do (
    echo Formatting files in %%d
    for %%f in (%%d\*.cpp, %%d\*.h) do (
        %clang_format% -i %%f
    )
)

echo Done formatting!

popd
endlocal