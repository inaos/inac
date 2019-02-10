
```C
#ifndef _LIBINAC_REGEX_H_
```

Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
/* Opaque regexp handle */
```

Links with ideas:
- Russ Cox on regex: https://swtch.com/~rsc/regexp/


```C
typedef struct ina_regex_match_s {
```
regexp match
```C
INA_API(ina_rc_t) ina_regex_new(const char *pattern, ina_regex_t **regex);
```

Creates new compiled regular expression.


**Parameters**
 - `pattern`: Regular expression pattern
 - `regex`: Where to store the newly created regular expression



**Return**

INA_SUCCESS if all went well


```C
INA_API(void) ina_regex_free(ina_regext_t **regex);
```

Destroy a regular expression.


**Parameters**
 - `regex`: Regular expression to free


```C
INA_API(ina_rc_t) ina_regex_exec(const ina_regex_t *regex,
```

Execute a regular expression.


**Parameters**
 - `regex`: Compiled regular expression.
 - `string`: Input string
 - `nmatch`: Max. number of possible matches
matches Where to store the matches



**Return**

INA_SUCCESS if all went well

