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

typedef struct __ina_target_s __ina_target_t;
typedef ina_rc_t(*__ina_write_fn_t)(__ina_target_t*, const char*);

typedef struct __ina_target_s {
    __ina_write_fn_t write_fn;
    ina_str_t filepath;
    ina_log_level_t level;
    ina_log_target_t type;
    ina_list_node_t node;
    size_t buffer_size;
    void *buffer;
    FILE *fp;
} __ina_target_t;

/* Log context/configuration */
struct ina_log_s {
    ina_list_t * targets;
    int pid;
};

static ina_rc_t __ina_log(const ina_log_t*, ina_log_level_t, ina_str_t);
static ina_rc_t __ina_free_target(void *data)
{
    __ina_target_t *target = (__ina_target_t*)data;
    switch (target->type) {
        case INA_LOG_STDOUT:
        case INA_LOG_STDERR:
            break;
        case INA_LOG_FILE: {
            fflush(target->fp);
            fclose(target->fp);
            break;
        }
        default:
            break;
    }
    INA_STR_FREE_SAFE(target->filepath);
    INA_MEM_FREE_SAFE(target->buffer);
    INA_MEM_FREE_SAFE(target);

}
static ina_rc_t __ina_write_to_file(__ina_target_t *target, const char* msg)
{
    if (target->fp == NULL) {
        target->fp = fopen(target->filepath, "a");
    }
    fputs(msg, target->fp);
}

static ina_rc_t __ina_write_to_buffer(__ina_target_t *target, const char* msg);
static ina_rc_t __ina_write_to_syslog(__ina_target_t *target, const char* msg);


INA_API(ina_rc_t) ina_log(const ina_log_t *log, ina_log_level_t level, const char* fmt, ...)
{
    va_list ap;
    ina_rc_t rc;

    INA_VERIFY_NOT_NULL(log);
    INA_VERIFY_NOT_NULL(fmt);

    va_start(ap, fmt);
    rc = ina_log_v(log, level, fmt, ap);
    va_end(ap);

    return rc;
}

INA_API(ina_rc_t) ina_log_v(const ina_log_t *log, ina_log_level_t level,
                           const char* fmt, va_list ap)
{
    static ina_str_t msg = NULL;
    
    if (!msg) {
        msg = ina_str_new(1024);
    }
 
    INA_VERIFY_NOT_NULL(log);
    INA_VERIFY_NOT_NULL(fmt);
    INA_VERIFY(strlen(fmt));

    ina_str_vsnprintf(&msg, ina_str_size(msg)-1, fmt, ap);
   
    return __ina_log(log, level, msg);
}

INA_API(ina_rc_t) ina_log_new(const char* category, const char *cfg_filepath, ina_log_t **log) {
    INA_VERIFY_NOT_NULL(log);
    INA_VERIFY_NOT_NULL(cfg_filepath);
    INA_VERIFY_NOT_NULL(category);

    *log = (ina_log_t *) ina_mem_alloc(sizeof(ina_log_t));
    INA_RETURN_IF_NULL(*log);
    ina_mem_set(*log, 0, sizeof(ina_log_t));

}

INA_API(void) ina_log_free(ina_log_t **log)
{
    INA_FREE_CHECK(log);
    ina_list_foreach((*log)->targets, __ina_free_target);
    ina_list_free(&(*log)->targets);
    INA_MEM_FREE_SAFE(*log);
}


static ina_rc_t __ina_log(const ina_log_t *log, ina_log_level_t level, ina_str_t msg) {
    static const char *c = ".-*#";
    static char buf[64];
    static char buf2[2048];
    static struct tm *lt;
    ina_list_node_t *next;
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
    sprintf(buf2,"[%d] %s %c %s\n", log->pid, buf, c[level], msg);

    if (INA_SUCCEED(ina_list_head(log->targets, &next))) {
        while (next) {
            __ina_target_t *target = next->data;
            if (target->level&level) {
                target->write_fn(target, buf2);
            }
            next = next->next;
        }
    }
    return INA_SUCCESS;
}
