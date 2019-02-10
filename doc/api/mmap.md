
```C
#ifndef _LIBINAC_MMAP_H_
```

Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
#ifdef __cplusplus
```


TODO:
-> CreateFileMappingNuma? Linux alternative.. or use huge-pages?


```C
typedef enum ina_mmap_mem_prot_e {
```
IO protection
```C
typedef enum ina_mmap_mem_share_e {
```
Share mode
```C
typedef enum ina_mmap_map_type_e {
```
MMAP mode
```C
typedef enum ina_mmap_mem_advice_e {
```
MMAP advice
```C
typedef struct ina_mmap_ctx_s ina_mmap_ctx_t;/* opaque mmap mapping */
```
opaque mmap context
```C
INA_API(ina_rc_t) ina_mmap_ctx_new(ina_mmap_ctx_t **ctx);
```

Creates and initialize a MMAP context.


**Parameters**
 - `ctx`: Where to store the newly created



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_mmap_ctx_free(ina_mmap_ctx_t **ctx);
```

Destroy a MMAP context.


**Parameters**
 - `ctx`: MMAP context to free



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_mmap_new(ina_mmap_ctx_t *ctx,
```

Creates a new mapping in the virtual address space of the calling process.


**Parameters**
 - `ctx`:  MMAP context
 - `fd`: 
 - `share`: Defines share mode
 - `type`: Defines mapping type
 - `offset`: Starting address for the new mapping
 - `length`: The length argument specifies the length of the mapping
 - `mapping`: Where to store the created mapping



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_mmap_free(ina_mmap_ctx_t *ctx, ina_mmap_mapping_t **mapping);
```

Destroy mapping


**Parameters**
 - `ctx`: MMAP context
mapping Mapping to free



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_mmap_sync(ina_mmap_mapping_t *mapping);
```

Flushes changes made to the in-core copy of a file that was mapped into
memory. Without use of this call there is no guarantee that changes are
written back before ina_mmap_free is called.


**Parameters**
 - `mapping`: Mapping to synch



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_mmap_memory_head(ina_mmap_mapping_t *mapping, void **memory);
```

Get head of mapping.


**Parameters**
 - `mapping`: MMAP mapping
 - `memory`: Where to store the start address of mapping



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_mmap_memory_tail(ina_mmap_mapping_t *mapping, void **memory);
```

Get tail of mapping.


**Parameters**
 - `mapping`: MMAP mapping
 - `memory`: WHere to store the end address of mapping



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_mmap_advice(ina_mmap_mapping_t *mapping,
```

Advises the kernel about how to handle paging input/output in the address
range beginning of mapping and with size length bytes.
In Windows 8 there will be PrefetchVirtualMemory for now this will be a
noop in Windows.


**Parameters**
 - `mapping`: MMAP mapping
 - `length`: size of paging handling in bytes
 - `advice`: Defines the advice



**Return**

INA_SUCCESS if all went well

