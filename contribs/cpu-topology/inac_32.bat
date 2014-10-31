REM  Assemble  asm source file for 32-bit environment
ml /c get_cpuid.asm
lib /NODEFAULTLIB /out:intel-cpu-topo.lib get_cpuid.obj
