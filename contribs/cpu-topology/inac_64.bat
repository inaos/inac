REM  Assemble  asm source file for 64-bit environment
ml64 /c /DX86_64 get_cpuid.asm
lib /NODEFAULTLIB /out:cpu-topology.lib get_cpuid.obj
