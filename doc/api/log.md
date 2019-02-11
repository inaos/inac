

---

```C
#ifndef _LIBINAC_LOG_H_
#define _LIBINAC_LOG_H_

```

Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.


---

```C
#ifdef INA_LOG_ENABLED
#define INA_LOG(cfg, level, INA_AT, fmt,  ...) ina_log(cfg, level, INA_AT, fmt, ##__VA_ARGS__)
#else
#define INA_LOG(cfg, level, ...)
#endif

```
Base log macros, user INA_LOG_DEBUG/INFO/WARNING/ERROR instead

---

```C
typedef enum ina_log_level_e {
    INA_LOG_LEVEL_DEBUG   = 1,
    INA_LOG_LEVEL_INFO    = 2,
    INA_LOG_LEVEL_WARNING = 4,
    INA_LOG_LEVEL_ERROR   = 8
} ina_log_level_t;
```
Log level

---

```C
typedef enum ina_log_target_e {
    INA_LOG_STDOUT,
    INA_LOG_STDERR,
    INA_LOG_FILE,
#ifndef INA_OS_WIN32
    INA_LOG_SYSLOG,
    INA_LOG_PIPELINE
#endif
} ina_log_target_t;
```
Log target

---

```C
typedef struct ina_log_s ina_log_t;
```
Log context/configuration

---

```C
INA_API(ina_rc_t) ina_log_new(const char* category, ina_log_t **log);
```

Open a log context  based on a log configuration.


**Parameters**
 - `category`: log category
 - `log`: Where to store the newly created log



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_log(const ina_log_t *log,
                          ina_log_level_t level,
                          const char *location,
                          const char* fmt,
                          ...);
```

Log a  message to current targets and level.


**Parameters**
 - `cfg`: Log context
 - `level`: Level for message
 - `fmt`: Format of log message
 - `...`: Argument for log message



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_log_v(const ina_log_t *log, ina_log_level_t level,
                            const char* location, const char* fmt, va_list ap);
```

Log a message to current targets and level. Variable list version.


**Parameters**
 - `cfg`: Log context
 - `level`: Level for message
 - `fmt`: Format of log message
 - `ap`: Variable argument list



**Return**

INA_SUCCESS



---

```C
INA_API(void) ina_log_free(ina_log_t **log);
```

Close a log context.


**Parameters**
 - `log`: Log context to close.

