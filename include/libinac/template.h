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
#ifndef _LIBINAC_TEMPLATE_H_
#define _LIBINAC_TEMPLATE_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* opaque template context */
typedef struct ina_template_ctx_s ina_template_ctx_t;

/* opaque template */
typedef struct ina_template_env_s ina_template_env_t;

/* opaque template table */
typedef struct ina_template_table_s ina_template_table_t;

/*
 * 
 */
INA_API(ina_rc_t) ina_template_init(ina_template_ctx_t **ctx);
/*
 * 
 */
INA_API(ina_rc_t) ina_template_destroy(ina_template_ctx_t **ctx);
/*
 * 
 */
INA_API(ina_rc_t) ina_template_compile(ina_template_ctx_t *ctx, const char *tpl, ina_template_env_t **env);
/*
 * 
 */
INA_API(ina_rc_t) ina_template_render(ina_template_ctx_t *ctx, ina_template_env_t *env, ina_str_t *out);
/*
 * 
 */
INA_API(ina_rc_t) ina_template_set_number(ina_template_ctx_t *ctx, ina_template_env_t *env, double num);
/*
 * 
 */
INA_API(ina_rc_t) ina_template_set_string(ina_template_ctx_t *ctx, ina_template_env_t *env, ina_str_t str);
/*
 * 
 */
INA_API(ina_rc_t) ina_template_new_table(ina_template_ctx_t *ctx, ina_template_env_t *env, ina_template_table_t **tbl);
/*
 * 
 */
INA_API(ina_rc_t) ina_template_table_set_number(ina_template_ctx_t *ctx, ina_template_table_t *t, double num);
/*
 * 
 */
INA_API(ina_rc_t) ina_template_table_set_string(ina_template_ctx_t *ctx, ina_template_table_t *t, ina_str_t str);

#ifdef __cplusplus
}
#endif

#endif
