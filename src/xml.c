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
#include <libinac/lib.h>
#include "config.h"

#include <libinac/utlist.h>

#include <contribs/rapidxml/rapidxml.h>

struct ina_xml_attr_s {
	rapidxml_attr_t *attr;
} ina_xml_attr_s;

struct ina_xml_elem_s {
    ina_xml_parser_t *p;
	ina_xml_attr_t attr;
	rapidxml_node_t *elem;
} ina_xml_elem_s;

struct ina_xml_parser_s {
    ina_file_ctx_t *fctx;
	rapidxml_doc_t *doc;
	ina_xml_elem_t root;
    ina_mempool_t *elem_pool;
    ina_mempool_t *parse_pool;
	struct ina_xml_parser_s *next;
	struct ina_xml_parser_s *prev;
} ina_xml_parser_s;

static void* __rapidxml_alloc_func(void *pool, size_t size)
{
    return ina_mem_alloc(size);
}
static void __rapidxml_free_func(void *pool, void *ptr)
{
    ina_mem_free(ptr);
}

static void* __rapidxml_alloc_func_mp(void *pool, size_t size)
{
    return ina_mempool_dalloc((ina_mempool_t*)pool, size);
}
static void __rapidxml_free_func_mp(void *pool, void *ptr)
{
    /* nothing to do as we use a mempool */
}

INA_API(ina_rc_t) ina_xml_init(ina_xml_ctx_t **ctx, int parser_pool_size)
{
	int i;

	*ctx = (ina_xml_ctx_t*)ina_mem_alloc(sizeof(ina_xml_ctx_t));
	(*ctx)->parser_pool_size = parser_pool_size;
	(*ctx)->parsers = NULL;
    (*ctx)->mp = 0;
	
	for (i = 0; i < parser_pool_size; i++) {
		ina_xml_parser_t *p = (ina_xml_parser_t*)ina_mem_alloc(sizeof(ina_xml_parser_t));
		rapidxml_parser_init(&p->doc, 0, NULL, NULL, __rapidxml_alloc_func, __rapidxml_free_func);
        if (!INA_SUCCEED(ina_mempool_create(&p->elem_pool, 1024, INA_MEM_DYNAMIC, NULL))) {
            return INA_ERR_PUSH_LAST;
        }
        if (!INA_SUCCEED(ina_mempool_create(&p->parse_pool, 4*1024, INA_MEM_DYNAMIC, NULL))) {
            return INA_ERR_PUSH_LAST;
        }
        if (!INA_SUCCEED(ina_file_init(&p->fctx, S_IRUSR))) {
            return INA_ERR_PUSH_LAST;
        }
		DL_APPEND((*ctx)->parsers, p);
	}
	
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_init_using_pool(ina_xml_ctx_t **ctx, 
                                          int parser_pool_size, 
                                          ina_mempool_t *pool)
{
    int i;

    *ctx = (ina_xml_ctx_t*)ina_mempool_dalloc(pool, sizeof(ina_xml_ctx_t));
	(*ctx)->parser_pool_size = parser_pool_size;
	(*ctx)->parsers = NULL;
    (*ctx)->mp = 1;
	
	for (i = 0; i < parser_pool_size; i++) {
        ina_xml_parser_t *p = (ina_xml_parser_t*)ina_mempool_dalloc(pool, sizeof(ina_xml_parser_t));
		rapidxml_parser_init(&p->doc, 0, NULL, pool, __rapidxml_alloc_func_mp, __rapidxml_free_func_mp);
        if (!INA_SUCCEED(ina_file_init(&p->fctx, S_IRUSR))) {
            return INA_ERR_PUSH_LAST;
        }
        p->elem_pool = pool;
        p->parse_pool = pool;
		DL_APPEND((*ctx)->parsers, p);
	}

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_destroy(ina_xml_ctx_t **ctx)
{
    ina_xml_ctx_t *c = *ctx;
    ina_xml_parser_t *p, *ptmp;
    int cnt = 0;

    INA_ASSERT_NOTNULL(*ctx);

    DL_FOREACH_SAFE(c->parsers, p, ptmp) {
        rapidxml_parser_destroy(&p->doc);
        if (p->fctx != NULL) {
            ina_file_destroy(&p->fctx);
        }
        if (!(*ctx)->mp) {
            ina_mempool_release(p->elem_pool, INA_YES);
            ina_mempool_release(p->parse_pool, INA_YES);
        }
        DL_DELETE(c->parsers, p);
        if (!(*ctx)->mp) {
            ina_mem_free(p);
        }
        cnt++;
    }

    if (!(*ctx)->mp) {
        ina_mem_free(*ctx);
    }

    if (cnt != c->parser_pool_size) {
        /* FIXME: push proper error */
        return INA_FAILURE;
    }
    *ctx = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_parser_borrow(ina_xml_ctx_t *ctx, ina_xml_parser_t **p)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(p);

    /* we ran out of parsers */
    if (ctx->parsers == NULL) {
        *p = NULL;
        return INA_FAILURE;
    }

    /* return the head and delete from the list */
    *p = ctx->parsers;
    DL_DELETE(ctx->parsers, *p);

    rapidxml_parser_reset((*p)->doc);

    if (!ctx->mp) {
        ina_mempool_release((*p)->elem_pool, INA_NO);
        ina_mempool_release((*p)->parse_pool, INA_NO);
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_parser_release(ina_xml_ctx_t *ctx, ina_xml_parser_t **p)
{
    ina_xml_parser_t *parser;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(p);
    parser = *p;

    /* return parser */
    DL_APPEND(ctx->parsers, parser);
    *p = NULL;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_parser_execute(ina_xml_parser_t *p, ina_str_t source, ina_xml_elem_t **root)
{
    INA_ASSERT_NOTNULL(p);
    INA_ASSERT_NOTNULL(source);
    INA_ASSERT_NOTNULL(root);

	if (rapidxml_parser_exec(p->doc, ina_str_cstr(source)) > 0) {
		/* FIXME: proper error handling */
		return INA_FAILURE;
	}
	if (rapidxml_parser_root(p->doc, &p->root.elem) > 0) {
		/* FIXME: proper error handling */
		return INA_FAILURE;
	}
    p->root.p = p;
	*root = &p->root;
	
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_parser_execute_from_file(ina_xml_parser_t *p, 
                                                   const char *input_file, 
                                                   ina_xml_elem_t **root)
{
    ina_rc_t ret = INA_SUCCESS;
    ina_str_t source;
    ina_file_t *file;
    ina_file_stat_t *file_stat = NULL;
    uint64_t in_size = 0;
    size_t out_size = 0;
    int64_t total_size = 0;
    ina_file_cursor_t *fcur;
    const char *buffer = NULL;
    
    INA_ASSERT_NOTNULL(p);

    if (!INA_SUCCEED(ina_file_new(p->fctx, input_file, 
        INA_FILE_ACCESS_MODE_READ, 
        INA_FILE_CREATE_MODE_OPEN, 
        INA_FILE_SHARE_MODE_READ, 0, &file))) {
            ret = INA_ERR_PUSH_LAST;
            goto free;
    }
    if (!INA_SUCCEED(ina_file_stat_new(file, &file_stat))) {
        ret = INA_ERR_PUSH_LAST;
        goto free;
    }

    ina_file_stat_file_size(file_stat, &in_size);
    if (!INA_SUCCEED(ina_file_cursor_new_using_pool(file, 
        INA_FILE_CURSOR_TYPE_FILEIO, 
        INA_FILE_CURSOR_MODE_READWRITE_TEXT_CHUNK, 
        in_size, &fcur, NULL, p->parse_pool))) {
            ret = INA_ERR_PUSH_LAST;
            goto free;
    }
    while (1) {
        ina_file_cursor_text_read_chunk(fcur, (size_t)in_size, &out_size, &buffer);
        total_size += out_size;
        if (out_size < in_size) {
            break;
        } 
    }
    source = ina_str_new_fromblk_using_pool(buffer, strlen(buffer), p->parse_pool);
    ina_file_cursor_free(&fcur);

    if (!INA_SUCCEED(ina_xml_parser_execute(p, source, root))) {
        ret = INA_ERR_PUSH_LAST;
        goto free;
    }

free:
    if (file_stat != NULL) {
        ina_file_stat_free(file, &file_stat);
    }
    if (file != NULL) {
        ina_file_free(p->fctx, &file);
    }

    return ret;
}

INA_API(ina_rc_t) ina_xml_elem_first(ina_xml_elem_t *elem, ina_xml_elem_t **first)
{
    INA_ASSERT_NOTNULL(elem);
    INA_ASSERT_NOTNULL(first);

    *first = (ina_xml_elem_t*)ina_mempool_dalloc(elem->p->elem_pool, sizeof(ina_xml_elem_t));
    (*first)->p = elem->p;
	if (rapidxml_node_first(elem->elem, &(*first)->elem) > 0) {
		/* FIXME: proper error handling */
        *first = NULL;
		return INA_FAILURE;
	}
	
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_elem_next(ina_xml_elem_t *elem, ina_xml_elem_t **next)
{
    INA_ASSERT_NOTNULL(elem);
    INA_ASSERT_NOTNULL(next);

    *next = elem;
	if (rapidxml_node_next(elem->elem, &(*next)->elem) > 0) {
		/* FIXME: proper error handling */
        return INA_FAILURE;
	}
    
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_elem_name(ina_xml_elem_t *elem, const char **name, size_t *len)
{
    INA_ASSERT_NOTNULL(elem);
    INA_ASSERT_NOTNULL(name);
    INA_ASSERT_NOTNULL(len);

	if (rapidxml_node_get_name(elem->elem, name, len) > 0) {
		/* FIXME: proper error handling */
        return INA_FAILURE;
	}
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_elem_value(ina_xml_elem_t *elem, const char **name, size_t *len)
{
    INA_ASSERT_NOTNULL(elem);
    INA_ASSERT_NOTNULL(name);
    INA_ASSERT_NOTNULL(len);

	if (rapidxml_node_get_value(elem->elem, name, len) > 0) {
		/* FIXME: proper error handling */
		return INA_FAILURE;
	}
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_elem_attr_first(ina_xml_elem_t *elem, ina_xml_attr_t **first)
{
    INA_ASSERT_NOTNULL(elem);
    INA_ASSERT_NOTNULL(first);

	if (rapidxml_node_first_attribute(elem->elem, &elem->attr.attr) > 0) {
		/* FIXME: proper error handling */
		return INA_FAILURE;
	}
	*first = &elem->attr;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_attr_next(ina_xml_attr_t *attr, ina_xml_attr_t **next)
{
    INA_ASSERT_NOTNULL(attr);
    INA_ASSERT_NOTNULL(next);
	if (rapidxml_attribute_next(attr->attr, &attr->attr) > 0) {
		/* FIXME: proper error handling */
		return INA_FAILURE;
	}
	*next = attr;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_attr_name(ina_xml_attr_t *attr, const char **name, size_t *len)
{
    INA_ASSERT_NOTNULL(attr);
    INA_ASSERT_NOTNULL(name);
    INA_ASSERT_NOTNULL(len);

	if (rapidxml_attribute_get_name(attr->attr, name, len) > 0) {
		/* FIXME: proper error handling */
		return INA_FAILURE;
	}
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_xml_attr_value(ina_xml_attr_t *attr, const char **value, size_t *len)
{
    INA_ASSERT_NOTNULL(attr);
    INA_ASSERT_NOTNULL(value);
    INA_ASSERT_NOTNULL(len);

	if (rapidxml_attribute_get_value(attr->attr, value, len) > 0) {
		
		return INA_FAILURE;
	}
	return INA_SUCCESS;
}

