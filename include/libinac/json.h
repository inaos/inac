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
#ifndef _LIBINAC_JSON_H_
#define _LIBINAC_JSON_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque structures */
typedef struct ina_json_parser_s ina_json_parser_t;
typedef struct ina_json_generator_s ina_json_generator_t;

typedef struct ina_json_ctx_s {
	int parser_pool_size;
        int generator_pool_size;
	ina_json_parser_t *parsers;
        ina_json_generator_t *generators;
} ina_json_ctx_t;

typedef enum ina_json_parse_event_e {
    INA_JSON_PARSE_EVENT_START_OBJECT = 0,
    INA_JSON_PARSE_EVENT_OBJECT_KEY,
    INA_JSON_PARSE_EVENT_END_OBJECT,
    INA_JSON_PARSE_EVENT_START_ARRAY,
    INA_JSON_PARSE_EVENT_END_ARRAY,
    INA_JSON_PARSE_EVENT_DATA_NULL,
    INA_JSON_PARSE_EVENT_DATA_INT,
    INA_JSON_PARSE_EVENT_DATA_DOUBLE,
    INA_JSON_PARSE_EVENT_DATA_STRING,
    INA_JSON_PARSE_EVENT_DATA_BOOL,
} ina_json_parse_event_t;

typedef struct ina_json_data_s {
    ina_json_parse_event_t event;
    const unsigned char *str_val;
    size_t str_len;
    int bool_val;
    int64_t int_val;
    double double_val;
} ina_json_data_t;

/*
 * 
 */
INA_API(ina_rc_t) ina_json_init(ina_json_ctx_t **ctx, int parser_pool_size
                                int generator_pool_size);
/*
 * 
 */
INA_API(ina_rc_t) ina_json_destroy(ina_json_ctx_t **ctx);
/*
 * 
 */
INA_API(ina_rc_t) ina_json_parser_borrow(ina_json_ctx_t *ctx, ina_json_parser_t **p);
/*
 * 
 */
INA_API(ina_rc_t) ina_json_parser_release(ina_json_ctx_t *ctx, ina_json_parser_t **p);
/*
 * 
 */
INA_API(ina_rc_t) ina_json_parser_execute(ina_json_parser_t *p, unsigned char *buffer, size_t buf_len);
/*
 *
 */
INA_API(ina_rc_t) ina_json_generator_borrow(ina_json_ctx_t *ctx, ina_json_generator_t **p);
/*
 *
 */
INA_API(ina_rc_t) ina_json_generator_release(ina_json_ctx_t *ctx, ina_json_generator_t **p);
/*
 *
 */
INA_API(ina_rc_t) ina_json_parser_try_data(ina_json_parser_t *p, ina_json_data_t **data);
/*
 *
 */ 
INA_API(ina_rc_t) ina_json_generator_start_object(ina_json_generator_t *g);
/*
 *
 */
INA_API(ina_rc_t) ina_json_generator_end_object(ina_json_generator_t *g);
/*
 *
 */
INA_API(ina_rc_t) ina_json_generator_start_array(ina_json_generator_t *g);
/*
 *
 */
INA_API(ina_rc_t) ina_json_generator_end_array(ina_json_generator_t *g);
/*
 *
 */
INA_API(ina_rc_t) ina_json_generator_add_null(ina_json_generator_t *g);
/*
 *
 */
INA_API(ina_rc_t) ina_json_generator_add_boolean(ina_json_generator_t *g, int bool_val);
/*
 *
 */
INA_API(ina_rc_t) ina_json_generator_add_integer(ina_json_generator_t *g, int64_t int_val);
/*
 *
 */
INA_API(ina_rc_t) ina_json_generator_add_double(ina_json_generator_t *g, double dbl_val);
/*
 *
 */
INA_API(ina_rc_t) ina_json_generator_add_string(ina_json_generator_t *g, const char *str, size_t len);

#ifdef __cplusplus
}
#endif

#endif
