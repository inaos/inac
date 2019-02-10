
```C
#ifndef _LIBINAC_LJIT_H_
```

Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
typedef struct ina_ljit_ctx_s {
```
LuaJIT/Lua context
```C
#define INA_LJIT_TOCSTRING(ctx, index)                               \
```
Cast Lua raw cdata pointer to a const char
```C
#define INA_LJIT_TOPOINTER(ctx, index, type)                         \
```
Cast raw cdata pointer to a typed pointer
```C
#define INA_LJIT_TOINTEGER(ctx, index)                               \
```
Cast Lua number to an int
```C
#define INA_LJIT_TODOUBLE(ctx, index)                                \
```
Cast Lua number to double
```C
#define INA_LJIT_TOBOOLEAN(ctx, index)                               \
```
Cast Lua value to an int
```C
#define INA_LJIT_EXPORT(package, symbol)                               \
```

Expose C Symbol.

```C
#ifdef __cplusplus
```

Import a LuaJIT module.

```C
#define INA_LJIT_PACKAGE(package)                                      \
```

Import LuaJIT Bytecode. Works only for modules generated using
standard naming convention.

```C
INA_API(ina_rc_t) ina_ljit_ctx_new(ina_ljit_ctx_t **ctx);
```

Initialize LuaJIT context


**Parameters**
 - `ctx`: Pointer to LuaJIT state pointer



**Return**

INA_SUCCESS if no error occurred.


```C
INA_API(void) ina_ljit_ctx_free(ina_ljit_ctx_t **ctx);
```

Destroy LuaJIT context.


**Parameters**
 - `ctx`: Pointer to LuaJIT state pointer to free



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_ljit_call(ina_ljit_ctx_t *ctx,
```

Call a Lua function.

Parameter
ctx        LuaJIT state
fname      Function name. For methods use the object.method() syntax.
signature  Function signature. Each parameter is represente by a char
indicating the type.
s = string
d = double
i = int
c = cdata (const void pointer)
Return values are defined by the same way. After after the
< char.
Example of signature taking 2 strings, 1 double, 1 int and
returning an int:
"ssdi<i"
...        Function arguments.


**Return**

INA_SUCCESS if function called without any error.


```C
INA_API(ina_rc_t) ina_ljit_dostring(ina_ljit_ctx_t *ctx, const char* code);
```

Load lua code an execute it.


**Parameters**
 - `cxt`: LuaJIT state
 - `code`: Lua code to execute



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_ljit_dump_stack(ina_ljit_ctx_t *ctx);
```

Printout LUA stack to the stdout.


**Parameters**
 - `ctx`: LuaJIT context



**Return**

INA_SUCCESS


```C
INA_API(const void*) ina_ljit_checkcdata(ina_ljit_ctx_t *ctx, int narg);
```

Check C data type.


**Parameters**
 - `ctx`: LuaJIT context
 - `narg`: Stack index



**Return**

Return underling C pointer for CDATA type


```C
INA_API(unsigned long) ina_ljit_hash_sbdm(const char *str);
```

Calculate SBDM hash


**Parameters**
 - `str`: The string to be calculated



**Return**

Calculated hash

