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

#undef TRUE
#define TRUE 1
#undef FALSE
#define FALSE 0

#define MAX_HEADERS 13
#define MAX_ELEMENT_SIZE 2048

struct message {
  const char *name; // for debugging purposes
  const char *raw;
  ina_http_parser_type_t type;
  ina_http_parser_method_t method;
  int status_code;
  char request_path[MAX_ELEMENT_SIZE];
  char request_url[MAX_ELEMENT_SIZE];
  char fragment[MAX_ELEMENT_SIZE];
  char query_string[MAX_ELEMENT_SIZE];
  char body[MAX_ELEMENT_SIZE];
  size_t body_size;
  const char *host;
  const char *userinfo;
  uint16_t port;
  int num_headers;
  enum { NONE=0, FIELD, VALUE } last_header_element;
  char headers [MAX_HEADERS][2][MAX_ELEMENT_SIZE];
  int should_keep_alive;

  const char *upgrade; // upgraded body

  unsigned short http_major;
  unsigned short http_minor;

  int body_is_final;
};

const struct message requests[] =
#define REQUEST_1 0
{ {"firefox get"
  ,"GET /favicon.ico HTTP/1.1\r\n"
         "Host: 0.0.0.0=5000\r\n"
         "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) Gecko/2008061015 Firefox/3.0\r\n"
         "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
         "Accept-Language: en-us,en;q=0.5\r\n"
         "Accept-Encoding: gzip,deflate\r\n"
         "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
         "Keep-Alive: 300\r\n"
         "Connection: keep-alive\r\n"
         "\r\n"
  ,INA_HTTP_PARSER_TYPE_REQUEST
  ,INA_HTTP_PARSER_METHOD_GET
  ,0
  ,"/favicon.ico"
  ,"/favicon.ico"
  ,""
  ,""
  ,""
  ,0
  ,""
  ,""
  ,0
  ,8
  ,0
  ,{ 
      { "Host", "0.0.0.0=5000" }
    , { "User-Agent", "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) Gecko/2008061015 Firefox/3.0" }
    , { "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" }
    , { "Accept-Language", "en-us,en;q=0.5" }
    , { "Accept-Encoding", "gzip,deflate" }
    , { "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7" }
    , { "Keep-Alive", "300" }
    , { "Connection", "keep-alive" }
   }
  ,TRUE
  ,""
  ,1
  ,1
  ,0
  }
, { NULL } /* sentinel */
};

const struct message responses[] =
#define RESPONSE_1 0
{ { "no carriage ret"
  ,"HTTP/1.1 200 OK\n"
         "Content-Type: text/html; charset=utf-8\n"
         "Connection: close\n"
         "\n"
         "these headers are from http://news.ycombinator.com/"
  ,INA_HTTP_PARSER_TYPE_RESPONSE
  ,INA_HTTP_PARSER_METHOD_GET
  ,200
  ,""
  ,""
  ,""
  ,""
  ,"these headers are from http://news.ycombinator.com/"
  ,0
  ,""
  ,""
  ,0
  ,2
  ,0
  ,{ {"Content-Type", "text/html; charset=utf-8" }
   , {"Connection", "close" }
   }
  ,FALSE
  ,""
  ,1
  ,1
  ,0
  }
, { NULL } /* sentinel */
};

void test_http_simple_req_resp()
{
	ina_http_ctx_t *ctx;
	ina_http_parser_t *parser;
	ina_http_url_t *url;
	int more = 1;
	int skal = 0;
	int met = 0;
	unsigned short vmj, vmi = 0;
	const char *begin;
	uint16_t ulen;
	size_t len;
	uint16_t port;
	ina_http_header_t *h;
	int z = 0;
	unsigned short status = 0;

	INA_TEST_ASSERT_SUCCEED(ina_http_init(&ctx, INA_HTTP_PARSER_TYPE_BOTH, 16));
	INA_TEST_ASSERT_SUCCEED(ina_http_parser_borrow(ctx, &parser));		
	
	INA_TEST_ASSERT_SUCCEED(ina_http_parser_execute(parser, requests[0].raw, strlen(requests[0].raw), &more));
	
	INA_TEST_ASSERT_SUCCEED(ina_http_parser_should_keep_alive(parser, &skal));
	INA_TEST_ASSERT_EQUAL(requests[0].should_keep_alive, skal);
	INA_TEST_ASSERT_SUCCEED(ina_http_parser_method(parser, &met));
	INA_TEST_ASSERT_EQUAL(requests[0].method, met);
	INA_TEST_ASSERT_SUCCEED(ina_http_parser_httpversion(parser, &vmj, &vmi));
	INA_TEST_ASSERT_EQUAL(requests[0].http_major, vmj);
	INA_TEST_ASSERT_EQUAL(requests[0].http_minor, vmi);
	
	INA_TEST_ASSERT_SUCCEED(ina_http_parser_url_get(parser, &url));
	INA_TEST_ASSERT_SUCCEED(ina_http_url_get_field(url, INA_HTTP_PARSER_UF_PATH, &begin, &ulen));
		
	INA_TEST_ASSERT(strncmp("/favicon.ico", begin, 12) == 0);
	INA_TEST_ASSERT_SUCCEED(ina_http_url_get_port(url, &port));
	INA_TEST_ASSERT_EQUAL(80, port);

	INA_TEST_ASSERT_SUCCEED(ina_http_parser_header_first(parser, &h));
	while (h != NULL) {
		INA_TEST_ASSERT_SUCCEED(ina_http_parser_header_get_field(parser, h, &begin, &len));
		INA_TEST_ASSERT(strncmp(requests[0].headers[z][0], begin, len) == 0);
		INA_TEST_ASSERT_SUCCEED(ina_http_parser_header_get_value(parser, h, &begin, &len));
		INA_TEST_ASSERT(strncmp(requests[0].headers[z][1], begin, len) == 0);
		z++;	
	}
	INA_TEST_ASSERT_SUCCEED(ina_http_parser_header_next(parser, &h));

	INA_TEST_ASSERT_SUCCEED(ina_http_parser_release(ctx, &parser));
	INA_TEST_ASSERT_SUCCEED(ina_http_parser_borrow(ctx, &parser));

	INA_TEST_ASSERT_SUCCEED(ina_http_parser_execute(parser, responses[0].raw, strlen(responses[0].raw), &more));

        INA_TEST_ASSERT_SUCCEED(ina_http_parser_should_keep_alive(parser, &skal));
	INA_TEST_ASSERT_EQUAL(responses[0].should_keep_alive, skal);	
	INA_TEST_ASSERT_SUCCEED(ina_http_parser_status_code(parser, &status));
	INA_TEST_ASSERT_EQUAL(responses[0].status_code, status);	

	INA_TEST_ASSERT_SUCCEED(ina_http_parser_release(ctx, &parser));
	INA_TEST_ASSERT_SUCCEED(ina_http_destroy(&ctx));	
}

