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

if not exist %INAC_BUILD_SCRIPT% goto fail_no_build_script

SET INAC_WIN32_BUILD_NAME=sqlite
SET INAC_WIN32_PROJECT_DIR=contribs\sqlite-3.7.14.1
SET INAC_WIN32_C_SOURCE_DIR=.
SET INAC_WIN32_C_BUILD_TOOL=cmake-nmake

call %INAC_BUILD_SCRIPT% %1 %2

goto exit

:fail_no_build_script
echo Error: Something is wrong with your INAC_HOME setting: %INAC_HOME%
goto exit


:exit

REM Clean-up
REM ---------------------------------

SET INAC_HOME=
SET INAC_BUILD_SCRIPT=

goto:eof