
```C
#ifndef _LIBINAC_CPU_H_
```

Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
#define INA_CPU_SUPPORTED_VENDOR "GenuineIntel"
```
Supported Vendors
```C
#define INA_CPU_FEATURE_FPU        (1LL <<  0) /* Onboard x87 FPU */
```

Intel feature flags:
- http://en.wikipedia.org/wiki/CPUID

```C
typedef struct ina_cpu_ctx_s ina_cpu_ctx_t;
```
opaque context - private
```C
INA_API(ina_rc_t) ina_cpu_init(void);
```

PRIVATE: One should never call this function - its only called internally
during initialization.


**Return**

INA_SUCCESS if all went well


```C
INA_API(void) ina_cpu_destroy(void);
```

PRIVATE: One should never call this function - its only called internally
during shutdown.


**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_cpu_get_package_count(int *package_count);
```

Get package count.


**Parameters**
 - `package_count`: Where to store package count



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_cpu_get_core_count(int *core_count);
```

Get core count.


**Parameters**
 - `core_count`: Where to store core count



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_cpu_get_thread_count(int *thread_count);
```

Get thread count.


**Parameters**
 - `thread_count`: Where to store thread count



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_cpu_get_total_logical_count(int *logical_count);
```

Get count of logical processors in the system.


**Parameters**
 - `logical_count`: Where to store number of logical processors or 0 if number
can not be calculated.



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_cpu_get_features(ina_cpu_feature_t *features);
```

Get cpu features.


**Parameters**
 - `features`: Where to store cpu features



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_cpu_get_brand_string(ina_str_t *brand);
```

Get cpu brand string.


**Parameters**
 - `brand`: Where to store the brand string



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_cpu_is_supported(int *supported);
```

Query if module is suppoerted.


**Parameters**
 - `supported`: Where to store the result



**Return**

INA_SUCCESS


TODO: Use function return to indicate if supported or not

```C
INA_API(ina_rc_t) ina_cpu_pin_to_core(int core);
```

Schedule current thread to a specific core.


**Parameters**
 - `core`: Core number



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_cpu_process_promote();
```

Promotes the the process to real-time priority.

It is designed to work with single-threaded processes


**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_cpu_process_query_core(int *core);
```

Get the current executing core of the process.

It is designed to work with single-threaded processes


**Parameters**
 - `core`: Core number



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_cpu_hyperthreading_enabled(int *enabled);
```

Is hyperthrading enabled


**Parameters**
 - `enabled`: Here we store the boolean value



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_cpu_get_signature(uint8_t *family,
```

Retrieve cpu signature , such aa family, model and stepping


**Parameters**
 - `family`: Where to store cpu family id
 - `model`: Where to sture cpu model id
 - `stepping`: Where to store cpu stepping



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_cpu_get_frequency_os(int *mHz);
```

Get CPU frequency from operating system


**Parameters**
 - `mHz`: Where to store the frequency in MHz



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_cpu_get_ipc_dp(int *ipc);
```

Retrieve instructions per cycle for double-precision


**Parameters**
 - `ipc`: Where to store the number of instructions value



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_cpu_get_ipc_sp(int *ipc);
```

Retrieve instructions per cycle for single-precision


**Parameters**
 - `ipc`: Where to store the number of instructions value



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_cpu_get_gflops_dp(double *gflops);
```

CPU performance in GFlops =
(CPU speed in GHz) x (number of CPU cores) x (CPU instruction per cycle)

Double precision


**Parameters**
 - `gflops`: Where to store the number of GFlops



```C
INA_API(ina_rc_t) ina_cpu_get_gflops_sp(double *gflops);
```

CPU performance in GFlops =
(CPU speed in GHz) x (number of CPU cores) x (CPU instruction per cycle)

Single precision


**Parameters**
 - `gflops`: Where to store the number of GFlops



```C
INA_API(ina_rc_t) ina_cpu_get_l1_cache_size(size_t *bytes);
```

Retrieve CPU cache size for its L1 cache


**Parameters**
 - `bytes`: Where to store the number of bytes



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_cpu_get_l2_cache_size(size_t *bytes);
```

Retrieve CPU cache size for its L2 cache


**Parameters**
 - `bytes`: Where to store the number of bytes



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_cpu_get_l3_cache_size(size_t *bytes);
```

Retrieve CPU cache size for its L3 cache


**Parameters**
 - `bytes`: Where to store the number of bytes



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_cpu_get_cache_line_size(size_t *bytes);
```

Retrieve CPU cache line size


**Parameters**
 - `bytes`: Where to store the number of bytes



**Return**

INA_SUCCESS

