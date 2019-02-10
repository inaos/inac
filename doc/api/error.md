
```C
#ifndef _LIBINAC_ERROR_H_
```

Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
#define INA_SUCCESS  (0ULL)
```
Indicate no errors
```C
extern INA_TLS(ina_rc_t) __rc;
```
Global return code
```C
#define INA_RC_BIT_E 63U
```
Bit-shifts
```C
#define INA_RC_EFLAG(rc)   ((uint32_t)(((rc) >> INA_RC_BIT_E) & 0x1))
```
Accessors
```C
#define INA_ERR_ERROR               (  1ULL << INA_RC_BIT_E) /* Error-bit  */
```
Flags
```C
#define INA_ERR_A                   (  1ULL << INA_RC_BIT_C)
```
Errors codes
```C
#define INA_ERR_NOT_A               (INA_ERR_NOT | INA_ERR_A)
```
Error codes (negate forms)
```C
#define INA_ERR_UNDEFINED           (INA_ERR_NOT_DEFINED)
```
Error codes aliases
```C
#define INA_ES_NONE                 (0U)
```
Error subjects
```C
#define INA_ES_USER_DEFINED         (1024UL)
```
Start of user defined error subjects
```C
#define INA_ERR_INVALID_ARGUMENT  (INA_ERR_INVALID|INA_ES_ARGUMENT)
```

Global error messages

```C
typedef const char* (*ina_err_subject_cb_t)(int);
```

Subject dictionary callback

```C
INA_API(ina_rc_t) ina_err_init(void);
```

Initialize the error module. This function should never be called,
because is part of the INAC initialization and  called in ina_init().


**Return**

INA_SUCCESS if all went well.


Error messages
INA_ERR_OUT_OF_MEMORY

```C
INA_API(void) ina_err_destroy(void);
```

Destroy error module. This function is called at program exit and
should never called directly.

```C
INA_API(ina_err_subject_cb_t) ina_err_register_dict(ina_err_subject_cb_t cb);
```

Register an error subject dictionary callback.


**Parameters**
 - `cb`: Dictionary callback



**Return**

Previously registered dictionary callback or NULL


```C
INA_INLINE ina_rc_t ina_err_set_rc(ina_rc_t rc)
```

Set global return code. Use INA_ERROR() or INA_OS_ERROR() as shortcut or use
INA_RC_PACK to generate an valid RC.


**Parameters**
 - `rc`: New return code to set



**Return**

Current RC


```C
INA_INLINE ina_rc_t ina_err_get_rc(void)
```

Return current global return code.


**Return**

Current RC


```C
INA_INLINE ina_rc_t ina_err_clear_rc(ina_rc_t rc)
```

Mark a RC as handled.


**Parameters**
 - `rc`: Valid RC to mark as handled. If a error was already marked as handled
no error occurs.



**Return**

Ceared RC


```C
INA_INLINE ina_rc_t ina_err_reset(void)
```

Reset the global RC by marking it as handled.


**Return**

Current global RC


```C
INA_INLINE ina_rc_t ina_err_set_ubits(uint8_t ubits)
```

Set user bit on the global RC.


**Parameters**
 - `ubits`: User defined bits



**Return**

Current global RC


```C
INA_INLINE uint8_t ina_err_get_ubits(ina_rc_t rc)
```

Return user defined bit of an given RC.


**Parameters**
 - `rc`: Valid RC where to extract user defined bits



**Return**

User defined bits


```C
INA_API(const char*) ina_err_strerror(ina_rc_t rc);
```

Format the error message for a given RC.


**Parameters**
 - `rc`: Valid RC



**Return**

Error message


```C
#ifdef INA_LIB
```
Pack a RC
```C
#define INA_FAILED(rc) ((rc)&(INA_ERR_ERROR))
```
Check return code: failure
```C
#define INA_SUCCEED(rc) (!INA_FAILED((rc)))
```
Check return code: successful or handled
```C
#define INA_MUST_SUCCEED(rc) do { if (INA_UNLIKELY(INA_FAILED((rc)))) abort(); } while(0)
```
Checkpoint must succeed
```C
#define INA_ERROR(x) ina_err_set_rc(INA_RC_PACK((x), 0))
```
Set global RC
```C
#ifndef INA_OS_WIN32
```
Set global RC and capture errno