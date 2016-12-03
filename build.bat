
@echo off
if not exist build mkdir build
pushd build

cl -nologo -Zi ../main.c -FeSoftwareRenderer.exe user32.lib gdi32.lib opengl32.lib -link -SUBSYSTEM:WINDOWS

popd