@echo off
setlocal enabledelayedexpansion

set root="%~dp0..\"
pushd %root%

for /D %%G in ("build-artifacts") do (
    move "%%G" "tests\bin"
)

popd
endlocal