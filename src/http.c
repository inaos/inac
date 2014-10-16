/*
 * Copyright (c) 2013-2014, INAOS GmbH
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

#include <contribs/http-parser/http_parser.h>

/* keep the initial size bigger then the alloc size for the pool to work */
#define __INA_HTTP_INITIAL_HEADER_POOL_SIZE 16
#define __INA_HTTP_INITIAL_HEADER_ALLOC_SIZE 8

struct ina_http_url_s {
	const char *begin;
	size_t len;
	struct http_parser_url iu;
} ina_http_url_s;

struct ina_http_header_s {
	uint32_t key;
	const char *field_begin;
	size_t field_len;
	const char *value_begin;
	size_t value_len;
	UT_hash_handle hh;
	struct ina_http_header_s *prev;
	struct ina_http_header_s *next;
} ina_http_header_s;

struct ina_http_parser_s {
	int finished;
	int ipt; /* internal http_parser_type */
	struct http_parser intp; /* internal parser */
	ina_http_url_t url;
	ina_http_header_t *headers;
	ina_http_header_t *cur_header;
	ina_http_header_t *itr;
	ina_http_header_t *header_pool;
	int header_pool_size;
	int headers_used;
	const char *payload_ptr;
	size_t payload_len;
	struct http_parser_settings settings;
	struct ina_http_parser_s *prev;
	struct ina_http_parser_s *next;
} ina_http_parser_s;

static int __ina_http_on_body(http_parser *parser, const char *at, size_t length)
{
	ina_http_parser_t *p; 

	INA_ASSERT_NOTNULL(parser);
	p = (ina_http_parser_t*)parser->data;
	INA_ASSERT_NOTNULL(p);

	p->payload_ptr = at;
	p->payload_len = length;

	return 0;
}

static int __ina_http_on_headers_complete(http_parser *parser)
{
	return 0;
}

static int __ina_http_on_header_field(http_parser *parser, const char *at, size_t length)
{
	ina_http_header_t *h;
	ina_http_parser_t *p;

	INA_ASSERT_NOTNULL(parser);
	p = (ina_http_parser_t*)parser->data;
	INA_ASSERT_NOTNULL(p);

	/* check and get a header from pool */
	if (p->headers_used == p->header_pool_size) {
		int i;
		for (i = 0; i < __INA_HTTP_INITIAL_HEADER_ALLOC_SIZE; i++) {
			ina_http_header_t *h = (ina_http_header_t*)ina_mem_alloc(sizeof(ina_http_header_t));
			h->field_begin = NULL;
			h->field_len = 0;
			h->value_begin = NULL;
			h->value_len = 0;
			DL_APPEND(p->header_pool, h);
			p->header_pool_size++;
		}
	}
	h = p->header_pool;
	DL_DELETE(p->header_pool, h);
	p->headers_used++;

	/* put header to hash-map */
	h->field_begin = at;
	h->field_len = length;
	h->key = ina_util_hash_sdbm(0, h->field_begin, h->field_len);
	HASH_ADD_INT(p->headers, key, h);

	/* set current header */
	p->cur_header = h;

	return 0;
}

static int __ina_http_on_header_value(http_parser *parser, const char *at, size_t length)
{
	ina_http_header_t *h;
	ina_http_parser_t *p;

	INA_ASSERT_NOTNULL(parser);
	p = (ina_http_parser_t*)parser->data;
	INA_ASSERT_NOTNULL(p);
	h = p->cur_header;

	h->value_begin = at;
	h->value_len = length;

	return 0;
}

static int __ina_http_on_message_begin(http_parser *parser)
{
	return 0;
}

static int __ina_http_on_message_complete(http_parser *parser)
{
    ina_http_parser_t *p;
    INA_ASSERT_NOTNULL(parser);
    p = (ina_http_parser_t*)parser->data;
    INA_ASSERT_NOTNULL(p);

    p->finished = 1;

	return 0;
}

static int __ina_http_on_status_complete(http_parser *parser)
{
	return 0;
}

static int __ina_http_on_url(http_parser *parser, const char *at, size_t length)
{
	ina_http_parser_t *p; 
	INA_ASSERT_NOTNULL(parser);
	p = (ina_http_parser_t*)parser->data;
	INA_ASSERT_NOTNULL(p);

	p->url.begin = at;
	p->url.len = length;

	return 0;
}

INA_API(ina_rc_t) ina_http_init(ina_http_ctx_t **ctx, ina_http_parser_type_t parser_type, int parser_pool_size)
{
	int i,z;
	INA_ASSERT_NOTNULL(ctx);

	*ctx = (ina_http_ctx_t*)ina_mem_alloc(sizeof(ina_http_ctx_t));
	if (*ctx == NULL) {
		return INA_ERR_PUSH_LAST;
	}
	(*ctx)->parser_pool_size = parser_pool_size;
	(*ctx)->parsers = NULL;

	for (i = 0; i < parser_pool_size; i++) {
		ina_http_parser_t *p = (ina_http_parser_t*)ina_mem_alloc(sizeof(ina_http_parser_t));
		if (p == NULL) {
			return INA_ERR_PUSH_LAST;
		}
		
		p->settings.on_body = __ina_http_on_body;
		p->settings.on_headers_complete = __ina_http_on_headers_complete;
		p->settings.on_header_field = __ina_http_on_header_field;
		p->settings.on_header_value = __ina_http_on_header_value;
		p->settings.on_message_begin = __ina_http_on_message_begin;
		p->settings.on_message_complete = __ina_http_on_message_complete;
		p->settings.on_status_complete = __ina_http_on_status_complete;
		p->settings.on_url = __ina_http_on_url;

		switch (parser_type) {
			case INA_HTTP_PARSER_TYPE_REQUEST:
				p->ipt = HTTP_REQUEST;
				break;
			case INA_HTTP_PARSER_TYPE_RESPONSE:
				p->ipt = HTTP_RESPONSE;
				break;
			case INA_HTTP_PARSER_TYPE_BOTH:
				p->ipt = HTTP_BOTH;
				break;
		}

		p->headers = NULL;
		p->header_pool = NULL;
		p->header_pool_size = __INA_HTTP_INITIAL_HEADER_POOL_SIZE;
		p->headers_used = 0;
		for (z = 0; z < p->header_pool_size; z++) {
			ina_http_header_t *h = (ina_http_header_t*)ina_mem_alloc(sizeof(ina_http_header_t));
			h->field_begin = NULL;
			h->field_len = 0;
			h->value_begin = NULL;
			h->value_len = 0;
			DL_APPEND(p->header_pool, h);
		}

		p->finished = 0;
		p->intp.data = p;

		DL_APPEND((*ctx)->parsers, p);
	}

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_destroy(ina_http_ctx_t **pctx)
{
	ina_http_parser_t *p, *ptmp;
	ina_http_header_t *h, *htmp;
	int cnt = 0;

	INA_ASSERT_NOTNULL(pctx);
	INA_ASSERT_NOTNULL(*pctx);

	DL_FOREACH_SAFE((*pctx)->parsers, p, ptmp) {
		DL_FOREACH_SAFE(p->header_pool, h, htmp) {
			DL_DELETE(p->header_pool, h);
			ina_mem_free(h);
		}
		DL_DELETE((*pctx)->parsers, p);
		ina_mem_free(p);
		cnt++;
	}

	if (cnt != ctx->parser_pool_size) {
		/* push error parser leak */
		return INA_FAILURE;
	}

	ina_mem_free(*pctx);
	*pctx = NULL;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_parser_borrow(ina_http_ctx_t *ctx, ina_http_parser_t **p)
{
	INA_ASSERT_NOTNULL(ctx);

	/* we ran out of parsers */
	if (ctx->parsers == NULL) {
		*p = NULL;
		return INA_FAILURE;
	}

	/* return the head and delete from the list */
	*p = ctx->parsers;
	INA_ASSERT_NOTNULL(p);
	DL_DELETE(ctx->parsers, *p);

	http_parser_init(&(*p)->intp, (*p)->ipt);

	(*p)->finished = 0;

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_parser_release(ina_http_ctx_t *ctx, ina_http_parser_t **parser)
{
	ina_http_parser_t *p;
	ina_http_header_t *h, *htmp;

	INA_ASSERT_NOTNULL(ctx);
	INA_ASSERT_NOTNULL(parser);
	p = *parser;
	INA_ASSERT_NOTNULL(p);

	/* clean-up headers
     * if we grew the header pool not a problem we adjust dynamically to the 
     * appropriate usage scanario for the parser
     */
	HASH_ITER(hh, p->headers, h, htmp) {
        h->field_begin = NULL;
		h->field_len = 0;
		h->value_begin = NULL;
		h->value_len = 0;
		DL_APPEND(p->header_pool, h);
		HASH_DELETE(hh, p->headers, h);
		p->headers_used--;
	}

	INA_ASSERT_EQUAL(0, p->headers_used);
	INA_ASSERT_EQUAL(0, HASH_COUNT(p->headers));

	/* return parser */
	DL_APPEND(ctx->parsers, p);
	parser = NULL;
	
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_parser_url_get(ina_http_parser_t *p, ina_http_url_t **url)
{
	INA_ASSERT_NOTNULL(p);
	INA_ASSERT_NOTNULL(url);

	if (!p->finished) {
		return INA_FAILURE;
	}
	if (http_parser_parse_url(p->url.begin, p->url.len, 0, &p->url.iu) != 0) {
		*url = NULL;
		return INA_FAILURE;
	}
	*url = &p->url;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_url_get_field(ina_http_url_t *url, uint16_t mask, const char **begin, uint16_t *len)
{
	INA_ASSERT_NOTNULL(url);

	if (url->iu.field_set & (1 << mask)) {
		*begin = url->begin + url->iu.field_data[mask].off;
		*len = url->iu.field_data[mask].len;
		return INA_SUCCESS;
	}
	return INA_FAILURE;
}

INA_API(ina_rc_t) ina_http_url_get_port(ina_http_url_t *url, uint16_t *port)
{
	INA_ASSERT_NOTNULL(url);
	INA_ASSERT_NOTNULL(port);

	*port = url->iu.port;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_parser_header_first(ina_http_parser_t *p, ina_http_header_t **first)
{
	INA_ASSERT_NOTNULL(p);
	INA_ASSERT_NOTNULL(first);

	if (!p->finished) {
		return INA_FAILURE;
	}
	p->itr = p->headers;
	*first = p->itr;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_parser_header_next(ina_http_parser_t *p, ina_http_header_t **next)
{
	INA_ASSERT_NOTNULL(p);
	INA_ASSERT_NOTNULL(next);

	if (!p->finished) {
		return INA_FAILURE;
	}

    p->itr = (ina_http_header_t*)p->itr->hh.next;
    *next = p->itr;
	
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_parser_header_by_name(ina_http_parser_t *p, const char *name, ina_http_header_t **header)
{
	uint32_t key;
	ina_http_header_t *f = NULL;

	INA_ASSERT_NOTNULL(p);
	INA_ASSERT_NOTNULL(name);
	INA_ASSERT_NOTNULL(header);

	if (!p->finished) {
		return INA_FAILURE;
	}
	key = ina_util_hash_sdbm(0, name, strlen(name));
	HASH_FIND_INT(p->headers, &key, f);
	*header = f;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_parser_header_get_field(ina_http_parser_t *p, ina_http_header_t *header, const char **begin, size_t *len)
{
	INA_ASSERT_NOTNULL(p);
	INA_ASSERT_NOTNULL(header);
	INA_ASSERT_NOTNULL(begin);
	INA_ASSERT_NOTNULL(len);

	if (!p->finished) {
		return INA_FAILURE;
	}

	*begin = header->field_begin;
	*len = header->field_len;

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_parser_header_get_value(ina_http_parser_t *p, ina_http_header_t *header, const char **begin, size_t *len)
{
	INA_ASSERT_NOTNULL(p);
	INA_ASSERT_NOTNULL(header);
	INA_ASSERT_NOTNULL(begin);
	INA_ASSERT_NOTNULL(len);

	if (!p->finished) {
		return INA_FAILURE;
	}

	*begin = header->value_begin;
	*len = header->value_len;

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_parser_payload_get(ina_http_parser_t *p, unsigned char **payload, size_t *payload_len)
{
	INA_ASSERT_NOTNULL(p);
	INA_ASSERT_NOTNULL(payload);
	INA_ASSERT_NOTNULL(payload_len);

	if (!p->finished) {
		return INA_FAILURE;
	}

	*payload = (unsigned char*)p->payload_ptr;
	*payload_len = p->payload_len;

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_parser_status_code(ina_http_parser_t *p, unsigned short *status)
{
	INA_ASSERT_NOTNULL(p);
	INA_ASSERT_NOTNULL(status);

	if (!p->finished) {
		return INA_FAILURE;
	}
	*status = p->intp.status_code;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_parser_method(ina_http_parser_t *p, int *method)
{
	INA_ASSERT_NOTNULL(p);
	INA_ASSERT_NOTNULL(method);

	if (!p->finished) {
		return INA_FAILURE;
	}
	*method = p->intp.method;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_parser_httpversion(ina_http_parser_t *p, unsigned short *major, unsigned short *minor)
{
	INA_ASSERT_NOTNULL(p);
	INA_ASSERT_NOTNULL(major);
	INA_ASSERT_NOTNULL(minor);

	if (!p->finished) {
		return INA_FAILURE;
	}

	*major = p->intp.http_major;
	*minor = p->intp.http_minor;

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_parser_execute(ina_http_parser_t *p, const char *in, size_t inlen, int *more)
{
    size_t nread;

	INA_ASSERT_NOTNULL(p);
	INA_ASSERT_NOTNULL(in);
	INA_ASSERT_NOTNULL(more);
	
	nread = http_parser_execute(&p->intp, &p->settings, in, inlen);

    if (nread != inlen) {
        return INA_FAILURE;
    }

    if (p->finished) {
	    *more = 0;
    }
    else {
        *more = 1;
    }

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_parser_eof(ina_http_parser_t *p)
{
    size_t nread;
    INA_ASSERT_NOTNULL(p);

    nread = http_parser_execute(&p->intp, &p->settings, NULL, 0);
    if (nread != 0) {
        return INA_FAILURE;
    }

    INA_TEST_ASSERT_EQUAL_INTEGER(1, p->finished);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_http_parser_should_keep_alive(ina_http_parser_t *p, int *should_keep_alive)
{
	INA_ASSERT_NOTNULL(p);
	INA_ASSERT_NOTNULL(should_keep_alive);
	
	*should_keep_alive = http_should_keep_alive(&p->intp);
	return INA_SUCCESS;
}

