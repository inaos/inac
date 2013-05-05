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
#include <libinac/lib.h>
#include "config.h"

#include <libinac/utlist.h>

#include <contribs/rapidxml/rapidxml.h>

struct ina_xml_attr_s {
	rapidxml_attr_t *attr;
} ina_xml_attr_s;

struct ina_xml_elem_s {
	ina_xml_attr_t *attr;
	rapidxml_node_t *elem;
} ina_xml_elem_s;

struct ina_xml_parser_s {
	rapidxml_doc_t *doc;
	ina_xml_elem_t root;
	struct ina_xml_parser_s *next;
	struct ina_xml_parser_s *prev;
} ina_xml_parser_s;

INA_API(ina_rc_t) ina_xml_init(ina_xml_ctx_t **ctx, int parser_pool_size)
{
	int i;

	*ctx = (ina_xml_ctx_t*)ina_mem_alloc(sizeof(ina_xml_ctx_t));
	(*ctx)->parser_pool_size = parser_pool_size;
	(*ctx)->parsers = NULL;
	
	for (i = 0; i < parser_pool_size; i++) {
		ina_xml_parser_t *p = (ina_xml_parser_t*)ina_mem_alloc(sizeof(ina_xml_parser_t));
		rapidxml_parser_init(&p->doc);
		DL_APPEND((*ctx)->parsers, p);	
	}
	
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_destory(ina_xml_ctx_t **ctx)
{
	ina_xml_ctx_t *c = *ctx;
	ina_xml_parser_t *p, *ptmp;
	int cnt = 0;

	INA_ASSERT_NOTNULL(*ctx);	

	DL_FOREACH_SAFE(c->parsers, p, ptmp) {
		rapidxml_parser_destroy(&p->doc);
		DL_DELETE(c->parsers, p);
		ina_mem_free(p);
		cnt++;
	}

	ina_mem_free(*ctx);

	if (cnt != c->parser_pool_size) {
		/* FIXME: push proper error */
		return INA_FAILURE;
	}

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_parser_borrow(ina_xml_ctx_t *ctx, ina_xml_parser_t **p)
{
	INA_ASSERT_NOTNULL(ctx);

        /* we ran out of parsers */
        if (ctx->parsers == NULL) {
                *p = NULL;
                return INA_FAILURE;
        }

        /* return the head and delete from the list */
        *p = ctx->parsers;
        DL_DELETE(ctx->parsers, *p);

	rapidxml_parser_reset((*p)->doc);

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_parser_release(ina_xml_ctx_t *ctx, ina_xml_parser_t **p)
{
	ina_xml_parser_t *parser = *p;

        INA_ASSERT_NOTNULL(ctx);
        INA_ASSERT_NOTNULL(parser);

        /* return parser */
        DL_APPEND(ctx->parsers, parser);
        p = NULL;

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_parser_execute(ina_xml_parser_t *p, ina_str_t source, ina_xml_elem_t **root)
{
	if (rapidxml_parser_exec(p->doc, ina_str_cstr(source)) > 0) {
		/* FIXME: proper error handling */
		return INA_FAILURE;
	}
	if (rapidxml_parser_root(p->doc, &p->root.elem) > 0) {
		/* FIXME: proper error handling */
		return INA_FAILURE;
	}
	*root = &p->root;
	
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_elem_next(ina_xml_elem_t *elem, ina_xml_elem_t **next)
{
	if (rapidxml_node_next(elem->elem, &elem->elem) > 0) {
		/* FIXME: proper error handling */
		return INA_FAILURE;
	}
	*next = elem;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_elem_name(ina_xml_elem_t *elem, const char **name, size_t *len)
{
	if (rapidxml_node_get_name(elem->elem, name, len) > 0) {
		/* FIXME: proper error handling */
		return INA_FAILURE;
	}
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_elem_value(ina_xml_elem_t *elem, const char **name, size_t *len)
{
	if (rapidxml_node_get_value(elem->elem, name, len) > 0) {
		/* FIXME: proper error handling */
		return INA_FAILURE;
	}
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_elem_attr_first(ina_xml_elem_t *elem, ina_xml_attr_t **first)
{
	if (rapidxml_node_first_attribute(elem->elem, &elem->attr->attr) > 0) {
		/* FIXME: proper error handling */
		return INA_FAILURE;
	}
	*first = elem->attr;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_attr_next(ina_xml_attr_t *attr, ina_xml_attr_t **next)
{
	if (rapidxml_attribute_next(attr->attr, &attr->attr) > 0) {
		/* FIXME: proper error handling */
		return INA_FAILURE;
	}
	*next = attr;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_attr_name(ina_xml_attr_t *attr, const char **name, size_t *len)
{
	if (rapidxml_attribute_get_name(attr->attr, name, len) > 0) {
		/* FIXME: proper error handling */
		return INA_FAILURE;
	}
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_attr_value(ina_xml_attr_t *attr, const char **value, size_t *len)
{
	if (rapidxml_attribute_get_value(attr->attr, value, len) > 0) {
		
		return INA_FAILURE;
	}
	return INA_SUCCESS;
}

