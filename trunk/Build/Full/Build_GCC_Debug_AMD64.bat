cd ../../
cd Src/Dependencies/Boost
set BOOST_ROOT=%CD%
cd ../../../
bjam -j 4 toolset=gcc address-model=64 debug > Build\Full\Build_GCC_Debug_AMD64.txt