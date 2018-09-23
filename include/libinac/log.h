/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_LOG_H_
#define _LIBINAC_LOG_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Base log macros, user INA_LOG_DEBUG/INFO/WARNING/ERROR instead */
#ifdef INA_LOG_ENABLED
#define INA_LOG(cfg, level, fmt,  ...) ina_log(cfg, level, fmt, ##__VA_ARGS__)
#else
#define INA_LOG(cfg, level, ...)
#endif

#ifndef INA_LOG_LEVEL
#define INA_LOG_LEVEL 3
#endif

#if INA_LOG_LEVEL>0
#define INA_LOG_ERROR(cfg,fmt,...)                          \
    INA_LOG(cfg, INA_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)       
#else
#define INA_LOG_ERROR(cfg,fmt,...)
#endif
#if INA_LOG_LEVEL>1
#define INA_LOG_WARNING(cfg,fmt,...)                         \
    INA_LOG(cfg, INA_LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)   
#else
#define INA_LOG_WARNING(cfg,fmt,...)
#endif
#if INA_LOG_LEVEL>2
#define INA_LOG_INFO(cfg,fmt,...)                            \
    INA_LOG(cfg, INA_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#else
#define INA_LOG_INFO(cfg,fmt,...)
#endif
#if INA_LOG_LEVEL>3
#define INA_LOG_DEBUG(cfg,fmt,...)                          \
    INA_LOG(cfg, INA_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#else
#define INA_LOG_DEBUG(cfg,fmt,...)
#endif

/* Log level */
typedef enum ina_log_level_e {
    INA_LOG_LEVEL_DEBUG,
    INA_LOG_LEVEL_INFO,
    INA_LOG_LEVEL_WARNING,
    INA_LOG_LEVEL_ERROR
} ina_log_level_t;

/* Log target */
typedef enum ina_log_target_e {
    INA_LOG_STDOUT = 0x0001,
    INA_LOG_FILE = 0x0002,
#ifndef WIN32
    INA_LOG_SYSLOG = 0x0004
#endif
} ina_log_target_t;

/* Log context/configuration */
typedef struct ina_log_cfg_s {
    FILE *fp1;
    FILE *fp2;
    ina_log_level_t level;
    int target;
    ina_str_t logfile;
    ina_str_t syslog_ident;
    int syslog_facility;
    int pid;
} ina_log_cfg_t;

/*
 * Open a log context  based on a log configuration.
 *
 * Parameters
 *  cfg      Where to store the newly created log context
 *  target   Defines log targets
 *  level    Defines log level
 *  logfile  Path to log file, relevant if target INA_LOG_FILE is requested
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_log_new(ina_log_cfg_t **cfg,
                               int32_t target,
                               ina_log_level_t level,
                               const char *logfile);

/*
 * Log a  message to current targets and level.
 *
 * Parameters
 *  cfg    Log context
 *  level  Level for message
 *  fmt    Format of log message
 *  ...    Argument for log message
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_log(const ina_log_cfg_t *cfg,
                          ina_log_level_t level,
                          const char* fmt,
                          ...);

/*
 * Log a message to current targets and level. Variable list version.
 *
 * Parameters
 *  cfg    Log context
 *  level  Level for message
 *  fmt    Format of log message
 *  ap     Variable argument list
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_log_v(const ina_log_cfg_t *cfg, ina_log_level_t level, 
                            const char* fmt, va_list ap);

/*
 * Close a log context.
 *
 * Parameters
 *  cfg  Log context to close.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_log_free(ina_log_cfg_t **cfg);

#ifdef __cplusplus
}
#endif

#endif
