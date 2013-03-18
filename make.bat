@echo off

SET BUILD_DIR=buildall
SET LUAJIT=contribs\luajit-2.0.0\src\luajit.exe
SET LIB_CMD=lib /nologo
SET LUA_PATH=contribs\luajit-2.0.0\src\?.lua
SET BUILD_TYPE=Debug
SET MAKEHEADERS=..\%BUILD_DIR%\makeheaders.exe

if not "%1" == "" goto set_build_type

:setup_cmake
SET CMAKE_CMD=cmake
SET CMAKE_CMD=%CMAKE_CMD% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
SET CMAKE_CMD=%CMAKE_CMD% -G"NMake Makefiles"

if not defined INCLUDE goto :fail

if not exist %BUILD_DIR% goto make_build_dir

:build_lua

if not exist %BUILD_DIR%\lua goto make_lua_dir

%LUAJIT% -b src\conffile.lua %BUILD_DIR%\lua\conffile.obj

%LIB_CMD% /OUT:%BUILD_DIR%\inac-lua.lib %BUILD_DIR%\lua\*.obj

goto build

:set_build_type
SET BUILD_TYPE=%1
goto setup_cmake

:make_build_dir
mkdir %BUILD_DIR%
goto build_lua

:make_lua_dir
mkdir %BUILD_DIR%\lua
goto build_lua

:build

cd %BUILD_DIR%
call %CMAKE_CMD% ..

nmake

cd..
cd tests

for /r %%i in (test_*.c) do %MAKEHEADERS% %%i

echo #ifndef _SUITES_H_ > suites.h
echo #define _SUITES_H_ >> suites.h
for /r %%i in (test_*.h) do echo #include "%%i" >> suites.h

echo #include "suites.h" > suites.c
echo void runtests() { >> suites.c
for /r %%z in (test_*.h) do (
	for /F "eol=/ tokens=2" %%i in (%%z) do echo %%i >> suites.c
)
echo } >> suites.c;
echo void runtests(); >> suites.h
echo #endif >> suites.h

mkdir build
cd build
call %CMAKE_CMD% ..
nmake
cd..

cd..

goto success

:success
echo.
echo === Successfully built inac for Windows ===
goto exit

:fail
echo You must open a "Visual Studio .NET Command Prompt" to run this script
:exit
