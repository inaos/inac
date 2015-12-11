/*
 * Copyright (c) 2014-2015, INAOS GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the INAOS GmbH nor the names of its contributors
 *       may be used to endorse or promote products derived from this software 
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#ifndef _LIBINAC_FILE_CURSOR_H_
#define _LIBINAC_FILE_CURSOR_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ina_file_cursor_type_e {
	INA_FILE_CURSOR_TYPE_MMAP,
	INA_FILE_CURSOR_TYPE_FILEIO
} ina_file_cursor_type_t;

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

#define INA_FILE_CUROSR_MMAP_BUFFER_128MB    1024*1024*128
#define INA_FILE_CURSOR_MMAP_BUFFER_256MB    1024*1024*256
#define INA_FILE_CURSOR_MMAP_BUFFER_512MB    1024*1024*512
#define INA_FILE_CURSOR_MMAP_BUFFER_1GB      1024*1024*1024
#ifdef INA_CPU_X86_64
#define INA_FILE_CURSOR_MMAP_BUFFER_2GB      1024LL*1024LL*1024LL*2LL
#define INA_FILE_CURSOR_MMAP_BUFFER_4GB      1024LL*1024LL*1024LL*4LL
#define INA_FILE_CURSOR_MMAP_BUFFER_8GB      1024LL*1024LL*1024LL*8LL
#define INA_FILE_CURSOR_MMAP_BUFFER_16GB     1024LL*1024LL*1024LL*16LL
#define INA_FILE_CURSOR_MMAP_BUFFER_32GB     1024LL*1024LL*1024LL*32LL
#define INA_FILE_CURSOR_MMAP_BUFFER_64GB     1024LL*1024LL*1024LL*64LL
#define INA_FILE_CURSOR_MMAP_BUFFER_128GB    1024LL*1024LL*1024LL*128LL
#endif

#define INA_FILE_CURSOR_FILEIO_BUFFER_512B    512
#define INA_FILE_CURSOR_FILEIO_BUFFER_4KB     1024*4
#define INA_FILE_CURSOR_FILEIO_BUFFER_8KB     1024*8
#define INA_FILE_CURSOR_FILEIO_BUFFER_12KB    1024*12
#define INA_FILE_CURSOR_FILEIO_BUFFER_16KB    1024*16
#define INA_FILE_CURSOR_FILEIO_BUFFER_32KB    1024*32


/* opaque file types */
typedef struct ina_file_cursor_s ina_file_cursor_t;

typedef ina_rc_t (*ina_file_cursor_free_fp)(ina_file_cursor_t **cursor);
typedef ina_rc_t (*ina_file_cursor_get_buffer_size_fp)(const ina_file_cursor_t *cursor, uint64_t *buffer_size);
typedef ina_rc_t (*ina_file_cursor_set_pos_fp)(ina_file_cursor_t *cursor, uint64_t position);
typedef ina_rc_t (*ina_file_cursor_get_pos_fp)(const ina_file_cursor_t *cursor, uint64_t *position);
typedef ina_rc_t (*ina_file_cursor_set_bof_fp)(ina_file_cursor_t *cursor);
typedef ina_rc_t (*ina_file_cursor_set_eof_fp)(ina_file_cursor_t *cursor);
typedef ina_rc_t (*ina_file_cursor_text_read_line_fp)(ina_file_cursor_t *cursor, const char **begin_line, size_t *len);
typedef ina_rc_t (*ina_file_cursor_binary_read_chunk_fp)(ina_file_cursor_t *cursor, size_t requested, size_t *read, const unsigned char **chunk);
typedef ina_rc_t (*ina_file_cursor_binary_readwrite_chunk_fp)(ina_file_cursor_t *cursor, size_t requested, size_t *actual, unsigned char **chunk);
typedef ina_rc_t (*ina_file_cursor_text_read_chunk_fp)(ina_file_cursor_t *cursor, size_t requested, size_t *read, const char **chunk);

/*
 *
 */
INA_API(ina_rc_t) ina_file_cursor_new(ina_file_t *file, 
									  ina_file_cursor_type_t cursor_type,
									  ina_file_cursor_mode_t mode, uint64_t buffer_size,
									  ina_file_cursor_t **cursor,
                                      ina_mmap_ctx_t *mmap_ctx);
/*
 *
 */
INA_API(ina_rc_t) ina_file_cursor_free(ina_file_cursor_t **cursor);
/*
 *
 */
INA_API(ina_rc_t) ina_file_cursor_get_file(const ina_file_cursor_t *cursor, ina_file_t **file);
/*
 *
 */
INA_API(ina_rc_t) ina_file_cursor_get_buffer_size(const ina_file_cursor_t *cursor, uint64_t *buffer_size);

/*
 *
 */
INA_API(ina_rc_t) ina_file_cursor_get_mode(const ina_file_cursor_t *cursor, ina_file_cursor_mode_t *mode);

/*
 *
 */
INA_API(ina_rc_t) ina_file_cursor_get_type(const ina_file_cursor_t *cursor, ina_file_cursor_type_t *type);

/*
 *
 */
INA_API(ina_rc_t) ina_file_cursor_get_pos(const ina_file_cursor_t *cursor, uint64_t *position);

/*
 *
 */
INA_API(ina_rc_t) ina_file_cursor_set_pos(ina_file_cursor_t *cursor, uint64_t position);

/*
 *
 */
INA_API(ina_rc_t) ina_file_cursor_set_bof(ina_file_cursor_t *cursor);

/*
 *
 */
INA_API(ina_rc_t) ina_file_cursor_set_eof(ina_file_cursor_t *cursor);

/*
 *
 */
INA_API(ina_rc_t) ina_file_cursor_text_read_line(ina_file_cursor_t *cursor, const char **begin_line, size_t *len);

/*
 *
 */
INA_API(ina_rc_t) ina_file_cursor_binary_read_chunk(ina_file_cursor_t *cursor, size_t requested,
												    size_t *read, const unsigned char **chunk);

/*
 *
 */
INA_API(ina_rc_t) ina_file_cursor_binary_readwrite_chunk(ina_file_cursor_t *cursor, size_t requested,
												         size_t *actual, unsigned char **chunk);
/*
 *
 */
INA_API(ina_rc_t) ina_file_cursor_text_read_chunk(ina_file_cursor_t *cursor, size_t requested,
												  size_t *read, const char **chunk);


#ifdef __cplusplus
}
#endif

#endif
