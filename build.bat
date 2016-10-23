
@echo off
if not exist build mkdir build
pushd build

cl -nologo -Zi ../main.cc -FeSoftwareRenderer.exe user32.lib gdi32.lib

popd