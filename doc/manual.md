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

All constants are prefixed with INA_. Other identifiers are prefixed with ina_.
Type names are suffixed with _t and typedef‘d so that the struct keyword need
not be used.

### For library consumers
Initialize the library context as soon as possible:

    ina_libinit();

For each call of `ina_initlib()` you have to call `ina_exit()`.

### For applications
For applications, initialize the application context. This must be the first
function in your program. You must call `ina_exit()` once before you quit 
your program.

    int main(int argc, char *argv) 
    {
	    ina_appinit(argc, argv);

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

## API Reference

### Library Version
The INAOS Common C Library version is of the form A.B.C, where A is the major 
version, B is the minor version and C is the micro version. If the micro 
version is zero, it’s omitted from the version string, i.e. the version string 
is just A.B.
When a new release only fixes bugs and doesn’t add new features or 
functionality, the micro version is incremented. When new features are added
in a backwards compatible way, the minor version is incremented and the micro 
version is set to zero. When there are backwards incompatible changes, the 
major version is incremented and others are set to zero.

The following preprocessor constants specify the current version of the 
library:

`INA_MAJOR_VERSION, INA_MINOR_VERSION, INA_MICRO_VERSION`

Integers specifying the major, minor and micro versions, respectively.

`INA_VERSION`

A string representation of the current version, e.g. "1.2.1" or "1.3".

`INA_VERSION_HEX`

A 3-byte hexadecimal representation of the version, e.g. 0x010201 for version 
1.2.1 and 0x010300 for version 1.3. This is useful in numeric comparisions,
e.g.:

    #if INA_VERSION_HEX >= 0x010201
    /* Code specific to version 1.2.1 and above */
    #endif

## Memory handling

### Custom Memory Allocation
By default, INAOS Common C Library  uses malloc() and free() for memory 
allocation. These functions can be overridden if custom behavior is needed.



## String handling


## Error handling
INAOS Common C Library uses a single struct type to pass error information to 
the user.


## Testing






