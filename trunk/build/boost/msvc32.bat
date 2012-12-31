set OLDCD=%CD%
pushd %BOOST_ROOT%
b2 -j %NUMBER_OF_PROCESSORS% --without-python toolset=msvc address-model=32 architecture=x86 --stagedir=stage/msvc-x86 link=static runtime-link=static threading=multi debug-symbols=on define=WINVER=_WIN32_WINNT_VISTA define=_WIN32_WINNT=_WIN32_WINNT_VISTA stage > %OLDCD%\msvc32.txt 2>&1
popd
