REM  Assemble  asm source file for 32-bit environment
ml /safeseh /c get_cpuid.asm
cl cpu_topo.c util_os.c
lib /NODEFAULTLIB /out:cpu-topology.lib get_cpuid.obj
