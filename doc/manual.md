# INAOS Common C Library

The INAOS Common C Library is a collection of header files and library routines 
used to implement common operations, such as input/output, character string 
handling, memory, error and event handling. This library is designed and 
optimized for singled threaded applications and is used as common base for 
all INAOS programs/libraries written in C. The library includes a built-in
LuaJIT engine (http://luajit.org)

High level objectives:

* Be minimal but complete (keep it simple)
* High Performance
* Low complexity
* Low resource consumption
* Ease of maintenance, testing and debugging
* Fully documented

# Getting started

## Building on Windows

Building on Windows requires some programs to be present on your system.

### Prerequisites

* [Visual Studio 2017 Community Edition][1]
* [CMake][3.x]
  * Use the binary installer you don't need to build from source
  * Make sure you add cmake to your PATH

### Build

* Open a Visual Studio command prompt
* Navigate to the INAC root folder
* Type: make.bat all debug

### Using the Intel Compiler

* Open a Visual Studio command prompt for the Intel Compiler
* Navigate to the INAC root folder
* Type: make.bat all debug

## Building on Linux or OS X

To build the library, simply type `sudo make`. To select the debug build, 
type `sudo make debug`.

## Versioning and compatibility

Describes the approach for API and ABI compatibility when INAC is used as binary dependency.

### Decisions

* INAC is only distributed as static library
* Major releases break the compatibility
* Minor releases are backward compatible ABI and API wise
* Introduce compiler warnings to deprecate API's

## Dependency handling

* CMake based
* User must provide:
	* Repository: Local or URL (https://inaos.jfrog.io/inaos/webapp/...)
	* Package name
	* Version
* CMake macro will compile full path
* On Windows we use environment variable: ${ENV:VisualStudioVersion} for version info
* Unpacking should happen in user home e.g. %USERPROFILE%\.ina\cmake\...
* Transitive dependencies must be managed by the User. CMake will only throw and error if a library is not present


## Guidelines

### Resource management

### General function argument structure and behavior

* Output parameters to function are always after input parameters
* Structure/Object creation follows the pattern: Output parameter pointer-to-pointer. Functions  always return error-codes expect `free()` and `destroy()` which are void.
* Modules that require static resources provide `init()` and `destroy()` methods that have to be called by the user exactly once per process-lifetime. In debug mode the application will crash if `init()` or `destroy()` are executed more than once.
* For every `_new` function call in a module the user has to issue a `_free` function call.
* 

### Initialization and destruction

* For modules we should have only ```init()```and ```destroy()```. if we have a `init()` we must have a `destroy()` also.
* For allocation we should have only ```new()```and ```free()```. if we have a `new()` we must have a `free()`
* `init()` and `new()` must return always a valid RC for error handling
* `destroy()` and `free` must always succeed. They are therefore exceptionally defined as INA_API(void). 
* Errors during `destroy()` and `free()` must be logged
* Errors in `destroy()` and `free()` should abort execution.
* Output parameters for `init()` and `new()`  must be after input parameters.
* Output parameters must be properly initialized by `init()` and `new()`. They should be set to NULL on failure 
- `destroy()` and `free()` must set the pointer to NULL after releasing the resources.
- Every `destroy()` and `free()` must check if the pointer-pointer argument can be dereferenced and should return if the pointer is already NULL.

### Use of `const` keywords for pointer arguments in API functions

Usage should reflect reality. Use only if memory is readonly! E.g. a constant parameter. Casting a `const` argument into a non constant pointer should be strictly avoided in the implementation.


## Compile time configuration
 * `INA_TRACE_ENABLED`  : Enable/disable tracing. Default enabled.
 * `INA_TRACE_LEVEL`    : Set trace level (1-3). Default 1.
 * `INA_LOG_ENABLED`    : Enable/disable logging. Default enabled.
 * `INA_LOG_LEVEL`      : Set log level from 1 (errors) to 4(debug). 
                          Default 3 (info).


All constants are prefaced with `INA_` . Other identifiers are prefaced with
`ina_`. Type names are suffixed with `_t` and typedef so that the struct 
keyword need not be used.

## Starting to code

Start by including the INAOS library header in your code:

	#include <libinac/lib.h>;

## For library consumers
Initialize the library context as soon as possible:

	ina_init();

## For applications
For applications, initialize the application context with `ina_app_init()`. 
This must be  the first function call in your program.

	int main(int argc, char **argv,) 
	{
	    if (INA_SUCCEED(ina_app_init(argc, argv, NULL)) {
	        while (... {
	            ...
	        }
	    }
	    ina_exit(EXIT_SUCCESS);
	}

### Command line options
The library provides a builtin command line processor. For that purpose the 
`ina_app_init()` takes as fourth argument an array of `ina_opt_t` containing the 
command line options definition consisting in string, number and flag options. 
Use the designated macros to build the options array. Options are defined with 
a short, a long option name and a description. On string and number options a 
default value can de defined. 

* `INA_OPT_STRING(short,long,default,description)`: define a string option
* `INA_OPT_INT(short,long,default,description)`: define a int option
* `INA_OPT_FLAG(short,long,description)`: define a flag (default is false)


Use `INA_OPT(array-name)` to declare the option array:

	INA_OPTS(opt,
        INA_OPT_STRING("h", "host", NULL, "Hostname"),
        INA_OPT_INT("p", "port", 999, "Port"),
        INA_OPT_FLAG("k", "keep-alive", "Keep connection alive")));

Register and parse the options by passing the options array to `ina_app_init()`. 
The function fails with RC `INA_EOPT` if current command line options don't  
match with the registered definition and simple a usage screen will be printed 
out to the standard output.

	if (INA_SUCCEED(ina_app_init(argc, argv, opt)) {
	    while (... {
	            ...
	    }

To query a flag is whenever or not set use `ina_opt_isset()`:

	if (INA_SUCCEED(ina_opt_isset("keep-alive")) {

To get a int value use `ina_opt_get_int()`:
	
	int value = 0;
	ina_opt_get_int("port", &value);

To get a string value use `ina_opt_get_string()`:
	
	ina_str_t value = NULL;
	ina_opt_get_string("host", &value);

The command line options values are preserved for the until the application 
stops. 

# Portable Header
This library provides with his portable header (portable.h) macros, functions 
and types to help writing cross-platform libraries and applications.

## Compiler detection
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
 

## Target OS detection
Following target operating systems are currently supported and defined if 
detected.

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


## Target CPU detection
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

## Integral types

__FIXME__

## Misc macros

* INA_INLINE
* INA_CASSERT
* INA_LOW32
* INA_HIGH32
* INA_LOW
* INA_HIGH
* INA_TOWORD
* INA_SIZE_T_FMT
* INA_INT64_T_FMT
* INA_UINT64_T_FMT
* INA_ALIGNED
* INA_VSALIGNEDxxx
* INA_ALIGNEDxxx
* INA_PACKED
* INA_VS_BEGIN_PACK
* INA_VS_ENDPACK_PACK
* INA_PATH_SEPARATOR
* INA_ASM
* INA_VOLATILE
* INA_ATOMIC_INC
* INA_ATOMIC_DEC
* INA_ATOMIC_SWAP
* INA_LIKELY
* INA_UNLIKELY
* INA_RESTRICT
* INA_BSWAP_16
* INA_BSWAP_32
* INA_BSWAP_64
* INA_TLS
* INA_DISABLE_WARNING_CLANG
* INA_ENABLE_WARNING_CLANG
* INA_DISABLE_WARNING_GCC
* INA_ENABLE_WARNING_GCC
* INA_DISABLE_WARNING_MSVC
* INA_ENABLE_WARNING_MSVC


__FIXME__

# API Reference

## Library Version
The INAOS Common C Library version is of the form A.B.C, where A is the major 
version, B is the minor version and C is the patch version. If the patch
version is zero, it's omitted from the version string, i.e. the version string 
is just A.B.
When a new release only fixes bugs and doesn't add new features or 
functionality, the patch version is incremented. When new features are added
in a backwards compatible way, the minor version is incremented and the patch
version is set to zero. When there are backwards incompatible changes, the 
major version is incremented and others are set to zero.

The following preprocessor constants specify the current version of the 
library:

`INA_MAJOR_VERSION, INA_MINOR_VERSION, INA_PATCH_VERSION`

Integers specifying the major, minor and patch versions, respectively.

`INA_VERSION`

A string representation of the current version, e.g. "1.2.1" or "1.3".

`INA_VERSION_HEX`

A 3-byte hexadecimal representation of the version, e.g. 0x010201 for version 
1.2.1 and 0x010300 for version 1.3. This is useful in numeric comparisons,
e.g.:

	#if INA_VERSION_HEX >= 0x010201
	/* Code specific to version 1.2.1 and above */
	#endif

## Strings

__TODO__

## Error handling

A good error handling should know as much as possible about an error.  Easy and fast 
access to the error information are also important. That's why INAC stores 
 error information in one single value. 
We call it 'Return Code' or simply RC. RC is defined by `ina_rc_t` which is 
in fact a 64bit unsigned integer value and can contains an error indicator, API version 
 information, Native OS error and application error message. Almost all API
 The RC is packed as follow:
  
    RC 64bit mask
    
    |- 16bit header -|-------- 48bit descriptor  --------------------------|
    |                |                 |-- 32bit error message           --|
    |                |                          |9bit cde |                |
    ERRRVVVV|RRRRRRRR|OOOOOOOO|OOOOOOOO|UUUUUUUU|AAAAAAAA|NSSSSSSS|SSSSSSSS|
    | |  |       |          |              |         |     |    |
    | |  |       |          |              |         |     |    +-->15 bit - Subject
    | |  |       |          |              |         |     +-------> 1 bit - Negate flag
    | |  |       |          |              |         +--- ---------> 8 bit - Adjective/verb
    | |  |       |          |              +-----------------------> 8 bit - User defined data (unused)
    | |  |       |          +-------------------------------------->16 bit - Native OS error
    | |  |       +-------------------------------------------------> 8 bit - API revision
    | |  +---------------------------------------------------------> 3 bit - API version
    | +------------------------------------------------------------> 3 bit - Reserved    
    +--------------------------------------------------------------> 1 bit - Error indicator

INAC pursues the goal of understandable human error messages therefore the 
error code contains information on adjective / verb, possible negation 
and subject

#### Error indicator
The highest bit is used (if ON) to indicate if the RC should be treated as a
failure. Therefor one 


One can clear the error indicator with `ina_err_clear()`. This can be useful 
when handling an error situation we need to return information about the 
handled error.
`` 
 

### API version information
INAC store the API major and minor(revision) version in the RC. To extract 

#### Error message
An error message has the form 'ADJECTIVE/VERB'  or ' NOT ADJECTIVE/VERB' like 
"Empty", "Not valid", "Not initialized", "Not running", "Unavailable", etc.

Additionally, one can be more specific on the error subject in the form of
'SUBJECT + (NOT) + ADJECTIVE/VERB'  errors are also supported like
"Argument invalid", "Network not initialized", "File not out", "Disk full", etc.

INAC provides standard subjects and adjectives/verbs which can be used to 
compose RCs. Subjects are identified by an 15 bit integer. Therefor there 
are 32767 possible subjects. The first 1024 are reserved by INAC libraries.
  
Subjects can be app-defined. For this purpose INAC provides a user defined dictionary 
callback function. Subjects are resolved by calling the user-provided dictionary 
function of type `ina_err_subject_cb_t` which is registered by `ina_err_register_dict()`.
The user defined dictionary is used to retrieve the full error message.

INAC subjects and adj/verb are defined in `libinac/error.h` and should almost any
kind of error for an application/library.

	 #define INAWS_ERR_NOCONNECTION    INA_ERR_NOT_CONNECTED
	 #define INAWS_ERR_CONNECT         INA_ES_CONNECTION|INA_ERR_NOT_CONNECTED


There is also the possibility to define errors using user defined subjects. 
Define first your user defined subjects:

    #define INAWS_ES_WORKSTATION      INA_ES_USER_DEFINED+1
    
Then define the error message combining adjective/verb and subject

    #define INAWS_ERR_WS_NOT_FOUND     INAWS_ES_WORKSRATION|INA_ERR_NOT_FOUND
    
Declare and implement a dictionary callback for the user defined subjects.

    static const char* __get_err_getsubject(int id) {
        switch (id) {
            case INAWS_ES_WORKSTATION:
                return "WORKSTATION";
            default:
                return "";
        }
    }

Register your dictionary callback asap at program startup
    
    int main(int argc,  char** argv) {
        ina_err_register_dict(__get_err_subject);
    
        ...
     }
    

#### General
Generally one will set error state by using _INA_ERROR()_

    #define E_OBJ_IN_USE INA_ES_OBJECT|INA_ERR_IN_USE
    #define E_FILE_IN_USE INA_ES_FILE|INA_ERR_IN_USE
    
    ina_rc_t release_object(obj_t m) {
        if (m->c_ref != 0) {
            return INA_ERROR(E_OBJ_IN_USE);
        } else {
            .. try to close file...
            if (m->file_open) {
               return INA_ERROR(E_FILE_IN_USE);
            }
            ...
        }
        return INA_SUCCESS;
     }
     
     ....
     if (INA_FAILED(release_object(m)) {
        switch (INA_RC_ERROR(ina_err_get_rc())) {
            case INA_ERR_IN_USE:
                ...
        }
     
    
We can get access to the reason of failure by `INA_RC_ERROR` macro.

	switch (INA_RC_REASON(rc)) {
	   case INAWS_ERR_CONNECT:
	      .....

Use  _INA_SUCCEED()_ or _INA_FAILED()_ macro to determine if an RC is an
error or not.

    if (INA_SUCCEED(inaws_start_server()) {
        ....
    }

    if (INA_FAILED(inaws_start_server()) {
        ....
    }    
    
#### Capture and handling of native errors 
Use _INA_OS_ERROR() to capture the last native error. This is errno on
unix based os and GetLastError() on Windows platforms. To extract the 
native error code use the _INA_RC_ERRNO()_ macro.
    
    ina_rc_t openfile(const char* fn)
    {
        f = fopen("test.csv", "r");
        if (f == NULL) {
            INA_OS_ERROR(INA_ES_FILE|INA_ERR_NOT_OPEN);
        }
        return INA_SUCCESS
    }
    
    ...
    if (INA_FAILED(openfile(fn)) {
        if (INA_RC_ERRNO(ina_err_get_rc()) == EFULL) {
            
    
   
### Return Code

#### Error flag
Indicate RC is an error. Use  _INA_EFLAG()_ to extract the error flag.
#### Version
Contains the major version number defined by _INA_ERROR_VER_ or 0 if not defined.
Use _INA_RC_VER()_ to extract the version number from RC.
#### Revision
Contains the revision number defined by _INA_ERROR_REV_. Use _INA_RC_REV()_ to extract the
revision number from.
#### Native OS error 
Contains the captured native error code. Use _INA_RC_ERRNO()_ to extract the
native error code.
#### Negate flag
Use _INA_RC_NFLAG()_ to extract the negate flag. 
#### Attribute code
There are a max of 256 predefined error code
#### User defined code

#### Cleanup the error state

To reset the  error state use `ina_err_reset()`. 

	/* make sure error state is clean */
	ina_err_reset();
	/* do the work now */
	if (INA_FAILED(inaws_server_start())) {

### Logging
U




## Memory Handling

The INAOS Common C Library provide custom memory allocation and memory pooling.
Main Goals of those components:

- Avoid memory leaks. Especially in continuous server processes.
- Speed. By reducing significantly time consuming memory allocations and 
  employing better memory allocators.
- Hide complexity. In fact consumers doesn't have to care about releasing
  previously allocated memory.

### Architecture
#### Allocator
#### Memory Pool
##### Fixed sized pool
##### Dynamic sized pool
##### Auto sized pool
##### Using shared memory
#### Memory strategies
##### Standard
##### Best fit
### Using the API
#### Working with pools
### Error codes

## High-Level Communication : ISCP

__FIXME__

## Time

__FIXME__

## Timer

__FIXME__

## LuaJIT API

__FIXME__

## Configuration file

INAC provides a configuration file parser which works for C and Lua as well.

### Creating the configuration file

The configuration file is a pure Lua script and consists of sections. Those 
section can be named or unnamed and they contains one more key/value pairs.
Sections and keys can be marked as required. Values for key can be string or
number type.  

    -- Unnamed section
    debug {
        command-latency=1000
    }
    -- Named section with key lo1
    iface "lo1" { 
        ip="127.0.0.2", 
        mask="255.0.0.0" 
    }
    -- Named section with key lo0
    iface "lo0" { 
        ip="127.0.0.1", 
        mask="255.0.0.0" 
    }

Configuration definition 

    sections = {}
    sections.debug = {
        name = "debug",
        named = false,
        required = true,
        keys = {
            command-latency = {
                required = true,
                typename = "number"
            },
            other_latency = {
                required = false,
                typename = "number"
            }
        },
        configured = false
    }

    sections.iface = {
        name = "iface",
        named = true,
        required = true,
        keys = {
            ip = {
                required = true,
                typename = "string"
            },
            mask = {
                required = true,
                typename = "string"
            },
        },
        configured = false
    }

### Working with configuration files
   
For basic usage use the appropriates macros. Start by declaring a variable to
hold the instance for the configuration file.
   
    ina_conffile_t *cf = NULL;
    
Declare 

    INA_CONFFILE(cf, NULL,
        INA_CONFFILE_SECTION("debug", INA_YES, NULL,
            INA_CONFFILE_NUMBER_KEY("command-latency", INA_YES)),
        INA_CONFFILE_NAMED_SECTION("iface", INA_NO, NULL,
            INA_CONFFILE_STRING_KEY("ip", INA_YES),
            INA_CONFFILE_NUMBER_KEY("mask", INA_NO)));

Create a configuration file instance by calling `ina_conffile_new()`.

    ina_conffile_t *cf = NULL;
   
    if (INA_SUCCEED(ina_conffile_new(&cf, NULL)) {

After calling you will get an new configurations file instance. You can 
optionally pass a file path as second argument to override the standard pattern of
configuration file location. By convention the configuration file path is 
[binary-name].conf in the current working directory if nothing else is 
specified.

Remember that each instance need to be destroyed with `ina_conffile_free()`. 

Define section and keys
   	
    ina_conffile_section_t *section = NULL;
    
    /* Add a unnamed section */
    ina_conffile_add_section(cf, &section, "debug", INA_YES);
    
    /* Add a key for a numeric required value to a section */
    ina_conffile_add_key(section, "command-latency", 
        INA_CONFFILE_VALUE_TYPE_NUMBER, INA_YES);
        
    /* Add a unamed section */
    ina_conffile_add_section(cf, &section, "iface", INA_YES);

Sample processor written in Lua

    -- sample processor
    for sk,s in pairs(sections) do
      if s.configured then
          print(sk)
          if not s.named then
              for k,v in pairs(s.keys) do
                  if v.has_value then
                      print(k,v.value)
                  end
               end
           else
               for nsk, ns in pairs(s.children) do
                   print("Named section: "..nsk)
                   for k,v in pairs(s.keys) do
                       if ns[k].has_value then
                           print(k,ns[k].value)
                       end
                    end
                end
            end
        end
    end

## Console

__FIXME__

## Cron - Scheduling

From Wikipedia: Cron is the time-based job scheduler in Unix-like computer 
operating systems. 
Cron enables users to schedule jobs (commands or shell scripts) to run 
periodically at certain times or dates. It is commonly used to automate system
maintenance or administration, though its general-purpose nature means that it
can be used for such things as connecting to the Internet and downloading email.

This introduction also explains why we have named our scheduling component 
Cron. First of all because its has the same functional goals as cron daemon 
has for an OS our cron is targeted at server-applications that have to execute
general tasks according to a schedule. Second reason is because it uses the
same syntax as the well know cron daemon to define tasks.

### Overview

There are two main design choices that are important to know up-front:

1. The scheduling does not support in-process activities - if you need short 
   term in process activities then you should look at the Timer API. Our Cron
   component is design for the invocation of designeted processes that execute
   a batch style activity. Therefore it only takes the path and arguments to
   the executable as arguments.
   
2. One has multiple options to persist the task definitions. The Cron component
   itself does not support any persistence. However it defines callback methods
   that one can implement to persist task definitions.
   
   * One can use the configuration in order to persist the task-definitions
   * One can use the callbacks to persist the task-definitions in a datastore

### Usage

First initialize the cron context

    INA_SUCCEED(ina_cron_init(&ctx, NULL, NULL));
	
Alternatively one can pass `load()` and `save()` callbacks.

Then add a task:

    ina_str_t cmd = ina_str_fromcstr("pwd.exe .");
    ina_str_t wd = ina_str_fromcstr("c:\\windows");
    INA_SUCCEED(ina_cron_task_new(ctx, "pwd", "0 23 * * *", 0, cmd, wd));
    ina_str_destroy(cmd);
    ina_str_destroy(wd);
	
Get a task by id:

    INA_SUCCEED(ina_cron_task_by_id(ctx, "pwd", &task));
	
Iterating through tasks:

    INA_SUCCEED(ina_cron_task_new_iter(ctx, &itr));
    while (task != NULL) {
        int running = 0;
        ina_str_t patt;
        INA_SUCCEED(ina_cron_task_is_running(task, &running));
        INA_SUCCEED(ina_cron_task_get_pattern(task, &patt));
        found++;
        INA_SUCCEED(ina_cron_task_next(itr, &task));
    }
    INA_SUCCEED(ina_cron_task_free_iter(&itr));

Give the component the chance to execute tasks - this needs to be called in 
the main loop

    int suggested_sleep_time;
    time_t now = time(NULL);
    INA_SUCCEED(ina_cron_process(ctx, now, &suggested_sleep_time));
    now += suggested_sleep_time;
    INA_SUCCEED(ina_cron_process(ctx, now, &suggested_sleep_time));
	
Destroying the context:

    INA_SUCCEED(ina_cron_destroy(&ctx));
	
## HTTP

The INAC HTTP parser is using Joyents HTTP-Parser which is a natural fit with 
the goals of INAC. The only thing that had to be changed in the change from 
PUSH to a PULL API like any other INAC API.

### Overview

It follows the same model as other INAC components that it allocates a pool of
parsers upon creation of the context and that you can borrow and return parser
instances.
Client and Server HTTP parsing is supported.

### Usage

First, create the context:

    INA_SUCCEED(ina_http_init(&ctx, INA_HTTP_PARSER_TYPE_BOTH, 2));
	
This will initialize parsers with client and server capabilities. Then we 
borrow an instance:
	
    INA_SUCCEED(ina_http_parser_borrow(ctx, &parser));
	
Then we execute the parser on some data:

    INA_SUCCEED(ina_http_parser_execute(parser, requests[0].raw, strlen(requests[0].raw), &more));

Note: That we pass in a flag that will indicate whether the parser want to be 
called again or whether it has finished parsing the whole request.
Once the parsing is completed, we can access properties like the keep-alive:
	
    INA_SUCCEED(ina_http_parser_should_keep_alive(parser, &skal));

Once you're finished with the usage of the parser return it to the pool:

	INA_SUCCEED(ina_http_parser_release(ctx, &parser));
	
And destroy the context:

	INA_SUCCEED(ina_http_destroy(&ctx));

## Logging

__FIXME__

## Networking

__FIXME__

## XML

The current XML parser implementation - is a high-performance in-situ parser. 
This means it is not suitable for large documents. Currently in-situ parsing 
is the only option supported by INAC. Going forward we might add an SAX based 
model as well that is better suitable for processing large files. 
The in-situ parser is pure C port of the RapidXML parser originally 
written in C++.

### Overview

To use an XML parser you have to borrow an instance from the context and 
return it after usage. This model is geared towards server usage where you 
know up-front how many parser instances you'll need (concurrency) and 
therefore do all the resource-allocation up-front. During request processing 
ideally no memory allocations should be done at all.

### Usage

First initialize the context:

    ina_xml_ctx_t *ctx = NULL;
    ina_xml_parser_t *parser = NULL;
    INA_SUCCEED(ina_xml_init(&ctx, 16));

Borrow a parser instance:    
	
    INA_SUCCEED(ina_xml_parser_borrow(ctx, &parser));

Do some parsing:

    INA_SUCCEED(ina_xml_parser_execute(parser, data->source, &root)); 

Release the parser back to the pool:
	
    INA_SUCCEED(ina_xml_parser_release(ctx, &parser));

And destroy the context:
	
    INA_SUCCEED(ina_xml_destroy(&ctx));

## Testing

### Tracing

Tracing feature can be enabled an disabled by combiler time settings 
`INA_TRACE_ENABLED`.  Also the tracing level can be define at compile time. 
The library know about 3 tracing levels. Trace messages are ended by a 
newline "\n" 

INAC provides 2 macros which can be used for print debug messages when DEBUG 
is defined

     INA_TRACE
     INA_TRACE_MSG

Use `INA_TRACE_MSG` to print simple messages and `INA_TRACE` to print debug 
messages having var args.

    INA_TRACE_MSG("Server started");
    INA_TRACE("Buffer size is %d", bufsize);
    INA_TRACE1("Same as the %s macro", "INA_TRACE");
    INA_TRACE2("A bit more %s trace", "detailed");
    INA_TRACE3("A %s trace", "fully detailed"); 

### Unit testing

INAC provides a built-in test framework. This framework is almost independent 
from the library itself. 

#### Features  

 * Easy adding tests with minimal effort. Non header files required.
 * Supports test suites
 * Supports fixtures (setup, teardown)
 * Easy to parse output
 * Colored output
 * Supports skipping
 * Minimal memory footprint (no allocations)
 * Supports test helpers
 * Working the same way on Linux/OS-X/Win 
 * Support tap and junit result format
 
Possibles improvements:

 * Possibility to add small description to each test for documentation purpose.
 * Variable output format
 * Display elapsed time

#### Adding tests 

To add your first test to a test suite simply the following lines of code.

    INA_TEST(my_suite, my_first_test_with_inac) {
        INA_ASSERT_FLOATING(1.0, 1.0);
    }


#### Adding fixtures  

To added fixtures to your test use `INA_TEST_FIXTURE` macro. Fixtures need a 
fixture data struct which is defined by `INA_TEST_DATA` macro.  Optionally you
can define a setup and teardown for your test. Setup and Teardown is call on 
any test in the suite.  Fixture data is passed to Setup/Teardown and Run of 
any test in the suite.  Follow the next sample. 

    INA_TEST_DATA(iscp_tcp) {
        ina_iscp_ctx_t *iscp;
    };

    INA_TEST_SETUP(iscp_tcp) {
        ina_iscp_create_tcp(&data->iscp, "127.0.0.1", 9999);
    }

    INA_TEST_TEARDOWN(iscp_tcp) {
        ina_iscp_destroy(&data->iscp);
    }

    INA_TEST_FIXTURE(iscp_tcp, send_negative_double) {
        INA_TEST_ASSERT_SUCCEED(ina_iscp_register(data->iscp, 3, 3, NULL));
        INA_TEST_ASSERT_SUCCEED(ina_iscp_send(data->iscp, 1, 
            INA_ISCP_TYPE_DBL, -3.2));
    }

NOTE: Do not forget the semicolon after `INA_TEST_DATA()`

#### How to skip tests 

To skip existing test use the _SKIP version of `INA_TEST` or `INA_TEST_FIXTURE`. 

    INA_TEST_SKIP(my_suite, my_first_test_with_inac) {
        INA_ASSERT_FLOATING(1.0, 1.0);
    }

    INA_TEST_FIXTURE(iscp_tcp, send_negative_double) {


#### How to run the test suites

To run the tests simply call `ina_test_run()` by passing arguments count and 
arguments received from the command line.

    int main(int argc, char** argv) 
    { 
        ina_test_run(argc, argv);

From the command line prompt you can start all tests or a single suite

    ./test
    ./test test_suite
    
You can choose alternative result formats like `tap` or `JUnit` with the 
`format` options.

    ./test --format=tap
    ./test --format=junit

    
#### Helpers 
A more advanced feature of this test framework are provided by helper macros.
The framework supports in-situ helper and external helpers as well. Each helper
is started in a new process. Further it's possible chose to between 
wait/or spawn mode.

##### Adding in-situ Helpers
In-situ helpers are compiled directly in the test binary by using the
`INA_TEST_HELPER`macro. The macro takes two arguments: the suite name and
helper name. The `argc` and `argv` from the `main()` function are available in
the code body. For easy use and access use  Each Helper should assign a valid
RC to `retval` before leaving.

    INA_TEST_HELPER(tcp, dummy_dns_server) {
        const char* addr;
        int port;

        /* We need 2 arguments
        INA_TEST_HELPER_CHECK_ARGC(2);
        /* Get arguments */
        addr = INA_TEST_HELPER_CHAR_ARG(0);
        port = INA_TEST_HELPER_INTEGER_ARG(1);

        /* Starting coding your dummy tcp DNS server */
            ...

        INA_TEST_HELPER_SET_RC(EXIT_SUCCESS);
    }


##### Invoking in-situ Helpers	
Use `INA_TEST_HELPER_INVOKE` to start a child helper process.

    INA_TEST(tcp, dns_ping) {
        /* Invoke helper */
        ina_test_hid_t hid;
        INA_TEST_HELPER_INVOKE(&hid, tcp, dummy_dns_server, "127.0.0.1", 
                                "9001", NULL);

        /* Make some tests */
        INA_TEST_ASSERT_TRUE(dns_ping("120.0.0.1", 9001));

        /* Kill helper process */
        INA_TEST_HELPER_TERMINATE(hid);
    }

    INA_TEST(ullc, read_ring_buffer) {
        /* Invoke helper */
        ina_test_hid_t hid;
        INA_TEST_HELPER_INVOKE_WAIT(&hid&, tcp, create_ring_buffer, 5000, NULL);

        /* Make some tests */
        INA_TEST_ASSERT_TRUE(read_ring_buffer());
    }

To test or start an in-situ helper from the command line juste type

    ./test -h suite_name helper_name

  
##### External Helpers

    INA_TEST(tcp, dns_ping) {
        /* Invoke helper */
        ina_test_hid_t hid;
        INA_TEST_HELPER_CMD(&hid, "c:/test/dns.exe" "127.0.0.1", "9001", NULL);

        /* Make some tests */
        INA_TEST_ASSERT_TRUE(dns_ping("120.0.0.1", 9001));

        /* Kill helper process */
        INA_TEST_HELPER_TERMINATE(hid);
    }
  
    INA_TEST(tcp, dns_ping) {
        /* Invoke helper */
        ina_test_hid_t hid;
        INA_TEST_HELPER_CMD_WAIT("c:/test/dns.exe", 5000, "127.0.0.1", 
                                    "9001", NULL);

        /* Make some tests */
        INA_TEST_ASSERT_TRUE(dns_ping("120.0.0.1", 9001));  
    }

### Performance testing

#### Features
 * Easy adding benchmarks test with minimal effort. Non header files required.
 * CSV output
 * Integrated stopwatch
 * Supports skipping series
 * Minimal memory footprint
 * Working the same way on Linux, Windows and OS X 
 
#### Adding benchmarks
To add a benchmark use the INA_BENCH_* macros. Every benchmark is composed at least 
of 1 series which is executed at least 1 time. A benchmark has the following phases:
- __Setup__: called for every series in the benchmark
- __Begin__: called once for a series
- __Scale__: called for the current series at every iteration 
- __Benchmark__: called for the current series at every iteration
- __End__: called once for a series
- __Teardown__: called for every series in the benchmark

Every phase must be declared by the corresponding macro.

##### The setup phase

Use _INA_BENCH_SETUP_ to define the setup phase. This phase is destined to run 
common setup code for series.

`INA_BENCH_SETUP([benchmark name])`

You may call _ina_bench_set_scale_label()_, _ina_bench_get_iterations()_, 
_ina_bench_get_name()_, _ina_bench_get_series_name()_ during this phase.

```C
INA_BENCH_SETUP(sort) {
    ina_bench_set_scale_label("ns");
    data->nr_of_elements = 1000000;
    data->elements = ina_mem_alloc(sizeof(element)*data->nr_of_elements);
}
```

If the setup phase is not need just declare it with an empty body. 

```C
INA_BENCH_SETUP(sort) {}
```

##### The begin phase
The begin phase is designated to run setup code for a single series.

`INA_BENCH_BEGIN([benchmark name], [series name])`

You may call _ina_bench_get_iterations()_, 
_ina_bench_get_name()_, _ina_bench_get_series_name()_ during this phase.

```C
INA_BENCH_BEGIN(sort, quick_sort) {
    data->sort_fn = __ina_quicksort;
}

```
If the begin phase is not need just declare it with an empty body. 
```
INA_BENCH_BEGIN(sort, quick_sort) {}
```

##### The scale phase
The scale phase is intended to capture the scale value. You need to
call _ina_bench_set_scale()_ before leaving the phase. This phase is called 
for each iteration just before running the benchmark code.

`INA_BENCH_SCALE([benchmark name])`

You may also call _ina_bench_get_iterations()_, _ina_bench_get_iteration()_, 
_ina_bench_get_name()_, _ina_bench_get_series_name()_, 
_ina_bench_stopwatch_start()_ during this phase.

```C
INA_BENCH_SCALE(sort) {
    ina_bench_set_scale(data->nr_of_elements*ina_bench_get_iteration());
}
```


##### The benchmark phase
This phase is intended to run the effective benchmark code and capture the
measurement. The benchmark phase is called for each series and iteration. 
_ina_bench_set_value()_ must called  before leaving the benchmark phase.

```INA_BENCH(benchmark name], [series name], [nr of iterations])```

Most of the time the measurements consists of time measurements 
. The benchmark framework provide _ina_bench_stopwatch_start()_ and 
_ina_bench_stopwatch_stop()_ to this end.

You may also call _ina_bench_get_iterations()_, _ina_bench_get_iteration()_, 
_ina_bench_get_name()_, _ina_bench_get_series_name()_ during this phase.

```C
INA_BENCH(sort, quick_sort, 100) {
    ina_bench_stopwatch_start();
    data->sort_fn(data->elements, data->nr_of_elements);
    ina_bench_set_value(ina_bench_stopwatch_stop());
}
```

##### The end phase
The begin phase is designated to run cleanup code for a single series.

`INA_BENCH_END([benchmark name], [series name])`

You may also call _ina_bench_get_iterations()_,  _ina_bench_get_name()_, 
_ina_bench_get_series_name()_, _ina_bench_stopwatch_stop()_ during 
this phase.

```C
INA_BENCHEND(sort, quick_sort) {
   __ina_do_something_after_bench();
}
```

If the end phase is not need just declare it with an empty body. 
```
INA_BENCH_END(sort, quick_sort) {}
```

##### The teardown phase
Use _INA_BENCH_TEARDOWN_ to define the teardown phase. This phase is 
destined to run common teardown code for series.

`INA_BENCH_TEARDOWN([benchmark name])`

You may call _ina_bench_get_iterations()_, _ina_bench_get_name()_, 
_ina_bench_get_series_name()_ during this phase.

```C

INA_BENCH_TEARDOWN(sort) {
    ina_mem_free(data->elements);
}
```

If the teardown phase is not need just declare it with an empty body. 

```C
INA_BENCH_TEARDOWN(sort) {}
```

#### How to run benchmarks

To run the benchmarks simply call _ina_bench_run()_ by passing arguments count and 
arguments received from the command line.

    int main(int argc, char** argv) 
    { 
        ina_bench_run(argc, argv);

From the command line prompt you can run all benchmarks, a single benchmark or
a group of benchmarks. 

    ./bench
    ./bench my_benchmark
    ./bench my_

The location where reports should be stored can be specified with 
_-r_ or _--report-path_ as first command line option.

    ./bench -r=/home/reports
    ./bench --report-path=/home/reports my_benchmark
    
If omitted the reports are generated in the working directory.


[1]:	https://www.visualstudio.com/downloads/
[2]:	https://cmake.org/download/
