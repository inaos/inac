/*
 * Copyright (c) 2013, INAOS GmbH
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
#ifndef _LIBINAC_RDB_H_
#define _LIBINAC_RDB_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* opaque relational database context */
typedef struct ina_rdb_ctx_s ina_rdb_ctx_t;

typedef struct ina_rdb_datasource_s {
	ina_str_t host;
	int port;
	ina_str_t database;
	ina_str_t username;
	ina_str_t password;
} ina_rdb_datasource_t;

typedef ina_rc_t (*ina_rdb_get_boolean_cn)(ina_rdb_ctx_t *ctx, const char *name, int *v);
typedef ina_rc_t (*ina_rdb_get_blob_cn)(ina_rdb_ctx_t *ctx, const char *name, unsigned char **v);
typedef ina_rc_t (*ina_rdb_get_int_cn)(ina_rdb_ctx_t *ctx, const char *name, int *v);
typedef ina_rc_t (*ina_rdb_get_long_cn)(ina_rdb_ctx_t *ctx, const char *name, long *v);
typedef ina_rc_t (*ina_rdb_get_double_cn)(ina_rdb_ctx_t *ctx, const char *name, double *v);
typedef ina_rc_t (*ina_rdb_get_string_cn)(ina_rdb_ctx_t *ctx, const char *name, ina_str_t *v);
typedef ina_rc_t (*ina_rdb_get_time_cn)(ina_rdb_ctx_t *ctx, const char *name, ina_time_t *v);

typedef ina_rc_t (*ina_rdb_get_boolean_ci)(ina_rdb_ctx_t *ctx, int idx, int *v);
typedef ina_rc_t (*ina_rdb_get_blob_ci)(ina_rdb_ctx_t *ctx, int idx, unsigned char **v);
typedef ina_rc_t (*ina_rdb_get_int_ci)(ina_rdb_ctx_t *ctx, int idx, int *v);
typedef ina_rc_t (*ina_rdb_get_long_ci)(ina_rdb_ctx_t *ctx, int idx, long *v);
typedef ina_rc_t (*ina_rdb_get_double_ci)(ina_rdb_ctx_t *ctx, int idx, double *v);
typedef ina_rc_t (*ina_rdb_get_string_ci)(ina_rdb_ctx_t *ctx, int idx, ina_str_t *v);
typedef ina_rc_t (*ina_rdb_get_time_ci)(ina_rdb_ctx_t *ctx, int idx, ina_time_t *v);

typedef struct ina_rdb_row_handler_s {
	ina_rdb_get_boolean_cn get_boolean_cn;
	ina_rdb_get_blob_cn get_blob_cn;
	ina_rdb_get_int_cn get_int_cn;
	ina_rdb_get_long_cn get_long_cn;
	ina_rdb_get_double_cn get_double_cn;
	ina_rdb_get_string_cn get_string_cn;
	ina_rdb_get_time_cn get_time_cn;
	ina_rdb_get_boolean_ci get_boolean_ci;
	ina_rdb_get_blob_ci get_blob_ci;
	ina_rdb_get_int_ci get_int_ci;
	ina_rdb_get_long_ci get_long_ci;
	ina_rdb_get_double_ci get_double_ci;
	ina_rdb_get_string_ci get_string_ci;
	ina_rdb_get_time_ci get_time_ci;
} ina_rdb_row_handler_t;

typedef ina_rc_t (*ina_rdb_row_callback)(ina_rdb_ctx_t *ctx, ina_rdb_row_handler_t *handler, void *user_data);



/*
 * 
 */
INA_API(ina_rc_t) ina_rdb_init(ina_rdb_ctx_t **ctx);
/*
 * 
 */
INA_API(ina_rc_t) ina_rdb_destroy(ina_rdb_ctx_t **ctx);

INA_API(ina_rc_t) ina_rdb_simple_query(ina_rdb_ctx_t *ctx, ina_str_t query, ina_rdb_row_callback cb, void *user_data);


#ifdef __cplusplus
}
#endif

#endif
