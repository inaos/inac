/*
 * Copyright (c) 2016, INAOS GmbH
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
#ifndef _LIBINAC_REGEXP_H_
#define _LIBINAC_REGEXP_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque regexp handle */
typedef struct ina_regex_s ina_regex_t;

/* regexp match */
typedef struct ina_regex_match_s {
   size_t  offset;
   size_t  len;
} ina_regexp_match_t;


/*
 * Creates new compiled regular expression.
 *
 * Parameters
 *  pattern   Regular expression pattern
 *  regex     Where to store the newly created regular expression
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_regex_new(const char *pattern, ina_regex_t **regex);

/*
 * Destroy a regular expression.
 *
 * Parameters
 *  regex  Regular expression to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_regex_free(ina_regext_t **regex);


/*
 * Exeucte a regular expression.
 *
 * Parameters
 *  regex   Comiled regular expression.
 *  string  Input string
 *  nmatch  Max. number of possibile matches
 *  matches Where to store the matches
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_regex_exec(const ina_regex_t *regex,
                                 const char string,
                                 size_t nmatch,
                                 ina_regex_match_t matches[]);



#ifdef __cplusplus
}
#endif 

#endif

