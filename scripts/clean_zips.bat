@echo off
setlocal
setlocal enabledelayedexpansion

set root="%~dp0..\"
pushd %root%

set texturePacksPath=ui\texturepacks

for /r "%texturePacksPath%" %%i in (*.zip *.001 *.002) do (
    del /f /q "%%i"
)

popd
endlocal