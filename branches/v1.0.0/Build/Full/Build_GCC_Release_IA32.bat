cd ../../
cd Src/Dependencies/Boost
set BOOST_ROOT=%CD%
cd ../../../
bjam -j 4 toolset=gcc address-model=32 release > Build\Full\Build_GCC_Release_IA32.txt
