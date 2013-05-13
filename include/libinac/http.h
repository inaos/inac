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
#ifndef _LIBINAC_HTTP_H_
#define _LIBINAC_HTTP_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ina_http_parser_type_e {
	INA_HTTP_PARSER_TYPE_REQUEST, 
	INA_HTTP_PARSER_TYPE_RESPONSE, 
	INA_HTTP_PARSER_TYPE_BOTH
} ina_http_parser_type_t;

typedef enum ina_http_parser_method_e {
	INA_HTTP_PARSER_METHOD_DELETE,
  	INA_HTTP_PARSER_METHOD_GET,
  	INA_HTTP_PARSER_METHOD_HEAD,
  	INA_HTTP_PARSER_METHOD_POST,
  	INA_HTTP_PARSER_METHOD_PUT,
  	INA_HTTP_PARSER_METHOD_CONNECT,
  	INA_HTTP_PARSER_METHOD_OPTIONS,
  	INA_HTTP_PARSER_METHOD_TRACE,
} ina_http_parser_method_t;

typedef enum http_parser_url_fields_e {
	INA_HTTP_PARSER_UF_SCHEMA    = 0,
	INA_HTTP_PARSER_UF_HOST      = 1,
	INA_HTTP_PARSER_UF_PORT      = 2,
	INA_HTTP_PARSER_UF_PATH      = 3,
	INA_HTTP_PARSER_UF_QUERY     = 4,
	INA_HTTP_PARSER_UF_FRAGMENT  = 5,
	INA_HTTP_PARSER_UF_USERINFO  = 6,
	INA_HTTP_PARSER_UF_MAX        = 7
} http_parser_url_fields_t;

/* opaque */
typedef struct ina_http_parser_s ina_http_parser_t;

/* opaque */
typedef struct ina_http_url_s ina_http_url_t;

/* opaque */
typedef struct ina_http_header_s ina_http_header_t;

typedef struct ina_http_ctx_s {
	int parser_pool_size;
	ina_http_parser_t *parsers;
} ina_http_ctx_t;

/*
 * 
 */
INA_API(ina_rc_t) ina_http_init(ina_http_ctx_t **ctx, ina_http_parser_type_t parser_type, int parser_pool_size);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_destroy(ina_http_ctx_t **ctx);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_parser_borrow(ina_http_ctx_t *ctx, ina_http_parser_t **p);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_parser_release(ina_http_ctx_t *ctx, ina_http_parser_t **p);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_parser_url_get(ina_http_parser_t *p, ina_http_url_t **url);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_url_get_field(ina_http_url_t *url, uint16_t mask, const char **begin, uint16_t *len);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_url_get_port(ina_http_url_t *url, uint16_t *port);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_parser_header_first(ina_http_parser_t *p, ina_http_header_t **first);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_parser_header_next(ina_http_parser_t *p, ina_http_header_t **next);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_parser_header_by_name(ina_http_parser_t *p, const char *name, ina_http_header_t **header);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_parser_header_get_field(ina_http_parser_t *p, ina_http_header_t *header, const char **begin, size_t *len);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_parser_header_get_value(ina_http_parser_t *p, ina_http_header_t *header, const char **begin, size_t *len);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_parser_payload_get(ina_http_parser_t *p, unsigned char **payload, size_t *payload_len);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_parser_status_code(ina_http_parser_t *p, unsigned short *status);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_parser_method(ina_http_parser_t *p, int *method);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_parser_httpversion(ina_http_parser_t *p, unsigned short *major, unsigned short *minor);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_parser_execute(ina_http_parser_t *p, const char *in, size_t inlen, int *more);
/*
 * 
 */
INA_API(ina_rc_t) ina_http_parser_eof(ina_http_parser_t *p);
/*
 *
 */
INA_API(ina_rc_t) ina_http_parser_should_keep_alive(ina_http_parser_t *p, int *should_keep_alive);
			
#ifdef __cplusplus
}
#endif

#endif
