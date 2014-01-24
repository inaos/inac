/*
 * Copyright (c) 2013-2014, INAOS GmbH
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
#include <libinac/lib.h>
#include "config.h"

static ina_rc_t __ina_init(ina_log_cfg_t*);
static ina_rc_t __ina_log(const ina_log_cfg_t*, ina_log_level_t, ina_str_t);

INA_API(ina_rc_t) ina_log(const ina_log_cfg_t *cfg, ina_log_level_t level, const char* fmt, ...)
{
    va_list ap;
    ina_rc_t rc;

    INA_ASSERT_NOTNULL(cfg);

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
 
    INA_ASSERT_NOTNULL(cfg);
    INA_ASSERT_NOTNULL(fmt);
    INA_ASSERT_TRUE(strlen(fmt));

    if ((level&0xff) < cfg->level) {
        return INA_SUCCESS;
    }

    ina_str_vsnprintf(&msg, ina_str_size(msg)-1, fmt, ap);
   
    return __ina_log(cfg, level, msg);
}

INA_API(ina_rc_t) ina_log_open(ina_log_cfg_t **cfg, int32_t target, 
                                ina_log_level_t level, const char  *logfile)
{
    *cfg = (ina_log_cfg_t*)ina_mem_alloc(sizeof(ina_log_cfg_t));
    (*cfg)->fp1 = NULL;
    (*cfg)->fp2 = NULL;
    (*cfg)->logfile = ina_str_new_fromcstr(logfile);
    (*cfg)->target = target;
    (*cfg)->level = level;
    (*cfg)->syslog_facility = 0;
    (*cfg)->syslog_ident = NULL;
    return __ina_init(*cfg);
}

INA_API(ina_rc_t) ina_log_close(ina_log_cfg_t **cfg)
{   
    INA_ASSERT_NOTNULL(*cfg);

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
    }
#ifdef WIN32
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

    time_t now = time(NULL);

    strftime(buf,sizeof(buf),"%d %b %H:%M:%S",localtime(&now));

    if (cfg->fp1 != NULL) {
        fprintf(cfg->fp1,"[%d] %s %c %s\n", cfg->pid, buf, c[level], msg);
        fflush(cfg->fp1);
    }
    if (cfg->fp2 != NULL) {
        fprintf(cfg->fp2,"[%d] %s %c %s\n", cfg->pid, buf, c[level], msg);
        fflush(cfg->fp2);
    }
#ifndef INA_OS_WIN32
    if (cfg->target == INA_LOG_SYSLOG) {
        syslog(cfg->syslog_facility, "%s", msg);
    }
#endif
    return INA_SUCCESS;
}
