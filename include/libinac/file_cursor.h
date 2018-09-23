/*
 * Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_FILE_CURSOR_H_
#define _LIBINAC_FILE_CURSOR_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Cursor type */
typedef enum ina_file_cursor_type_e {
    INA_FILE_CURSOR_TYPE_MMAP,
    INA_FILE_CURSOR_TYPE_FILEIO
} ina_file_cursor_type_t;

/* Cursor IO mode */
typedef enum ina_file_cursor_mode_e {
    INA_FILE_CURSOR_MODE_READ_BINARY,
    INA_FILE_CURSOR_MODE_READWRITE_BINARY,
    INA_FILE_CURSOR_MODE_READ_TEXT_LINE,
    INA_FILE_CURSOR_MODE_READWRITE_TEXT_LINE,
    INA_FILE_CURSOR_MODE_READ_TEXT_CHUNK,
    INA_FILE_CURSOR_MODE_READWRITE_TEXT_CHUNK,
    INA_FILE_CURSOR_MODE_WRITE_BINARY,
    INA_FILE_CURSOR_MODE_WRITE_TEXT_LINE,
    INA_FILE_CURSOR_MODE_WRITE_TEXT_CHUNK,
} ina_file_cursor_mode_t;

#define INA_FILE_CUROSR_MMAP_BUFFER_128MB    (1024*1024*128)
#define INA_FILE_CURSOR_MMAP_BUFFER_256MB    (1024*1024*256)
#define INA_FILE_CURSOR_MMAP_BUFFER_512MB    (1024*1024*512)
#define INA_FILE_CURSOR_MMAP_BUFFER_1GB      (1024*1024*1024)
#ifdef INA_CPU_X86_64
#define INA_FILE_CURSOR_MMAP_BUFFER_2GB      (1024LL*1024LL*1024LL*2LL)
#define INA_FILE_CURSOR_MMAP_BUFFER_4GB      (1024LL*1024LL*1024LL*4LL)
#define INA_FILE_CURSOR_MMAP_BUFFER_8GB      (1024LL*1024LL*1024LL*8LL)
#define INA_FILE_CURSOR_MMAP_BUFFER_16GB     (1024LL*1024LL*1024LL*16LL)
#define INA_FILE_CURSOR_MMAP_BUFFER_32GB     (1024LL*1024LL*1024LL*32LL)
#define INA_FILE_CURSOR_MMAP_BUFFER_64GB     (1024LL*1024LL*1024LL*64LL)
#define INA_FILE_CURSOR_MMAP_BUFFER_128GB    (1024LL*1024LL*1024LL*128LL)
#endif

#define INA_FILE_CURSOR_FILEIO_BUFFER_512B    (512)
#define INA_FILE_CURSOR_FILEIO_BUFFER_4KB     (1024*4)
#define INA_FILE_CURSOR_FILEIO_BUFFER_8KB     (1024*8)
#define INA_FILE_CURSOR_FILEIO_BUFFER_12KB    (1024*12)
#define INA_FILE_CURSOR_FILEIO_BUFFER_16KB    (1024*16)
#define INA_FILE_CURSOR_FILEIO_BUFFER_32KB    (1024*32)


/* opaque cursor type */
typedef struct ina_file_cursor_s ina_file_cursor_t;

/* function pointer type to free a cursor */
typedef ina_rc_t (*ina_file_cursor_free_fp)(ina_file_cursor_t **cursor);

/* function pointer to set cursors buffer size in bytes */
typedef ina_rc_t (*ina_file_cursor_get_buffer_size_fp)
        (const ina_file_cursor_t *cursor,
         uint64_t *buffer_size);

/* function pointer set current position of a cursor */
typedef ina_rc_t (*ina_file_cursor_set_pos_fp)
        (ina_file_cursor_t *cursor,
         uint64_t position);

/* function pointer for getting the current position of a cursor */
typedef ina_rc_t (*ina_file_cursor_get_pos_fp)
        (const ina_file_cursor_t *cursor,
         uint64_t *position);

/* function pointer to set a cursor to BOF */
typedef ina_rc_t (*ina_file_cursor_set_bof_fp)(ina_file_cursor_t *cursor);
/* function pointer to set a cursor to EOF */
typedef ina_rc_t (*ina_file_cursor_set_eof_fp)(ina_file_cursor_t *cursor);

/* function pointer to read a text line with a cursor */
typedef ina_rc_t (*ina_file_cursor_text_read_line_fp)
        (ina_file_cursor_t *cursor,
         const char **begin_line, size_t *len);

/* function pointer to read a chunk of binary data with a cursor */
typedef ina_rc_t (*ina_file_cursor_binary_read_chunk_fp)
        (ina_file_cursor_t *cursor,
         size_t requested,
         size_t *read,
         const unsigned char **chunk);

/* function pointer to read/write chunk of binary data using a cursor */
typedef ina_rc_t (*ina_file_cursor_binary_readwrite_chunk_fp)
        (ina_file_cursor_t *cursor,
         size_t requested,
         size_t *actual,
         unsigned char **chunk);

/* function pointer to read a chunk of text using a cursor */
typedef ina_rc_t (*ina_file_cursor_text_read_chunk_fp)
        (ina_file_cursor_t *cursor,
         size_t requested,
         size_t *read,
         const char **chunk);

/*
 * Create and initialize a new file cursor.
 *
 * Parameters
 *  cursor_type  Define type of cursor
 *  mode         Define the cursor IO mode
 *  buffer_size  Size in bytes for the internal read/write buffer
 *  cursor       Where to store the newly created file cursor
 *  mmap_ctx     MMAP context if cursor_type is INA_FILE_CURSOR_TYPE_MMAP
 *
 * Return
 *  INA_SUCCESS if all went well
 *
 * FIXME: Combine pool version and normal version together for simplicity
 */
INA_API(ina_rc_t) ina_file_cursor_new(ina_file_t *file,
                                      ina_file_cursor_type_t cursor_type,
                                      ina_file_cursor_mode_t mode,
                                      uint64_t buffer_size,
                                      ina_file_cursor_t **cursor,
                                      ina_mmap_ctx_t *mmap_ctx);
/*
 * Create and initialize a new file cursor. Allocate the cursor internal
 * buffers (buffer, line) on a memory pool
 *
 * Note: When using mmap this has no effect!
 *
 * Parameters
 *  cursor_type  Define type of cursor
 *  mode         Define the cursor IO mode
 *  buffer_size  Size in bytes for the internal read/write buffer
 *  cursor       Where to store the newly created file cursor
 *  mmap_ctx     MMAP context if cursor_type is INA_FILE_CURSOR_TYPE_MMAP
 *  pool         Memory pool to be used
 *
 * Return
 *  INA_SUCCESS if all went well
 *
 * FIXME: Combine pool version and normal version together for simplicity
 */
INA_API(ina_rc_t) ina_file_cursor_new_using_pool(ina_file_t *file, 
                                                 ina_file_cursor_type_t cursor_type,
                                                 ina_file_cursor_mode_t mode,
                                                 uint64_t buffer_size,
                                                 ina_file_cursor_t **cursor,
                                                 ina_mmap_ctx_t *mmap_ctx,
                                                 ina_mempool_t *pool);
/*
 * Destroy a cursor.
 *
 * Parameters
 *  cursor  Cursor to free.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_file_cursor_free(ina_file_cursor_t **cursor);

/*
 * Retrieve the underlying file handle for a cursor.
 *
 * Parameters
 *  cursor  Cursor
 *  file    Where to store the file handle
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_file_cursor_get_file(const ina_file_cursor_t *cursor,
                                           ina_file_t **file);

/*
 * Get the current internal buffer size.
 *
 * Parameter
 *  cursor   Cursor
 *  buffer_size  Where to store the current buffer sitze
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_file_cursor_get_buffer_size(const ina_file_cursor_t *cursor,
                                                  uint64_t *buffer_size);

/*
 * Get the current mode of an cursor.
 *
 * Parameters
 *  cursor  Cursor
 *  mode    Where to store the current cursor.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_file_cursor_get_mode(const ina_file_cursor_t *cursor,
                                           ina_file_cursor_mode_t *mode);

/*
 * Get the current type of a cursor.
 *
 * Parameters
 *  cursor   Cursor
 *  type     Where to store the current type
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_file_cursor_get_type(const ina_file_cursor_t *cursor,
                                           ina_file_cursor_type_t *type);

/*
 * Get the current positin of a cursor.
 *
 * Parameters
 *  cursor    Cursor
 *  position  Where to store the current position.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_file_cursor_get_pos(const ina_file_cursor_t *cursor,
                                          uint64_t *position);

/*
 * Set file position for a cursor.
 *
 * Parameters
 *  cursor    Cursor
 *  position  new cursor position
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_cursor_set_pos(ina_file_cursor_t *cursor,
                                          uint64_t position);

/*
 * Set cursor at the beginning of the file.
 *
 * Parameters
 *  cursor  Cursor
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_cursor_set_bof(ina_file_cursor_t *cursor);

/*
 * Set cursor at the end of file.
 *
 * Parameters
 *  cursor  Cursor
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_cursor_set_eof(ina_file_cursor_t *cursor);

/*
 * Read a text line
 *
 * Parameters
 *  cursor      Cursor
 *  begin_line  Buffer where to store the text line
 *  len         Max size of line length
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_cursor_text_read_line(ina_file_cursor_t *cursor,
                                                 const char **begin_line,
                                                 size_t *len);

/*
 * Read a binary chunk of data.
 *
 * Parameters
 *  cursor     Cursor
 *  requested  Number of bytes minimum requested
 *  read       Where to store number of bytes read
 *  chunk      Where to store read data
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_cursor_binary_read_chunk(ina_file_cursor_t *cursor,
                                                    size_t requested,
                                                    size_t *read,
                                                    const unsigned char **chunk);

/*
 * Read/Write binary chunk of data.
 *
 * Parameters
 *  cursor     Cursor
 *  requested  Number of bytes minimum requested
 *  actual     Where to store actual byted read or written
 *  chunk      Read/write buffer
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_cursor_binary_readwrite_chunk(ina_file_cursor_t *cursor,
                                                         size_t requested,
                                                         size_t *actual,
                                                         unsigned char **chunk);

/*
 * Read a chunk of text data.
 *
 * Parameters
 *  cursor     Cursor
 *  requested  Number of bytes minimum requested
 *  read       Where to store the number of bytes read
 *  chunk      Where to store the read data
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_cursor_text_read_chunk(ina_file_cursor_t *cursor,
                                                  size_t requested,
                                                  size_t *read,
                                                  const char **chunk);


#ifdef __cplusplus
}
#endif

#endif
