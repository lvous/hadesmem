cd ../../
cd Src/Dependencies/Boost
set BOOST_ROOT=%CD%
cd ../../../
b2 -j 4 toolset=msvc-10.0 address-model=32 release > Build\Full\Build_MSVC100_Release_IA32.txt