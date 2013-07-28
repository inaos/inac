@echo off

REM
REM Copyright (c) 2013, INAOS GmbH
REM All rights reserved.
REM
REM Redistribution and use in source and binary forms, with or without
REM modification, are permitted provided that the following conditions are met:
REM     * Redistributions of source code must retain the above copyright
REM       notice, this list of conditions and the following disclaimer.
REM     * Redistributions in binary form must reproduce the above copyright
REM       notice, this list of conditions and the following disclaimer in the
REM       documentation and/or other materials provided with the distribution.
REM     * Neither the name of the INAOS GmbH nor the names of its contributors
REM       may be used to endorse or promote products derived from this software 
REM       without specific prior written permission.
REM
REM THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
REM AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
REM IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
REM ARE DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY DIRECT, 
REM INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
REM (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
REM SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
REM CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
REM STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
REM ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
REM OF SUCH DAMAGE.
REM

REM Set general Environment variables
REM ---------------------------------

SET INAC_HOME=%CD%
SET INAC_BUILD_SCRIPT=%INAC_HOME%\script\shell\win32\windows_build.bat

if not defined INCLUDE (
	if not defined VS110COMNTOOLS goto fail_vs_2012
	if not exist "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat" goto fail_vs_2012
	call "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat" x86
)

if not exist %INAC_BUILD_SCRIPT% goto fail_no_build_script

REM Determine build-type and build-stage
if not "%2" == "" (
	call %INAC_BUILD_SCRIPT% %1 %2 eval_params
) else (
	call %INAC_BUILD_SCRIPT% %1 dummy eval_params
)
if not defined INAC_W32_BUILD_TYPE goto exit
if not defined INAC_W32_BUILD_STAGE goto exit
if "%INAC_W32_BUILD_STAGE%" == "dummy" goto exit
if not "%INAC_W32_BUILD_STAGE%" == "clean" (
	if "%INAC_W32_BUILD_TYPE%" == "dummy" goto exit
)

REM Build 3rd party
REM ---------------------------------

REM build luajit
cd contribs\luajit\src
if not exist msvcbuild.bat goto fail_no_luajit1
if not exist msvcbuild_debug.bat goto fail_no_luajit2
if "%INAC_W32_BUILD_STAGE%" == "clean" (
	if exist lua51.lib del lua51.lib
	if exist lua51d.lib del lua51d.lib
) else (
	if "%INAC_W32_BUILD_TYPE%" == "debug" (
		if not exist lua51d.lib (
			call msvcbuild_debug.bat static
		)
	)
	if "%INAC_W32_BUILD_TYPE%" == "release" (
		if not exist lua51.lib (
			call msvcbuild.bat static
		)
	)
)
cd %INAC_HOME%

REM Build INAC
REM ---------------------------------

SET INAC_WIN32_BUILD_NAME=inac
SET INAC_WIN32_PROJECT_DIR=.
SET INAC_WIN32_C_SOURCE_DIR=.
SET INAC_WIN32_C_BUILD_TOOL=cmake-nmake

SET INAC_TIME_BACKEND=time-os

if not "%3" == "" SET INAC_TIME_BACKEND=%3

call %INAC_BUILD_SCRIPT% %1 %2

REM reset the main environment variables because they might have been deleted by the previous build
SET INAC_HOME=%CD%
SET INAC_BUILD_SCRIPT=%INAC_HOME%\script\shell\win32\windows_build.bat

SET INAC_TIME_BACKEND=

SET INAC_WIN32_BUILD_NAME=inac
SET INAC_WIN32_PROJECT_DIR=.
SET INAC_WIN32_LUA_SOURCE_DIR=src
SET INAC_WIN32_LUA_LIB_NAME=libinac_lua.lib

call %INAC_BUILD_SCRIPT% %1 %2
if not "%INAC_W32_BUILD_STAGE%" == "clean" (
	if "%INAC_W32_BUILD_TYPE%" == "debug" (
		LIB.EXE /OUT:%INAC_HOME%\buildall\libinac.lib %INAC_HOME%\buildall\libinac_c.lib %INAC_HOME%\buildall\libinac_lua.lib ^
			%INAC_HOME%\buildall\anet.lib %INAC_HOME%\buildall\skiplist.lib %INAC_HOME%\buildall\http_parser.lib ^
			%INAC_HOME%\buildall\rapidxml.lib %INAC_HOME%\buildall\sqlite.lib %INAC_HOME%\contribs\luajit\src\lua51d.lib
	) else (
		LIB.EXE /OUT:%INAC_HOME%\buildall\libinac.lib %INAC_HOME%\buildall\libinac_c.lib %INAC_HOME%\buildall\libinac_lua.lib ^
			%INAC_HOME%\buildall\anet.lib %INAC_HOME%\buildall\skiplist.lib %INAC_HOME%\buildall\http_parser.lib ^
			%INAC_HOME%\buildall\rapidxml.lib %INAC_HOME%\buildall\sqlite.lib %INAC_HOME%\contribs\luajit\src\lua51.lib
	)
)

REM reset the main environment variables because they might have been deleted by the previous build
SET INAC_HOME=%CD%
SET INAC_BUILD_SCRIPT=%INAC_HOME%\script\shell\win32\windows_build.bat

SET INAC_WIN32_BUILD_NAME=inac
SET INAC_WIN32_PROJECT_DIR=.
SET INAC_WIN32_C_BUILD_TOOL=cmake-nmake
SET INAC_WIN32_C_TEST_SOURCE_DIR=tests
SET INAC_WIN32_C_TEST_SUITE_EXEC=buildtest\test.exe

call %INAC_BUILD_SCRIPT% %1 %2

goto exit

:fail_vs_2012
echo Error: Something is wrong with your INAC_HOME setting: %INAC_HOME%
goto exit

:fail_no_build_script
echo Error: Something is wrong with your INAC_HOME setting: %INAC_HOME%
goto exit

:fail_no_luajit1
echo Error: Luajit build script msvcbuild.bat not found
goto exit

:fail_no_luajit2
echo Error: Luajit debug build script msvcbuild.bat not found
goto exit


:exit

REM Clean-up
REM ---------------------------------

SET INAC_W32_BUILD_TYPE=
SET INAC_W32_BUILD_STAGE=

SET INAC_HOME=
SET INAC_BUILD_SCRIPT=

goto:eof