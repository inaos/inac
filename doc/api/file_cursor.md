
```C
#ifndef _LIBINAC_FILE_CURSOR_H_
```

Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
typedef enum ina_file_cursor_type_e {
```
Cursor type
```C
typedef enum ina_file_cursor_mode_e {
```
Cursor IO mode
```C
typedef struct ina_file_cursor_s ina_file_cursor_t;
```
opaque cursor type
```C
typedef ina_rc_t (*ina_file_cursor_free_fp)(ina_file_cursor_t **cursor);
```
function pointer type to free a cursor
```C
typedef ina_rc_t (*ina_file_cursor_get_buffer_size_fp)
```
function pointer to set cursors buffer size in bytes
```C
typedef ina_rc_t (*ina_file_cursor_set_pos_fp)
```
function pointer set current position of a cursor
```C
typedef ina_rc_t (*ina_file_cursor_get_pos_fp)
```
function pointer for getting the current position of a cursor
```C
typedef ina_rc_t (*ina_file_cursor_set_bof_fp)(ina_file_cursor_t *cursor);/* function pointer to set a cursor to EOF */
```
function pointer to set a cursor to BOF
```C
typedef ina_rc_t (*ina_file_cursor_text_read_line_fp)
```
function pointer to read a text line with a cursor
```C
typedef ina_rc_t (*ina_file_cursor_binary_read_chunk_fp)
```
function pointer to read a chunk of binary data with a cursor
```C
typedef ina_rc_t (*ina_file_cursor_binary_readwrite_chunk_fp)
```
function pointer to read/write chunk of binary data using a cursor
```C
typedef ina_rc_t (*ina_file_cursor_text_read_chunk_fp)
```
function pointer to read a chunk of text using a cursor
```C
INA_API(ina_rc_t) ina_file_cursor_new(ina_file_t *file,
```

Create and initialize a new file cursor.


**Parameters**
 - `cursor_type`: Define type of cursor
 - `mode`: 
 - `buffer_size`: Size in bytes for the internal read/write buffer
 - `cursor`: 
 - `mmap_ctx`: MMAP context if cursor_type is INA_FILE_CURSOR_TYPE_MMAP



**Return**

INA_SUCCESS if all went well


FIXME: Combine pool version and normal version together for simplicity

```C
INA_API(ina_rc_t) ina_file_cursor_new_using_pool(ina_file_t *file,
```

Create and initialize a new file cursor. Allocate the cursor internal
buffers (buffer, line) on a memory pool

Note: When using mmap this has no effect!


**Parameters**
 - `cursor_type`: Define type of cursor
 - `mode`: 
 - `buffer_size`: Size in bytes for the internal read/write buffer
 - `cursor`: 
 - `mmap_ctx`: MMAP context if cursor_type is INA_FILE_CURSOR_TYPE_MMAP
 - `pool`: 



**Return**

INA_SUCCESS if all went well


FIXME: Combine pool version and normal version together for simplicity

```C
INA_API(ina_rc_t) ina_file_cursor_free(ina_file_cursor_t **cursor);
```

Destroy a cursor.


**Parameters**
 - `cursor`: Cursor to free.



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_file_cursor_get_file(const ina_file_cursor_t *cursor,
```

Retrieve the underlying file handle for a cursor.


**Parameters**
 - `cursor`: Cursor
 - `file`: Where to store the file handle



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_file_cursor_get_buffer_size(const ina_file_cursor_t *cursor,
```

Get the current internal buffer size.

Parameter
cursor   Cursor
buffer_size  Where to store the current buffer sitze


**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_file_cursor_get_mode(const ina_file_cursor_t *cursor,
```

Get the current mode of an cursor.


**Parameters**
 - `cursor`: Cursor
 - `mode`: Where to store the current cursor.



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_file_cursor_get_type(const ina_file_cursor_t *cursor,
```

Get the current type of a cursor.


**Parameters**
 - `cursor`: Cursor
 - `type`: Where to store the current type



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_file_cursor_get_pos(const ina_file_cursor_t *cursor,
```

Get the current positin of a cursor.


**Parameters**
 - `cursor`: Cursor
 - `position`: Where to store the current position.



**Return**

INA_SUCCESS


```C
INA_API(ina_rc_t) ina_file_cursor_set_pos(ina_file_cursor_t *cursor,
```

Set file position for a cursor.


**Parameters**
 - `cursor`: Cursor
 - `position`: new cursor position



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_file_cursor_set_bof(ina_file_cursor_t *cursor);
```

Set cursor at the beginning of the file.


**Parameters**
 - `cursor`: Cursor



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_file_cursor_set_eof(ina_file_cursor_t *cursor);
```

Set cursor at the end of file.


**Parameters**
 - `cursor`: Cursor



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_file_cursor_text_read_line(ina_file_cursor_t *cursor,
```

Read a text line


**Parameters**
 - `cursor`:  Cursor
 - `begin_line`: Buffer where to store the text line
 - `len`: 



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_file_cursor_binary_read_chunk(ina_file_cursor_t *cursor,
```

Read a binary chunk of data.


**Parameters**
 - `cursor`: Cursor
 - `requested`: Number of bytes minimum requested
 - `read`: 
 - `chunk`:  Where to store read data



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_file_cursor_binary_readwrite_chunk(ina_file_cursor_t *cursor,
```

Read/Write binary chunk of data.


**Parameters**
 - `cursor`: Cursor
 - `requested`: Number of bytes minimum requested
 - `actual`: Where to store actual byted read or written
 - `chunk`:  Read/write buffer



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_file_cursor_text_read_chunk(ina_file_cursor_t *cursor,
```

Read a chunk of text data.


**Parameters**
 - `cursor`: Cursor
 - `requested`: Number of bytes minimum requested
 - `read`: 
 - `chunk`:  Where to store the read data



**Return**

INA_SUCCESS if all went well

