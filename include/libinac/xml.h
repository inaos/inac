/*
 * Copyright (c) 2013-2014,2016 INAOS GmbH
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

typedef struct ina_xml_ctx_s {
    int parser_pool_size;
    ina_xml_parser_t *parsers;
    int mp;
} ina_xml_ctx_t;

/*
 * Creates and initializes a xml context.
 *
 * Parameters
 *  ctx               Where to store the created xml context
 *  parser_pool_size  Defines the number of parser to preallocate
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_xml_init(ina_xml_ctx_t **ctx, int parser_pool_size);

/*
 * Creates and initializes a xml context using a memory pool
 *
 * Parameters
 *  ctx               Where to store the created xml context
 *  parser_pool_size  Defines the number of parser to preallocate
 *  pool              Memory pool
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_xml_init_using_pool(ina_xml_ctx_t **ctx, 
                                          int parser_pool_size, 
                                          ina_mempool_t *pool);

/*
 * Destroy a XML context. Ensure that all parsers are release before
 * calling this function.
 *
 * Parameters
 *  ctx  Context to free.
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_xml_destroy(ina_xml_ctx_t **ctx);

/*
 * Borrow a parser from a XML context.
 *
 * Parameters
 *  ctx  XML context
 *  p    Where to store the parser
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_xml_parser_borrow(ina_xml_ctx_t *ctx,
                                        ina_xml_parser_t **p);

/*
 * Release a parser previously borrowed from a XML context
 *
 * Parameters
 *  ctx  XML context
 *  p    Parser to release
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_xml_parser_release(ina_xml_ctx_t *ctx,
                                         ina_xml_parser_t **p);

/*
 * Parse a XML source string.
 *
 * Parameters
 *  p       Parser
 *  source  XML source
 *  root    Where to store the XML root element
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_xml_parser_execute(ina_xml_parser_t *p,
                                         ina_str_t source,
                                         ina_xml_elem_t **root);

/*
 * Parse a XML source from file.
 *
 * Parameters
 *  p     Parser
 *  file  File path
 *  root  Where to store the XML root element
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_xml_parser_execute_from_file(ina_xml_parser_t *p,
                                                   const char *file,
                                                   ina_xml_elem_t **root);

/*
 * Get the fist child element of a element.
 *
 * Parameters
 *  elem   Element
 *  first  Where to store the first child element
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_xml_elem_first(ina_xml_elem_t *elem,
                                     ina_xml_elem_t **first);

/*
 * Get the next element child element of an element
 *
 * Parameters
 *  elem  Element
 *  next  Where to store the next element
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_xml_elem_next(ina_xml_elem_t *elem,
                                    ina_xml_elem_t **next);

/*
 * Get element name
 *
 * Parameters
 *  elem  Element
 *  name  Where to store element name
 *  len   Where to store name length
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_xml_elem_name(ina_xml_elem_t *elem,
                                    const char **name,
                                    size_t *len);

/*
 * Get element value
 *
 * Parameters
 *  elem   Element
 *  value  Where to store element value
 *  len    Where to store value length
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_xml_elem_value(ina_xml_elem_t *elem,
                                     const char **value,
                                     size_t *len);

/*
 * Get first attribute of an element.
 *
 * Parameters
 *  elem   Element
 *  first  Where to store the attribute
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_xml_elem_attr_first(ina_xml_elem_t *elem,
                                          ina_xml_attr_t **first);

/*
 * Get next attribute of an element.
 *
 * Parameters
 *  attr  Attribute
 *  next  Where to store the next attribute
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_xml_attr_next(ina_xml_attr_t *attr,
                                    ina_xml_attr_t **next);

/*
 * Get attribute name.
 *
 * Parameters
 *  attr  Attribute
 *  name  Where to store attribute name
 *  len   Where to store attribute name length
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_xml_attr_name(ina_xml_attr_t *attr,
                                    const char **name,
                                    size_t *len);

/*
 * Get attribute value.
 *
 * Parameters
 *  attr   Attribute
 *  value  Where to store attribute value
 *  len    Where to store attribute value length
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_xml_attr_value(ina_xml_attr_t *attr,
                                     const char **value,
                                     size_t *len);

#ifdef __cplusplus
}
#endif

#endif
