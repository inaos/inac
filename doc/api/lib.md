
```C
#ifndef _LIBINAC_LIB_H_
```

Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
#define INA_VERSION_HEX  ((INA_MAJOR_VERSION << 16) |   \
```
Version as a 3-byte hex number, e.g. 0x010201 == 1.2.1. Use this
for numeric comparisons, e.g. #if INA_VERSION_HEX >= ...
```C
#define INA_REVISION_HEX ((INA_MINOR_VERSION << 8)  |   \
```
Revsion number as 2-byte hex number e.g 0x900 == 0.9. Use this
for numeric comparisons, e.g. #if INA_REVISION_HEX >= ...
```C
#define INA_RETURN_IF(x) do {if ((x)) return ina_err_get_rc(); } while(0)
```
Return with last rc if condition x fails
```C
#define INA_RETURN_IF_NULL(x) do {if ((x) == NULL) return ina_err_get_rc();} while(0)
```
Return with last rc if x == NULL
```C
#define INA_RETURN_IF_FAILED(rc) do { if (INA_FAILED((rc))) return ina_err_get_rc(); } while (0)
```
Return with last rc if failed
```C
#define INA_RETURN_IF_SUCCEED(rc) do {if (INA_SUCCEED((rc))) return ina_err_get_rc(); } while (0)
```
Return with last rc if succeed
```C
#define INA_AT __FILE__ ":" INA_NUM2STR(__LINE__)
```
Source location
```C
#define INA_OPT_FLAG(short_opt, long_opt, desc)           \
```
Add flag option
```C
#define INA_OPT_STRING(short_opt, long_opt, dft, desc)    \
```
Add string option
```C
#define INA_OPT_INT(short_opt, long_opt, dft, desc)       \
```
Add int option
```C
#define INA_OPT_FLOAT(short_opt, long_opt, dft, desc)       \
```
Add float option
```C
#define INA_OPTS(name, ...)                        \
```
Define options map
```C
typedef struct ina_opt_s {
```
Command line option builder
```C
typedef enum ina_signal_behavior_e {
```
Signal handling behavior
```C
typedef void (*ina_cleanup_handler_t) (int, int*);
```
Application cleanup handler .
```C
typedef void (*ina_signal_handler_t) (ina_signal_t, ina_signal_behavior_t*, int*);
```
Signal handler
```C
INA_API(const char*) ina_app_get_name(void);
```

Return the program name

```C
INA_API(const char*) ina_app_get_path(void);
```

Return path to the running application

```C
INA_API(ina_rc_t) ina_app_init(int argc,
```

Startup application with argc, argv in order to deal with
platform-specific quirks. This must be the first function called for any
program.


**Parameters**
 - `argc`: 
 - `argv`: 
 - `opt`: 



**Return**

INA_SUCCESS  if no error occurred


```C
INA_API(ina_rc_t) ina_opt_get_key_value(int index,
```

Get the string key and value of an option at index.


**Parameters**
 - `index`: options index starting by 0
 - `key`: long name of option
 - `value`: option value as string



**Return**

INA_SUCCESS if option is available otherwise INA_FAILURE


```C
INA_API(ina_rc_t) ina_opt_isset(const char *opt);/*
```

Check whenever an option is available.


**Parameters**
 - `opt`: name of option



**Return**

INA_SUCCESS if option is available


```C
INA_API(ina_rc_t) ina_opt_get_int(const char *opt, int *value);
```

Get the integer value of an option.


**Parameters**
 - `opt`: name of option
 - `value`: Where to store the value



**Return**

INA_SUCCESS if option is available


```C
INA_API(ina_rc_t) ina_opt_get_float(const char *opt, float *value);
```

Get the float value of an option.


**Parameters**
 - `opt`: name of option
 - `value`: Where to store the value



**Return**

INA_SUCCESS if option is available


```C
INA_API(ina_rc_t) ina_init(void);
```

Initialize all internal data structures. This must be the first function
called for any library.


**Return**

INA_SUCCESS  if no error occurred


```C
INA_API(ina_cleanup_handler_t) ina_set_cleanup_handler(
```

Set a custom termination routine to call in case of an
a termination signal. The purpose of such a routine is to give consumers
a last chance to cleanup before the program exits.


**Parameters**
 - `handler`: Cleanup routine. A cleanup should return EXIT_SUCCESS or
EXIT_FAILURE depending on type of signal. On a program error
the return of cleanup routines will be ignored.



**Return**

Previously defined handler


```C
INA_API(ina_signal_handler_t) ina_register_signal_handler(ina_signal_t sig,
```

Register a custom signal handler for sig.


**Parameters**
 - `sig`: Signal identifier
handler Signal handler



**Return**

Previously register handler


```C
INA_API(void) ina_exit(void);
```

Release and cleanup all internal data structures. This function is called
automatically once before the application terminate.

FIXME: make it private
