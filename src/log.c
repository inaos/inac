/*
 * Copyright (c) 2012, INAOS GmbH
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
static ina_rc_t __ina_log(const ina_log_cfg_t*, ina_log_level_t, const char *);

INA_API(ina_rc_t) ina_log(const ina_log_cfg_t *cfg, ina_log_level_t level, const char* fmt, ...)
{
    va_list ap;
    char msg[2048];
    INA_ASSERT_NOTNULL(cfg);
    INA_ASSERT_NOTNULL(fmt);

    if ((level&0xff) < cfg->level) {
        return INA_SUCCESS;
    }

    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    return __ina_log(cfg, level, msg);
}

INA_API(ina_rc_t) ina_log_open(ina_log_cfg_t **cfg, ina_log_target_t target, 
                                ina_log_level_t level)
{
    *cfg = (ina_log_cfg_t*)ina_mem_alloc(sizeof(ina_log_cfg_t));
    (*cfg)->fp = NULL;
    (*cfg)->logfile = NULL;
    (*cfg)->target = target;
    (*cfg)->level = level;
    (*cfg)->syslog_facility = 0;
    (*cfg)->syslog_ident = NULL;
    return __ina_init(*cfg);
}

INA_API(ina_rc_t) ina_log_close(ina_log_cfg_t **cfg)
{   
    INA_ASSERT_NOTNULL(*cfg);

    if ((*cfg)->fp != NULL && (*cfg)->target == INA_LOG_FILE) {
        fclose((*cfg)->fp);
    }
    if ((*cfg)->logfile != NULL) {
        ina_str_destroy((*cfg)->logfile);
    }
    if ((*cfg)->syslog_ident != NULL) {
        ina_str_destroy((*cfg)->syslog_ident);
    }
    *cfg = NULL;
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_init(ina_log_cfg_t *cfg)
{
    if (cfg->target == INA_LOG_STDOUT) {
        cfg->fp = stdout;    
    }
    else if (cfg->target == INA_LOG_FILE) {
        cfg->fp = (cfg->logfile == NULL) ? stdout : fopen(ina_str_cstr(cfg->logfile),"a");
    }
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_log(const ina_log_cfg_t *cfg, ina_log_level_t level, const char *msg) {
    const char *c = ".-*#";
    time_t now = time(NULL);
    
    char buf[64];
    strftime(buf,sizeof(buf),"%d %b %H:%M:%S",localtime(&now));

#ifdef WIN32
    fprintf(cfg->fp,"[%d] %s %c %s\n", (int)GetCurrentProcessId(), buf, c[level] ,msg);
#else
    if (cfg->target == INA_LOG_SYSLOG) {
        syslog(cfg->syslog_facility, "%s", msg);
    }
    else {
        fprintf(cfg->fp,"[%d] %s %c %s\n", (int)getpid(), buf, c[level] ,msg);
    }
#endif

    fflush(cfg->fp);
    return INA_SUCCESS;
}