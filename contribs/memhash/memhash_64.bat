REM  Assemble  asm source file for 64-bit environment
ml64 /Fomemhash.obj /c /DX86_64 msvc\x86_64\memhash.asm
lib /NODEFAULTLIB /out:memhash.lib memhash.obj
