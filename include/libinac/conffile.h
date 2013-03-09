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
#ifndef _LIBINAC_CONFFILE_H_
#define _LIBINAC_CONFFILE_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * INAOS Configuration-File Management 
 */

typedef enum ina_conffile_value_type_e {
	INA_CONFFILE_VALUE_TYPE_STRING = 1,
	INA_CONFFILE_VALUE_TYPE_NUMBER,
} ina_conffile_value_type_t;

typedef struct ina_conffile_entry_s ina_conffile_entry_t;
typedef struct ina_conffile_section_s ina_conffile_section_t;

typedef ina_rc_t (*section_callback)(ina_conffile_entry_t *entries);
typedef ina_rc_t (*named_section_callback)(const char *section_name, ina_conffile_entry_t *entries);

/*
 *
 *
 */
INA_API(ina_rc_t) ina_conffile_init(void);
/*
 *
 *
 */
INA_API(ina_rc_t) ina_conffile_add_section(const char *name, int required, 
	section_callback cb, ina_conffile_section_t **section);
/*
 *
 *
 */
INA_API(ina_rc_t) ina_conffile_add_named_section(const char *name, int required, 
	named_section_callback cb, ina_conffile_section_t **section);
/*
 *
 *
 */
INA_API(ina_rc_t) ina_conffile_add_key(ina_conffile_section_t *section, const char *name, 
	ina_conffile_value_type_t value_type, int required);
/*
 *
 *
 */
INA_API(ina_rc_t) ina_conffile_has_value(ina_conffile_entry_t *entries, const char* key, int *has_value);
/*
 *
 *
 */
INA_API(ina_rc_t) ina_conffile_get_string(ina_conffile_entry_t *entries, const char* key, ina_str_t *value);
/*
 *
 *
 */
INA_API(ina_rc_t) ina_conffile_get_number(ina_conffile_entry_t *entries, const char* key, double *value);
/*
 *
 *
 */
INA_API(ina_rc_t) ina_conffile_process(int pos, char **argv);
/*
 *
 *
 */
INA_API(ina_rc_t) ina_conffile_destroy(void);

#ifdef __cplusplus
}
#endif 

#endif

