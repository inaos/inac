

---

```C
#ifndef _LIBINAC_IPC_H_
#define _LIBINAC_IPC_H_

```

Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.


---

```C
typedef struct ina_ipc_flags_data_s ina_ipc_flags_data_t;
```

Opaque types for IPC flag


---

```C
typedef struct ina_ipc_counter_data_s ina_ipc_counter_data_t;
```

Opaque types for IPC counter


---

```C
INA_API(ina_rc_t) ina_ipc_flags_new(const char* name,
                                    int64_t initial,
                                    ina_ipc_flags_t **flags);
```

Create a new IPC flags.


**Parameters**
 - `name`: Name of flags. Must be a unique name at host level.
 - `initial`: Defines initial flags value
 - `flags`: Where to store the newly created IPC flags



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_ipc_flags_open(const char* name, ina_ipc_flags_t **flags);
```

Open an new IPC flags. A IPC flag must be created using ina_ipc_flags_new()
before i can be opened.


**Parameters**
 - `name`: Name of flags to open.
 - `flag`: Where to store the IPC flags



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(void) ina_ipc_flags_free(ina_ipc_flags_t **flags);
```

Destroy IPC flags.


**Parameters**
 - `flag`: IPC flags to free



---

```C
INA_API(ina_rc_t) ina_ipc_flags_get_name(const ina_ipc_flags_t *flags,
                                         const char **name);
```

Get the name of a IPC flag.


**Parameters**
 - `flags`: IPC flags
 - `name`: Where the store the name



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_ipc_flags_get(const ina_ipc_flags_t *flags,
                                    uint64_t *value);
```

Get the current value of IPC flags.


**Parameters**
 - `flags`: IPC flags
 - `value`: Where to store the current value



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_ipc_flags_set(ina_ipc_flags_t *flags, uint64_t value);
```

Turn on one or more IPC flags. For each turned on flag a reference counter
is incremented.


**Parameters**
 - `flags`: IPC flags
 - `value`: Bit mask



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_ipc_flags_is_set(const ina_ipc_flags_t *flags,
                                       uint64_t value);
```

Query if one or more flags are set (on).


**Parameters**
 - `flags`: IPC flags
 - `value`: Bit mask



**Return**

INA_SUCCESS if all flags defined by the bit mask are set (on)
INA_FAILURE if one or more flags defined by the bit mask are not set (off)



---

```C
INA_API(ina_rc_t) ina_ipc_flags_unset(ina_ipc_flags_t *flags, uint64_t value);
```

Unset one or more IPC Flags. For each turned off flag a reference counter
is decremented.


**Parameters**
 - `flags`: IPC flags
 - `value`: Bit mask to turn off one or more flags



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_ipc_flags_clear(ina_ipc_flags_t *flags, uint64_t value);
```

Clear one or more IPC Flags. For each cleared flag his reference counter
is set to 0.


**Parameters**
 - `flags`: IPC flags
 - `value`: Bit mask to clear one or more flags



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_ipc_flags_wait(const ina_ipc_flags_t *flags,
                                     uint64_t wait_for,
                                     time_t msec_timeout);
```

Wait until flags are set.


**Parameters**
 - `flags`: IPC flags
 - `wait_for`: Bit mask defining IPC flags waiting for.
 - `msec_timeout`: Number of milliseconds before timeout occurs.



**Return**

INA_SUCCESS if all went well.
INA_FAILURE if timeout occurred


FIXME: Return specific error when timeout occurs


---

```C
INA_API(ina_rc_t) ina_ipc_flags_dump(const ina_ipc_flags_t *flags);
```

Dumps IPC flags to the standard output


**Parameters**
 - `flags`: IPC flags



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_ipc_counter_new(const char* name,
                                      uint64_t initial,
                                      ina_ipc_counter_t **counter);
```

Create an new IPC counter.


**Parameters**
 - `name`: Counter name. Must be unique on host level
 - `initial`: Initial counter value
 - `counter`: Where to store the newly created counter



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_ipc_counter_open(const char* name,
                                       ina_ipc_counter_t **counter);
```

Open an IPC counter. A counter must be created by calling ina_ipc_counter_new()
before it can be opened.


**Parameters**
 - `name`: Counter name
 - `counter`: Where to store the opened counter



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(void) ina_ipc_counter_free(ina_ipc_counter_t **counter);
```

Destroy an IPC counter.


**Parameters**
 - `counter`: IPC counter to free.



---

```C
INA_API(ina_rc_t) ina_ipc_counter_get(const ina_ipc_counter_t *counter,
                                      uint64_t *value);
```

Get current value of an IPC counter.


**Parameters**
 - `counter`: IPC counter
 - `value`: Where to store the current counter value



**Return**

INA_SUCCESS



---

```C
INA_API(uint64_t) ina_ipc_counter_increment(ina_ipc_counter_t *counter,
                                            uint64_t value);
```

Increment an IPC counter by a value (e.g. 1)


**Parameters**
 - `counter`: IPC counter
 - `value`: Value to add



**Return**

INA_SUCCESS



---

```C
INA_API(uint64_t) ina_ipc_counter_decrement(ina_ipc_counter_t *counter,
                                            uint64_t value);
```

Decrement an IPC counter by a value (e.g. 1)


**Parameters**
 - `counter`: IPC counter
 - `value`: Value to subtract



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_ipc_counter_set(ina_ipc_counter_t *counter,
                                      uint64_t value);
```

Set an IPC counter to a specific value.


**Parameters**
 - `counter`: IPC counter
 - `value`: New counter value



**Return**

INA_SUCCESS

