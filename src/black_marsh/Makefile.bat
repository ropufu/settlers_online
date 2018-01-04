@echo off
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set compilerflags=/O2 /W4 /wd4710 /wd4711 /wd4514 /EHsc /I.\..\..\..\aftermath /I.\..\..\..\opensource
set linkerflags=/OUT:black_marsh.exe
cl.exe %compilerflags% main.cpp /link %linkerflags%
del main.obj