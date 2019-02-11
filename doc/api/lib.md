

---

```C
#define INA_VERSION_HEX  ((INA_MAJOR_VERSION << 16) |   \
                          (INA_MINOR_VERSION << 8)  |   \
                          (INA_PATCH_VERSION << 0))

```
Version as a 3-byte hex number, e.g. 0x010201 == 1.2.1. Use this
for numeric comparisons, e.g. #if INA_VERSION_HEX >= ...

---

```C
#define INA_REVISION_HEX ((INA_MINOR_VERSION << 8)  |   \
                          (INA_PATCH_VERSION << 0))

```
Revsion number as 2-byte hex number e.g 0x900 == 0.9. Use this
for numeric comparisons, e.g. #if INA_REVISION_HEX >= ...

---

```C
#define INA_RETURN_IF(x) do {if ((x)) return ina_err_get_rc(); } while(0)
/* Return with last rc if x == NULL */
#define INA_RETURN_IF_NULL(x) do {if ((x) == NULL) return ina_err_get_rc();} while(0)
/* Return with last rc if failed */
#define INA_RETURN_IF_FAILED(rc) do { if (INA_FAILED((rc))) return ina_err_get_rc(); } while (0)
/* Return with last rc if succeed */
#define INA_RETURN_IF_SUCCEED(rc) do {if (INA_SUCCEED((rc))) return ina_err_get_rc(); } while (0)

```
Return with last rc if condition x fails

---

```C
#define INA_AT __FILE__ ":" INA_NUM2STR(__LINE__)

```
Source location

---

```C
#define INA_OPT_FLAG(short_opt, long_opt, desc)           \
 { short_opt, long_opt, INA_OPT_TYPE_FLAG, NULL, desc }

```
Add flag option

---

```C
#define INA_OPT_STRING(short_opt, long_opt, dft, desc)    \
 { short_opt, long_opt, INA_OPT_TYPE_STRING, dft, desc }
 
```
Add string option

---

```C
#define INA_OPT_INT(short_opt, long_opt, dft, desc)       \
 { short_opt, long_opt, INA_OPT_TYPE_INT, INA_NUM2STR(dft), desc }

```
Add int option

---

```C
#define INA_OPT_FLOAT(short_opt, long_opt, dft, desc)       \
 { short_opt, long_opt, INA_OPT_TYPE_FLOAT, INA_NUM2STR(dft), desc }

```
Add float option

---

```C
#define INA_OPTS(name, ...)                        \
ina_opt_t name[] = {                               \
    __VA_ARGS__,                                   \
    {NULL, NULL, INA_OPT_TYPE_INT, NULL, NULL}     \
};

```
Define options map

---

```C
typedef struct ina_opt_s {
    const char *short_opt;  /* short option, nomally 1 char */
    const char *long_opt;   /* long option */
    ina_opt_type_t type;    /* option type */
    const char *dft;        /* default value */
    const char *desc;       /* short description, used in usage */
} ina_opt_t;

```
Command line option builder

---

```C
typedef enum ina_signal_behavior_e {
    INA_SIGNAL_BEHAVIOR_DFT,      /* Default behavior */
    INA_SIGNAL_BEHAVIOR_IGNORE    /* Ignore default behavior */
} ina_signal_behavior_t;

```
Signal handling behavior

---

```C
typedef void (*ina_cleanup_handler_t) (int, int*);
```
Application cleanup handler .

---

```C
typedef void (*ina_signal_handler_t) (ina_signal_t, ina_signal_behavior_t*, int*);
```
Signal handler

---

```C
INA_API(const char*) ina_app_get_name(void);
```

Return the program name


---

```C
INA_API(const char*) ina_app_get_path(void);
```

Return path to the running application


---

```C
INA_API(ina_rc_t) ina_app_init(int argc,
                               char **argv,
                               ina_opt_t *opt);

```

Startup application with argc, argv in order to deal with
platform-specific quirks. This must be the first function called for any
program.


**Parameters**
 - `argc`: argc of main() function
 - `argv`: Pointer to the argv of main() function
 - `opt`: Array of options to parse



**Return**

INA_SUCCESS  if no error occurred



---

```C
INA_API(ina_rc_t) ina_opt_get_key_value(int index,
                                        ina_str_t *key,
                                        ina_str_t *value);

```

Get the string key and value of an option at index.


**Parameters**
 - `index`: options index starting by 0
 - `key`: long name of option
 - `value`: option value as string



**Return**

INA_SUCCESS if option is available otherwise INA_FAILURE



---

```C
INA_API(ina_rc_t) ina_opt_isset(const char *opt);
```

Check whenever an option is available.


**Parameters**
 - `opt`: name of option



**Return**

INA_SUCCESS if option is available



---

```C
INA_API(ina_rc_t) ina_opt_get_string(const char *opt, ina_str_t *value);
```

Get the string value of an option.


**Parameters**
 - `opt`: name of option
 - `value`: Where to store the value



**Return**

INA_SUCCESS if option is available



---

```C
INA_API(ina_rc_t) ina_opt_get_int(const char *opt, int *value);
```

Get the integer value of an option.


**Parameters**
 - `opt`: name of option
 - `value`: Where to store the value



**Return**

INA_SUCCESS if option is available



---

```C
INA_API(ina_rc_t) ina_opt_get_float(const char *opt, float *value);
```

Get the float value of an option.


**Parameters**
 - `opt`: name of option
 - `value`: Where to store the value



**Return**

INA_SUCCESS if option is available



---

```C
INA_API(ina_rc_t) ina_init(void);
```

Initialize all internal data structures. This must be the first function
called for any library.


**Return**

INA_SUCCESS  if no error occurred



---

```C
INA_API(ina_cleanup_handler_t) ina_set_cleanup_handler(
                                        ina_cleanup_handler_t handler);

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



---

```C
INA_API(ina_signal_handler_t) ina_register_signal_handler(ina_signal_t sig,
                                                ina_signal_handler_t handler);

```

Register a custom signal handler for sig.


**Parameters**
 - `sig`: Signal identifier
handler Signal handler



**Return**

Previously register handler



---

```C
INA_API(void) ina_exit(void);
```

Release and cleanup all internal data structures. This function is called
automatically once before the application terminate.

FIXME: make it private
