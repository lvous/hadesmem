mkdir ..\build
cd ..\build
cmake .. -G"Visual Studio 9 2008 Win64" -DASMJIT_BUILD_SAMPLES=1
cd ..\scripts
pause
