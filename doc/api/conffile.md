
```C
#ifndef _LIBINAC_CONFFILE_H_
```

Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
typedef enum ina_conffile_value_type_e {
```
Availables value types
```C
typedef struct ina_conffile_entries_s ina_conffile_entries_t;/* Configuration file section, can be named or unnamed */
```
Configuration file entry
```C
typedef struct ina_conffile_s ina_conffile_t;
```
Configuration file data
```C
typedef ina_rc_t (*ina_conffile_section_cb_t)(const char *section_name,
```

Callback for section procession, called by ina_conffile_processs()


**Parameters**
 - `section_name`: Name of the current processing section.
 - `section_key`: Key of current named section, NULL for unamed sections.
 - `entries`: 
ina_conffile_get_string_from_section() or
ina_conffile_get_number_from_section() for retrieve values
from section entries.
 - `user_data`: Pointer to user data passed in ina_conffile_process()



**Return**

Returning other than INA_SUCCESS will stop the configuration file
processing.


```C
INA_API(ina_rc_t) ina_conffile_new(ina_conffile_t **cf);
```

Initialize a configuration file.


**Parameters**
 - `cf`: Pointer to configuration file pointer



**Return**

INA_SUCCESS if no error occured.


```C
INA_API(ina_rc_t) ina_conffile_add_section(ina_conffile_t *cf, const char *name,
```

Add a section to the configuration file. A section can be named or unnamned.


**Parameters**
 - `cf`: 
 - `name`:  Section name
 - `required`: INA_YES to define a required section, otherwise INA_NO.
 - `named`: INA_YES to mark the section as a named section, otherwise
INA_NO.
 - `cb`: 
entries should not be processed.
 - `section`: Pointer to an section pointer. Contains the newly created
section or NULL if any error occurred.



**Return**

INA_SUCCESS if section was created successfully.


```C
INA_API(ina_rc_t) ina_conffile_add_key(ina_conffile_section_t *section,
```

Add a value key to a configuration section.


**Parameters**
 - `section`:  Configuration file section
 - `name`: 
 - `value_type`: Define the type of value bind the key. A value can be a string
(INA_CONFFILE_VALUE_TYPE_STRING) or a number
(INA_CONFFILE_VALUE_TYPE_NUMBER)
 - `required`: Mark an value key as required by passing INA_YES. By passing
INA_NO value key is marked as optional.



**Return**

INA_SUCCESS if value key was successfully added.


```C
INA_API(ina_rc_t) ina_conffile_has_value(ina_conffile_t *cf,
```

Query if a value with key and section exists in a configuration file.


**Parameters**
 - `cf`: 
 - `section_name`: Section name
 - `section_key`: Section key for named section, NULL for unamed section
 - `key`: 



**Return**

INA_SUCCESS  Value exists
INA_FAILURE  Value doesn't exists


```C
INA_API(ina_rc_t) ina_conffile_get_string(ina_conffile_t *cf,
```

Get a string value for section and key from a configuration file.


**Parameters**
 - `cf`: 
 - `section_name`: Section name
 - `section_key`: Section key for named section, NULL for unamed section
 - `key`: 
 - `value`: 



**Return**

INA_SUCCESS Value found
INA_FAILURE Value not found


```C
INA_API(ina_rc_t) ina_conffile_get_number(ina_conffile_t *cf,
```

Get a number value for section and key from a configuration file.


**Parameters**
 - `cf`: 
 - `section_name`: Section name
 - `section_key`: Section key for named section, NULL for unnamed section
 - `key`: 
 - `value`: 



**Return**

INA_SUCCESS Value found
INA_FAILURE Value not found


```C
INA_API(ina_rc_t) ina_conffile_has_value_in_entries(ina_conffile_entries_t *entries,
```

Query if a value with key exists in a section. Use this function in
a section processing callback.


**Parameters**
 - `entries`: Section entries obtained from callback
 - `key`:  Key for value



**Return**

INA_SUCCESS  Value exists
INA_FAILURE  Value doesn't exists


```C
INA_API(ina_rc_t) ina_conffile_get_string_from_entries(
```

Get a string value from a section. Use this function in a section processing
callback.


**Parameters**
 - `entries`: Section entries obtained from callback
 - `key`:  Key for value
 - `value`: Output string containing the value



**Return**

INA_SUCCESS  Value found
INA_FAILURE  Value not found


```C
INA_API(ina_rc_t) ina_conffile_get_number_from_entries(
```

Get a number value from a section. Use this function in a section processing
callback.


**Parameters**
 - `entries`: Section entries obtained from callback
 - `key`:  Key for value
 - `value`: Output double containing the value



**Return**

INA_SUCCESS  Value found
INA_FAILURE  Value not found


```C
INA_API(ina_rc_t) ina_conffile_process(ina_conffile_t *cf, const char *filepath, void *user_data);
```

Process a configuration file.


**Parameters**
 - `cf`: 
 - `filepath`: Absolute or relative file path. If filepath is NULL the config-
uration file must be located in the working directory and named
[binary-name].conf.
 - `user_data`: Pointer to user defined data. this pointer is passed as third argument
ib the section callback.



**Return**

INA_SUCCESS if no error occurred.


```C
INA_API(void) ina_conffile_free(ina_conffile_t **cf);
```

Destroy a confiuration file.


**Parameters**
 - `cf`: Pointer of a configuration file pointer.


```C
#define INA_CONFFILE_STRING_KEY(name, required) \
```

Add a string value key to the configuration file.


**Parameters**
 - `name`: 
 - `required`: boolean


```C
#define INA_CONFFILE_NUMBER_KEY(name, required) \
```

Add a number value key to the configuration file.


**Parameters**
 - `name`:  string
 - `required`: boolean


```C
#define INA_CONFFILE_SECTION(name, required, handler, ...) \
```

Add an unnamed section to the configuration file.


**Parameters**
 - `name`:  string
 - `required`: boolean
 - `handler`: callback
 - `...`: 
macros for adding key values


```C
#define INA_CONFFILE_NAMED_SECTION(name, required, handler, ...) \
```

Add a named section to the configuration file.


**Parameters**
 - `name`:  string
 - `required`: boolean
 - `handler`: callback
 - `...`: 
value keys to the section


```C
#define INA_CONFFILE(cf, fp, ud, ...)                     \
```

Define configuration file using the standard pattern.


**Parameters**
 - `cf`: Pointer to a configuration file. NULL if it's not intended to use
the configuration values after processing the configuration file
Nested INA_CONFFILE_SECTION or INA_CONFFILE_NAMED_SECTION to add
named or unnamed section to the configuration file.
 - `fp`: Path to the configfile or NULL
 - `ud`: Pointer to user data or NULL

