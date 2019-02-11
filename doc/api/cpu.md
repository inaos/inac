

---

```C
#ifndef _LIBINAC_CPU_H_
#define _LIBINAC_CPU_H_

```

Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.


---

```C
#define INA_CPU_SUPPORTED_VENDOR "GenuineIntel"

```
Supported Vendors

---

```C
#define INA_CPU_FEATURE_FPU        (1LL <<  0) /* Onboard x87 FPU */
#define INA_CPU_FEATURE_VME        (1LL <<  1) /* Virtual 8086 mode extensions (such as VIF, VIP, PIV) */
#define INA_CPU_FEATURE_DE         (1LL <<  2) /* Debugging extensions (CR4 bit 3) */
#define INA_CPU_FEATURE_PSE        (1LL <<  3) /* Page Size Extension */
#define INA_CPU_FEATURE_TSC        (1LL <<  4) /* Time Stamp Counter */
#define INA_CPU_FEATURE_MSR        (1LL <<  5) /* Model-specific registers */
#define INA_CPU_FEATURE_PAE        (1LL <<  6) /* Physical Address Extension */
#define INA_CPU_FEATURE_MCE        (1LL <<  7) /* Machine Check Exception */
#define INA_CPU_FEATURE_CX8        (1LL <<  8) /* CMPXCHG8 (compare-and-swap) instruction */
#define INA_CPU_FEATURE_APIC       (1LL <<  9) /* Onboard Advanced Programmable Interrupt Controller */
#define INA_CPU_FEATURE_SEP        (1LL << 10) /* SYSENTER and SYSEXIT instructions */
#define INA_CPU_FEATURE_MTRR       (1LL << 11) /* Memory Type Range Registers */
#define INA_CPU_FEATURE_PGE        (1LL << 12) /* Page Global Enable bit in CR4 */
#define INA_CPU_FEATURE_MCA        (1LL << 13) /* Machine check architecture */
#define INA_CPU_FEATURE_CMOV       (1LL << 14) /* Conditional move and FCMOV instructions */
#define INA_CPU_FETUARE_PAT        (1LL << 15) /* Page Attribute Table */
#define INA_CPU_FEATURE_PSE36      (1LL << 16) /* 36-bit page size extension */
#define INA_CPU_FEATURE_PSN        (1LL << 17) /* Processor Serial Number */
#define INA_CPU_FEATURE_CLFSH      (1LL << 18) /* CLFLUSH instruction (SSE2) */
#define INA_CPU_FEATURE_DS         (1LL << 19) /* Debug store: save trace of executed jumps */
#define INA_CPU_FEATURE_ACPI       (1LL << 20) /* Onboard thermal control MSRs for ACPI */
#define INA_CPU_FEATURE_MMX        (1LL << 21) /* MMX instructions */
#define INA_CPU_FEATURE_FXSR       (1LL << 22) /* FXSAVE, FXRESTOR instructions, CR4 bit 9 */
#define INA_CPU_FEATURE_SSE        (1LL << 23) /* SSE instructions (a.k.a. Katmai New Instructions) */
#define INA_CPU_FEATURE_SSE2       (1LL << 24) /* SSE2 instructions */
#define INA_CPU_FEATURE_SS         (1LL << 25) /* CPU cache supports self-snoop */
#define INA_CPU_FEATURE_HTT        (1LL << 26) /* Hyper-threading */
#define INA_CPU_FEATURE_TM         (1LL << 27) /* Thermal monitor automatically limits temperature */
#define INA_CPU_FEATURE_IA64       (1LL << 28) /* IA64 processor emulating x86 */
#define INA_CPU_FEATURE_PBE        (1LL << 29) /* Pending Break Enable (PBE# pin) wakeup support */
#define INA_CPU_FEATURE_SSE3       (1LL << 30) /* Prescott New Instructions-SSE3 (PNI) */
#define INA_CPU_FEATURE_PCLMULQDQ  (1LL << 31) /* PCLMULQDQ support */
#define INA_CPU_FEATURE_DTES64     (1LL << 32) /* 64-bit debug store (edx bit 21) */
#define INA_CPU_FEATURE_MONITOR    (1LL << 33) /* MONITOR and MWAIT instructions (SSE3) */
#define INA_CPU_FEATURE_DSCPL      (1LL << 34) /* CPL qualified debug store */
#define INA_CPU_FEATURE_VMX        (1LL << 35) /* Virtual Machine eXtensions */
#define INA_CPU_FEATURE_SMX        (1LL << 36) /* Safer Mode Extensions (LaGrande) */
#define INA_CPU_FEATURE_EST        (1LL << 37) /* Enhanced SpeedStep */
#define INA_CPU_FEATURE_TM2        (1LL << 38) /* Thermal Monitor 2 */
#define INA_CPU_FEATURE_SSSE3      (1LL << 39) /* Supplemental SSE3 instructions */
#define INA_CPU_FEATURE_CNXTID     (1LL << 40) /* L1 Context ID */
#define INA_CPU_FEATURE_FMA        (1LL << 41) /* Fused multiply-add (FMA3) */
#define INA_CPU_FEATURE_CX16       (1LL << 42) /* CMPXCHG16B instruction */
#define INA_CPU_FEATURE_XTPR       (1LL << 43) /* Can disable sending task priority messages */
#define INA_CPU_FEATURE_PDCM       (1LL << 44) /* Perfmon & debug capability */
#define INA_CPU_FEATURE_PCID       (1LL << 45) /* Process context identifiers (CR4 bit 17) */
#define INA_CPU_FEATURE_DCA        (1LL << 46) /* Direct cache access for DMA writes */
#define INA_CPU_FEATURE_SSE41      (1LL << 47) /* SSE4.1 instructions */
#define INA_CPU_FEATURE_SSE42      (1LL << 48) /* SSE4.2 instructions */
#define INA_CPU_FEATURE_X2APIC     (1LL << 49) /* x2APIC support */
#define INA_CPU_FEATURE_MOVBE      (1LL << 50) /* MOVBE instruction (big-endian) */
#define INA_CPU_FEATURE_POPCNT     (1LL << 51) /* POPCNT instruction */
#define INA_CPU_FEATURE_TSCDL      (1LL << 52) /* APIC supports one-shot operation using a TSC deadline value */
#define INA_CPU_FEATURE_AES        (1LL << 53) /* AES instruction set */
#define INA_CPU_FEATURE_XSAVE      (1LL << 54) /* XSAVE, XRESTOR, XSETBV, XGETBV */
#define INA_CPU_FEATURE_OSXSAVE    (1LL << 55) /* XSAVE enabled by OS */
#define INA_CPU_FEATURE_AVX        (1LL << 56) /* Advanced Vector Extensions */
#define INA_CPU_FEATURE_F16C       (1LL << 57) /* F16C (half-precision) FP support */
#define INA_CPU_FEATURE_RDRND      (1LL << 58) /* RDRAND (on-chip random number generator) support */
#define INA_CPU_FEATURE_HYPERVISOR (1LL << 59) /* Running on a hypervisor (always 0 on a real CPU, but also with some hypervisors) */

```

Intel feature flags:
- http://en.wikipedia.org/wiki/CPUID


---

```C
typedef struct ina_cpu_ctx_s ina_cpu_ctx_t;
```
opaque context - private

---

```C
INA_API(ina_rc_t) ina_cpu_init(void);
```

PRIVATE: One should never call this function - its only called internally
during initialization.


**Return**

INA_SUCCESS if all went well



---

```C
INA_API(void) ina_cpu_destroy(void);
```

PRIVATE: One should never call this function - its only called internally
during shutdown.


**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cpu_get_package_count(int *package_count);
```

Get package count.


**Parameters**
 - `package_count`: Where to store package count



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cpu_get_core_count(int *core_count);
```

Get core count.


**Parameters**
 - `core_count`: Where to store core count



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cpu_get_thread_count(int *thread_count);
```

Get thread count.


**Parameters**
 - `thread_count`: Where to store thread count



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cpu_get_total_logical_count(int *logical_count);
```

Get count of logical processors in the system.


**Parameters**
 - `logical_count`: Where to store number of logical processors or 0 if number
can not be calculated.



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cpu_get_features(ina_cpu_feature_t *features);
```

Get cpu features.


**Parameters**
 - `features`: Where to store cpu features



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cpu_get_brand_string(ina_str_t *brand);
```

Get cpu brand string.


**Parameters**
 - `brand`: Where to store the brand string



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cpu_is_supported(int *supported);
```

Query if module is suppoerted.


**Parameters**
 - `supported`: Where to store the result



**Return**

INA_SUCCESS


TODO: Use function return to indicate if supported or not


---

```C
INA_API(ina_rc_t) ina_cpu_pin_to_core(int core);
```

Schedule current thread to a specific core.


**Parameters**
 - `core`: Core number



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_cpu_process_promote();
```

Promotes the the process to real-time priority.

It is designed to work with single-threaded processes


**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_cpu_process_query_core(int *core);
```

Get the current executing core of the process.

It is designed to work with single-threaded processes


**Parameters**
 - `core`: Core number



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_cpu_hyperthreading_enabled(int *enabled);
```

Is hyperthrading enabled


**Parameters**
 - `enabled`: Here we store the boolean value



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_cpu_get_signature(uint8_t *family,
                                        uint8_t *model,
                                        uint8_t *stepping);
```

Retrieve cpu signature , such aa family, model and stepping


**Parameters**
 - `family`: Where to store cpu family id
 - `model`: Where to sture cpu model id
 - `stepping`: Where to store cpu stepping



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cpu_get_frequency_os(int *mHz);
```

Get CPU frequency from operating system


**Parameters**
 - `mHz`: Where to store the frequency in MHz



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cpu_get_ipc_dp(int *ipc);
```

Retrieve instructions per cycle for double-precision


**Parameters**
 - `ipc`: Where to store the number of instructions value



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cpu_get_ipc_sp(int *ipc);
```

Retrieve instructions per cycle for single-precision


**Parameters**
 - `ipc`: Where to store the number of instructions value



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cpu_get_gflops_dp(double *gflops);
```

CPU performance in GFlops =
(CPU speed in GHz) x (number of CPU cores) x (CPU instruction per cycle)

Double precision


**Parameters**
 - `gflops`: Where to store the number of GFlops




---

```C
INA_API(ina_rc_t) ina_cpu_get_gflops_sp(double *gflops);
```

CPU performance in GFlops =
(CPU speed in GHz) x (number of CPU cores) x (CPU instruction per cycle)

Single precision


**Parameters**
 - `gflops`: Where to store the number of GFlops




---

```C
INA_API(ina_rc_t) ina_cpu_get_l1_cache_size(size_t *bytes);
```

Retrieve CPU cache size for its L1 cache


**Parameters**
 - `bytes`: Where to store the number of bytes



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cpu_get_l2_cache_size(size_t *bytes);
```

Retrieve CPU cache size for its L2 cache


**Parameters**
 - `bytes`: Where to store the number of bytes



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cpu_get_l3_cache_size(size_t *bytes);
```

Retrieve CPU cache size for its L3 cache


**Parameters**
 - `bytes`: Where to store the number of bytes



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cpu_get_cache_line_size(size_t *bytes);
```

Retrieve CPU cache line size


**Parameters**
 - `bytes`: Where to store the number of bytes



**Return**

INA_SUCCESS

