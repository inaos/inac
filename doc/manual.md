# INAOS Common C Library

The INAOS Common C Library is a collection of header files and library routines 
used to implement common operations, such as input/output, character string 
handling, memory, error and event handling. This library is designed and 
optimized for singled threaded applications and is used as common base for 
all INAOS programs/libraries written in C. The library includes a built-in
LuaJIT engine (http://luajit.org)

High level objectives:

* Be minimal but complete (keep it simple)
* Low complexity
* Low Resource consumption/High performance
* Ease of maintenance, testing and debugging
* Fully documented

# Getting started

## Building on Windows

Building on Windows requires some programs to be present on your system.

### Prerequisites

* [Visual Studio 2012][1]
* [CMake][2]
  * Use the binary installer you don't need to build from source
  * Make sure you add cmake to your PATH

### Build

* Open a Visual Studio command prompt
* Navigate to the INAC root folder
* Type: make.bat all debug

## Building on Linux or OS X

To build and install the library, simply type `sudo make && make install`. To select
the debug build, type `make debug`.

## Compile time configuration
 * `INA_CSTRING_ENABLED`: Enable C-runtime strings (Default)
 * `INA_BSTRING_ENABLED`: Enable BSTRING string (The Better String Library)
 * `INA_ISTRING_ENABLED`: Enable INAOS string 
 * `INA_SYSMEMPOOL_SIZE`: Define the capacity in bytes of the internal memory pool 
		                  Default is 8 MB		                  
 * `INA_TRACE_ENABLED`  : Enable/disable tracing. Default enabled.
 * `INA_TRACE_LEVEL`    : Set trace level (1-3). Default 1.
 * `INA_LOG_ENABLED`    : Enable/disable logging. Default enabled.
 * `INA_LOG_LEVEL`      : Set log level from 1 (errors) to 4(debug). Default 3 (info).
 


All constants are prefaced with `INA_` . Other identifiers are prefaced with `ina_`.
Type names are suffixed with `_t` and typedef‘d so that the struct keyword need
not be used.

## Starting to code

Start by including the INOAS library header in your code:

	#include <libinac/lib.h>;

## For library consumers
Initialize the library context as soon as possible:

	ina_init(0);

For each call of `ina_init()` you have to call `ina_exit()`. You can override the 
size system memory pool by passing the pool size in bytes as argument.  

## For applications
For applications, initialize the application context with `ina_appinit()`. This must be 
the first function call in your program. You must call `ina_exit()` once before you quit 
your program. You can override the system memory pool size by passing the pool size in
bytes as third argument.

	int main(int argc, char **argv,) 
	{
	    if (INA_SUCCEED(ina_appinit(argc, argv, 0, NULL)) {
	        while (... {
	            ...
	        }
	    }
	    ina_exit(EXIT_SUCCESS);
	}

### Command line options
The library provides a builtin command line processor. For that purpose the 
`ina_appinit()` takes as firth argument an array of `ina_opt_t` containing the 
command line options definition consisting in string, number and flag options. 
Use the designated macros to build the options array. Options are defined with a short, a long option name and a description. On string and number options a default
value can de defined. 

* `INA_OPT_STRING(short,long,default,description)`: define a string option
* `INA_OPT_INT(short,long,default,description)`: define a int option
* `INA_OPT_FLAG(short,long,description)`: define a flag (default is false)


Use `INA_OPT(array-name)` to declare the option array:

	INA_OPTS(opt,
        INA_OPT_STRING("h", "host", NULL, "Hostname"),
        INA_OPT_INT("p", "port", 999, "Port"),
        INA_OPT_FLAG("k", "keep-alive", "Keep connection alive")));

Register and parse the options by passing the options array to `ina_appinit()`. 
The function fails with RC `INA_EOPT` if current command line options don't  
match with the registered definition and simple a usage screen will be printed 
out to the standard output.

	if (INA_SUCCEED(ina_appinit(argc, argv, 0, opt)) {
	    while (... {
	            ...
	    }

To query a flag is whenever or not set use `ina_opt_isset()`:

	if (INA_SUCCESS(ina_opt_isset("keep-alive")) {

To get a int value use `ina_opt_get_int()`:
	
	int value = 0;
	ina_opt_get_int("port", &value);

To get a string value use `ina_opt_get_string()`:
	
	ina_str_t value = NULL;
	ina_opt_get_string("host", &value);

The command line options values are preserved for the until the application stops. 


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

## Misc macros

# API Reference

## Library Version
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
reserved to the INAOS Common C Library. Define user error codes starting
by 129. For instance:

	 #define INAWS_ERR_NOCONNECTION    INA_ERR_USER+1

We can get access to the reason by ÌNA\_RC\_REASON\` macro.

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
`INA_MOD_STRING` identifies the string compilation unit. Developers can define
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

#### Push
Use the `INA_ERR_PUSH` macro to push an error to the global error state.

	INA_ERR_PUSH(INAWS_ERR_NOCONNECT, 
	    INAWS_MOD_SERVER, INA_OSFN_NONE, "Connection failed");

For simplification, use the `INA_ERR_PUSH_BASIC` or `INA_ERR_PUSH_OSFN` 
macros on depending the error information you have.

	INA_ERR_PUSH_BASIC(INAWS_ERR_NOCONNECT, "Connection failed");
	INA_ERR_PUSH_OSFN(INAWS_ERR_NOCONNECT, INA_OSFN_NONE, "Connection failed");

#### Peek
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

#### Cleanup the error state
To reset the entire error state use `ina_err_reset()`. All errors including 
the most recently  pushed are removed from the error state.

	/* make sure error state is clean */
	ina_err_reset();
	/* do the work now */
	if (!INA_SUCCEED(inaws_server_start())) {
	    rc = ina_err_peek();
	    if (!INA_ERR_FATAL(RC)) 

### Cleanup handler
There is a possibility to define a callback function which is called in case 
the program is being terminated because of fatal error like segmentation fault
or an interruption request like ctrl-c.
Use `ina_err_set_cleanup_handler()` to define such a callback. 
Keep in mind that this cleanup handler will be called only in case of abnormal
program termination.

### Utilities
The error handling module of this library provide two useful functions. They 
are used internally but they are for public use as well.

- `ina_err_trace()` printout current error state to the standard output.

## Memory Handling
The INAOS Common C Library provide custom memory allocation and memory pooling.
Main Goals of those components:

- Avoid memory leaks. Especially in continuos server processes.
- Speed. By reducing significantly time consuming memory allocations and 
  employing better memory allocators.
- Hide complexity. In fact consumers doesn't have to care about releasing
  previously allocated memory.

### Architecture
#### Internal memory pool
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

## Time & Timer

## LuaJIT API

## Configuration file
INAC provides a configuration file parser witch works for C and Lua as well.

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
   
For basic usage use the appropriates macros. Start by declaring a variable to hold the instance for the configuration file.
   
    ina_conffile_t *cf = NULL;
    
Declare 

    INA_CONFFILE(cf,
    	INA_CONFFILE_SECTION("debug", INA_YES, NULL,
    		INA_CONFFILE_NUMBER_KEY("command-latency", INA_YES)),
    	INA_CONFFILE_NAMED_SECTION("iface", INA_NO, NULL,
    		INA_CONFFILE_STRING_KEY("ip", INA_YES),
    		INA_CONFFILE_NUMBER_KEY("mask", INA_NO)));
    		

Create a configuration file instance by calling `ina_conffile_init()`.

    ina_conffile_t *cf = NULL;
   
    if (INA_SUCCEED(ina_conffile_init(&cf, NULL)) {

After calling you will get an new configurations file instance. You can 
optinally pass a filepath as second argument to overide the standard pattern of
configuation file location. By convention the configuration file path is 
[binary-name].conf in the current working directory if nothing else is 
specified.
Remember that each instance need to be destroyed with `ina_conffile_destroy()`. 

Define section and keys
   	
	ina_conffile_section_t *section = NULL;
     
   	/* Add a unnamed section */
   	ina_conffile_add_section(cf, &section, "debug", INA_YES);
   	
   	/* Add a key for a numeric required value to a section */
   	ina_conffile_add_key(section, "command-latency", INA_CONFFILE_VALUE_TYPE_NUMBER, INA_YES);
   	
   	/* Add a unamed section */
	ina_conffile_add_section(cf, &section, "iface", INA_YES);
 

Sample processor witten un LUA

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
		

## Testing
### Tracing
Tracing feature can be enabled an disabled by combiler time settings `INA_TRACE_ENABLED`.  Also the
tracing level can be define at compile time. The library know about 3 tracing levels. Trace messages
are ended by a newline "\n" 
INAC provides 2 macros which can be used for print debug messages when DEBUG is defined
	INA_TRACE
	INA_TRACE_MSG
Use `INA_TRACE_MSG` to print simple messages and `INA_TRACE` to print debug messages having var args.

	INA_TRACE_MSG("Server started");
	INA_TRACE("Buffer size is %d", bufsize);
	INA_TRACE1("Same as the %s macro", "INA_TRACE");
	INA_TRACE2("A bit more %s trace", "detailed");
	INA_TRACE3("A %s trace", "fully detailed"); 

### Unit testing
INAC provides a built-in test framework. This framework is almost independent from the library itself. 

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
 
Possibles improvements :
 * Possibility to add small description to each test for documentation purpose.
 * Variable output format
 * Display elapsed time
 


#### Adding tests 
To add your first test to a test suite simply the following lines of code.

    INA_TEST(my_suite, my_first_test_with_inac) {
    	INA_ASSERT_FLOATING(1.0, 1.0);
	}


#### Adding fixtures  
To added fixtures to your test use `INA_TEST_FIXTURE` macro. Fixtures need a fixture data struct which is 
defined by `INA_TEST_DATA` macro.  Optionally you cann define a setup and teardown for your test. Setup and 
Teardown is call on any test in the suite.  Fixture data is passed to Setup/Teardown and Run of any test in
the suite.  Follow the next sample. 

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
      	INA_TEST_ASSERT_SUCCEED(ina_iscp_send(data->iscp, 1, INA_ISCP_TYPE_DBL, -3.2));
	}

NOTE: Do not forget the semicolon after `INA_TEST_DATA()`

#### How to skip tests 
To skip existing test use the _SKIP version of `INA_TEST` or `INA_TEST_FIXTURE`. 

    INA_TEST_SKIP(my_suite, my_first_test_with_inac) {
    	INA_ASSERT_FLOATING(1.0, 1.0);
	}
	
    INA_TEST_FIXTURE(iscp_tcp, send_negative_double) {


#### How to run the test suites
To run the tests simply call `ina_test_run()` by passing arguments count and arguments received from
the command line.

	int main(int argc, char** argv) 
	{ 
    	ina_test_run(argc, argv);

From the command line prompt you can start all tests or a single suite
	
	./test
	./test test_suite
	
    
#### Helpers 
A more advanced feature of this test framework are provided by helper macros. The framework supports in-situ 
helper and external helpers as well. Each helper is started in a new process. 

##### Adding in-situ Helpers
In-situ helpers are compiled directly in the test binary by using the `INA_TEST_HELPER`macro. The macro takes
two arguments: the suite name and helper name. The `argc` and `argv` from the `main()` function are available in 
the code body.

	INA_HELPER(tcp, dummy_dns_server) {
	  /* Starting coding your dummy tcp DNS server */
	  if (argc > 0) {
	  	...
	}

##### Invoking in-situ Helpers	

    INA_TEST(tcp, dns_ping) {
    	/* Invoke helper */
    	ina_test_hid_t hid;
   		INA_TEST_HELPER_INVOKE(tcp, dummy_dns_server, "127.0.0.1", 9001);
   		
        /* Make some tests */
		INA_TEST_ASSERT_TRUE(dns_ping("120.0.0.1", 9001));
		
		/* Kill helper process */
		INA_TEST_HELPER_STOP(hid);
	}
	
	INA_TEST(ullc, read_ring_buffer) {
    	/* Invoke helper */
    	ina_test_hid_t hid;
   		INA_TEST_HELPER_INVOKE_WAIT(tcp, create_ring_buffer, 5000);
   		
        /* Make some tests */
		INA_TEST_ASSERT_TRUE(read_ring_buffer());
		
	}

	
To test or start an in-situ helper from the command line juste type

	./test -h suite_name helper_name

  
##### External Helpers

    INA_TEST(tcp, dns_ping) {
    	/* Invoke helper */
    	ina_test_hid_t hid;
   		INA_TEST_HELPER_CMD("c:/test/dns.exe "127.0.0.1", 9001);
   		
        /* Make some tests */
		INA_TEST_ASSERT_TRUE(dns_ping("120.0.0.1", 9001));
		
		/* Kill helper process */
		INA_TEST_HELPER_STOP(hid);
	}
  
   INA_TEST(tcp, dns_ping) {
    	/* Invoke helper */
    	ina_test_hid_t hid;
   		INA_TEST_HELPER_CMD_WAIT("c:/test/dns.exe, 5000, "127.0.0.1", 9001);
   		
        /* Make some tests */
		INA_TEST_ASSERT_TRUE(dns_ping("120.0.0.1", 9001));
	}
  

   
	
#### Performance testing






[1]:	http://www.microsoft.com/visualstudio/eng/products/visual-studio-express-products
[2]:	http://www.cmake.org/cmake/resources/software.html
