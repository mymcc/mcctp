@echo off
setlocal enabledelayedexpansion

set root="%~dp0..\"
pushd %root%

set texturePacksPath=ui\texturepacks

for /r "%texturePacksPath%" %%i in (*.bin) do (
    del /f /q "%%i"
)

for /r "%texturePacksPath%" %%i in (*.idx) do (
    del /f /q "%%i"
)

popd
endlocal