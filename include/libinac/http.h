/*
 * Copyright (c) 2013-2016, INAOS GmbH
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

/* Parser instance type */
typedef enum ina_http_parser_type_e {
    INA_HTTP_PARSER_TYPE_REQUEST,
    INA_HTTP_PARSER_TYPE_RESPONSE,
    INA_HTTP_PARSER_TYPE_BOTH
} ina_http_parser_type_t;

/* Supported HTTP methods */
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

/* Supported URL scheme fields */
typedef enum http_parser_url_fields_e {
    INA_HTTP_PARSER_UF_SCHEMA    = 0,
    INA_HTTP_PARSER_UF_HOST      = 1,
    INA_HTTP_PARSER_UF_PORT      = 2,
    INA_HTTP_PARSER_UF_PATH      = 3,
    INA_HTTP_PARSER_UF_QUERY     = 4,
    INA_HTTP_PARSER_UF_FRAGMENT  = 5,
    INA_HTTP_PARSER_UF_USERINFO  = 6,
    INA_HTTP_PARSER_UF_MAX       = 7
} http_parser_url_fields_t;

/* Opaque parser handle */
typedef struct ina_http_parser_s ina_http_parser_t;

/* Opaque URL container */
typedef struct ina_http_url_s ina_http_url_t;

/* Opaque HTTP header struct */
typedef struct ina_http_header_s ina_http_header_t;

/* HTTP context */
typedef struct ina_http_ctx_s {
    int parser_pool_size;
    ina_http_parser_t *parsers;
} ina_http_ctx_t;

/*
 * Create and initialize a new HTTP context.
 *
 * Parameters
 *  ctx          Where to store the newly created context
 *  parser_type  Define type of parser to preallocate
 *  parser_pool_size  Number of parser_type to preallocate
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_http_init(ina_http_ctx_t **ctx,
                                ina_http_parser_type_t parser_type,
                                int parser_pool_size);

/*
 * Destroy a HTTP context. Release and free parser pool.
 *
 * Parameters
 *  ctx  HTTP context to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_http_destroy(ina_http_ctx_t **ctx);

/*
 * Borrow a parser from the context parser pool. A borrowed parser must be
 * released after usage by calling ina_http_parser_release()
 *
 * Parameters
 *  ctx  HTTP context
 *  p    Where to store the borrowed parser. Is NULL if no more parser available
 *
 * Return
 *  INA_SUCCESS if all went well
 *  INA_FAILURE if no more parser available
 */
INA_API(ina_rc_t) ina_http_parser_borrow(ina_http_ctx_t *ctx,
                                         ina_http_parser_t **p);

/*
 * Release a previously borrowed parser back to the context pool.
 *
 * Parameters
 *  ctx  HTTP context
 *  p    Parser to release
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_http_parser_release(ina_http_ctx_t *ctx,
                                          ina_http_parser_t **p);

/*
 * Get URL from the last request.
 *
 * Parameters
 *  p    HTTP parser
 *  url  Where to store the URL
 *
 * Return
 *  INA_SUCCESS if all went well
 *  INA_FAILURE if request still on progress or URL could not be parsed.
 */
INA_API(ina_rc_t) ina_http_parser_url_get(ina_http_parser_t *p,
                                          ina_http_url_t **url);

/*
 * Get a scheme field part from a URL.
 *
 * Parameters
 *  url    URL
 *  mask   Field selection mask
 *  begin  Field start offset
 *  len    field length
 *
 * Return
 *  INA_SUCCESS if field exists
 *  INA_FAILURE if requested field doesn't exists
 */
INA_API(ina_rc_t) ina_http_url_get_field(ina_http_url_t *url,
                                         uint16_t mask,
                                         const char **begin,
                                         uint16_t *len);

/*
 * Get port number from URL
 *
 * Parameters
 *  url   URL
 *  port  Where to store the port number
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_http_url_get_port(ina_http_url_t *url, uint16_t *port);

/*
 * Get the first header from a HTTP request.
 *
 * Parameters
 *  p      HTTP parser
 *  first  Where to store the first header. Is NULL if no headers found.
 *
 * Return
 *  INA_SUCCESS if all went well
 *  INA_FAILURE if request still in progress
 */
INA_API(ina_rc_t) ina_http_parser_header_first(ina_http_parser_t *p,
                                               ina_http_header_t **first);

/*
 * Get the next available header from a request.
 *
 * Parameters
 *  p     HTTP parser
 *  next  Where to store the header. Is NULL if last header was read.
 *
 * Return
 *  INA_SUCCESS if all went well
 *  INA_FAILURE if request ist not completed
 *
 */
INA_API(ina_rc_t) ina_http_parser_header_next(ina_http_parser_t *p,
                                              ina_http_header_t **next);
/*
 * Get a header by name.
 *
 * Parameters
 *  p       HTTP parser
 *  name    Name of header
 *  header  Where to store the header. Is NULL if header with name not found
 *
 * Return
 *  INA_SUCCESS if all went well
 *  INA_FAILURE if request not completed
 */
INA_API(ina_rc_t) ina_http_parser_header_by_name(ina_http_parser_t *p,
                                                 const char *name,
                                                 ina_http_header_t **header);

/*
 * Get header field, name of header.
 *
 * Parameters
 *  p       HTTP parser
 *  header  HTTP header
 *  begin   Where to store the start address of field
 *  len     Where to store the length of field
 *
 * Return
 *  INA_SUCCESS if all went well
 *  INA_FAILURE if the request is not completed
 *
 * FIXME: We don't need to pass the parser as argument
 */
INA_API(ina_rc_t) ina_http_parser_header_get_field(ina_http_parser_t *p,
                                                   ina_http_header_t *header,
                                                   const char **begin,
                                                   size_t *len);

/*
 * Get value of a header.
 *
 * Parameters
 *  p       HTTP parser
 *  header  HTTP header
 *  begin   Where to store the start address of value
 *  len     Where to store the length of value
 *
 * Return
 *  INA_SUCCESS if all went well
 *  INA_FAILURE if the request is not completed
 *
 * FIXME: We don't need to pass the parser as argument
 */
INA_API(ina_rc_t) ina_http_parser_header_get_value(ina_http_parser_t *p,
                                                   ina_http_header_t *header,
                                                   const char **begin,
                                                   size_t *len);

/*
 * Get raw HTTP payload.
 *
 * Parameters
 *  p            HTTP parser
 *  payload      Where to store the raw payload
 *  payload_len  Where to store the payload length
 *
 * Return
 *  INA_SUCCESS if all went
 *  INA_FAILURE if the request is not completed
 *
 */
INA_API(ina_rc_t) ina_http_parser_payload_get(ina_http_parser_t *p,
                                              unsigned char **payload,
                                              size_t *payload_len);

/*
 * Get HTTP status code of last request.
 *
 * Parameters
 *  p       HTTP parser
 *  status  Where to store the HTTP status
 *
 * Return
 *  INA_SUCCESS if all went well
 *  INA_FAILURE if the request is not completed
 */
INA_API(ina_rc_t) ina_http_parser_status_code(ina_http_parser_t *p,
                                              unsigned short *status);

/*
 * Get HTTP method of last request
 *
 * Parameters
 *  p       HTTP parser
 *  method  Were to store the HTTP method
 *
 * Return
 *  INA_SUCCESS if all went well
 *  INA_FAILURE if the request is not completed
 */
INA_API(ina_rc_t) ina_http_parser_method(ina_http_parser_t *p, int *method);

/*
 * Get HTTP protocol version of last request.
 *
 * Parameters
 *  p      HTTP parser
 *  major  Where to store the major version number
 *  minor  Where to store the minor version number
 *
 * Return
 *  INA_SUCCESS if all went well
 *  INA_FAILURE if the request is not completed
 */
INA_API(ina_rc_t) ina_http_parser_httpversion(ina_http_parser_t *p,
                                              unsigned short *major,
                                              unsigned short *minor);
/*
 * Process HTTP payload.
 *
 * Parameters
 *  p      HTTP parser
 *  in     Input payload
 *  inlen  Defines input payload length
 *  more   Where to store more indicator. INA_YES indicate more payload is
 *         needed to complete the request.
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_http_parser_execute(ina_http_parser_t *p,
                                          const char *in,
                                          size_t inlen,
                                          int *more);

/*
 * HTTP needs to know where the end of the stream is. For example, sometimes
 * servers send responses without Content-Length and expect the client to
 * consume input (for the body) until EOF. Use this function to notify about
 * EOF of a stream.
 *
 * Parameters
 *  p  HTTP parser
 *
 * Return
 *  INA_SUCCESS if all went well.
 */
INA_API(ina_rc_t) ina_http_parser_eof(ina_http_parser_t *p);

/*
 * If should_keep_alive flag is 0, then this should be the last message on the
 * connection.
 * If you are the server, respond with the "Connection: close" header.
 * If you are the client, close the connection.
 *
 * Parameters
 *  p                  HTTP parser
 *  should_keep_alive  Where to store keep alive flag.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_http_parser_should_keep_alive(ina_http_parser_t *p,
                                                    int *should_keep_alive)

#ifdef __cplusplus
}
#endif

#endif
