cd ..\..\Src\Dependencies\Boost
set BOOST_ROOT=%CD%
b2 -j 4 --without-mpi --without-python --stagedir=stage/msvc-x86 toolset=msvc-10.0 threading=multi link=shared runtime-link=shared address-model=32 debug release > ..\..\..\Build\Boost\Build_MSVC100_IA32.txt
