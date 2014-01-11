/*
 * Copyright (c) 2012-2014, INAOS GmbH
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
#ifndef _LIBINAC_LOG_H_
#define _LIBINAC_LOG_H_

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
 * Open a log context for based on a log configuration
 */                          
INA_API(ina_rc_t) ina_log_open(ina_log_cfg_t **cfg, int32_t target, 
                               ina_log_level_t level, const char *logfile);

/*
 * Log a message to current targets and level.
 */
INA_API(ina_rc_t) ina_log(const ina_log_cfg_t *cfg, ina_log_level_t level, 
                          const char* fmt, ...);

/*
 * Close a log context
 */
INA_API(ina_rc_t) ina_log_close(ina_log_cfg_t **cfg);

#ifdef __cplusplus
}
#endif

#endif
