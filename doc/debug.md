# Debug
## Overview

INAC provides a set of macros for tracing and assertions.

## Usage

### Tracing
Enable tracing by defining `INA_TRACE_ENABLED` and the trace level. To define
 the level use compile definition `INA_TRACE_LEVEL`. The trace level can be 
between 0 (disabled) and 3. Default is level `1`

*INAC CMake Build Framework* provide a macro to enable tracing for a
specific build type. If omitted tracing is enabled on level 3 for 
debug build and disabled for release builds.

    inac_enable_trace(Debug 3)
    inac_enable_trace(Release 1)


Trace output is filter by the trace category. Only traces matching to the 
category filter are printed. If the filter is not set, only the category "*" 
is considered for the output. The filter must be defined as an environment 
variable with the name `INAC_TRACE`.

    # all categories starting with "inac."
    export INAC_TRACE=inac.* 
    # print only categoreis maching exactly to "inac" .
    export INAC_TRACE=inac   
    # print categories maching to "inac.file", and starting wwith "examples"
    export INAC_TRACE=inac.file,examples.* 


Write trace output using `INA_TRACEx()` macro. 

    INA_TRACE1(*, "Trace level  %d is enabled and will ", 1)
    INA_TRACE1(inac.error, "Trace level  %d is enabled", 1)
    INA_TRACE2(inac.info, "Trace level  %d is enabled", 2)
    INA_TRACE3(inac.debug, "Trace level  %d is enabled", 3)
    
One can force trace output with `INA_TRACE()`

    INA_TRACE(inac.debug, "Always printed");
    
Traces are written to the standard output. To change this behavior set
the file handle to use as target by defining it with `INA_TRACE_TARGET`.

    FILE *trace;
    INA_TRACE_TARGET trace;
    
    trace = fopen("trace.log", "w");
    INA_TRACE1(inac.debug, "this trace goes to a file");
    
Ensure that the file is open before writing the first trace and not closed
before the last trace is written.

In case one needs to write only some of the trace output in to a file, use the
`INA_TRACE_TO_FILE`, `INA_TRACE1_TO_FILE`, `INA_TRACE2_TO_FILE` or 
`INA_TRACE3_TO_FILE`.

    INA_TRACE1_TO_FILE(stderr, inac.error, "Error in main.c");



### Asserting
INAC offers a number of assert macros to make assertion easier to write.
To use asserts `INA_USE_ASSERTS` must defined. Asserts are enabled by
default in debug builds.

*INAC CMake Build Framework* provides a macro to enable assertion for a
specific build type. 

    inac_use_asserts(Release)


Write assertions:

Assertion for not implemented code
    
    INA_NOT_IMPL
    
Arbitrary assertion
  
    INA_ASSERT(3 != 2)
    
Assertion assuming false
    
    INA_ASSERT_FALSE(v)

Assertion assuming true

    INA_ASSERT_TRUE(v)
    
Assertion assuming NULL

    INA_ASSERT_NULL(my_var)
    
Assertion assuming not NULL

    INA_ASSERT_NOT_NULL(my_var)

Assertion assuming equality 

    INA_ASSERT_EQUAL(3, my_counter)

Assertion assuming inequality 
    
    INA_ASSERT_NOTEQUAL(3, my_counter)
    
Assertion assuming return code INA_SUCCESS   
 
    INA_ASSERT_SUCCESS(init())
    
Assertion assuming return code INA_FAILURE
    
    INA_ASSERT_FAILURE(rc)

Assuming a return code indicating success

    INA_ASSERT_SUCCEED(should_succeed())
    
Assuming a return code indicating failure
    
    INA_ASSERT_NOTSUCCEED(should_not_succeed())        

To mark a variable used only by assertion. This prevents warning about unused 
variables for release builds.

    void test(int flags, int value)
    {
       INA_USED_BY_ASSERT(flags);
       INA_ASSERT_FALSE(flags == 0);
       return value != 0;
    }