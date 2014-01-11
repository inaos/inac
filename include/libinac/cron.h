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
#ifndef _LIBINAC_CRON_H_
#define _LIBINAC_CRON_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* forward decl */
struct ina_cron_ctx_s;

/* opaque structs */
typedef struct ina_cron_task_s ina_cron_task_t;
typedef struct ina_cron_func_s ina_cron_func_t;

typedef ina_rc_t (*ina_cron_load_cb)(struct ina_cron_ctx_s *ctx);
typedef ina_rc_t (*ina_cron_save_cb)(struct ina_cron_ctx_s *ctx, ina_cron_task_t *task);

typedef ina_rc_t (*ina_cron_func_cb)(struct ina_cron_ctx_s *ctx, void *user_data);

typedef struct ina_cron_ctx_s {
    ina_cron_load_cb load_cb;
	ina_cron_save_cb save_cb;
	void *data;
	ina_cron_task_t *task_head;
    ina_cron_func_t *func_head;
	time_t t1;
	time_t t2;
	short stime;
} ina_cron_ctx_t;

typedef struct ina_cron_task_itr_s ina_cron_task_itr_t;

/*
 * 
 */
INA_API(ina_rc_t) ina_cron_init(ina_cron_ctx_t **ctx, ina_cron_load_cb load_cb, ina_cron_save_cb save_cb);
/*
 * 
 */
INA_API(ina_rc_t) ina_cron_destroy(ina_cron_ctx_t **ctx);
/*
 * 
 */
INA_API(ina_rc_t) ina_cron_task_new_iter(ina_cron_ctx_t *ctx, ina_cron_task_itr_t **iter);
/*
 * 
 */
INA_API(ina_rc_t) ina_cron_task_free_iter(ina_cron_task_itr_t **iter);
/*
 * 
 */
INA_API(ina_rc_t) ina_cron_task_next(ina_cron_task_itr_t *iter, ina_cron_task_t **task);
/*
 * 
 */
INA_API(ina_rc_t) ina_cron_task_add(ina_cron_ctx_t *ctx, const char *id, const char *pattern, 
	int persistent, ina_str_t cmd, ina_str_t working_dir);
/*
 * 
 */	
INA_API(ina_rc_t) ina_cron_task_by_id(ina_cron_ctx_t *ctx, const char *id, ina_cron_task_t **task);
/*
 * 
 */
INA_API(ina_rc_t) ina_cron_task_is_running(ina_cron_task_t *task, int *running);
/*
 * 
 */
INA_API(ina_rc_t) ina_cron_task_get_pattern(ina_cron_task_t *task, ina_str_t *pattern);
/*
 *
 */
INA_API(ina_rc_t) ina_cron_process(ina_cron_ctx_t *ctx, time_t now, int *suggested_next_time);
/*
 *
 */
INA_API(ina_rc_t) ina_cron_register_function(ina_cron_ctx_t *ctx, const char *id, 
                                             const char *pattern, void *user_data, 
                                             ina_cron_func_cb cb);
/*
 *
 */
INA_API(ina_rc_t) ina_cron_last_exec_systime(ina_cron_ctx_t *ctx, ina_str_t pattern, 
                                             time_t now, time_t *last_exec_time);
/*
 *
 */
INA_API(ina_rc_t) ina_cron_register_pull(ina_cron_ctx_t *ctx, const char *id, const char *pattern, unsigned long *key);
/*
 *
 */
INA_API(ina_rc_t) ina_cron_try_pull(ina_cron_ctx_t *ctx, unsigned long *key);

#ifdef __cplusplus
}
#endif

#endif