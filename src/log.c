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

#define __INA_MAX_BUFFER_SIZE (2*1024*1024)
#define __INA_DFT_BUFFER_SIZE (4*1024)

static char* __ina_dft_spec = "global {\n"
                              "    buffer_size=4096\n"
                              "}\nrule \"*.DEBUG\" {\n"
                              "    target=\">stdout\"\n"
                              "}\n"
                              "rule \"*.WARNING\" {\n"
                              "    target=\">stdout\"\n"
                              "}\n"
                              "rule \"*.INFO\" {\n"
                              "    target=\">stdout\"\n"
                              "}\n"
                              "rule \"*.ERROR\" {\n"
                              "    target=\">stderr\"\n"
                              "}";

typedef struct __ina_target_s __ina_target_t;
typedef ina_rc_t(*__ina_write_fn_t)(__ina_target_t*, ina_log_level_t,  const char*);

typedef struct __ina_target_s {
    __ina_write_fn_t write_fn;
    ina_str_t filepath;
    ina_str_t syslog_ident;
    ina_log_level_t level;
    ina_log_target_t type;
    ina_list_node_t node;
    size_t buffer_size;
    ina_str_t open_mode;
    unsigned char *buffer_pos;
    unsigned char *buffer;
    FILE *fp;
} __ina_target_t;

/* Log context/configuration */
struct ina_log_ctx_s {
    ina_list_t *targets;
    ina_str_t category;
    size_t buffer_size;
    int pid;
};
int        __init_from_file = INA_NO;
ina_str_t  __cfg_file_or_string = NULL;
ina_log_ctx_t  *__dft_ctx = NULL;

static ina_rc_t __ina_log(const ina_log_ctx_t*, ina_log_level_t, const char*, ina_str_t);
static ina_rc_t __ina_free_target(void *data)
{
    __ina_target_t *target = (__ina_target_t*)data;
    switch (target->type) {
        case INA_LOG_STDOUT:
        case INA_LOG_STDERR:
            fflush(target->fp);
            break;
        case INA_LOG_FILE: {
            if (target->fp) {
                if (target->buffer != NULL && target->buffer != target->buffer_pos) {
                    if (target->fp == NULL) {
                        target->fp = fopen(target->filepath, "a");
                    }
                    fwrite(target->buffer, target->buffer_pos - target->buffer, 1, target->fp);
                }
                fflush(target->fp);
                fclose(target->fp);
            }
            break;
        }
        default:
            break;
    }
    ina_str_free(target->syslog_ident);
    ina_str_free(target->filepath);
    ina_str_free(target->open_mode);
    INA_MEM_FREE_SAFE(target->buffer);
    INA_MEM_FREE_SAFE(target);
    return INA_SUCCESS;
}
static ina_rc_t __ina_write_to_file(__ina_target_t *target, ina_log_level_t level, const char* msg)
{
    INA_UNUSED(level);
    if (target->fp == NULL) {
        target->fp = fopen(target->filepath, "a");
    }
    fputs(msg, target->fp);
    return INA_SUCCESS;
}

static ina_rc_t __ina_write_to_buffer(__ina_target_t *target, ina_log_level_t level, const char* msg)
{
    INA_UNUSED(level);
    if (target->buffer == NULL) {
        target->buffer = ina_mem_alloc(target->buffer_size);
        target->buffer_pos = target->buffer;
    }
    if (target->buffer_pos-target->buffer < (int)strlen(msg)+1) { /* its save to cast here, since buffer not > 2GB */
        if (target->fp == NULL) {
            target->fp = fopen(target->filepath, target->open_mode);
        }
        fwrite(target->buffer, target->buffer_pos - target->buffer, 1, target->fp);
        target->buffer_pos = target->buffer;
    }
    ina_mem_cpy(target->buffer_pos, msg, strlen(msg));
    target->buffer_pos += strlen(msg);
    return INA_SUCCESS;
}
#ifndef  INA_OS_WINDOWS
static ina_rc_t __ina_write_to_syslog(__ina_target_t *target, ina_log_level_t level, const char* msg)
{
    openlog(target->syslog_ident, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
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
    return INA_SUCCESS;
}
#endif

static ina_rc_t __ina_process_global_section(const char *section_name,
                                          const char* section_key,
                                          ina_conffile_entries_t *entries,
                                          void* user_data)
{
    ina_log_ctx_t *ctx;
    double cfg_value;
    INA_UNUSED(section_name);
    INA_UNUSED(section_key);

    ctx = (ina_log_ctx_t*)user_data;
    if (INA_SUCCEED(ina_conffile_get_number_from_entries(entries, "buffer_size", &cfg_value))) {
        ctx->buffer_size = (size_t)cfg_value;
        if (ctx->buffer_size > __INA_MAX_BUFFER_SIZE) {
            ctx->buffer_size = __INA_MAX_BUFFER_SIZE;
        }
    }
    return INA_SUCCESS;
}
static ina_rc_t __ina_process_rule_section(const char *section_name,
                                            const char* section_key,
                                            ina_conffile_entries_t *entries,
                                            void* user_data)
{
    ina_str_t *tokens;
    ina_str_t key;
    size_t count;
    int match_cat = 0;
    uint32_t levels;
    ina_log_ctx_t *ctx = (ina_log_ctx_t*)user_data;
    INA_UNUSED(section_name);
    key = ina_str_new_fromcstr(section_key);
    tokens = ina_str_split(key, ".", &count);
    if (count == 2) {
        if (strcmp(tokens[0], "*") == 0 ||
            strcmp(tokens[0], ctx->category)== 0) {
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
        } else {
			ina_str_free(key);
			ina_str_split_free_tokens(tokens);
			return INA_ERROR(INA_ERR_INVALID_ARGUMENT);
		}
    } else {
		ina_str_free(key);
		ina_str_split_free_tokens(tokens);
		return INA_ERROR(INA_ERR_INVALID_ARGUMENT);
	}
    ina_str_free(key);
    ina_str_split_free_tokens(tokens);

    if (match_cat) {
        ina_str_t value;
        double cfg_value;
        __ina_target_t *t = ina_mem_alloc(sizeof(__ina_target_t));
        ina_mem_set(t, 0, sizeof(__ina_target_t));
        t->level = levels;
        if (INA_SUCCEED(ina_conffile_get_string_from_entries(entries, "target", &value))) {
            if (strcmp(value, ">stdout") == 0) {
                t->type = INA_LOG_STDOUT;
                t->fp = stdout;
                t->write_fn = __ina_write_to_file;
            } else if (strcmp(value, ">stderr") == 0) {
                t->type = INA_LOG_STDERR;
                t->fp = stderr;
                t->write_fn = __ina_write_to_file;
#ifndef INA_OS_WINDOWS
            } else if (strcmp(value, ">syslog") == 0) {
                if (INA_FAILED(ina_conffile_get_string_from_entries(entries, "syslog_ident", &value))) {
                    t->syslog_ident = ina_str_new_fromcstr(ina_app_get_name());
                } else {
                    t->syslog_ident = ina_str_dup(value);
                }
                t->write_fn = __ina_write_to_syslog;
                t->type = INA_LOG_SYSLOG;
#endif
            } else {
                t->type = INA_LOG_FILE;
                t->filepath = ina_str_dup(value);
                t->write_fn = __ina_write_to_buffer;
                t->buffer_size = ctx->buffer_size;
                if (INA_SUCCEED(ina_conffile_get_number_from_entries(entries, "buffer_size", &cfg_value))) {
                    t->buffer_size = (size_t)cfg_value;
                }
                if (INA_SUCCEED(ina_conffile_get_string_from_entries(entries, "truncate", &value))) {
                    if (stricmp(value, "true") == 0) {
                        t->open_mode = ina_str_new_fromcstr("w");
                    } else {
                        t->open_mode = ina_str_new_fromcstr("a");
                    }
                }
            }
        }
        t->node.data = t;
        ina_list_insert_tail(ctx->targets, &t->node);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_log_init_from_file(const char* cfg_filepath)
{
    __init_from_file = INA_YES;
    return ina_log_init(cfg_filepath);
}

INA_API(ina_rc_t) ina_log_init(const char* cfg)
{
    INA_INIT_GUARD();
    __cfg_file_or_string = ina_str_new_fromcstr(cfg);
    if (ina_str_len(__cfg_file_or_string) == 0) {
        __cfg_file_or_string = ina_str_catcstr(__cfg_file_or_string, __ina_dft_spec);
    }
    INA_RETURN_IF_FAILED(ina_log_ctx_new("*", &__dft_ctx));
    return INA_SUCCESS;
}


INA_API(void) ina_log_destroy(void)
{
    INA_DESTROY_GUARD();
    ina_log_ctx_free(&__dft_ctx);
    ina_str_free(__cfg_file_or_string);
}

INA_API(ina_rc_t) ina_log_write(const ina_log_ctx_t *ctx, ina_log_level_t level, const char *location, const char* fmt, ...)
{
    va_list ap;
    ina_rc_t rc;

    INA_VERIFY_NOT_NULL(fmt);
    INA_VERIFY(strlen(fmt));

    if (ctx == NULL) {
        ctx = __dft_ctx;
    }

    va_start(ap, fmt);
    rc = ina_log_write_v(ctx, level, location, fmt, ap);
    va_end(ap);

    return rc;
}

INA_API(ina_rc_t) ina_log_write_v(const ina_log_ctx_t *log, ina_log_level_t level,
                            const char* location, const char* fmt, va_list ap)
{
    static ina_str_t msg = NULL;
    
    if (!msg) {
        msg = ina_str_new(1024);
    }

    INA_VERIFY_NOT_NULL(log);
    INA_VERIFY_NOT_NULL(fmt);
    INA_VERIFY(strlen(fmt));

    ina_str_vsnprintf(&msg, ina_str_size(msg)-1, fmt, ap);
   
    return __ina_log(log, level, location, msg);
}

INA_API(ina_rc_t) ina_log_ctx_new(const char* category, ina_log_ctx_t **ctx)
{
    ina_conffile_t *cf = NULL;

    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(category);

    *ctx = (ina_log_ctx_t *) ina_mem_alloc(sizeof(ina_log_ctx_t));
    INA_RETURN_IF_NULL(*ctx);
    INA_MEM_SET_ZERO(*ctx, ina_log_ctx_t);
    (*ctx)->buffer_size = __INA_DFT_BUFFER_SIZE;
    (*ctx)->category = ina_str_new_fromcstr(category);
#ifdef INA_OS_WINDOWS
    (*ctx)->pid = (int)GetCurrentProcessId();
#else
    (*ctx)->pid = (int)getpid();
#endif
    INA_FAIL_IF_ERROR(ina_list_new(INA_LIST_CF_NOMALLOC, &(*ctx)->targets));

    INA_CONFFILE(&cf,
            INA_CONFFILE_SECTION("global", INA_YES, __ina_process_global_section,
                    INA_CONFFILE_NUMBER_KEY("buffer_size", INA_NO)),
            INA_CONFFILE_NAMED_SECTION("rule", INA_NO, __ina_process_rule_section,
                    INA_CONFFILE_STRING_KEY("target", INA_YES),
                    INA_CONFFILE_STRING_KEY("truncate", INA_NO),
                    INA_CONFFILE_STRING_KEY("syslog_ident", INA_NO),
                    INA_CONFFILE_NUMBER_KEY("buffer_size", INA_NO)));
    if (__init_from_file == INA_NO) {
        INA_FAIL_IF_ERROR(ina_conffile_process_string(cf, __cfg_file_or_string, *ctx));
    } else {
        INA_FAIL_IF_ERROR(ina_conffile_process(cf, __cfg_file_or_string, *ctx));
    }

    ina_conffile_free(&cf);
    return INA_SUCCESS;

fail:
    ina_conffile_free(&cf);
    ina_log_ctx_free(ctx);
    return ina_err_get_rc();
}

INA_API(void) ina_log_ctx_free(ina_log_ctx_t **ctx)
{
    INA_VERIFY_FREE(ctx);
    if ((*ctx)->targets != NULL) {
        ina_list_foreach((*ctx)->targets, __ina_free_target);
        ina_list_free(&(*ctx)->targets);
    }
    ina_str_free((*ctx)->category);
    INA_MEM_FREE_SAFE(*ctx);
}

static ina_rc_t __ina_log(const ina_log_ctx_t *ctx, ina_log_level_t level, const char *location, ina_str_t msg) {
    static const char *c = " .- *   #";
    static char buf[64];
    static char buf2[2048];
    static struct tm *lt;
    ina_list_node_t *next;
#ifdef INA_OS_LINUX
    static struct tm rtm;
#endif
    INA_UNUSED(location);
    time_t now = time(NULL);

#ifdef INA_OS_LINUX
    lt = localtime_r(&now, &rtm);
#else
    lt = localtime(&now);
#endif
    INA_ASSERT_NOT_NULL(lt);
    
    strftime(buf, sizeof(buf),"%d %b %H:%M:%S", lt);
    snprintf(buf2, 2047, "[%d] %s %c %s\n", ctx->pid, buf, c[level], msg);

    if (INA_SUCCEED(ina_list_head(ctx->targets, &next))) {
        while (next) {
            __ina_target_t *target = next->data;
            if (target->level&level) {
                target->write_fn(target, level, buf2);
            }
            next = next->next;
        }
    }
    return INA_SUCCESS;
}
