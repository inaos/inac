/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include "config.h"

static ina_rc_t __ina_init(ina_log_cfg_t*);
static ina_rc_t __ina_log(const ina_log_cfg_t*, ina_log_level_t, ina_str_t);

INA_API(ina_rc_t) ina_log(const ina_log_cfg_t *cfg, ina_log_level_t level, const char* fmt, ...)
{
    va_list ap;
    ina_rc_t rc;

    INA_VERIFY_NOT_NULL(cfg);
    INA_VERIFY_NOT_NULL(fmt);

    va_start(ap, fmt);
    rc = ina_log_v(cfg, level, fmt, ap);
    va_end(ap);

    return rc;
}

INA_API(ina_rc_t) ina_log_v(const ina_log_cfg_t *cfg, ina_log_level_t level, 
                           const char* fmt, va_list ap)
{
    static ina_str_t msg = NULL;
    
    if (!msg) {
        msg = ina_str_new(1024);
    }
 
    INA_VERIFY_NOT_NULL(cfg);
    INA_VERIFY_NOT_NULL(fmt);
    INA_VERIFY(strlen(fmt));

    if ((level&0xff) < cfg->level) {
        return INA_SUCCESS;
    }

    ina_str_vsnprintf(&msg, ina_str_size(msg)-1, fmt, ap);
   
    return __ina_log(cfg, level, msg);
}

INA_API(ina_rc_t) ina_log_new(ina_log_cfg_t **cfg, int32_t target,
                                ina_log_level_t level, const char  *logfile)
{
    INA_VERIFY_NOT_NULL(cfg);
    *cfg = (ina_log_cfg_t*)ina_mem_alloc(sizeof(ina_log_cfg_t));
    INA_RETURN_IF_NULL(*cfg);

    (*cfg)->fp1 = NULL;
    (*cfg)->fp2 = NULL;
    (*cfg)->logfile = ina_str_new_fromcstr(logfile);
    (*cfg)->target = target;
    (*cfg)->level = level;
#ifdef INA_OS_WIN32
    (*cfg)->syslog_facility = 0;
    (*cfg)->syslog_ident = NULL;
#else
    switch (level) {
        case INA_LOG_LEVEL_INFO:
            (*cfg)->syslog_facility = LOG_UPTO(LOG_NOTICE);
            break;
        case INA_LOG_LEVEL_WARNING:
            (*cfg)->syslog_facility = LOG_UPTO(LOG_WARNING);
            break;
        case INA_LOG_LEVEL_ERROR:
            (*cfg)->syslog_facility = LOG_UPTO(LOG_ERR);
            break;
        default:
            (*cfg)->syslog_facility = LOG_UPTO(LOG_DEBUG);
            break;
    }
    if (logfile == NULL) {
        (*cfg)->syslog_ident = ina_str_new_fromcstr(ina_app_get_name());
    } else {
        const char* basename = strrchr(logfile, INA_PATH_SEPARATOR);

        if (basename) {
            (*cfg)->syslog_ident = ina_str_new_fromcstr(++basename);
        } else {
            (*cfg)->syslog_ident = ina_str_new_fromcstr(logfile);
        }
    }
#endif
    return __ina_init(*cfg);
}

INA_API(ina_rc_t) ina_log_free(ina_log_cfg_t **cfg)
{   
    INA_VERIFY_NOT_NULL(cfg);
    INA_VERIFY_NOT_NULL(*cfg);

    if ((*cfg)->fp2 != NULL) {
        fclose((*cfg)->fp2);
    }
    if ((*cfg)->logfile != NULL) {
        ina_str_free((*cfg)->logfile);
    }
    if ((*cfg)->syslog_ident != NULL) {
        ina_str_free((*cfg)->syslog_ident);
    }
    *cfg = NULL;
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_init(ina_log_cfg_t *cfg)
{
    if ((cfg->target & INA_LOG_STDOUT) == INA_LOG_STDOUT) {
        cfg->fp1 = stdout;    
    }
    if ((cfg->target & INA_LOG_FILE) == INA_LOG_FILE) {
        cfg->fp2 = (cfg->logfile == NULL) ? stdout : fopen(ina_str_cstr(cfg->logfile),"a");
        if (cfg->fp2 == NULL) {
            return INA_OS_ERROR(INA_NN_FILE|INA_ERR_OPEN);
        }
    }
#ifdef INA_OS_WIN32
    cfg->pid = (int)GetCurrentProcessId();
#else
    cfg->pid = (int)getpid();
#endif
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_log(const ina_log_cfg_t *cfg, ina_log_level_t level, ina_str_t msg) {
    static const char *c = ".-*#";
    static char buf[64];
    static struct tm *lt;
#ifdef INA_OS_LINUX
    static struct tm rtm;
#endif

    time_t now = time(NULL);

#ifdef INA_OS_LINUX
    lt = localtime_r(&now, &rtm);
#else
    lt = localtime(&now);
#endif
    INA_ASSERT_NOTNULL(lt);
    
    strftime(buf, sizeof(buf),"%d %b %H:%M:%S", lt);

    if (cfg->fp1 != NULL) {
        fprintf(cfg->fp1,"[%d] %s %c %s\n", cfg->pid, buf, c[level], msg);
        fflush(cfg->fp1);
    }
    if (cfg->fp2 != NULL) {
        fprintf(cfg->fp2,"[%d] %s %c %s\n", cfg->pid, buf, c[level], msg);
        fflush(cfg->fp2);
    }
#ifndef INA_OS_WIN32
    if (cfg->target&INA_LOG_SYSLOG) {
        setlogmask(cfg->syslog_facility);
        openlog(cfg->syslog_ident, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
        switch (level) {
            case INA_LOG_LEVEL_DEBUG:
                syslog(LOG_DEBUG, "%s", msg);
                break;
            case INA_LOG_LEVEL_INFO:
                syslog(LOG_INFO, "%s", msg);
                break;
            case INA_LOG_LEVEL_WARNING:
                syslog(LOG_WARNING, "%s", msg);
                break;
            case INA_LOG_LEVEL_ERROR:
                syslog(LOG_ERR, "%s", msg);
                break;
        }
        closelog();
    }
 #endif
    return INA_SUCCESS;
}
