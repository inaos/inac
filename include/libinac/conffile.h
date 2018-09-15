/*
 * Copyright (c) 2013-2018, INAOS GmbH
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



#ifdef __cplusplus
extern "C" {
#endif

    #include <libinac/lib.h>

/* Availables value types */
typedef enum ina_conffile_value_type_e {
    INA_CONFFILE_VALUE_TYPE_STRING = 1, 
    INA_CONFFILE_VALUE_TYPE_NUMBER,
} ina_conffile_value_type_t;

/* Configuration file entry */
typedef struct ina_conffile_entries_s ina_conffile_entries_t;
/* Configuration file section, can be named or unnamed */
typedef struct ina_conffile_section_s ina_conffile_section_t;
/* Configuration file data */
typedef struct ina_conffile_s ina_conffile_t;

/* 
 * Callback for section procession, called by ina_conffile_processs() 
 *
 * Parameters
 *  section_name  Name of the current processing section.
 *  section_key   Key of current named section, NULL for unamed sections.
 *  entries       Section entries, see ina_conffile_has_value_in_entries(),
 *               ina_conffile_get_string_from_section() or
 *               ina_conffile_get_number_from_section() for retrieve values
 *               from section entries.
 *
 * Return
 *  Returning other than INA_SUCCESS will stop the configuration file
 *  processing.
 */
typedef ina_rc_t (*ina_conffile_section_cb_t)(const char *section_name, 
                                              const char *section_key,
                                              ina_conffile_entries_t *entries);

/*
 * Initialize a configuration file.
 *
 * Parameters
 * cf  Pointer to configuration file pointer
 *
 * Return
 * INA_SUCCESS if no error occured.
 */
INA_API(ina_rc_t) ina_conffile_new(ina_conffile_t **cf);

/*
 * Add a section to the configuration file. A section can be named or unnamned.
 *
 * Parameters
 *  cf        Configuration file
 *  name      Section name
 *  required  INA_YES to define a required section, otherwise INA_NO.
 *  named     INA_YES to mark the section as a named section, otherwise
 *            INA_NO.
 *  cb        Callback to process the entries for that section or NULL if
 *            entries should not be processed.
 *  section   Pointer to an section pointer. Contains the newly created
 *            section or NULL if any error occurred.
 *
 * Return
 *  INA_SUCCESS if section was created successfully.
 */
INA_API(ina_rc_t) ina_conffile_add_section(ina_conffile_t *cf, const char *name,
                                           int required, int named,
                                           ina_conffile_section_cb_t cb,
                                           ina_conffile_section_t **section);

/*
 * Add a value key to a configuration section.
 *
 * Parameters
 *  section      Configuration file section
 *  name         Name of value key
 *  value_type   Define the type of value bind the key. A value can be a string
 *               (INA_CONFFILE_VALUE_TYPE_STRING) or a number
 *               (INA_CONFFILE_VALUE_TYPE_NUMBER)
 *  required     Mark an value key as required by passing INA_YES. By passing
 *               INA_NO value key is marked as optional.
 *
 * Return
 *  INA_SUCCESS if value key was successfully added.
 */
INA_API(ina_rc_t) ina_conffile_add_key(ina_conffile_section_t *section, 
                    const char *name, ina_conffile_value_type_t value_type, 
                    int required);
/*
 * Query if a value with key and section exists in a configuration file.
 *
 * Parameters
 *  cf            Pointer to a configuration file.
 *  section_name  Section name
 *  section_key   Section key for named section, NULL for unamed section
 *  key           Name of value key
 *
 * Return
 *  INA_SUCCESS  Value exists
 *  INA_FAILURE  Value doesn't exists
 */ 
INA_API(ina_rc_t) ina_conffile_has_value(ina_conffile_t *cf, 
                    const char *section_name, const char *section_key, 
                    const char* key);

/*
 * Get a string value for section and key from a configuration file.
 *
 * Parameters
 *  cf            Configuration file
 *  section_name  Section name
 *  section_key   Section key for named section, NULL for unamed section
 *  key           Name of value key
 *  value         Output string containing the value
 *
 * Return
 *  INA_SUCCESS Value found
 *  INA_FAILURE Value not found
 */
INA_API(ina_rc_t) ina_conffile_get_string(ina_conffile_t *cf, 
                    const char *section_name, const char *section_key, 
                    const char* key, const ina_str_t *value);

/*
 * Get a number value for section and key from a configuration file.
 *
 * Parameters
 *  cf            Configuration file
 *  section_name  Section name
 *  section_key   Section key for named section, NULL for unnamed section
 *  key           Name of value key
 *  value         Output double containing the number value
 *
 * Return
 *  INA_SUCCESS Value found
 *  INA_FAILURE Value not found
 */
INA_API(ina_rc_t) ina_conffile_get_number(ina_conffile_t *cf,
                                          const char *section_name,
                                          const char *section_key,
                                          const char* key,
                                          double *value);

/*
 * Query if a value with key exists in a section. Use this function in 
 * a section processing callback.
 *
 * Parameters
 *  entries  Section entries obtained from callback
 *  key      Key for value
 *
 * Return
 *  INA_SUCCESS  Value exists
 *  INA_FAILURE  Value doesn't exists
 */
INA_API(ina_rc_t) ina_conffile_has_value_in_entries(ina_conffile_entries_t *entries,
                                                    const char* key);

/*
 * Get a string value from a section. Use this function in a section processing
 * callback.
 *
 * Parameters
 *  entries  Section entries obtained from callback
 *  key      Key for value
 *  value    Output string containing the value
 *
 * Return
 *  INA_SUCCESS  Value found
 *  INA_FAILURE  Value not found
 */
INA_API(ina_rc_t) ina_conffile_get_string_from_entries(
        ina_conffile_entries_t *entries,
                                                const char* key,
                                                const ina_str_t *value);
/*
 * Get a number value from a section. Use this function in a section processing
 * callback.
 *
 * Parameters
 *  entries  Section entries obtained from callback
 *  key      Key for value
 *  value    Output double containing the value
 *
 * Return
 *  INA_SUCCESS  Value found
 *  INA_FAILURE  Value not found
 */
INA_API(ina_rc_t) ina_conffile_get_number_from_entries(
        ina_conffile_entries_t *entries,
                                const char* key,
                                double *value);

/*
 * Process a configuration file.
 *
 * Parameters
 *  cf         Configuration file
 *  filepath   Absolute or relative file path. If filepath is NULL the config-
 *             uration file must be located in the working directory and named
 *             [binary-name].conf.
 *
 * Return
 *  INA_SUCCESS if no error occured.
 */
INA_API(ina_rc_t) ina_conffile_process(ina_conffile_t *cf, const char *filepath);

/*
 * Destroy a confiuration file.
 *
 * Parameters
 *  cf  Pointer of a configuration file pointer.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_conffile_free(ina_conffile_t **cf);

/*
 *  Add a string value key to the configuration file.
 *  
 *  Parameters
 *   name        string   Name of value key
 *   required    boolean  Define if value is required or optional
 */
#define INA_CONFFILE_STRING_KEY(name, required) \
ina_conffile_add_key(__cs, name, INA_CONFFILE_VALUE_TYPE_STRING, required)

/*
 * Add a number value key to the configuration file.
 *
 * Parameters
 *  name      string     Name of value key
 *  required  boolean    Define if value is required or optional
 */
#define INA_CONFFILE_NUMBER_KEY(name, required) \
ina_conffile_add_key(__cs, name, INA_CONFFILE_VALUE_TYPE_NUMBER, required)

/* 
 * Add an unnamed section to the configuration file.
 *
 * Parameters
 *  name      string     Section name
 *  required  boolean    Define if the section is required or optional
 *  handler   callback   Callback for section handling
 *  ...       Nested INA_CONFFILE_STRING_KEY or INA_CONFFILE_NUMBER key
 *            macros for adding key values
 */
#define INA_CONFFILE_SECTION(name, required, handler, ...) \
ina_conffile_add_section(__cf, name, required, INA_NO, handler, &__cs); \
__VA_ARGS__

/*
 * Add a named section to the configuration file.
 *
 * Parameters
 *  name      string    Section name
 *  required  boolean   Define if the section is required or optional
 *  handler   callback  Callback to handle section
 *  ...       INA_CONFFILE_STRING_KEY or INA_CONFFILE_NUMBER_KEY to add
 *            value keys to the section
 */
#define INA_CONFFILE_NAMED_SECTION(name, required, handler, ...) \
ina_conffile_add_section(__cf, name, required, INA_YES, handler, &__cs); \
__VA_ARGS__

/*
 * Define configuration file using the standard pattern.
 *
 * Parameters
 *  cf  Pointer to a configuration file. NULL if it's not intended to use
 *      the configuration values after processing the configuration file
 *      Nested INA_CONFFILE_SECTION or INA_CONFFILE_NAMED_SECTION to add
 *      named or unnamed section to the configuration file.
 */
#define INA_CONFFILE(cf, fp, ...)                         \
do                                                        \
{                                                         \
    ina_conffile_t *__cf = NULL;                          \
    ina_conffile_section_t *__cs = NULL;                  \
    if (cf != NULL) __cf = cf;                            \
    if (!INA_SUCCEED(ina_conffile_new(&__cf)))        {   \
        exit(EXIT_FAILURE);                               \
    }                                                     \
    __VA_ARGS__;                                          \
    if (!INA_SUCCEED(ina_conffile_process(__cf, fp)))   { \
        exit(EXIT_FAILURE);                               \
    }                                                     \
    if (cf == NULL) {                                     \
        cf = __cf;                                        \
    } else {                                              \
        ina_conffile_free(&__cf);                         \
    }                                                     \
} while(0)

#ifdef __cplusplus
}
#endif 

#endif

