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

/* Availables value types */
typedef enum ina_conffile_value_type_e {
    INA_CONFFILE_VALUE_TYPE_STRING = 1, 
    INA_CONFFILE_VALUE_TYPE_NUMBER,
} ina_conffile_value_type_t;

typedef struct ina_conffile_entry_s ina_conffile_entry_t;
/* Config file section, can be namen or unnamed */
typedef struct ina_conffile_section_s ina_conffile_section_t;

/* Config file data */
typedef struct ina_conffile_s {
    ina_str_t filepath;                /* file path */
    ina_ljit_ctx_t *lctx;              /* LuaJIT context */
    ina_conffile_section_t *sections;  /* Holds all sections */
    int prepared;                      /* INA_YES if prepared */
} ina_conffile_t;

/* Callback sections  */
typedef ina_rc_t (*ina_conffile_section_cb_t)(const char *section_name, 
                                              const char *section_key,
                                              ina_conffile_entry_t *entries);

/*
 * Initialize a config file.
 *
 * Parameters
 * cf        Address of an config file pointer
 * filepath  Absolute or relaive file path. If filepath is NULL the config
 *
 */
INA_API(ina_rc_t) ina_conffile_init(ina_conffile_t **cf, const char *filepath);

/*
 * Add a section.
 *
 */
INA_API(ina_rc_t) ina_conffile_add_section(ina_conffile_t *cf, const char *name, 
                    int required, int named, ina_conffile_section_cb_t cb, 
                    ina_conffile_section_t **section);
/*
 * Add a value key to a configuration section.
 *
 */
INA_API(ina_rc_t) ina_conffile_add_key(ina_conffile_section_t *section, 
                    const char *name, ina_conffile_value_type_t value_type, 
                    int required);
/*
 * Query if a value with key and section exists
 */ 
INA_API(ina_rc_t) ina_conffile_has_value(ina_conffile_t *cf, 
                    const char *section_name, const char *section_key, 
                    const char* key);
/*
 * Get a string value for section and key
 */
INA_API(ina_rc_t) ina_conffile_get_string(ina_conffile_t *cf, 
                    const char *section_name, const char *section_key, 
                    const char* key, ina_str_t *value);

/*
 * Get a number value for section an key
 */
INA_API(ina_rc_t) ina_conffile_get_number(ina_conffile_t *cf, 
                    const char *section_name, const char *section_key, 
                    const char* key, double *value);
/*
 *
 *
 */
INA_API(ina_rc_t) ina_conffile_has_value_in_entries(ina_conffile_entry_t *entries, 
                    const char* key);

/*
 *
 *
 */
INA_API(ina_rc_t) ina_conffile_get_string_from_entries(ina_conffile_entry_t *entries, 
                    const char* key, ina_str_t *value);
/*
 *
 *
 */
INA_API(ina_rc_t) ina_conffile_get_number_from_entries(ina_conffile_entry_t *entries, 
                    const char* key, double *value);

/*
 *
 *
 */
INA_API(ina_rc_t) ina_conffile_prepare(ina_conffile_t *cf);
/*
 *
 *
 */
INA_API(ina_rc_t) ina_conffile_process(ina_conffile_t *cf);

/*
 *
 *
 */
INA_API(ina_rc_t) ina_conffile_destroy(ina_conffile_t **cf);


#define INA_CONFFILE_STRING_KEY(name, required) \
ina_conffile_add_key(__cs, name, INA_CONFFILE_VALUE_TYPE_STRING, required)

#define INA_CONFFILE_NUMBER_KEY(name, required) \
ina_conffile_add_key(__cs, name, INA_CONFFILE_VALUE_TYPE_NUMBER, required)

#define INA_CONFFILE_SECTION(name, required, handler, ...) \
ina_conffile_add_section(__cf, name, required, INA_NO, handler, &__cs); \
__VA_ARGS__

#define INA_CONFFILE_NAMED_SECTION(name, required, handler, ...) \
ina_conffile_add_section(__cf, name, required, INA_YES, handler, &__cs); \
__VA_ARGS__

#define INA_CONFFILE(cf,...)                             \
    ina_conffile_t *__cf = NULL;                         \
    ina_conffile_section_t *__cs = NULL;                 \
    if (cf != NULL) __cf = cf;                           \
    if (!INA_SUCCEED(ina_conffile_init(&__cf, NULL))) {  \
        abort();                                         \
    }                                                    \
    __VA_ARGS__

#ifdef __cplusplus
}
#endif 

#endif

