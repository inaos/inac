# INAOS Common C Library

The INAOS Common C Library is a collection of header files and library routines 
used to implement common operations, such as input/output, character string 
handling,  memory, error and event handling. This library is designed and 
optimized for singled threaded applications and is used as common base for 
all INAOS programs/libraries written in C.

High level objectives:

* Be minimal but complete (keep it simple)
* Low complexity
* Low Resource consumption/High performance
* Ease of maintenance, testing and debugging
* Fully documented

## Getting started

Build and install the library. Simply type `sudo make && make install`.

Start by including the INOAS library header in your code:
    #include <libinac/lib.h>;
   
### For library consumers
Initialize the library context as soon as possible:
    ina_initlib();

For each call of `ina_initlib()` you have to call `ina_exit()`.

### For applications
For applications, initialize the application context. This must be the first
function in your program. You must call `ina_exit()` once before you quit 
your program.

    int main(int argc, char *argv) 
    {
	    ina_initapp(argc, argv);

	    while (… {
	       ….
        }
     
        ina_exit();
    }

## Portable Header
This library provide with his portable header (portable.h) macros, functions 
and types to help writing cross-plattform libraries and applications.

### Compiler detection
A macro for each compiler will be defined if detected. The following compilers 
are  currently detected. 

* Borland C/C++: `INA_COMPILER_BORLAND`
* Compaq/DEC C/C++: `INA_COMPILER_DEC`
* Gnu GCC: `INA_COMPILER_GCC`
* Gnu GCC (Apple): `INA_COMPILER_APPLECC`
* HP-UX CC: `INA_COMPILER_HPCC`
* IBM C/C++: `INA_COMPILER_IBM`
* Intel C/C++: `INA_COMPILER_INTEL`
* MetroWerks CodeWarrior: `INA_COMPILER_METROWERKS`
* Microsoft Visual C++: `INA_COMPILER_MSVC`
* MIPSpro C/C++: `INA_COMPILER_MIPSPRO`
* Sun Pro: `INA_COMPILER_SUN`
* Watcom C/C++: `INA_COMPILER_WATCOM`

The name of detected compiler is defined by the `INA_COMPILER_STRING` macro. 
A warning is thrown by compile time if no compiler was detected.
 

### Target OS detection
Following target operating systems are currently supported and defined if detected.

* AIX: `INA_OS_AIX`
* Amiga: `INA_OS_AMIGA`
* BeOS: `INA_OS_BEOS`
* Cygwin: `INA_OS_CYGWIN32`
* DOS/32-bit: `INA_OS_DOS32`
* GameCube: `INA_OS_GAMECUBE`
* GO32/MS-DOS: `INA_OS_GO32`
* HP-UX: `INA_OS_HPUX`
* Irix: `INA_OS_IRIX`
* Linux: `INA_OS_LINUX`
* MacOS: `INA_OS_MACOS`
* MacOS X: `INA_OS_OSX`
* MinGW: `INA_OS_MINGW`
* Solaris: `INA_OS_SOLARIS`
* SunOS: `INA_OS_SUNOS`
* Tru64: `INA_OS_TRU64`
* PalmOS: `INA_OS_PALM`
* UNICOS: `INA_OS_UNICOS`
* Unix-like(generic): `INA_OS_UNIX`
* Windows CE: `INA_OS_WINCE`
* Win64: `INA_OS_WIN64`
* Win32: `INA_OS_WIN32`
* XBOX: `INA_OS_XBOX`

The name of detected target os is defined by the `INA_OS_STRING` macro.


### Target CPU detection
Following target CPUs are currently supported and defined if detected. The name
of detected target CPU is defined by the `INA_CPU_STRING` macro.

* AMD x86-64: `INA_CPU_X86`, `INA_CPU_X86_64`
* ARM: `INA_CPU_STRONGARM`
* AXP: `INA_CPU_AXP`
* Cray T3E (Alpha 21164): `INA_CPU_CRAYT3E`
* Hitachi SH-3: `INA_CPU_SH3`
* Hitachi SH-4: `INA_CPU_SH3, INA_CPU_SH4`
* IA64: `INA_CPU_IA64`
* IBM PowerPC 750 (NGC): `INA_CPU_PPC750`
* Intel 386+: `INA_CPU_X86`
* MC68000: `INA_CPU_68K`
* MIPS: `INA_CPU_MIPS`
* PA-RISC: `INA_CPU_HPPA`
* PowerPC64: `INA_CPU_PPC`
* Sparc/64: `INA_CPU_SPARC64`
* Sparc/32: `INA_CPU_SPARC`


### Integral types

### Misc macros

## Memory handling

## String handling

## Error handling

## Testing






