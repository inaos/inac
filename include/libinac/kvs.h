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
#ifndef _LIBINAC_KVS_H_
#define _LIBINAC_KVS_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ina_kvs_descriptor_s {
	
	ina_str_t environment_root_path;
} ina_kvs_descriptor_t;

typedef enum ina_kvs_cursor_op_e {
	INA_KVS_CURSOR_OP_FIRST,
	INA_KVS_CURSOR_OP_LAST,
	INA_KVS_CURSOR_OP_NEXT,
	INA_KVS_CURSOR_OP_PREV,
	INA_KVS_CURSOR_OP_SET,
	INA_KVS_CURSOR_OP_CURRENT
} ina_kvs_cursor_op_t;

/* opaque key-value context */
typedef struct ina_kvs_ctx_s ina_kvs_ctx_t;

/* opaque key */
typedef struct ina_kvs_key_s ina_kvs_key_t;

/* opaque data */
typedef struct ina_kvs_value_s ina_kvs_value_t;

typedef int (*ina_kvs_cmp_func)(const ina_kvs_key_t *a, const ina_kvs_key_t *b);

/* opaque database handle */
typedef struct ina_kvs_db_s ina_kvs_db_t;

/* opaque cursor handle */
typedef struct ina_kvs_cursor_s ina_kvs_curosr_t;

/*
 * 
 */
INA_API(ina_rc_t) ina_kvs_init(ina_kvs_ctx_t **ctx, ina_kvs_descriptor_t *descriptor);
/*
 * 
 */
INA_API(ina_rc_t) ina_kvs_destroy(ina_kvs_ctx_t **ctx);

INA_API(ina_rc_t) ina_kvs_backend_info(ina_str_t *info);

INA_API(ina_rc_t) ina_kvs_open_db(ina_kvs_ctx_t *ctx, ina_kvs_cmp_func cmp_func, ina_kvs_db_t **db);
INA_API(ina_rc_t) ina_kvs_close_db(ina_kvs_ctx_t *ctx, ina_kvs_db_t **db);

INA_API(ina_rc_t) ina_kvs_tx_begin(ina_kvs_ctx_t *ctx);
INA_API(ina_rc_t) ina_kvs_tx_commit(ina_kvs_ctx_t *ctx);
INA_API(ina_rc_t) ina_kvs_tx_rollback(ina_kvs_ctx_t *ctx);

INA_API(ina_rc_t) ina_kvs_key_size(ina_kvs_key_t* key, size_t *size);
INA_API(ina_rc_t) ina_kvs_key_address(ina_kvs_key_t* key, void **address);
INA_API(ina_rc_t) ina_kvs_value_size(ina_kvs_value_t *value, size_t *size);
INA_API(ina_rc_t) ina_kvs_value_address(ina_kvs_value_t *value, void **address);

INA_API(ina_rc_t) ina_kvs_append(ina_kvs_ctx_t *ctx, ina_kvs_key_t *key, ina_kvs_value_t **value);
INA_API(ina_rc_t) ina_kvs_get(ina_kvs_ctx_t *ctx, ina_kvs_key_t *key, ina_kvs_value_t **value);
INA_API(ina_rc_t) ina_kvs_put(ina_kvs_ctx_t *ctx, ina_kvs_key_t *key, ina_kvs_value_t *value);
INA_API(ina_rc_t) ina_kvs_delete(ina_kvs_ctx_t *ctx, ina_kvs_key_t *key, ina_kvs_value_t **value);

INA_API(ina_rc_t) ina_kvs_cursor_open(ina_kvs_ctx_t *ctx, ina_kvs_curosr_t **cursor);
INA_API(ina_rc_t) ina_kvs_cursor_close(ina_kvs_ctx_t *ctx, ina_kvs_curosr_t **cursor);
INA_API(ina_rc_t) ina_kvs_cursor_get(ina_kvs_ctx_t *ctx, ina_kvs_curosr_t *cursor, ina_kvs_cursor_op_t op);
INA_API(ina_rc_t) ina_kvs_cursor_put(ina_kvs_ctx_t *ctx, ina_kvs_curosr_t *cursor);


#ifdef __cplusplus
}
#endif

#endif
