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
    ina_list_t *targets;
    ina_str_t category;
    int pid;
};
ina_str_t  __cfg_filepath = NULL;

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

    return INA_SUCCESS;
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


static ina_rc_t __ina_process_rule(const char *section_name,
                                            const char* section_key,
                                            ina_conffile_entries_t *entries,
                                            void* user_data)
{
    ina_str_t *tokens;
    size_t count;
    int match_cat = 0;
    uint32_t levels;

    ina_log_t *log = (ina_log_t*)user_data;
    tokens = ina_str_split(section_key, ".", &count);
    if (count == 2) {
        if (strcmp(tokens[0], "*") == 0 ||
            strcmp(tokens[0], log->category)== 0) {
            match_cat = 1;
        }
        if (strcmp(tokens[1], "*") == 0) {
            levels = 1U|2U|4U|8U;
        } else if (strcmp(tokens[1], "DEBUG") == 0) {
            levels = 1U;
        } else if (strcmp(tokens[1], "INFO") == 0) {
            levels = 2U;
        } else if (strcmp(tokens[1], "WARNING") == 0) {
            levels = 4U;
        } else if (strcmp(tokens[1], "ERROR") == 0) {
            levels = 8U;
        }
    }
    ina_str_split_free_tokens(tokens);

    if (match_cat) {
        ina_str_t value;
        __ina_target_t *t = ina_mem_alloc(sizeof(__ina_target_t));
        ina_mem_set(t, 0, sizeof(__ina_target_t));
        t->level = levels;
        if (INA_SUCCEED(ina_conffile_get_string_from_entries(entries, "target", &value))) {
            if (strcmp(value, ">stdout") == 0) {
                t->type = INA_LOG_STDOUT;
                t->fp = stdout;
            } else if (strcmp(value, ">stderr") == 0) {
                t->type = INA_LOG_STDERR;
                t->fp = stderr;
            } else {
                t->type = INA_LOG_FILE;
            }
        }
        t->node.data = t;
        ina_list_insert_tail(log->targets, &t->node);
    }
}

INA_API(ina_rc_t) ina_log_init(const char* cfg_filepath)
{
    INA_INIT_GUARD();
    __cfg_filepath = ina_str_new_fromcstr(cfg_filepath);
    INA_RETURN_IF_NULL(__cfg_filepath);
    return INA_SUCCESS;
}

INA_API(void) ina_log_destroy(void)
{
    INA_DESTROY_GUARD();
    INA_STR_FREE_SAFE(__cfg_filepath);
}

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

INA_API(ina_rc_t) ina_log_new(const char* category, ina_log_t **log)
{
    ina_conffile_t *cf = NULL;
    INA_VERIFY_NOT_NULL(log);
    INA_VERIFY_NOT_NULL(category);

    *log = (ina_log_t *) ina_mem_alloc(sizeof(ina_log_t));
    INA_RETURN_IF_NULL(*log);
    ina_mem_set(*log, 0, sizeof(ina_log_t));
    (*log)->category = ina_str_new_fromcstr(category);

    INA_CONFFILE(cf, __cfg_filepath, *log,
                 INA_CONFFILE_SECTION("global", INA_YES, NULL,
                         INA_CONFFILE_NUMBER_KEY("buffer_size", INA_NO)),
                 INA_CONFFILE_NAMED_SECTION("rule", INA_NO, __ina_process_rule,
                         INA_CONFFILE_STRING_KEY("target", INA_YES)));

    return INA_SUCCESS;

}

INA_API(void) ina_log_free(ina_log_t **log)
{
    INA_FREE_CHECK(log);
    if ((*log)->targets != NULL) {
        ina_list_foreach((*log)->targets, __ina_free_target);
        ina_list_free(&(*log)->targets);
    }
    INA_STR_FREE_SAFE((*log)->category);
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
