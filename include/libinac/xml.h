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
#ifndef _LIBINAC_XML_H_
#define _LIBINAC_XML_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque structures */
typedef struct ina_xml_parser_s ina_xml_parser_t;
typedef struct ina_xml_elem_s ina_xml_elem_t;
typedef struct ina_xml_attr_s ina_xml_attr_t;
typedef struct ina_xml_itr_s ina_xml_itr_t;

typedef struct ina_xml_ctx_s {
	int parser_pool_size;
	ina_xml_parser_t *parsers;
} ina_xml_ctx_t;

/*
 * 
 */
INA_API(ina_rc_t) ina_xml_init(ina_xml_ctx_t **ctx, int parser_pool_size);
/*
 * 
 */
INA_API(ina_rc_t) ina_xml_destory(ina_xml_ctx_t **ctx);
/*
 * 
 */
INA_API(ina_rc_t) ina_xml_parser_borrow(ina_xml_ctx_t *ctx, ina_xml_parser_t **p);
/*
 * 
 */
INA_API(ina_rc_t) ina_xml_parser_release(ina_xml_ctx_t *ctx, ina_xml_parser_t **p);
/*
 * 
 */
INA_API(ina_rc_t) ina_xml_parser_execute(ina_xml_parser_t *p, ina_str_t source, ina_xml_elem_t **root);
/*
 * 
 */
INA_API(ina_rc_t) ina_xml_parser_get_child_itr(ina_xml_parser_t *p, ina_xml_elem_t *elem, ina_xml_itr_t **itr);
/*
 * 
 */
INA_API(ina_rc_t) ina_xml_itr_next(ina_xml_itr_t *itr, ina_xml_elem_t **elem);
/*
 * 
 */
INA_API(ina_rc_t) ina_xml_elem_name(ina_xml_elem_t *elem, const char **name, size_t *len);
/*
 * 
 */
INA_API(ina_rc_t) ina_xml_elem_value(ina_xml_elem_t *elem, const char **value, size_t *len);
/*
 * 
 */
INA_API(ina_rc_t) ina_xml_parser_get_attr_itr(ina_xml_parser_t *p, ina_xml_elem_t *elem, ina_xml_itr_t **attr_itr);
/*
 * 
 */
INA_API(ina_rc_t) ina_xml_attr_itr_next(ina_xml_itr_t *itr, ina_xml_attr_t **attr);
/*
 * 
 */
INA_API(ina_rc_t) ina_xml_attr_name(ina_xml_attr_t *attr, const char **name, size_t *len);
/*
 * 
 */
INA_API(ina_rc_t) ina_xml_attr_value(ina_xml_attr_t *attr, const char **value, size_t *len);

#ifdef __cplusplus
}
#endif

#endif
