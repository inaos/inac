@echo off

SET CMD=cmake
SET CMD=%CMD% -G"Visual Studio 11"


if not exist build mkdir build
cd build
call %CMD% ..
cd..

:exit
