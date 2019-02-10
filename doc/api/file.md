
```C
#ifndef _LIBINAC_FILE_H_
```

Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
#ifdef __cplusplus
```

DESIGN considerations
---------------------

- When it comes to reading from files or writing to files
we use a cursor to abstract how we're going to do it behind the scenes.

- Most important thing to know is whether you want to access the data
sequentially or randomly.

- Also it depends how long you need the data and whether one can benefit
from zero-copy or not.

- Depending on the use-case one might choose to use mmap or read/write
the cursor lets you choose what implementation to use.

- Its curcial to benchmark what is faster/more efficient for you use-case.

- General sentiment would be that mmap is mostly more efficient that read/write

Links:
-> http://marc.info/?l=linux-kernel&m=95496636207616&w=2

TODO:
-> Vectored I/O .. ReadFileScatter ... WriteFileGather


```C
typedef enum ina_file_access_mode_e {
```
File access mode
```C
typedef enum ina_file_create_mode_e {
```
File open mode
```C
typedef enum ina_file_share_mode_e {
```
File share mode
```C
typedef enum ina_file_seek_mode_e {
```
File seek mode
```C
typedef struct ina_file_ctx_s ina_file_ctx_t;/* Opaque file handle */
```
Opaque file context
```C
typedef struct ina_file_stat_s ina_file_stat_t;
```
Opaque file attributes handle
```C
INA_API(ina_rc_t) ina_file_ctx_new(ina_file_ctx_t **ctx, mode_t default_mode);
```

Create and initialize a new file context.


**Parameters**
 - `ctx`: 
 - `default_mode`: Default permissions



**Return**

INA_SUCCESS if all went well


```C
INA_API(void) ina_file_ctx_free(ina_file_ctx_t **ctx);
```

Free a file context.


**Parameters**
 - `ctx`: Context to free



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_file_new(ina_file_ctx_t *ctx,
```

Create a new file handle.


**Parameters**
 - `ctx`: 
 - `file_fqn`: Fully qualified name of file
 - `access`: Access mode
 - `create`: Creation mode
 - `share`: Share mode
 - `flags`: Combination on INA_FILE_FLAG_
 - `file`:  Where to store the newly created file handle



**Return**

INA_SUCCESS if all went well


```C
INA_API(void) ina_file_free(ina_file_t **file);
```

Free a file handle


**Parameters**
 - `ctx`: File context
 - `file`: File to free



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_file_stat_new(const ina_file_t *file, ina_file_stat_t **stat);
```

Create and initialize file attributes.


**Parameters**
 - `file`: File
 - `stat`: Where to store the file attributes



**Return**

INA_SUCCESS if all went well


```C
INA_API(void) ina_file_stat_free(ina_file_stat_t **stat);
```

Destroy file attributes.


**Parameters**
 - `stat`: File attributes to free



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_file_stat_synch(ina_file_stat_t *stat, const ina_file_t *file);
```

Synchronize file attributes.


**Parameters**
 - `file`: File
 - `stat`: Where to store the file attributes



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_file_get_filepath(const ina_file_t *file,
```

Get the filepath of a file


**Parameters**
 - `file`:  File handle
 - `filepath`: Where to store the filepath



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_file_get_mode(const ina_file_t *file, mode_t *mode);
```

Get current file permissions.


**Parameters**
 - `file`: File handle
 - `mode`: WHere to store current file permissions



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_file_set_mode(const ina_file_t *file, mode_t mode);
```

Set file permissions


**Parameters**
 - `file`: File handle
 - `mode`: File permissions



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_file_stat_is_dir(ina_file_stat_t *stat);
```

Query if file is a directory.


**Parameters**
 - `file`: File handle
 - `dir`: Where to store the result. 1 for a regular directory otherwise 0.



**Return**

INA_SUCCESS if is a directory


```C
INA_API(ina_rc_t) ina_file_stat_file_size(ina_file_stat_t *stat,
```

Get current file size in bytes.


**Parameters**
 - `stat`: 
 - `file_size`: Where to store the file size in bytes



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_file_stat_atime(ina_file_stat_t *stat,
```

Get last access time of a file.


**Parameters**
 - `stat`: 
 - `last_access`: Where to store te last access time.



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_file_stat_mtime(ina_file_stat_t *stat,
```

Get last modification timestamp of a file.


**Parameters**
 - `stat`: File attributes handle
 - `last_modification`: Where to store time of last modification



**Return**

INA_SUCCESS


```C
INA_API(ina_handle_t) ina_file_os_handle(ina_file_t *file);
```

Return the underlying native os handle of a INAC file handle


**Parameters**
 - `file`: INAC file handle



**Return**

Void pointer to the Native file handle


```C
INA_API(FILE*) ina_file_get_stream(ina_file_t *file);
```

Return the underlying C stream of a INAC file handle


**Parameters**
 - `file`: INAC file handle



**Return**

On successful completion return a FILE pointer. Otherwise, NULL is returned.


```C
INA_API(ina_rc_t) ina_file_read(ina_file_t *file,
```

Read 'len' bytes from a file into buf.


**Parameters**
 - `file`: File handle
 - `buf`: Read buffer, must be bigger than 'len' bytes
 - `len`: Number of bytes intended to read from file
 - `read`: Where to store the number of bytes read



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_file_write(ina_file_t *file,
```

Write 'len' bytes to a file.


**Parameters**
 - `file`: File handle
 - `buf`: Data to write to the file
 - `len`: Number of bytes to write
 - `wrote`: Where to store the number of bytes written



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_file_set_bof(ina_file_t *file);
```

Set file pointer to the beginning.


**Parameters**
 - `file`: File handle



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_file_set_pos(ina_file_t *file,
```

Set file pointer to a specific position.


**Parameters**
 - `file`: File handle
 - `offset`: Offset for increment
 - `mode`: Seek mode. INA_FILE_SEEK_MODE_SET for absolute positioning or
INA_FILE_SEEK_MODE_CUR for relative positioning



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_file_get_pos(ina_file_t *file, uint64_t *offset);
```

Get the current value of the position indicator of the file.


**Parameters**
 - `file`: File handle
 - `offset`: Where to store the value of the current file position



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_file_set_eof(ina_file_t *file);
```

Set file pointer to the end of file.


**Parameters**
 - `file`: File handle



**Return**

INA_SUCCESS if all went well

