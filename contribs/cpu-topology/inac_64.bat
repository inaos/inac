REM  Assemble  asm source file for 64-bit environment
ml64 /c /DX86_64 get_cpuid.asm
lib /NODEFAULTLIB /out:intel-cpu-topo.lib get_cpuid.obj
