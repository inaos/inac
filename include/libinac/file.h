/*
 * Copyright 2014-2020 INAOS GmbH, Thalwil
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _LIBINAC_FILE_H_
#define _LIBINAC_FILE_H_

/**
 * DESIGN considerations
 * ---------------------
 *
 * - When it comes to reading from files or writing to files 
 *   we use a cursor to abstract how we're going to do it behind the scenes.
 *
 * - Most important thing to know is whether you want to access the data 
 *   sequentially or randomly.
 *
 * - Also it depends how long you need the data and whether one can benefit 
 *   from zero-copy or not.
 *
 * - Depending on the use-case one might choose to use mmap or read/write
 *   the cursor lets you choose what implementation to use.
 * 
 *  - Its curcial to benchmark what is faster/more efficient for you use-case.
 * 
 *  - General sentiment would be that mmap is mostly more efficient that read/write
 *
 * Links:
 * -> http://marc.info/?l=linux-kernel&m=95496636207616&w=2
 *
 * TODO:
 * -> Vectored I/O .. ReadFileScatter ... WriteFileGather
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>

#define INA_FILE_FLAG_ATTR_NORMAL          0x00000001
#define INA_FILE_FLAG_ATTR_HIDDEN          0x00000002
#define INA_FILE_FLAG_RANDOM_ACCESS        0x00000004
#define INA_FILE_FLAG_SEQUENTIAL_ACCESS    0x00000008
#ifdef INA_OS_WINDOWS
#define INA_FILE_FLAG_WIN32_OVERLAPPED     0x00000016
#endif
#ifndef INA_OS_WINDOWS
#define INA_FILE_FLAG_POSIX_DIRECT         0x00000016
#endif

/* File access mode */
typedef enum ina_file_access_mode_e {
    INA_FILE_ACCESS_MODE_READ,
    INA_FILE_ACCESS_MODE_READWRITE,
    INA_FILE_ACCESS_MODE_WRITE
} ina_file_access_mode_t;

/* File open mode */
typedef enum ina_file_create_mode_e {
    INA_FILE_CREATE_MODE_OPEN,
    INA_FILE_CREATE_MODE_CREATE,
    INA_FILE_CREATE_MODE_APPEND
} ina_file_create_mode_t;

/* File share mode */
typedef enum ina_file_share_mode_e {
    INA_FILE_SHARE_MODE_EXCLUSIVE,
    INA_FILE_SHARE_MODE_READ,
    INA_FILE_SHARE_MODE_WRITE
} ina_file_share_mode_t;

/* File seek mode */
typedef enum ina_file_seek_mode_e {
    INA_FILE_SEEK_MODE_SET,  /* Seek from start */
    INA_FILE_SEEK_MODE_CUR   /* Seek from current file position */
} ina_file_seek_mode_t;

/* Opaque file context */
typedef struct ina_file_ctx_s ina_file_ctx_t;
/* Opaque file handle */
typedef struct ina_file_s ina_file_t;
/* Opaque file attributes handle */
typedef struct ina_file_stat_s ina_file_stat_t;

/*
 * Create and initialize a new file context.
 *
 * Parameters
 *  ctx           Where to store the newly created file context
 *  default_mode  Default permissions
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_ctx_new(ina_file_ctx_t **ctx, mode_t default_mode);

/*
 * Free a file context.
 *
 * Parameters
 *  ctx  Context to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(void) ina_file_ctx_free(ina_file_ctx_t **ctx);

/*
 * Create a new file handle.
 *
 * Parameters
 *  ctx       File context
 *  file_fqn  Fully qualified name of file
 *  access    Access mode
 *  create    Creation mode
 *  share     Share mode
 *  flags     Combination on INA_FILE_FLAG_*
 *  file      Where to store the newly created file handle
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_new(ina_file_ctx_t *ctx,
                               const char *file_fqn,
                               ina_file_access_mode_t access,
                               ina_file_create_mode_t create,
                               ina_file_share_mode_t share,
                               int flags,
                               ina_file_t **file);

/*
 * Free a file handle
 *
 * Parameters
 *  ctx   File context
 *  file  File to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(void) ina_file_free(ina_file_t **file);

/*
 * Create and initialize file attributes.
 *
 * Parameters
 *  file  File
 *  stat  Where to store the file attributes
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_stat_new(const ina_file_t *file, ina_file_stat_t **stat);

/*
 * Destroy file attributes.
 *
 * Parameters
 *  stat  File attributes to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(void) ina_file_stat_free(ina_file_stat_t **stat);

/*
 * Synchronize file attributes.
 *
 * Parameters
 *  file  File
 *  stat  Where to store the file attributes
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_stat_sync(const ina_file_t *file,
                                      ina_file_stat_t *stat);

/*
 * Get the filepath of a file
 *
 * Parameters
 *  file      File handle
 *  filepath  Where to store the filepath
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_file_get_filepath(const ina_file_t *file,
                                        ina_str_t *filepath);

/*
 * Get current file permissions.
 *
 * Parameters
 *  file  File handle
 *  mode  WHere to store current file permissions
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_file_get_mode(const ina_file_t *file, mode_t *mode);

/*
 * Set file permissions
 *
 * Parameters
 *  file  File handle
 *  mode  File permissions
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_file_set_mode(const ina_file_t *file, mode_t mode);

/*
 * Query if file is a directory.
 *
 * Parameters
 *  file  File handle
 *  dir   Where to store the result. 1 for a regular directory otherwise 0.
 *
 * Return
 * INA_SUCCESS if is a directory
 */
INA_API(ina_rc_t) ina_file_stat_is_dir(ina_file_stat_t *stat);

/*
 * Get current file size in bytes.
 *
 * Parameters
 *  stat        File attributes handle
 *  file_size   Where to store the file size in bytes
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_file_stat_file_size(ina_file_stat_t *stat,
                                          size_t *file_size);

/*
 * Get last access time of a file.
 *
 * Parameters
 *  stat         File attributes handle
 *  last_access  Where to store te last access time.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_file_stat_atime(ina_file_stat_t *stat,
                                      time_t *last_access);

/*
 * Get last modification timestamp of a file.
 *
 * Parameters
 *  stat  File attributes handle
 *  last_modification  Where to store time of last modification
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_file_stat_mtime(ina_file_stat_t *stat,
                                      time_t *last_modification);

/*
 * Return the underlying native os handle of a INAC file handle
 *
 * Parameters
 *  file   INAC file handle
 *
 * Return
 *  Void pointer to the Native file handle
 */
INA_API(ina_handle_t) ina_file_os_handle(ina_file_t *file);

/*
 * Return the underlying C stream of a INAC file handle
 *
 * Parameters
 *  file   INAC file handle
 *
 * Return
 *  On successful completion return a FILE pointer. Otherwise, NULL is returned.
 */
INA_API(FILE*) ina_file_get_stream(ina_file_t *file);

/*
 * Read 'len' bytes from a file into buf.
 *
 * Parameters
 *  file  File handle
 *  buf   Read buffer, must be bigger than 'len' bytes
 *  len   Number of bytes intended to read from file
 *  read  Where to store the number of bytes read
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_read(ina_file_t *file,
                                unsigned char *buf,
                                size_t len,
                                size_t *read);

/*
 * Write 'len' bytes to a file.
 *
 * Parameters
 *  file   File handle
 *  buf    Data to write to the file
 *  len    Number of bytes to write
 *  wrote  Where to store the number of bytes written
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_write(ina_file_t *file,
                                 unsigned char *buf,
                                 size_t len,
                                 size_t *wrote);

/*
 * Set file pointer to the beginning.
 *
 * Parameters
 *  file  File handle
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_set_bof(ina_file_t *file);

/*
 * Set file pointer to a specific position.
 *
 * Parameters
 *  file    File handle
 *  offset  Offset for increment
 *  mode    Seek mode. INA_FILE_SEEK_MODE_SET for absolute positioning or
 *          INA_FILE_SEEK_MODE_CUR for relative positioning
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_set_pos(ina_file_t *file,
                                   size_t offset,
                                   ina_file_seek_mode_t mode);

/*
 * Get the current value of the position indicator of the file.
 *
 * Parameters
 *  file    File handle
 *  offset  Where to store the value of the current file position
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_get_pos(ina_file_t *file, size_t *offset);

/*
 * Set file pointer to the end of file.
 *
 * Parameters
 *  file  File handle
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_file_set_eof(ina_file_t *file);

#ifdef __cplusplus
}
#endif

#endif
