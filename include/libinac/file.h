/*
 * Copyright (c) 2014, INAOS GmbH
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
#ifndef _LIBINAC_FILE_H_
#define _LIBINAC_FILE_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* opaque file context */
typedef struct ina_file_ctx_s ina_file_ctx_t;

/* opaque file */
typedef struct ina_file_s ina_file_t;

typedef enum ina_file_access_mode_e {
    INA_FILE_ACCESS_MODE_READ;
	INA_FILE_ACCESS_MODE_READWRITE;
} ina_file_access_mode_t;

typedef enum ina_file_create_mode_e {
    INA_FILE_CREATE_MODE_OPEN;
	INA_FILE_CREATE_MODE_CREATE;
	INA_FILE_CREATE_MODE_APPEND;
} ina_file_create_mode_t;

typedef enum ina_file_share_mode_e {
    INA_FILE_SHARE_MODE_EXCULSIVE;
	INA_FILE_SHARE_MODE_READ;
	INA_FILE_SHARE_MODE_WRITE;
} ina_file_share_mode_t;

/* opaque file statistics */
typedef struct ina_file_stat_s ina_file_stat_t;

/*
 *
 */
INA_API(ina_rc_t) ina_file_init(ina_file_ctx_t **ctx);

/*
 *
 */
INA_API(ina_rc_t) ina_file_destroy(ina_file_ctx_t **ctx);

/*
 *
 */
INA_API(ina_rc_t) ina_file_open(ina_file_ctx_t *ctx, const char *file_fqn,
                                ina_file_access_mode_t access, ina_file_create_mode_t create, 
								ina_file_share_mode_t share, int flags, ina_file_t **file);

/*
 * - GetFileSizeEx
 * - GetFileAttributes
 * - GetFileTime
 */								
INA_API(ina_rc_t) ina_file_stat(ina_file_t *ctx, ina_file_stat_t **stat);

/*
 *
 */								
INA_API(ina_rc_t) ina_file_stat_is_dir(ina_file_stat_t *stat, int **dir);

/*
 * 
 */								
INA_API(ina_rc_t) ina_file_stat_file_size(ina_file_stat_t *stat, size_t **file_size);

/*
 *
 */								
INA_API(ina_rc_t) ina_file_stat_atime(ina_file_stat_t *stat, time_t **last_access);

/*
 *
 */								
INA_API(ina_rc_t) ina_file_stat_mtime(ina_file_stat_t *stat, time_t **last_modification);

/*
 *
 */
INA_API(ina_rc_t) ina_file_close(ina_file_ctx_t *ctx, ina_file_t **file);

#ifdef __cplusplus
}
#endif

#endif