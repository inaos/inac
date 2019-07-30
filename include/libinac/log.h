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

#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>

/* Base log macros, user INA_LOG_DEBUG/INFO/WARNING/ERROR instead */
#ifdef INA_LOG_ENABLED
#define INA_LOG(log, level, fmt, ...) ina_log_write((log), level, INA_AT, fmt, ##__VA_ARGS__)
#else
#define INA_LOG(log, level, fmt, ...)
#endif

#ifndef INA_LOG_LEVEL
#define INA_LOG_LEVEL 3
#endif

#if INA_LOG_LEVEL>0
#define INA_LOG_RC(rc) \
    INA_LOG(NULL, INA_LOG_LEVEL_ERROR, "%s", ina_err_strerror((rc)))
#define INA_LOG_CTX_RC(ctx, rc) \
    INA_LOG(ctx, INA_LOG_LEVEL_ERROR, "%s", ina_err_strerror((rc)))

#define INA_LOG_ERROR(fmt, ...)                          \
    INA_LOG(NULL, INA_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define INA_LOG_CTX_ERROR(cat, fmt,...)                          \
    INA_LOG(ctx, INA_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#else
#define INA_LOG_ERROR(fmt,...)
#define INA_LOG_CTX_ERROR(ctx, fmt,...)
#define INA_LOG_RC(rc)
#define INA_LOG_CTX_RC(ctx, rc)
#endif
#if INA_LOG_LEVEL>1
#define INA_LOG_WARNING(fmt,...)                         \
    INA_LOG(NULL, INA_LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)
#define INA_LOG_CTX_WARNING(cat, fmt,...)                         \
    INA_LOG(ctx, INA_LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)
#else
#define INA_LOG_WARNING(fmt,...)
#define INA_LOG_CTX_WARNING(ctx,fmt,...)
#endif
#if INA_LOG_LEVEL>2
#define INA_LOG_INFO(fmt,...)                            \
    INA_LOG(NULL, INA_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define INA_LOG_CTX_INFO(ctx,fmt,...)                            \
    INA_LOG(ctx, INA_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#else
#define INA_LOG_INFO(fmt,...)
#define INA_LOG_CTX_INFO(ctx,fmt,...)
#endif
#if INA_LOG_LEVEL>3
#define INA_LOG_DEBUG(fmt,...)                          \
    INA_LOG(NULL, INA_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define INA_LOG_CTX_DEBUG(ctx,fmt,...)                          \
    INA_LOG(ctx, INA_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#else
#define INA_LOG_DEBUG(cat,fmt,...)
#define INA_LOG_CTX_DEBUG(ctx,fmt,...)
#endif

/* Log level */
typedef enum ina_log_level_e {
    INA_LOG_LEVEL_DEBUG   = 1,
    INA_LOG_LEVEL_INFO    = 2,
    INA_LOG_LEVEL_WARNING = 4,
    INA_LOG_LEVEL_ERROR   = 8
} ina_log_level_t;

/* Log target */
typedef enum ina_log_target_e {
    INA_LOG_STDOUT,
    INA_LOG_STDERR,
    INA_LOG_FILE,
#ifndef INA_OS_WINDOWS
    INA_LOG_SYSLOG,
    INA_LOG_PIPELINE
#endif
} ina_log_target_t;

/* Log category */
typedef struct ina_log_ctx_s ina_log_ctx_t;

/*
 * Initialize logging facility.
 *
 * Parameters
 *  cfg  Logging configuration. If NULL default configuration is
 *       applied which writes DEBUG,INFO,WARNING to stdout and
 *       ERROR to stderr.
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_log_init(const char* cfg);

/*
 * Initialize logging facility from file.
 *
 * Parameters
 *  cfg_filepath  Path to the configuration file. If NULL
 *                is passed, an attempt is made to open a
 *                configuration file with the same name as the
 *                executable file and the extension .conf.
 */
INA_API(ina_rc_t) ina_log_init_from_file(const char* cfg_filepath);


/*
 * Destroy the logging facility.
 */
INA_API(void) ina_log_destroy(void);
/*
 * Open a log context based on a log configuration.
 *
 * Parameters
 *  category  log category
 *  log       Where to store the newly created log
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_log_ctx_new(const char* category, ina_log_ctx_t **ctx);
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
INA_API(ina_rc_t) ina_log_write(const ina_log_ctx_t *ctx,
                          ina_log_level_t level,
                          const char *location,
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
INA_API(ina_rc_t) ina_log_write_v(const ina_log_ctx_t *cxt, ina_log_level_t level,
                            const char* location, const char* fmt, va_list ap);

/*
 * Close a log context.
 *
 * Parameters
 *  log  Log context to close.
 */
INA_API(void) ina_log_ctx_free(ina_log_ctx_t **ctx);

#ifdef __cplusplus
}
#endif

#endif
