
@echo off
if not exist build mkdir build
pushd build

cl -nologo -Zi ../main.cc -FeSoftwareRenderer.exe user32.lib gdi32.lib opengl32.lib -link -SUBSYSTEM:WINDOWS

popd