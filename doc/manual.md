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

### Compile time configuration
 * CSTRING_ENABLED: Enable C-runtime strings (Default)
 * BSTRING_ENABLED: Enable BSTRING string (The Better String Library)
 * SYSMEMPOOL_SIZE: Define the capacity in bytes of the internal memory pool 
                    Default is 8MB
 * MEMPOOL_SIZE:    Define the default capacity in bytes for a memory pool 
                    Default is 8MB


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
        if (INA_SUCCEED(ina_appinit(argc, argv)) {
            while (… {
                ….
            }
        }
        ina_exit(EXIT_SUCCESS);
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


## Strings


## Error handling

A good error handling should know as much as possible about an error. Things
like when, where, what, who, is it handed or not, and "should I abort my 
program" are such kind of information we want to know.  
The "who" question isn't really easy to implement, so we omitted  it.

Also important: Easy access to error information. That's why we pack the 
'where', 'what', 'handled or not' and 'abort or not' in one single value. 
We call it 'Return Code' or simply RC. RC is defined by `ina_rc_t' which is 
in fact a 32bit unsigned integer value. The RC is packed as follow:

     32bit |IIIIIIII|IIMMMMMM|OOOOOFHR|RRRRRRRR|
                |         |     |  ||      +->  9bit - Reason
                |         |     |  |+-------->  1bit - Handled flag
                |         |     |  +--------->  1bit - Fatal flag
                |         |     +------------>  5bit - OS function identifier   
                |         +------------------>  6bit - Module identifier
                +----------------------------> 10bit - Error identifier
                         
To know if an error occurred use `INA_SUCCEED` macro, which returns `1` if no
errors occurred or the last error was handled by a previous caller.

### Return Code

#### Reason
This value contain the error code (reason of failure). Values from 1-128 are
reserved to the INAOS Common C Library.   Define user error codes starting
by 129. For instance:

     #define INAWS_ERR_NOCONNECTION    INA_ERR_USER+1

We can get access to the reason by ÌNA_RC_REASON` macro.

    switch (INA_RC_REASON(rc)) {
       case INAWS_TOOMANY_FILES:
          .....

#### Fatal Flag
Indicate whenever you should about the program. Use `INA_ERR_FATAL(rc)` to 
verify a fatal condition. For instance:

    rc = inaws_server_start(...
    if (!INA_SUCCEED(rc)) {
        if (INA_ERR_FATAL(rc)) {
           --- abort here
  
#### Handled Flag
Indicate if an error was handled by a previous caller. Use `ina_err_clear` to
mark an error as handled. For instance:
    
    rc = inaws_server_start(...
    if (!INA_SUCCEED(rc)) {
       switch (INA_RC_REASON(rc)) {
          case INAWS_TOOMANY_FILES:
               ...do something to handle too many file problem ...
                
               /* mark error as handled  
               ina_err_clear(rc);

Once an error is marked as handled, there is no way to reset it to
"unhandled".  By marking an error as handled, all previous pushed errors are 
removed  from the error state.


#### OS function identifier
Give us the possibility to inform the caller about system function failure . 
For instance `fopen()`. In such a case the caller could retry with other 
parameters/values or let the user know about the real cause of failure. 
Use the `INA_RC_OSFN` macro to retrieve  the OS function identifier. 
For instance:

    rc = inaws_server_start(...
    if (!INA_SUCCEED(rc)) {
       switch (INA_RC_REASON(rc)) {
          case INAWS_LOGFILE_ERROR:
              /* actually want to check if there is a problem with fopen() */
              if (INA_RC_OSFN(rc) == INA_OSFN_FOPEN) {
                   /* may be the ownership is wrong */
                   if (!inaws_check_ownership(....) {
                      /* let the user know that he must fix file ownership or
                         fix the problem and retry again */
                    ...
              
OS function identifiers are defined in `<libinac/error.h>`. Only those 
identifiers are allowed. Don't define any others.           

#### Module identifier
Clearly identify the source (compilation unit) of error. For instance 
`INA_MOD_STRING` identify the string compilation unit. Developers can define
their own identifiers.   

### Push and peek instead of throw and catch
The basic concept of our error handling is that we push an error to a global
error state. The error state is a simple  pointer array which stores a 
certain number of errors (`__INA_ERR_STATE_SIZE`). In case the max number of 
errors is reached, the "first in" error will be dropped from the state.

The caller have the responsibility to take care about the pushed error(s).
He has in fact, depending on the error situation, 4 options:
1. Handle the error situation
2. Leave it unhandled and push a new error.
3. Leave it unhandled and return it to the caller
4. Abort the program

### Push
Use the `INA_ERR_PUSH`macro to push an error to the global error state.

    INA_ERR_PUSH(INAWS_ERR_NOCONNECT, 
        INAWS_MOD_SERVER, INA_OSFN_NONE, "Connection failed");

For simplification, use the `INA_ERR_PUSH_BASIC` or `INA_ERR_PUSH_OSFN` 
macros on depending the error information you have.

    INA_ERR_PUSH_BASIC(INAWS_ERR_NOCONNECT, "Connection failed");
    INA_ERR_PUSH_OSFN(INAWS_ERR_NOCONNECT, INA_OSFN_NONE, "Connection failed");

### Peek
With a peek operation we get the first unhandled error from the global state. 
Call `ina_err_peek()`to peek. Peek doesn't drop the error. For instance:
  
    if (!INA_SUCCEED(inaws_server_start())) {
        rc = ina_err_peek();
        ... do something now!

To know what is the first pushed error we use `ina_err_peek_last()`. It's 
maybe confusing but, in fact the first pushed error is the last error in our
global error state.  In others words, `ina_err_peek_last()` returns the root 
of failure (until no errors were dropped) .

We can walk through the global error state by using `ina_err_peek()` and 
`ina_err_peek_next()`

    if (!INA_SUCCEED(inaws_server_start())) {
        rc = ina_err_peek();
        while (!INA_SUCCEED(rc)) {
           /* check if we must abort ... */
          if (INA_ERR_FATAL(RC)) {
            abort();
          }
          rc = ina_err_peek_next(rc);
        }
         
For simplification we can set our RC to `INA_ERR_PEEK_FIRST` and then walk 
through using `ina_err_peek_next()`.
       
        rc =  INA_ERR_PEEK_FIRST;
        while (!(rc = ina_err_peek_next(rc)) {
           /* check if we must abort ... */
          if (INA_ERR_FATAL(RC)) {
            abort();
          }

## Cleanup the error state
To reset the entire error state use `ina_err_reset()`. All errors including 
the most recently  pushed are removed from the error state.

    /* make sure error state is clean */
    ina_err_reset();
    /* do the work now */
    if (!INA_SUCCEED(inaws_server_start())) {
        rc = ina_err_peek();
        if (!INA_ERR_FATAL(RC)) 

## Cleanup handler
There is a posibility to define a callback function which is called in case 
the program is being terminated because of fatal error like segmentation fault
or an interruption request like ctrl-c.
Use `ina_err_set_cleanup_handler()` to define such a callback. 
Keep in mind that this cleanup handler will be called only in case of abnormal
program termination.

## Utilities
The error handling module of this library provide two useful functions. They 
are used internally but they are for public use as well.

- `ina_err_trace()` printout current error state to the standard output.

## Testing
### Unit testing
### Performance testing






