/*
 * Copyright (c) 2013-2015, INAOS GmbH
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

/* Opaque type representing a JSON parser */
typedef struct ina_json_parser_s  ina_json_parser_t;
/* Opaque type representing a JSON generator */
typedef struct ina_json_gen_s     ina_json_gen_t;

/* JSON context */
typedef struct ina_json_ctx_s {
    int32_t parser_pool_size;     /* Pool size for pre-allocated parser */
    int32_t generator_pool_size;  /* Pool size for pre-allocated generators */
    ina_json_parser_t *parsers;   /* Pre-allocated parsers */
    ina_json_gen_t *generators;   /* Pre-allocated generators */
} ina_json_ctx_t;

/* Parsing events */
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

/* Data contaniner for a single parse event */
typedef struct ina_json_data_s {
    ina_json_parse_event_t event;
    size_t size;
    union {
        const unsigned char *s;
        int32_t b;
        int64_t i;
        double  d;
    } value;
} ina_json_data_t;

/*
 * Initialize a JSON context by preallocating parser and generators. Use 
 * borrow and release function to get access to them. The pool size is  
 * intended to be fixed.
 *
 * Parameters:
 * ctx                 Pointer to a context pointer to hold created JSON 
 *                     context.
 * parser_pool_size    Fixed size of parser pool
 * generator_pool_size Fixed size of generator pool
 *
 * Return:
 * INA_SUCCESS if no error occurred.
 */
INA_API(ina_rc_t) ina_json_init(ina_json_ctx_t **ctx, 
                                uint32_t parser_pool_size,
                                uint32_t generator_pool_size);

/*
 * Destroy a JSON context. Destroy fails if not all prevouslly borrowed
 * parser and/or generators are released.
 *
 * Parameters:
 * ctx  JSON context to destroy
 *
 * Return:
 * INA_SUCCESS if no error occurred.
 * RC INA_ELOGIC returned if not all parser and/or generators are released.
 */
INA_API(ina_rc_t) ina_json_destroy(ina_json_ctx_t **ctx);

/*
 * Borrow a parser from the context parser pool.
 *
 * Parameters:
 * ctx      JSON context
 * parser   Pointer to a parser pointer.
 *
 * Return:
 * INA_SUCCESS if no error occurred.
 * RC INA_ECAPAC returned if no more parser is avaiable.
 */
INA_API(ina_rc_t) ina_json_parser_borrow(ina_json_ctx_t *ctx, 
                                         ina_json_parser_t **parser);
/*
 * Release a previouslly borrowed parser.
 *
 * ctx      JSON context
 * parser   Parser to release.
 *
 * Return:
 * INA_SUCCESS
 * RC INA_EINVAL is returned if parser is already released.
 */
INA_API(ina_rc_t) ina_json_parser_release(ina_json_ctx_t *ctx, 
                                          ina_json_parser_t **parser);
/*
 * Start parsing over a buffer.
 *
 * Parameters:
 * parser       Parser to use for parsing.
 * buffer       Input buffer
 * len          Buffer length to parse
 * complete     Indicate whenever parsing is complete (INA_YES). This allow
 *              stream parsing (not yet implemented).
 *
 * Return:
 * INA_SUCCESS if no error occured.
 * RC INA_EOVRFL if stack size is exceeded.
 * RC INA_EINVAL if buffer dosen't contains valid JSON data.
 */
INA_API(ina_rc_t) ina_json_parser_execute(ina_json_parser_t *parser, 
                                          const unsigned char *buffer, 
                                          size_t buf_len,
                                          int32_t complete);
/*
 * Get next JSON parser data element. A data element contains information for
 * a single parsing event.
 *
 * Parameters:
 * parser   Parser
 * data     Pointer to JSON data element.
 * 
 * Return:
 * INA_SUCCESS if no error occurred.
 * RC INA_EEMPTY if no more data available.
 */
INA_API(ina_rc_t) ina_json_parser_try_data(ina_json_parser_t *parser, 
                                           const ina_json_data_t **data);
/*
 * Reset a parser for reuse. 
 *
 * Parameters:
 * parser      Parser to reset
 *
 * Return:
 * INA_SUCCESS if no error occurred. 
 */
INA_API(ina_rc_t) ina_json_parser_reset(ina_json_parser_t *parser);


/*
 * Borrow a generator from the context generator pool.
 *
 * Parameters:
 * ctx      JSON context
 * parser   Pointer to a generator pointer.
 *
 * Return:
 * INA_SUCCESS if no error occurred.
 * RC INA_ECAPAC returned if no more generator is avaiable.
 */
INA_API(ina_rc_t) ina_json_generator_borrow(ina_json_ctx_t *ctx, 
                                            ina_json_gen_t **generator);
/*
 * Release a previouslly borrowed parser.
 *
 * ctx      JSON context
 * parser   Parser to release.
 *
 * Return: 
 * INA_SUCCESS
 * RC INA_EINVAL is returned if parser is already released.
 */                     
INA_API(ina_rc_t) ina_json_generator_release(ina_json_ctx_t *ctx, 
                                             ina_json_gen_t **generator);
                                             
/*
 * Reset a generator for reuse. 
 *
 * Parameters:
 * parser      Generator to reset
 *
 * Return:
 * INA_SUCCESS if no error occurred. 
 */
INA_API(ina_rc_t) ina_json_generator_reset(ina_json_gen_t *generator);

/*
 * Add start object marker to the outbut buffer.
 *
 * Parameters:
 * generator    Generator instance
 *
 * Return:
 * INA_SUCCESS if no error occured.
 * RC INA_ELOGIC if operation failed. 
 */ 
INA_API(ina_rc_t) ina_json_generator_start_object(ina_json_gen_t *generator);

/*
 * Add end object marker to the outbut buffer.
 *
 * Parameters:
 * generator    Generator instance
 *
 * Return:
 * INA_SUCCESS if no error occured.
 * RC INA_ELOGIC if operation failed. 
 */ 
INA_API(ina_rc_t) ina_json_generator_end_object(ina_json_gen_t *generator);

/*
 * Add start array marker to the outbut buffer.
 *
 * Parameters:
 * generator    Generator instance
 *
 * Return:
 * INA_SUCCESS if no error occured.
 * RC INA_ELOGIC if operation failed. 
 */ 
INA_API(ina_rc_t) ina_json_generator_start_array(ina_json_gen_t *generator);

/*
 * Add end array marker to the outbut buffer.
 *
 * Parameters:
 * generator    Generator instance
 *
 * Return:
 * INA_SUCCESS if no error occured.
 * RC INA_ELOGIC if operation failed. 
 */ 
INA_API(ina_rc_t) ina_json_generator_end_array(ina_json_gen_t *generator);

/*
 * Add null value to the outbut buffer.
 *
 * Parameters:
 * generator    Generator instance
 *
 * Return:
 * INA_SUCCESS if no error occured.
 * RC INA_ELOGIC if operation failed. 
 */ 
INA_API(ina_rc_t) ina_json_generator_add_null(ina_json_gen_t *generator);

/*
 * Add boolean value to the outbut buffer.
 *
 * Parameters:
 * generator    Generator instance
 * value        Boolean value. Use INA_YES or INA_NO 
 *
 * Return:
 * INA_SUCCESS if no error occured.
 * RC INA_ELOGIC if operation failed. 
 */ 
INA_API(ina_rc_t) ina_json_generator_add_boolean(ina_json_gen_t *generator, 
                                                 int32_t value);
/*
 * Add integer value to the outbut buffer.
 *
 * Parameters:
 * generator    Generator instance
 * value        Integer value to add. 
 *
 * Return:
 * INA_SUCCESS if no error occured.
 * RC INA_ELOGIC if operation failed. 
 */ 
INA_API(ina_rc_t) ina_json_generator_add_integer(ina_json_gen_t *generator, 
                                                  int64_t value);
/*
 * Add double value to the outbut buffer.
 *
 * Parameters:
 * generator    Generator instance
 * value        Double value to add.
 *
 * Return:
 * INA_SUCCESS if no error occured.
 * RC INA_ELOGIC if operation failed. 
 */ 
INA_API(ina_rc_t) ina_json_generator_add_double(ina_json_gen_t *generator, 
                                                double value);

/*
 * Add string value to the outbut buffer.
 *
 * Parameters:
 * generator    Generator instance
 * str          Input string buffer
 * len          Length of string buffer to add.
 *
 * Return:
 * INA_SUCCESS if no error occured.
 * RC INA_ELOGIC if operation failed. 
 */ 
INA_API(ina_rc_t) ina_json_generator_add_string(ina_json_gen_t *generator, 
                                                const char *str, 
                                                size_t len);

/*
 * Get access the generator buffer.
 *
 * Parameters:
 * generator    Generator instance
 * buffer       Pointer to hold the buffer pointer.
 * size         Contains the used size of the gerenrator buffer. 
 */
INA_API(ina_rc_t) ina_json_generator_get_buffer(ina_json_gen_t *generator, 
                                                const unsigned char **buffer,
                                                size_t *len);


#ifdef __cplusplus
}
#endif

#endif
