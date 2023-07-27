@echo off
setlocal

set root="%~dp0..\"
pushd %root%

setlocal enabledelayedexpansion
set texturePackPath=ui\texturepacks
set zipExe=external\7z2201-x64\Files\7-Zip\7z.exe

for /r "%texturePackPath%" %%i in (*.zip *.001) do (
    if /I "%%~nxi" neq "loadingtexturepack.perm.bin.zip" (
        "%zipExe%" e "%%i" -o"%%~dpi" -y
    ) else (
        "%zipExe%" x "%%~dpi"
    )
)

popd
endlocal