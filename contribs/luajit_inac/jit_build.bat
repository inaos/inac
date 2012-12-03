@if not defined INCLUDE goto :FAIL

@set LUAJIT=..\luajit-2.0.0\src\luajit.exe
@set JIT_DIR=..\luajit-2.0.0\src\jit

@luajit -b %JIT_DIR%\bc.lua bc.obj
@luajit -b %JIT_DIR%\bcsave.lua bcsave.obj
@luajit -b %JIT_DIR%\dis_arm.lua dis_arm.obj
@luajit -b %JIT_DIR%\dis_mips.lua dis_mips.obj
@luajit -b %JIT_DIR%\dis_mipsel.lua dis_mipsel.obj
@luajit -b %JIT_DIR%\dis_ppc.lua dis_ppc.obj
@luajit -b %JIT_DIR%\dis_x64.lua dis_x64.obj
@luajit -b %JIT_DIR%\dis_x86.lua dis_x86.obj
@luajit -b %JIT_DIR%\dump.lua dump.obj
@luajit -b %JIT_DIR%\v.lua v.obj
@luajit -b %JIT_DIR%\vmdef.lua vmdef.obj

@lib /nologo /OUT:jit.lib *.obj

@echo "Successfully packaged jit to static library"

@goto :END
:FAIL
@echo You must open a "Visual Studio .NET Command Prompt" to run this script
:END