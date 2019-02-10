
```C
#ifndef _LIBINAC_DIR_H_
```

Copyright INAOS GmbH, Thalwil, 2016-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
typedef struct ina_dir_walker_s ina_dir_walker_t;
```
Opaque directory walker handle
```C
typedef struct ina_dir_stat_s ina_dir_stat_t;
```
Opaque directory stat handle
```C
typedef enum ina_dir_sort_order_e {
```
Directory sort order
```C
typedef enum ina_dir_sort_attrib_e {
```
Directory sort attrib
```C
typedef enum ina_dir_entry_type_e {
```
Directory entry typ
```C
typedef struct ina_dir_entry_s {
```
Directory entry
```C
INA_API(ina_rc_t) ina_dir_walker_new(const char *basedir, ina_dir_walker_t **walker);
```

Create a new directory walker.


**Parameters**
 - `basedir`: Base directory for newly created walker
 - `walker`: Where to store the newly created walker



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_dir_walker_enable_recursive(ina_dir_walker_t *walker);
```

Enable recursive directory walking.


**Parameters**
 - `walker`: Directory walker



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_dir_walker_disable_recursive(ina_dir_walker_t *walker);
```

Disable recursive directory walking.


**Parameters**
 - `walker`: Directory walker



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_dir_walker_get_sort_order(const ina_dir_walker_t *walker,
```

Get current sort order for walker.


**Parameters**
 - `walker`:  Directory walker
 - `sort_order`: Where to store current sort order



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_dir_walker_set_sort_order(ina_dir_walker_t *walker,
```

Set directory sort order for walker.


**Parameters**
 - `walker`:  Directory walker
 - `sort_order`: Sort order to set



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_dir_walker_get_sort_attrib(const ina_dir_walker_t *walker,
```

Get current directory sort attribute for walker.


**Parameters**
 - `walker`:  Directory walker
sort_attrib Where to store the current sort attribute



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_dir_walker_set_sort_attrib(ina_dir_walker_t *walker,
```

Set directory sort attribute for walker.


**Parameters**
 - `walker`: 
 - `sort_attrib`: Sort attribute to set



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_dir_walker_get_next_entry(ina_dir_walker_t *walker,
```

Get next directory entry of walker.


**Parameters**
 - `walker`: Directory walker
 - `entry`: Where to store the directory entry. Is NULL at end of list.



**Return**

INA_SUCCESS if all went well, INA_FAILURE on end of list.


```C
INA_API(ina_rc_t) ina_dir_walker_reset(ina_dir_walker_t *walker);
```

Reset walker. Does not reflect FS changes, sort order and sort attribute


**Parameters**
 - `walker`: Directory walker to reset.



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_dir_walker_reload(ina_dir_walker_t *walker);
```

Reload walker. Reflect FS changes, sort order and sort attrib.


**Parameters**
 - `walker`: Directory walker to reload


```C
INA_API(void) ina_dir_walker_free(ina_dir_walker_t **walker);
```

Free directory walker.

Parameter
walker  Directory walker to free

```C
INA_API(ina_rc_t) ina_dir_stat_new(const char *dir, ina_dir_stat_t **stat);
```

Create and initialize directory attributes for a give directory.


**Parameters**
 - `stat`: Where to store the directory attributes
 - `dir`: Directory


```C
INA_API(ina_rc_t) ina_dir_stat_bytes_capacity(const ina_dir_stat_t *stat,
```

Get total capacity in bytes for a directory.


**Parameters**
 - `stats`: 
 - `capacity_bytes`: Where to store directory capacity in bytes



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_dir_stat_bytes_free(const ina_dir_stat_t *stat,
```

Get free capacity in bytes for a directory.


**Parameters**
 - `stat`: 
 - `free_bytes`: Where too store the free capacity in bytes



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_dir_stat_pct_used(const ina_dir_stat_t *stat, int *pct_used);
```

Calculate used capacity of a directory in percent.


**Parameters**
 - `stats`: Directory attributes
 - `pct_used`: Where to store used capacity in percent



**Return**

INA_SUCCESS


```C
INA_API(void) ina_dir_stat_free(ina_dir_stat_t **stat);
```

Destroy directory attributes.


**Parameters**
 - `stats`: Directory attributes to free

