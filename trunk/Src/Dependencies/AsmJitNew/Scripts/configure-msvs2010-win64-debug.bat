mkdir ..\Build
cd ..\Build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DASMJIT_BUILD_LIBRARY=1 -DASMJIT_BUILD_TEST=1 -G"Visual Studio 10 Win64"
cd ..\Scripts
