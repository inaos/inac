@echo off

SET CMD=cmake
SET CMD=%CMD% -G"Visual Studio 11"

SET OLD_DIR=%CD%

cd ..\..\..\

SET INAC_TIME_BACKEND=time-os
if not "%1" == "" SET INAC_TIME_BACKEND=%1

if not exist build mkdir build
cd build
call %CMD% ..
cd..

cd tests
if not exist build mkdir build
cd build
call %CMD% ..
cd..

cd %OLD_DIR%

SET OLD_DIR=

:exit
