/*
 * Copyright (c) 2013-2018, INAOS GmbH
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

#define arysize(ary)	(sizeof(ary)/sizeof((ary)[0]))

struct ina_cron_task_s {
    unsigned long key;
	ina_str_t cmd;
	ina_str_t working_dir;
	ina_str_t pattern;
    int persistent;
    ina_process_t *process;
    int ready;
	char mins[60]; /* 0-59 */
    char hours[24];	/* 0-23 */
    char days[32]; /* 1-31 */
    char mons[12]; /* 0-11 */
    char dow[7]; /* 0-6, beginning sunday */
	UT_hash_handle hh;
} ina_cron_task_s;

struct ina_cron_task_itr_s {
    ina_cron_task_t *cursor;
} ina_cron_task_itr_s;

typedef enum __ina_cron_function_type_e {
    __INA_CRON_FUNCTION_TYPE_PUSH,
    __INA_CRON_FUNCTION_TYPE_PULL
} __ina_cron_function_type_t;

struct ina_cron_func_s {
    unsigned long key;
    __ina_cron_function_type_t func_type;
    char mins[60]; /* 0-59 */
    char hours[24];	/* 0-23 */
    char days[32]; /* 1-31 */
    char mons[12]; /* 0-11 */
    char dow[7]; /* 0-6, beginning sunday */
    ina_cron_func_cb cb;
    void *user_data;
    int callable;
    UT_hash_handle hh;
} ina_cron_func_s;

typedef enum __ina_cron_schedulable_item_e {
    __INA_CRON_SCHEDULABLE_ITEM_TASK,
    __INA_CRON_SCHEDULABLE_ITEM_FUNCTION
} __ina_cron_schedulable_item_t;

typedef struct __ina_cron_schedulable_s {
    __ina_cron_schedulable_item_t item;
    union {
        ina_cron_task_t *task;
        ina_cron_func_t *func;
    };
} __ina_cron_schedulable_t;

const char *dow_array[] = {
    "sun",
    "mon",
    "tue",
    "wed",
    "thu",
    "fri",
    "sat",

    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
    NULL
};

const char *mon_array[] = {
    "jan",
    "feb",
    "mar",
    "apr",
    "may",
    "jun",
    "jul",
    "aug",
    "sep",
    "oct",
    "nov",
    "dec",

    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec",
    NULL
};

/*
 *
 */
static char *__parse_field(char *ary, int modvalue, int off, const char **names, char *ptr)
{
    char *base = ptr;
    int n1 = -1;
    int n2 = -1;

    if (base == NULL) {
    	return(NULL);
	}

    while (*ptr != ' ' && *ptr != '\t' && *ptr != '\n') {
        int skip = 0;

		/*
		 * Handle numeric digit or symbol or '*'
		 */
        if (*ptr == '*') {
			n1 = 0;	/* everything will be filled */
			n2 = modvalue - 1;
            skip = 1;
			++ptr;
		} 
		else if (*ptr >= '0' && *ptr <= '9') {
			if (n1 < 0) {
				n1 = strtol(ptr, &ptr, 10) + off;
			}
			else {
				n2 = strtol(ptr, &ptr, 10) + off;
			}
			skip = 1;
		}
        else if (names) {
			int i;
			for (i = 0; names[i]; ++i) {
				if (strncmp(ptr, names[i], strlen(names[i])) == 0) {
					break;
				}
			}
			if (names[i]) {
				ptr += strlen(names[i]);
				if (n1 < 0) {
					n1 = i;
				}
				else {
					n2 = i;
				}
				skip = 1;
			}
		}

		/*
		 * handle optional range '-'
		 */
		if (skip == 0) {
			return(NULL);
		}
		if (*ptr == '-' && n2 < 0) {
			++ptr;
			continue;
		}

		/*
		 * collapse single-value ranges, handle skipmark, and fill
		 * in the character array appropriately.
		 */
		if (n2 < 0) {
			n2 = n1;
		}

		if (*ptr == '/') {
			skip = strtol(ptr + 1, &ptr, 10);
		}

		/*
		 * fill array, using a failsafe is the easiest way to prevent
		 * an endless loop
		 */
        {
            int s0 = 1;
			int failsafe = 1024;

            --n1;
            do {
                n1 = (n1 + 1) % modvalue;
				if (--s0 == 0) {
					ary[n1 % modvalue] = 1;
					s0 = skip;
				}
			} while (n1 != n2 && --failsafe);

			if (failsafe == 0) {
				return(NULL);
			}
		}
		if (*ptr != ',') {
			break;
		}
		++ptr;
		n1 = -1;
		n2 = -1;
    }

    if (*ptr != ' ' && *ptr != '\t' && *ptr != '\n') {
        return(NULL);
    }

    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n') {
        ++ptr;
	}

    return(ptr);
}
/*
 *
 */
static void __fix_day_dow(__ina_cron_schedulable_t *sched)
{
    unsigned short i;
    short weekUsed = 0;
    short daysUsed = 0;

    if (sched->item == __INA_CRON_SCHEDULABLE_ITEM_TASK) {
        ina_cron_task_t *task = sched->task;
        for (i = 0; i < arysize(task->dow); ++i) {
            if (task->dow[i] == 0) {
                weekUsed = 1;
                break;
		    }
        }
        for (i = 0; i < arysize(task->days); ++i) {
            if (task->days[i] == 0) {
                daysUsed = 1;
                break;
		    }
        }
        if (weekUsed && !daysUsed) {
            memset(task->days, 0, sizeof(task->days));
        }
        if (daysUsed && !weekUsed) {
            memset(task->dow, 0, sizeof(task->dow));
        }
    }
    else {
        ina_cron_func_t *func = sched->func;
        for (i = 0; i < arysize(func->dow); ++i) {
            if (func->dow[i] == 0) {
                weekUsed = 1;
                break;
		    }
        }
        for (i = 0; i < arysize(func->days); ++i) {
            if (func->days[i] == 0) {
                daysUsed = 1;
                break;
		    }
        }
        if (weekUsed && !daysUsed) {
            memset(func->days, 0, sizeof(func->days));
        }
        if (daysUsed) {
            memset(func->dow, 0, sizeof(func->dow));
        }
    }
    
}
/*
 *
 */
static ina_rc_t __parse_cron_pattern(char *pattern_buf, __ina_cron_schedulable_t *sched)
{
	/*
	 * parse date ranges
	 */
    if (sched->item == __INA_CRON_SCHEDULABLE_ITEM_TASK) {
        ina_cron_task_t *task = sched->task;
	    pattern_buf = __parse_field(task->mins, 60, 0, NULL, pattern_buf);
	    pattern_buf = __parse_field(task->hours,  24, 0, NULL, pattern_buf);
	    pattern_buf = __parse_field(task->days, 32, 0, NULL, pattern_buf);
	    pattern_buf = __parse_field(task->mons, 12, -1, mon_array, pattern_buf);
	    pattern_buf = __parse_field(task->dow, 7, 0, dow_array, pattern_buf);
    }
    else {
        ina_cron_func_t *func = sched->func;
        pattern_buf = __parse_field(func->mins, 60, 0, NULL, pattern_buf);
	    pattern_buf = __parse_field(func->hours,  24, 0, NULL, pattern_buf);
	    pattern_buf = __parse_field(func->days, 32, 0, NULL, pattern_buf);
	    pattern_buf = __parse_field(func->mons, 12, -1, mon_array, pattern_buf);
	    pattern_buf = __parse_field(func->dow, 7, 0, dow_array, pattern_buf);
    }

	/*
	 * check failure
	 */
	if (pattern_buf == NULL) {
		return INA_ERROR(INA_NN_PATTERN|INA_ERR_INVALID);
	}

	/*
	 * fix days and dow - if one is not * and the other
	 * is *, the other is set to 0, and vise-versa
	 */
	__fix_day_dow(sched);

	return INA_SUCCESS;
}
/*
 * determine which jobs need to be run.  Under normal conditions, the
 * period is about a minute (one scan).  Worst case it will be one
 * hour (60 scans).
 */
static int __test_jobs(ina_cron_ctx_t *ctx, time_t t1, time_t t2)
{
    short njobs = 0;
    time_t t;

    /*
     * Find jobs > t1 and <= t2
     */
    for (t = t1 - t1 % 60; t <= t2; t += 60) {
		if (t > t1) {
			ina_cron_task_t *task, *ttmp;
            ina_cron_func_t *func, *ftmp;
			struct tm *tp = localtime(&t);

            if (tp == NULL) {
                return -1;
            }
	    
			/* iterate through tasks */
			HASH_ITER(hh, ctx->task_head, task, ttmp) {
				if (task->mins[tp->tm_min] && task->hours[tp->tm_hour] &&
						(task->days[tp->tm_mday] || task->dow[tp->tm_wday]) &&
						task->mons[tp->tm_mon]) {
                    if (!INA_SUCCEED(ina_cron_task_is_running(task))) {
						task->ready = 1;
						++njobs;
					}
				}
			}

            /* iterate through function callbacks */
            HASH_ITER(hh, ctx->func_head, func, ftmp) {
                if (func->mins[tp->tm_min] && func->hours[tp->tm_hour] &&
						(func->days[tp->tm_mday] || func->dow[tp->tm_wday]) &&
						func->mons[tp->tm_mon]) {
                    if (func->func_type == __INA_CRON_FUNCTION_TYPE_PUSH) {
                        /* execute callback */
                        func->cb(ctx, func->user_data);
                    }
                    else {
                        func->callable = 1;
                    }
                }
            }
		}
	}
    return(njobs);
}
/*
 *
 */
static void __run_job(ina_cron_task_t* task)
{
    ina_process_start(task->process);
}
/*
 *
 */
static void __run_jobs(ina_cron_ctx_t *ctx)
{
    ina_cron_task_t *task, *ttmp;
    ina_fsm_state_t  state;
	
	/* iterate through tasks */
	HASH_ITER(hh, ctx->task_head, task, ttmp) {
		if (task->ready && INA_SUCCEED(ina_process_query_state(task->process, &state)) && state != INA_PROCESS_RUNNING) {
			task->ready = 0;
            __run_job(task);
		    task->ready = 1;
		}
	}
}
/*
 * Check for job completion, return number of jobs still running after
 * all done.
 */
static int __check_jobs(ina_cron_ctx_t *ctx)
{
    ina_cron_task_t *t, *ttmp;
    int still_running = 0;
    ina_fsm_state_t  state;

    /* iterate through tasks */
	HASH_ITER(hh, ctx->task_head, t, ttmp) {
        if (INA_SUCCEED(ina_process_query_state(t->process, &state)) &&
            state == INA_PROCESS_RUNNING) {
            ++still_running;
        }
    }
    return still_running;
}

static void __free_task(ina_cron_task_t **task)
{
    ina_cron_task_t *t = *task;

    if (t->cmd != NULL) {
        ina_str_free(t->cmd);
    }
    if (t->working_dir != NULL) {
        ina_str_free(t->working_dir);
    }
    if (t->pattern != NULL) {
        ina_str_free(t->pattern);
    }
    /*if (t->process != NULL) {
        ina_process_free(&t->process);
    }*/
    ina_mem_free(*task);
}

INA_API(ina_rc_t) ina_cron_init(ina_cron_ctx_t **ctx,
                                    ina_cron_load_cb load_cb,
                                    ina_cron_save_cb save_cb,
                                    ina_process_ctx_t *process_ctx)
{
    INA_VERIFY_NOT_NULL(ctx);

	*ctx = (ina_cron_ctx_t*)ina_mem_alloc(sizeof(ina_cron_ctx_t));
    INA_RETURN_IF_NULL(ctx);

    if (load_cb) {
		(*ctx)->load_cb = load_cb;
		/* load tasks */
		(*ctx)->load_cb(*ctx);
	}
	if (save_cb) {
		(*ctx)->save_cb = save_cb;
	}
	(*ctx)->data = NULL;
	(*ctx)->task_head = NULL;
    (*ctx)->func_head = NULL;
	(*ctx)->t1 = time(NULL);
	(*ctx)->t2 = 0;
	(*ctx)->stime = 60;

    if (process_ctx == NULL) {
        if (INA_FAILED(ina_process_init(&process_ctx))) {
            ina_mem_free(*ctx);
            return ina_err_get_last_rc();
        }
    }
    (*ctx)->process_ctx = process_ctx;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cron_destroy(ina_cron_ctx_t **ctx)
{
    ina_cron_task_t *t, *ttmp;
    ina_cron_func_t *f, *ftmp;

    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(*ctx);

    HASH_ITER(hh, (*ctx)->task_head, t, ttmp) {
		HASH_DELETE(hh, (*ctx)->task_head, t);
		__free_task(&t);
	}

    HASH_ITER(hh, (*ctx)->func_head, f, ftmp) {
        HASH_DELETE(hh, (*ctx)->func_head, f);
        ina_mem_free(f);
    }

    /*if ((*ctx)->process_ctx != NULL) {
        ina_process_destroy(&(*ctx)->process_ctx);
    }*/
    ina_mem_free(*ctx);
    *ctx = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cron_task_new(ina_cron_ctx_t *ctx, const char *id, const char *pattern,
	int persistent, const char *cmd, const char *working_dir)
{
    size_t cmd_parts_count;
    ina_str_t cmdstr;
    ina_str_t *cmd_parts;
    ina_process_descriptor_t *descriptor;
	ina_cron_task_t *task = NULL;
    ina_str_t skey;
    unsigned long key;
    
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(id);
    INA_VERIFY_NOT_NULL(pattern);
    INA_VERIFY_NOT_NULL(cmd);
    INA_VERIFY_NOT_NULL(working_dir);

    skey = ina_str_new_fromcstr(id);
    key = INA_HASH_STR_TO_SDBM(skey);
    ina_str_free(skey);

	/* check if we already have this task - by using the ID */
	HASH_FIND_ULONG(ctx->task_head, &key, task);
	
	/* create a new task */
	if (task == NULL) {
        __ina_cron_schedulable_t sched;
        size_t slen = strlen(pattern);
		char *buf;
		
		task = (ina_cron_task_t*)ina_mem_alloc(sizeof(ina_cron_task_t));
        if (task == NULL) {
            return ina_err_get_last_rc();
        }
        buf = (char*)ina_mem_alloc(slen+2);
        buf = strcpy(buf, pattern);
        buf[slen] = '\n';

        cmdstr = ina_str_new_fromcstr(cmd);
        cmd_parts = ina_str_split(cmdstr," ", &cmd_parts_count);

    	task->key = key;
        task->cmd = ina_str_new_fromcstr(cmd);
		task->working_dir = ina_str_new_fromcstr(working_dir);
        task->pattern = ina_str_new_fromcstr(pattern);
        task->ready = 0;
        sched.item = __INA_CRON_SCHEDULABLE_ITEM_TASK;
        sched.task = task;
        if (!INA_SUCCEED(__parse_cron_pattern(buf, &sched))) {
            ina_mem_free(buf);
            return ina_err_get_last_rc();
        }

		ina_mem_free(buf);

        INA_MUST_SUCCEED(ina_process_descriptor_new(ctx->process_ctx,
                                                    &descriptor,
                                                    ina_str_cstr(cmd_parts[0]),
                                                    working_dir,
                                                    ina_str_cstr(cmd_parts[1]),
                                                    INA_PROCESS_LIFECYCLE_TYPE_FIRE_AND_FORGET,
                                                    INA_PROCESS_MANAGED_TYPE_PARENT_LIFETIME,
                                                    NULL,
                                                    NULL,
                                                    30,
                                                    0));

        INA_MUST_SUCCEED(ina_process_new(ctx->process_ctx, descriptor, &task->process));

		
		/* persist if required */
        task->persistent = persistent;
        if (ctx->save_cb && persistent) {
			ctx->save_cb(ctx, task, INA_NO);
		}

        HASH_ADD_ULONG(ctx->task_head, key, task);
	}
	
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cron_task_free(ina_cron_ctx_t *ctx, ina_cron_task_t **task)
{
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(task);
    INA_VERIFY_NOT_NULL(*task);

    if (INA_FAILED(ina_cron_task_is_running(*task))) {
        HASH_DEL(ctx->task_head, *task);
        if (ctx->save_cb && (*task)->persistent) {
            ctx->save_cb(ctx, *task, INA_YES);
        }
        __free_task(task);
        return INA_SUCCESS;
    }
    return INA_ERROR(INA_NN_PROCESS|INA_ERR_RUNNING);
}

INA_API(ina_rc_t) ina_cron_process(ina_cron_ctx_t *ctx, time_t now, int *suggested_next_time)
{
    time_t dt;
	
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(suggested_next_time);

	ctx->t2 = now;
	dt = ctx->t2 - ctx->t1;
	
	/*
	 * check for disparity. A reverse-indexed disparity
	 * less then an hour causes us to effectively sleep until we
	 * match the original time (i.e. no re-execution of jobs that
	 * have just been run). A forward-indexed disparity less then
	 * an hour causes intermediate jobs to be run, but only once
	 * in the worst case.
	 *
	 * when running jobs, the inequality used is greater but not
	 * equal to t1, and less then or equal to t2.
	 */
	if (dt < -60*60 || dt > 60*60) {
		ctx->t1 = ctx->t2;
	}
	else if (dt > 0) {
		__test_jobs(ctx, ctx->t1, ctx->t2);
		__run_jobs(ctx);
		if (__check_jobs(ctx) > 0) {
		   ctx->stime = 10;
		}
		else {
		   ctx->stime = 60;
		}
		ctx->t1 = ctx->t2;
	}
	
	*suggested_next_time = (ctx->stime + 1) - (short)(time(NULL) % ctx->stime);
	
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cron_task_new_iter(ina_cron_ctx_t *ctx, ina_cron_task_itr_t **iter)
{
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(iter);

    *iter = (ina_cron_task_itr_t*)ina_mem_alloc(sizeof(ina_cron_task_itr_t));
    INA_RETURN_IF_NULL(*iter);
    (*iter)->cursor = ctx->task_head;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cron_task_free_iter(ina_cron_task_itr_t **iter)
{
    INA_VERIFY_NOT_NULL(iter);
    INA_VERIFY_NOT_NULL(*iter);
    ina_mem_free(*iter);
    *iter = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cron_task_next(ina_cron_task_itr_t *iter, ina_cron_task_t **task)
{
    INA_VERIFY_NOT_NULL(iter);
    INA_VERIFY_NOT_NULL(task);
    iter->cursor = (ina_cron_task_t*)iter->cursor->hh.next;
    *task = iter->cursor;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cron_task_by_id(ina_cron_ctx_t *ctx, const char *id, ina_cron_task_t **task)
{
    ina_cron_task_t *t;
    ina_str_t skey;
    unsigned long key;
    
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(id);
    INA_VERIFY_NOT_NULL(task);

    *task = NULL;

    skey = ina_str_new_fromcstr(id);
    key = INA_HASH_STR_TO_SDBM(skey);
    HASH_FIND_ULONG(ctx->task_head, &key, t);

    if (t != NULL) {
        *task = t;
    }
    ina_str_free(skey);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cron_task_is_running(ina_cron_task_t *task)
{
    ina_fsm_state_t state;
    INA_VERIFY_NOT_NULL(task);
    if (INA_SUCCEED(ina_process_query_state(task->process, &state)) &&
            state == INA_PROCESS_RUNNING) {
        return INA_SUCCESS;
    }
    return INA_ERROR(INA_NN_PROCESS|INA_ERR_NOT_RUNNING);
}

INA_API(ina_rc_t) ina_cron_task_get_pattern(ina_cron_task_t *task, ina_str_t *pattern)
{
    INA_VERIFY_NOT_NULL(task);
    INA_VERIFY_NOT_NULL(pattern);
    *pattern = task->pattern;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cron_register_function(ina_cron_ctx_t *ctx, const char *id, 
                                             const char *pattern, void *user_data, 
                                             ina_cron_func_cb cb)
{
    ina_cron_func_t *func = NULL;
    ina_str_t skey;
    unsigned long key;
  
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(id);
    INA_VERIFY_NOT_NULL(pattern);
    
    skey = ina_str_new_fromcstr(id);
    key = INA_HASH_STR_TO_SDBM(skey);

    ina_str_free(skey);
	
	/* check if we already have this function - by using the ID */
	HASH_FIND_ULONG(ctx->func_head, &key, func);

    /* create a function callback */
	if (func == NULL) {
        __ina_cron_schedulable_t sched;
        size_t slen = strlen(pattern);
		char *buf;
		
		func = (ina_cron_func_t*)ina_mem_alloc(sizeof(ina_cron_func_t));
        if (func == NULL) {
            return ina_err_get_last_rc();
        }
        buf = (char*)ina_mem_alloc(slen+2);
        buf = strcpy(buf, pattern);
        buf[slen] = '\n';

		func->key = key;
        func->func_type = __INA_CRON_FUNCTION_TYPE_PUSH;
        func->callable = 0;
        func->user_data = user_data;
        func->cb = cb;
		
        sched.item = __INA_CRON_SCHEDULABLE_ITEM_FUNCTION;
        sched.func = func;
        if (!INA_SUCCEED(__parse_cron_pattern(buf, &sched))) {
            ina_mem_free(buf);
            return ina_err_get_last_rc();
        }
		
		ina_mem_free(buf);

        HASH_ADD_ULONG(ctx->func_head, key, func);
	}
	
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cron_unregister_function(ina_cron_ctx_t *ctx, const char *id)
{
	ina_cron_func_t *func = NULL;
	ina_str_t skey;
    unsigned long key;

    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(id);

    skey = ina_str_new_fromcstr(id);
    key = INA_HASH_STR_TO_SDBM(skey);
    ina_str_free(skey);
	
	/* check if we already have this function - by using the ID */
	HASH_FIND_ULONG(ctx->func_head, &key, func);
	
	if (func != NULL) {
		HASH_DELETE(hh, ctx->func_head, func);
		ina_mem_free(func);
	}
	
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cron_last_exec_systime(ina_cron_ctx_t *ctx, const char *pattern, time_t now, time_t *last_exec_time)
{
    time_t t;
    ina_cron_func_t dummy;
    __ina_cron_schedulable_t sched;
    ina_str_t buf;

    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(pattern);
    INA_VERIFY_NOT_NULL(last_exec_time);

    buf = ina_str_new_fromcstr(pattern);
    buf = ina_str_catcstr(buf, "\n");

    ina_mem_set(&dummy, 0, sizeof(ina_cron_func_t));
    sched.item = __INA_CRON_SCHEDULABLE_ITEM_FUNCTION;
    sched.func = &dummy;
    if (INA_FAILED(__parse_cron_pattern(buf, &sched))) {
        ina_str_free(buf);
        return ina_err_get_last_rc();
    }
    ina_str_free(buf);

    for (t = now - now % 60; t > 0; t -= 60) {
        struct tm *tp = localtime(&t);
        if (tp == NULL) {
            return INA_OS_ERROR(INA_NN_TIME|INA_ERR_INVALID);
        }
        if (dummy.mins[tp->tm_min] && dummy.hours[tp->tm_hour] &&
                (dummy.days[tp->tm_mday] || dummy.dow[tp->tm_wday]) &&
                dummy.mons[tp->tm_mon]) {
                    break;
        }
    }

    *last_exec_time = t;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cron_register_pull(ina_cron_ctx_t *ctx, const char *id, const char *pattern, unsigned long *key_out)
{
    ina_cron_func_t *func = NULL;
    ina_str_t skey;
    unsigned long key;
    
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(id);
    INA_VERIFY_NOT_NULL(pattern);
    INA_VERIFY_NOT_NULL(key_out);

    skey = ina_str_new_fromcstr(id);
    key = INA_HASH_STR_TO_SDBM(skey);
    ina_str_free(skey);
	
	/* check if we already have this function - by using the ID */
	HASH_FIND_ULONG(ctx->func_head, &key, func);

    /* create a function callback */
	if (func == NULL) {
        __ina_cron_schedulable_t sched;
        size_t slen = strlen(pattern);
		char *buf;

		func = (ina_cron_func_t*)ina_mem_alloc(sizeof(ina_cron_func_t));
        if (func == NULL) {
            return ina_err_get_last_rc();
        }
        buf = (char*)ina_mem_alloc(slen+2);
        buf = strcpy(buf, pattern);
        buf[slen] = '\n';
        func->key = key;
        func->func_type = __INA_CRON_FUNCTION_TYPE_PULL;
        func->callable = 0;
        func->user_data = NULL;
        func->cb = NULL;
		
        sched.item = __INA_CRON_SCHEDULABLE_ITEM_FUNCTION;
        sched.func = func;
        if (!INA_SUCCEED(__parse_cron_pattern(buf, &sched))) {
            ina_mem_free(buf);
            return ina_err_get_last_rc();
        }
		ina_mem_free(buf);

        HASH_ADD_ULONG(ctx->func_head, key, func);
	}

    *key_out = key;
	
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cron_try_pull(ina_cron_ctx_t *ctx, unsigned long *key)
{
    ina_cron_func_t *func, *ftmp;

    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(key);
    *key = 0;

    HASH_ITER(hh, ctx->func_head, func, ftmp) {
        if (func->func_type == __INA_CRON_FUNCTION_TYPE_PULL && func->callable) {
            func->callable = 0;
            *key = func->key;
            break;
        }
    }
    return INA_SUCCESS;
}
