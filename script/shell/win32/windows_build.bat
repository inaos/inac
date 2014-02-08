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


REM
REM Required input
REM --------------
REM 1. Build-Stage (all/test/clean/dist) and Build-Type(release/debug) command-line args
REM 2. Environment variables that define what needs to be done and where
REM
REM Input environemnt variables
REM ---------------------------
REM * INAC_WIN32_BUILD_NAME: Name of the artefact to be built - Required
REM * INAC_WIN32_PROJECT_DIR: Directory reference for detailed artefacts - Required
REM * INAC_WIN32_C_SOURCE_DIR: Directory relative to PROJECT_DIR - Optional
REM * INAC_WIN32_C_TEST_SOURCE_DIR: Directory relative to PROJECT_DIR - Optional
REM * INAC_WIN32_C_TEST_SUITE_EXEC: Executable that invokes the test-suite, relative to PROJECT_DIR - Optional
REM * INAC_WIN32_C_TEST_MAKEHEADERS: makeheaders.exe to generate c-test-suits, relative to INAC_WIN32_C_TEST_SOURCE_DIR - Optional
REM * INAC_WIN32_LUA_TEST_SUITE_EXEC: Execute a Lua script to run a Lua test-suite - Optional
REM * INAC_WIN32_C_BUILD_TOOL: Either 'cmake-nmake' or 'cmake-vs' - Optional
REM * INAC_WIN32_LUA_SOURCE_DIR: Directory relative to PROJECT_DIR - Optional
REM * INAC_WIN32_LUA_LIB_NAME: Name of the library where the lua byte-code is stored - Optional
REM * INAC_WIN32_CODE_GEN_SCRIPT: Lua script that will be invoked before compilation, relative from PROJECT_DIR - Optional
REM * INAC_WIN32_DIST_PACKAGE_NAME: Full name of the zip package to be created (e.g. my-app-1.0.zip)
REM * INAC_WIN32_DIST_FILES: Batch array of (fully qualified) files that will be packaged for distribution.
REM
REM Environment variable rules
REM --------------------------
REM * You must define either INAC_WIN32_C_SOURCE_DIR or INAC_WIN32_LUA_SOURCE_DIR
REM * If you define INAC_WIN32_C_SOURCE_DIR or INAC_WIN32_C_TEST_SOURCE_DIR you have to define
REM   -> INAC_WIN32_C_BUILD_TOOL
REM * If you define INAC_WIN32_LUA_SOURCE_DIR then you have to define
REM   -> INAC_WIN32_LUA_LIB_NAME
REM * If you define INAC_WIN32_C_TEST_SOURCE_DIR you have to define
REM   -> INAC_WIN32_C_TEST_SUITE_EXEC
REM
REM Build stages
REM ------------
REM
REM The build stages mimic the well known values from UNIX makefiles:
REM
REM 'all' = Compiles and links all code include tests, also invokes code-generation if required
REM 'test' = Execute the tests
REM 'clean' = Delete all the build artefacts
REM 'dist' = Create distribuition package.
REM
REM General facts:
REM --------------
REM * The whole build script is restartable, you can invoke all as many times as 
REM   you like only new stuff will be built
REM
REM -------------------------------------------------------------------------

REM set constants
SET INAC_W32_BUILD_DIR=buildall
SET INAC_W32_BUILDTEST_DIR=buildtest
SET INAC_W32_BUILD_TYPE=debug

SET INAC_W32_BUILD_STAGE=all
SET INAC_W32_LIB_CMD=lib /nologo

REM -------------------------------------------------------------------------

REM check the environment
if not defined INCLUDE goto fail_vs_env
if not defined INAC_HOME goto fail_inac_home

REM set variables according to input
SET INAC_W32_LUAJIT=%INAC_HOME%\contribs\luajit\src\luajit.exe
SET INAC_W32_LUAJIT_DIR=%INAC_HOME%\contribs\luajit\src\jit
SET INAC_W32_ZIP_TOOL=%INAC_HOME%\script\shell\win32\ina_zip.vbs

if not defined ORIGINAL-LUA_PATH set ORIGINAL-LUA_PATH=%LUA_PATH%
SET LUA_PATH=%INAC_HOME%\contribs\luajit\src\?.lua;%ORIGINAL-LUA_PATH%

REM evaluate parameters
if not "%1" == "" (
	SET INAC_W32_BUILD_STAGE=%1
	CALL :LoCase INAC_W32_BUILD_STAGE
)
if not "%2" == "" (
	SET INAC_W32_BUILD_TYPE=%2
	CALL :LoCase INAC_W32_BUILD_TYPE
)

REM check build-stage
SET INAC_BUILD_STAGE_VALID=
if "%INAC_W32_BUILD_STAGE%" == "all" SET INAC_BUILD_STAGE_VALID=1
if "%INAC_W32_BUILD_STAGE%" == "test" SET INAC_BUILD_STAGE_VALID=1
if "%INAC_W32_BUILD_STAGE%" == "clean" SET INAC_BUILD_STAGE_VALID=1
if "%INAC_W32_BUILD_STAGE%" == "dist" SET INAC_BUILD_STAGE_VALID=1
if not defined INAC_BUILD_STAGE_VALID goto fail_wrong_build_stage

REM check build-type
SET INAC_BUILD_TYPE_VALID=
if "%INAC_W32_BUILD_TYPE%" == "debug" SET INAC_BUILD_TYPE_VALID=1
if "%INAC_W32_BUILD_TYPE%" == "release" SET INAC_BUILD_TYPE_VALID=1
if not "%INAC_W32_BUILD_STAGE%" == "clean" (
	if not defined INAC_BUILD_TYPE_VALID goto fail_wrong_build_type
)

REM return here if we only evalutated the parameters
if "%3" == "eval_params" goto exit

REM check luajit
if not exist %INAC_W32_LUAJIT% goto fail_no_luajit
if not exist %INAC_W32_LUAJIT_DIR% goto fail_no_luajit_dir

REM check required environment variables
if not defined INAC_WIN32_BUILD_NAME goto fail_ndef_build_name
if not defined INAC_WIN32_PROJECT_DIR goto fail_ndef_project_dir

REM check rules for optional environment variables

if defined INAC_WIN32_C_BUILD_TOOL (
	CALL :LoCase INAC_WIN32_C_BUILD_TOOL
	SET INAC_WIN32_C_BUILD_TOOL_VALID=
	if "%INAC_WIN32_C_BUILD_TOOL%" == "cmake-nmake" SET INAC_WIN32_C_BUILD_TOOL_VALID=1
	if "%INAC_WIN32_C_BUILD_TOOL%" == "cmake-vs" SET INAC_WIN32_C_BUILD_TOOL_VALID=1
	if not defined INAC_WIN32_C_BUILD_TOOL_VALID goto fail_wrong_build_tool
)

REM rule 1
SET INAC_BUILD_C_OR_LUA_VALID=
if defined INAC_WIN32_C_SOURCE_DIR SET INAC_BUILD_C_OR_LUA_VALID=1
if defined INAC_WIN32_LUA_SOURCE_DIR SET INAC_BUILD_C_OR_LUA_VALID=1
if defined INAC_WIN32_C_TEST_SOURCE_DIR SET INAC_BUILD_C_OR_LUA_VALID=1
if not defined INAC_BUILD_C_OR_LUA_VALID goto fail_rule_1

REM rule 2
SET INAC_BUILD_C_OR_TEST_VALID=
if defined INAC_WIN32_C_SOURCE_DIR SET INAC_BUILD_C_OR_TEST_VALID=1
if defined INAC_WIN32_C_TEST_SOURCE_DIR SET INAC_BUILD_C_OR_TEST_VALID=1
if defined INAC_BUILD_C_OR_TEST_VALID (
	if not defined INAC_WIN32_C_BUILD_TOOL goto fail_rule_2
)

REM rule 3
if defined INAC_WIN32_LUA_SOURCE_DIR (
	if not defined INAC_WIN32_LUA_LIB_NAME goto fail_rule_3
)

REM rule 4
if defined INAC_WIN32_C_TEST_SOURCE_DIR (
	if not defined INAC_WIN32_C_TEST_SUITE_EXEC goto fail_rule_4
)

REM phase 'dist'
if "%INAC_W32_BUILD_STAGE%" == "dist" (
	if not exist %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR% goto fail_dist_bd
	if not defined INAC_WIN32_DIST_PACKAGE_NAME goto fail_dist_package_name
	echo Creating distribution package ...
	mkdir %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\dist
	for /F "tokens=2* delims=.=" %%A in ('SET INAC_WIN32_DIST_FILES.') do copy %%B %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\dist
	cscript %INAC_W32_ZIP_TOOL% %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\dist %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\%INAC_WIN32_DIST_PACKAGE_NAME%
	rmdir /s /q %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\dist
	echo Created package %INAC_WIN32_DIST_PACKAGE_NAME%
	goto exit
)

REM invoke code-generator if necessary
SET "INAC_W32_CODE_GEN_FULL_PATH=%INAC_WIN32_PROJECT_DIR%\%INAC_WIN32_CODE_GEN_SCRIPT% %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%"
if defined INAC_WIN32_CODE_GEN_SCRIPT (
	if not "%INAC_W32_BUILD_STAGE%" == "clean" (
		if not exist %INAC_W32_CODE_GEN_FULL_PATH% goto fail_code_gen
		echo Invoke Code-Generator
		%INAC_W32_LUAJIT% %INAC_W32_CODE_GEN_FULL_PATH%
	)
)

REM C-Source invoke build tool - if necessary
SET INAC_WIN32_OLD_DIR=%CD%
if defined INAC_WIN32_C_SOURCE_DIR (
	if "%INAC_W32_BUILD_STAGE%" == "clean" (
		if exist %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR% (
			rmdir /s /q %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%
		)
	) else (
		if not exist %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR% mkdir %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%
		cd %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%
		if "%INAC_WIN32_C_BUILD_TOOL%" == "cmake-nmake" (
			call cmake -DCMAKE_BUILD_TYPE=%INAC_W32_BUILD_TYPE% -G"NMake Makefiles" ..\%INAC_WIN32_C_SOURCE_DIR%
			call nmake
		)
		if "%INAC_WIN32_C_BUILD_TOOL%" == "cmake-vs" (
			call cmake -DCMAKE_BUILD_TYPE=%INAC_W32_BUILD_TYPE% -G"Visual Studio 11" ..\%INAC_WIN32_C_SOURCE_DIR%
			for %%F in (*.sln) do (
				SET INAC_WIN32_SLN_FILE=%%F
				goto first_found
			)
			:first_found
			call msbuild %INAC_WIN32_SLN_FILE% /property:Configuration=%INAC_W32_BUILD_TYPE%
		)
	)
	cd %INAC_WIN32_OLD_DIR%
)

REM Test-Source invoke build tool - if necessary
SET INAC_WIN32_OLD_DIR=%CD%
if defined INAC_WIN32_C_TEST_SOURCE_DIR (
	if "%INAC_W32_BUILD_STAGE%" == "clean" (
		if exist %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILDTEST_DIR% (
			rmdir /s /q %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILDTEST_DIR%
		)
	) else (
		if defined INAC_WIN32_C_TEST_MAKEHEADERS (
			cd %INAC_WIN32_C_TEST_SOURCE_DIR%
			if not exist %INAC_WIN32_C_TEST_MAKEHEADERS% goto fail_makeheaders
			for /r %%i in (test_*.c) do %INAC_WIN32_C_TEST_MAKEHEADERS% %%i
			echo #ifndef _SUITES_H_ > suites.h
			echo #define _SUITES_H_ >> suites.h
			for /r %%i in (test_*.h) do echo #include "%%i" >> suites.h
			echo #include "suites.h" > suites.c
			echo void runtests^(const char* pattern^) { >> suites.c
			for /r %%z in (test_*.h) do (
				for /F "eol=/ tokens=2" %%i in (%%z) do echo %%i >> suites.c
			)
			echo } >> suites.c
			echo void runtests^(const char* pattern^); >> suites.h
			echo #endif >> suites.h
			cd %INAC_WIN32_OLD_DIR%
		)
		if not exist %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILDTEST_DIR% mkdir %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILDTEST_DIR%
		cd %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILDTEST_DIR%
		if "%INAC_WIN32_C_BUILD_TOOL%" == "cmake-nmake" (
			call cmake -DCMAKE_BUILD_TYPE=%INAC_W32_BUILD_TYPE% -G"NMake Makefiles" ..\%INAC_WIN32_C_TEST_SOURCE_DIR%
			call nmake
		)
		if "%INAC_WIN32_C_BUILD_TOOL%" == "cmake-vs" (
			call cmake -DCMAKE_BUILD_TYPE=%INAC_W32_BUILD_TYPE% -G"Visual Studio 11" ..\%INAC_WIN32_C_TEST_SOURCE_DIR%
			for %%F in (*.sln) do (
				SET INAC_WIN32_SLN_FILE=%%F
				goto first_found
			)
			:first_found
			call msbuild %INAC_WIN32_SLN_FILE% /property:Configuration=%INAC_W32_BUILD_TYPE%
		)
	)
	cd %INAC_WIN32_OLD_DIR%
)

REM Invoke the Test-Suite
if defined INAC_WIN32_C_TEST_SUITE_EXEC (
	if "%INAC_W32_BUILD_STAGE%" == "test" (
		start cmd /c %INAC_WIN32_C_TEST_SUITE_EXEC%
		REM FIXME: collect test logs and evalutate failure or success
	)
)

REM compile lua to byte code - if there is any
SET INAC_WIN32_OLD_DIR=%CD%
if defined INAC_WIN32_LUA_SOURCE_DIR (
	if "%INAC_W32_BUILD_STAGE%" == "clean" (
		if exist %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR% (
			rmdir /s /q %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%
		)
	) else (
		if not exist %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR% mkdir %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%
		if not exist %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\lua mkdir %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\lua
		for %%i in (%INAC_WIN32_LUA_SOURCE_DIR%\*.lua) do (
			echo Compiling...%%~nxi
			if "%INAC_W32_BUILD_TYPE%" == "debug" (
				%INAC_W32_LUAJIT% -bg %%i %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\lua\%%~nxi.obj
			) else (
				%INAC_W32_LUAJIT% -b %%i %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\lua\%%~nxi.obj
			)
		)
		%INAC_W32_LUAJIT% -b %INAC_W32_LUAJIT_DIR%\bc.lua %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\lua\bc.obj
		%INAC_W32_LUAJIT% -b %INAC_W32_LUAJIT_DIR%\bcsave.lua %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\lua\bcsave.obj
		%INAC_W32_LUAJIT% -b %INAC_W32_LUAJIT_DIR%\dis_x64.lua %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\lua\dis_x64.obj
		%INAC_W32_LUAJIT% -b %INAC_W32_LUAJIT_DIR%\dis_x86.lua %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\lua\dis_x86.obj
		%INAC_W32_LUAJIT% -b %INAC_W32_LUAJIT_DIR%\v.lua %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\lua\v.obj
		%INAC_W32_LUAJIT% -b %INAC_W32_LUAJIT_DIR%\vmdef.lua %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\lua\vmdef.obj
		%INAC_W32_LUAJIT% -b %INAC_W32_LUAJIT_DIR%\dump.lua %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\lua\dump.obj
	)
	cd %INAC_WIN32_OLD_DIR%
)

REM build a lib file from the lua-byte code - if necessary
if defined INAC_WIN32_LUA_LIB_NAME (
	%INAC_W32_LIB_CMD% /OUT:%INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\%INAC_WIN32_LUA_LIB_NAME% %INAC_WIN32_PROJECT_DIR%\%INAC_W32_BUILD_DIR%\lua\*.obj
)

echo Build for %INAC_WIN32_BUILD_NAME% successful
goto exit

:fail_vs_env
echo Error: You must open a "Visual Studio .NET Command Prompt" to run this script
goto exit

:fail_inac_home
echo Error: INAC_HOME variable is not set - make sure your wrapper-script defines it
goto exit

:fail_wrong_build_type
echo Error: Build type must be either 'debug' or 'release'
goto usage

:fail_wrong_build_stage
echo Error: Build stage must be one of the following: 'all', 'test', 'clean', 'dist'
goto usage

:fail_no_luajit
echo Error: luajit.exe not found; path: %INAC_W32_LUAJIT%
goto exit

:fail_no_luajit_dir
echo Error: 'jit' directory for luajit not found; path: %INAC_W32_LUAJIT_DIR%
goto exit

:fail_ndef_build_name
echo Error: 'INAC_WIN32_BUILD_NAME' not defined
goto exit

:fail_ndef_project_dir
echo Error: 'INAC_WIN32_PROJECT_DIR' not defined
goto exit

:fail_rule_1
echo Error: You must define either INAC_WIN32_C_SOURCE_DIR or INAC_WIN32_LUA_SOURCE_DIR
goto exit

:fail_rule_2
echo Error: If you define INAC_WIN32_C_SOURCE_DIR or INAC_WIN32_C_TEST_SOURCE_DIR you have to define INAC_WIN32_C_BUILD_TOOL
goto exit

:fail_rule_3
echo Error: If you define INAC_WIN32_LUA_SOURCE_DIR then you have to define INAC_WIN32_LUA_LIB_NAME
goto exit

:fail_rule_4
echo Error: If you define INAC_WIN32_C_TEST_SOURCE_DIR you have to define INAC_WIN32_C_TEST_SUITE_EXEC
goto exit

:fail_code_gen
echo Error: Code generation failed, because script: %INAC_W32_CODE_GEN_FULL_PATH% does not exist
goto exit

:fail_wrong_build_tool
echo Error: Build-Tool must be either 'cmake-nmake' or 'cmake-vs'
goto exit

:fail_makeheaders
echo Error: Could not find makeheaders.exe
goto exit

:fail_dist_bd
echo Error: dist failed, because there is no build-directory - maybe you should build 'all' first
goto exit

:fail_dist_filelist
echo Error: dist failed, because file-list not present
goto exit

:fail_dist_package_name
echo Error: dist failed, because package-name not present
goto exit

:usage
echo Usage: "windows_build.bat <build-stage> <build-type>"
goto exit

:exit

REM -------------------------------------------------------------------------

REM cleanup

SET INAC_W32_BUILD_DIR=
SET INAC_W32_BUILDTEST_DIR=
SET INAC_W32_LIB_CMD=

SET INAC_W32_LUAJIT=
SET INAC_W32_LUAJIT_DIR=
SET INAC_W32_ZIP_TOOL=

if defined ORIGINAL-LUA_PATH (
	SET LUA_PATH=%ORIGINAL-LUA_PATH%
)

SET INAC_BUILD_TYPE_VALID=
SET INAC_BUILD_STAGE_VALID=
SET INAC_BUILD_C_OR_LUA_VALID=
SET INAC_BUILD_C_OR_TEST_VALID=

if defined INAC_WIN32_C_BUILD_TOOL_VALID SET INAC_WIN32_C_BUILD_TOOL_VALID=
if defined INAC_W32_CODE_GEN_FULL_PATH SET INAC_W32_CODE_GEN_FULL_PATH=
if defined INAC_WIN32_OLD_DIR SET INAC_WIN32_OLD_DIR=
if defined INAC_WIN32_SLN_FILE SET INAC_WIN32_SLN_FILE=
if defined INAC_WIN32_TEST_GEN_1 SET INAC_WIN32_TEST_GEN_1=

if defined INAC_WIN32_BUILD_NAME SET INAC_WIN32_BUILD_NAME=
if defined INAC_WIN32_PROJECT_DIR SET INAC_WIN32_PROJECT_DIR=
if defined INAC_WIN32_C_SOURCE_DIR SET INAC_WIN32_C_SOURCE_DIR=
if defined INAC_WIN32_C_TEST_SOURCE_DIR SET INAC_WIN32_C_TEST_SOURCE_DIR=
if defined INAC_WIN32_C_TEST_SUITE_EXEC SET INAC_WIN32_C_TEST_SUITE_EXEC=
if defined INAC_WIN32_C_BUILD_TOOL SET INAC_WIN32_C_BUILD_TOOL=
if defined INAC_WIN32_LUA_SOURCE_DIR SET INAC_WIN32_LUA_SOURCE_DIR=
if defined INAC_WIN32_LUA_LIB_NAME SET INAC_WIN32_LUA_LIB_NAME=
if defined INAC_WIN32_CODE_GEN_SCRIPT SET INAC_WIN32_CODE_GEN_SCRIPT=
if defined INAC_WIN32_C_TEST_MAKEHEADERS SET INAC_WIN32_C_TEST_MAKEHEADERS=
if defined INAC_WIN32_DIST_FILES SET INAC_WIN32_DIST_FILES=
if defined INAC_WIN32_DIST_PACKAGE_NAME SET INAC_WIN32_DIST_PACKAGE_NAME=

goto:eof

REM -------------------------------------------------------------------------

REM -- Sub-Modules by:
REM -- http://www.robvanderwoude.com/battech_convertcase.php

:LoCase
:: Subroutine to convert a variable VALUE to all lower case.
:: The argument for this subroutine is the variable NAME.
FOR %%i IN ("A=a" "B=b" "C=c" "D=d" "E=e" "F=f" "G=g" "H=h" "I=i" "J=j" "K=k" "L=l" "M=m" "N=n" "O=o" "P=p" "Q=q" "R=r" "S=s" "T=t" "U=u" "V=v" "W=w" "X=x" "Y=y" "Z=z") DO CALL SET "%1=%%%1:%%~i%%"
GOTO:EOF

:UpCase
:: Subroutine to convert a variable VALUE to all UPPER CASE.
:: The argument for this subroutine is the variable NAME.
FOR %%i IN ("a=A" "b=B" "c=C" "d=D" "e=E" "f=F" "g=G" "h=H" "i=I" "j=J" "k=K" "l=L" "m=M" "n=N" "o=O" "p=P" "q=Q" "r=R" "s=S" "t=T" "u=U" "v=V" "w=W" "x=X" "y=Y" "z=Z") DO CALL SET "%1=%%%1:%%~i%%"
GOTO:EOF
