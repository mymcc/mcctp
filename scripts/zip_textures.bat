@echo off
setlocal

set root="%~dp0..\"
pushd %root%

setlocal enabledelayedexpansion
set texturePackPath=ui\texturepacks
set zipExe=external\7z2201-x64\Files\7-Zip\7z.exe

for /r "%texturePackPath%" %%i in (*) do (
    if /I "%%~xi" neq ".zip" (
        if /I "%%~nxi" neq "loadingtexturepack.perm.bin" (
            "%zipExe%" a -tzip "%texturePackPath%\%%~nxi.zip" "%%i"
        ) else (
            "%zipExe%" a -v99m "%texturePackPath%\%%~nxi.zip" "%%i"
        )
    )
)

popd
endlocal