/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

#define arysize(ary)	(sizeof(ary)/sizeof((ary)[0]))

struct ina_cron_event_s {
    ina_cron_ctx_t *ctx;
    uint32_t cf;
    uint32_t key;
    ina_str_t id;
	ina_str_t cmd;
	ina_str_t working_dir;
	ina_str_t pattern;
    ina_process_t *process;
    ina_cron_push_cb_t push_cb;
    int ready;
	char mins[60]; /* 0-59 */
    char hours[24];	/* 0-23 */
    char days[32]; /* 1-31 */
    char mons[12]; /* 0-11 */
    char dow[7]; /* 0-6, beginning sunday */
    void *pull_data;
    void *push_data;
    int callable;
};

struct ina_cron_event_iter_s {
    ina_hashtable_iter_t *iter;
};


/* cron context */
struct ina_cron_ctx_s {
    ina_process_ctx_t *process_ctx;
    ina_cron_load_cb load_cb;     /* load callback */
    ina_cron_save_cb save_cb;     /* save callback */
    ina_hashtable_t *events;
    time_t t1;                    /* ? */
    time_t t2;                    /* ? */
    short stime;                  /* ? */
};

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
		} else if (*ptr >= '0' && *ptr <= '9') {
			if (n1 < 0) {
				n1 = (int)strtol(ptr, &ptr, 10) + off;
			}
			else {
				n2 = (int)strtol(ptr, &ptr, 10) + off;
			}
			skip = 1;
		} else if (names) {
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
			skip = (int)strtol(ptr + 1, &ptr, 10);
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
static void __fix_day_dow(ina_cron_event_t *e)
{
    unsigned short i;
    short weekUsed = 0;
    short daysUsed = 0;

    for (i = 0; i < arysize(e->dow); ++i) {
        if (e->dow[i] == 0) {
            weekUsed = 1;
            break;
        }
    }
    for (i = 0; i < arysize(e->days); ++i) {
        if (e->days[i] == 0) {
            daysUsed = 1;
            break;
        }
    }
    if (weekUsed && !daysUsed) {
        memset(e->days, 0, sizeof(e->days));
    }
    if (daysUsed && !weekUsed) {
        memset(e->dow, 0, sizeof(e->dow));
    }
}
/*
 *
 */
static ina_rc_t __parse_cron_pattern(char *pattern_buf, ina_cron_event_t *e)
{
	/*
	 * parse date ranges
	 */
    pattern_buf = __parse_field(e->mins, 60, 0, NULL, pattern_buf);
    pattern_buf = __parse_field(e->hours,  24, 0, NULL, pattern_buf);
    pattern_buf = __parse_field(e->days, 32, 0, NULL, pattern_buf);
    pattern_buf = __parse_field(e->mons, 12, -1, mon_array, pattern_buf);
    pattern_buf = __parse_field(e->dow, 7, 0, dow_array, pattern_buf);

	/*
	 * check failure
	 */
	if (pattern_buf == NULL) {
		return INA_ERROR(INA_ES_PATTERN | INA_ERR_INVALID);
	}

	/*
	 * fix days and dow - if one is not * and the other
	 * is *, the other is set to 0, and vise-versa
	 */
	__fix_day_dow(e);

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
            ina_hashtable_iter_t *iter;
			ina_cron_event_t *e;
			struct tm *tp = localtime(&t);

            if (tp == NULL) {
                return -1;
            }
	    
			/* iterate through tasks */
			ina_hashtable_iter_new(ctx->events, &iter);

			while (INA_SUCCEED(ina_hashtable_iter_next(iter, (void**)&e))) {
			    if (e->mins[tp->tm_min] && e->hours[tp->tm_hour] &&
						(e->days[tp->tm_mday] || e->dow[tp->tm_wday]) &&
						e->mons[tp->tm_mon]) {

			        if ((e->cf&INA_CRON_CF_PUSH) && e->push_cb) {
                        if (INA_FAILED(e->push_cb(ctx, e->push_data))) {
                            continue;
                        }
                    }

			        if (INA_FAILED(ina_cron_event_is_running(e))) {
			            e->ready = 1;
			            ++njobs;
                    }

                    if ((e->cf&INA_CRON_CF_PULL) && e->key) {
                        e->callable = 1;
                    }
                }
			}
			ina_hashtable_iter_free(&iter);
		}
	}
    return(njobs);
}

/*
 *
 */
static void __run_jobs(ina_cron_ctx_t *ctx)
{
    ina_cron_event_t *e;
    ina_hashtable_iter_t *iter;
    ina_fsm_state_t  state;
	
	/* iterate through tasks */
    ina_hashtable_iter_new(ctx->events, &iter);
    while (INA_SUCCEED(ina_hashtable_iter_next(iter, (void**)&e))) {
		if ((e->cf&INA_CRON_CF_EXEC) &&
		     e->ready &&
		     INA_SUCCEED(ina_process_query_state(e->process, &state)) &&
		     state != INA_PROCESS_RUNNING) {
			e->ready = 0;
            ina_process_start(e->process);
		    e->ready = 1;
		}
	}
	ina_hashtable_iter_free(&iter);
}
/*
 * Check for job completion, return number of jobs still running after
 * all done.
 */
static int __check_jobs(ina_cron_ctx_t *ctx)
{
    ina_hashtable_iter_t *iter;
    ina_cron_event_t *e;
    int still_running = 0;
    ina_fsm_state_t  state;

    /* iterate through tasks */
    ina_hashtable_iter_new(ctx->events, &iter);
    while (INA_SUCCEED(ina_hashtable_iter_next(iter, (void**)&e))) {
        if ((e->cf&INA_CRON_CF_EXEC) &&
             INA_SUCCEED(ina_process_query_state(e->process, &state)) &&
             state == INA_PROCESS_RUNNING) {
            ++still_running;
        }
    }
    return still_running;
}

static ina_rc_t __ina_free_event(void *data)
{
    ina_cron_event_t *e = (ina_cron_event_t*)data;
    ina_cron_event_free(&e);
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_cron_ctx_new(ina_cron_load_cb load_cb, ina_cron_save_cb save_cb, ina_cron_ctx_t **ctx)
{
    INA_VERIFY_NOT_NULL(ctx);

	*ctx = (ina_cron_ctx_t*)ina_mem_alloc(sizeof(ina_cron_ctx_t));
    INA_RETURN_IF_NULL(ctx);
    INA_MEM_SET_ZERO(*ctx, ina_cron_ctx_t);

    if (load_cb) {
		(*ctx)->load_cb = load_cb;
		/* load tasks */
		(*ctx)->load_cb(*ctx);
	}
	if (save_cb) {
		(*ctx)->save_cb = save_cb;
	}
	INA_FAIL_IF_ERROR(ina_process_ctx_new(&(*ctx)->process_ctx));
    INA_FAIL_IF_ERROR(ina_hashtable_new(INA_HASHTABLE_STR_KEY,
                                     INA_HASH_DEFAULT,
                                     INA_HASHTABLE_TYPE_DEFAULT,
                                     INA_HASHTABLE_GROW_DEFAULT,
                                     INA_HASHTABLE_SHRINK_DEFAULT,
                                     INA_HASHTABLE_DEFAULT_CAPACITY,
                                     INA_HASHTABLE_CF_DEFAULT, &(*ctx)->events));
	(*ctx)->t1 = time(NULL);
	(*ctx)->t2 = 0;
	(*ctx)->stime = 60;
	return INA_SUCCESS;

fail:
    ina_cron_ctx_free(ctx);
    return ina_err_get_rc();
}

INA_API(void) ina_cron_ctx_free(ina_cron_ctx_t **ctx)
{
	INA_FREE_CHECK(ctx);
	if ((*ctx)->events) {
        ina_hashtable_foreach((*ctx)->events, __ina_free_event);
    }
    ina_hashtable_free(&(*ctx)->events);
    ina_process_ctx_free(&(*ctx)->process_ctx);
	INA_MEM_FREE_SAFE(*ctx);
}

INA_API(ina_rc_t) ina_cron_event_new(ina_cron_ctx_t *ctx,
                                     const char *id,
                                     const char *pattern,
                                     ina_cron_event_t **event)
{
    char* buf = NULL;
    size_t slen;
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(id);
    INA_VERIFY(strlen(id));
    INA_VERIFY(strlen(pattern));
    INA_VERIFY_NOT_NULL(pattern);
    INA_VERIFY_NOT_NULL(event);

    INA_RETURN_IF_SUCCEED(ina_hashtable_get_str(ctx->events, id, (void**)event));

    *event = (ina_cron_event_t*)ina_mem_alloc(sizeof(ina_cron_event_t));
    INA_RETURN_IF_NULL(*event);
    INA_MEM_SET_ZERO(*event, ina_cron_event_t);

    slen = strlen(pattern);
    buf = (char*)ina_mem_alloc(slen+2);
    buf = strcpy(buf, pattern);
    buf[slen] = '\n';
    INA_FAIL_IF_ERROR(__parse_cron_pattern(buf, *event));
    (*event)->id = ina_str_new_fromcstr(id);
    (*event)->pattern = ina_str_new_fromcstr(pattern);
    ina_mem_free(buf);
    return INA_SUCCESS;

fail:
    INA_MEM_FREE_SAFE(buf);
    ina_cron_event_free(event);
    return ina_err_get_rc();
}



INA_API(void) ina_cron_event_free(ina_cron_event_t **event)
{
    ina_cron_ctx_t *ctx;
    INA_FREE_CHECK(event);
    INA_ASSERT_FALSE(ina_cron_task_is_running(*event));
    ctx = (*event)->ctx;
    INA_MUST_SUCCEED(ina_hashtable_remove_str(ctx->events, (*event)->id, (void**)&event));
    if (ctx->save_cb && ((*event)->cf&INA_CRON_CF_PERSIST)) {
        INA_MUST_SUCCEED(ctx->save_cb(ctx, *event, INA_YES));
    }
    INA_STR_FREE_SAFE((*event)->id);
    INA_STR_FREE_SAFE((*event)->cmd);
    INA_STR_FREE_SAFE((*event)->working_dir);
    INA_STR_FREE_SAFE((*event)->pattern);
    ina_process_free(&(*event)->process);
    INA_MEM_FREE_SAFE(*event);
}

INA_API(const char*) ina_cron_event_id(const ina_cron_event_t *event)
{
    INA_ASSERT_NOT_NULL(event);
    return event->id;
}

INA_API(ina_rc_t) ina_cron_event_set_exec_params(ina_cron_event_t *event, const char* cmd, const char* working_dir, int persist)
{
    ina_str_t cmdstr;
    ina_str_t *cmd_parts = NULL;
    size_t cmd_parts_count;
    ina_process_descriptor_t *descriptor;
    INA_VERIFY_NOT_NULL(event);
    INA_VERIFY_NOT_NULL(cmd);

    event->cf = event->cf|INA_CRON_CF_EXEC;
    if (persist) {
        event->cf = event->cf|INA_CRON_CF_PERSIST;
    }
    cmdstr = ina_str_new_fromcstr(cmd);
    cmd_parts = ina_str_split(cmdstr," ", &cmd_parts_count);

    event->cmd = ina_str_new_fromcstr(cmd);
    event->working_dir = ina_str_new_fromcstr(working_dir);

    INA_FAIL_IF_ERROR(ina_process_descriptor_new(event->ctx->process_ctx,
                                                 ina_str_cstr(cmd_parts[0]),
                                                 working_dir,
                                                 ina_str_cstr(cmd_parts[1]),
                                                 30,
                                                 0,
                                                 &descriptor));

    INA_FAIL_IF_ERROR(ina_process_new(event->ctx->process_ctx, descriptor, &event->process));

    /* persist if required */
    if (event->ctx->save_cb && persist) {
        event->ctx->save_cb(event->ctx, event, INA_NO);
    }
    return INA_SUCCESS;
fail:
    return ina_err_get_rc();
}
INA_API(ina_rc_t) ina_cron_event_set_push_params(ina_cron_event_t *event, ina_cron_push_cb_t push_cb, void *user_data)
{
    INA_VERIFY_NOT_NULL(event);
    event->push_cb = push_cb;
    event->push_data = user_data;
    if (push_cb != NULL) {
        event->cf = event->cf|INA_CRON_CF_PUSH;
    }
    return INA_SUCCESS;

}
INA_API(ina_rc_t) ina_cron_event_set_pull_params(ina_cron_event_t *event, uint32_t  key, void *user_data)
{
    INA_VERIFY_NOT_NULL(event);
    event->key = key;
    event->pull_data = user_data;
    if (key) {
        event->cf = event->cf|INA_CRON_CF_PULL;
    }
    return INA_SUCCESS;
}

 INA_API(ina_rc_t) ina_cron_event_check_capability(const ina_cron_event_t *event, uint32_t cf)
{
    if (event->cf&cf) {
        return  INA_SUCCESS;
    }
    return INA_ERROR(INA_ERR_NOT_AVAILABLE);
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

INA_API(ina_rc_t) ina_cron_event_iter_new(ina_cron_ctx_t *ctx, ina_cron_event_iter_t **iter)
{
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(iter);

    *iter = (ina_cron_event_iter_t*)ina_mem_alloc(sizeof(ina_cron_event_iter_t));
    INA_RETURN_IF_NULL(*iter);
    INA_FAIL_IF_ERROR(ina_hashtable_iter_new(ctx->events, &(*iter)->iter));
    return INA_SUCCESS;
fail:
    ina_cron_event_iter_free(iter);
    return ina_err_get_rc();
}

INA_API(void) ina_cron_event_iter_free(ina_cron_event_iter_t **iter)
{
    INA_FREE_CHECK(iter);
    ina_hashtable_iter_free(&(*iter)->iter);
	INA_MEM_FREE_SAFE(*iter);
}

INA_API(ina_rc_t) ina_cron_task_iter_next(ina_cron_event_iter_t *iter, ina_cron_event_t **event)
{
    INA_VERIFY_NOT_NULL(iter);
    INA_VERIFY_NOT_NULL(event);
    return ina_hashtable_iter_next(iter->iter, (void**)event);
}

INA_API(ina_rc_t) ina_cron_event_by_id(ina_cron_ctx_t *ctx, const char *id, ina_cron_event_t **event)
{
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(id);
    INA_VERIFY_NOT_NULL(event);
    return ina_hashtable_get_str(ctx->events, id, (void**)event);
}

INA_API(ina_rc_t) ina_cron_task_is_running(const ina_cron_event_t *event)
{
    ina_fsm_state_t state;
    INA_VERIFY_NOT_NULL(event);
    if ((event->cf&INA_CRON_CF_EXEC) &&
        INA_SUCCEED(ina_process_query_state(event->process, &state)) &&
        state == INA_PROCESS_RUNNING) {
        return INA_SUCCESS;
    }
    return INA_ERROR(INA_ERR_NOT_RUNNING);
}

INA_API(const char*) ina_cron_event_pattern(const ina_cron_event_t *event)
{
    INA_ASSERT_NOT_NULL(event);
    return event->pattern;;
}


INA_API(ina_rc_t) ina_cron_last_exec_systime(ina_cron_ctx_t *ctx, const char *pattern, time_t now, time_t *last_exec_time)
{
    time_t t;
    ina_cron_event_t dummy;
    ina_str_t buf;

    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(pattern);
    INA_VERIFY_NOT_NULL(last_exec_time);

    buf = ina_str_new_fromcstr(pattern);
    buf = ina_str_catcstr(buf, "\n");
    INA_MEM_SET_ZERO(&dummy, ina_cron_event_t);


    if (INA_FAILED(__parse_cron_pattern(buf, &dummy))) {
        ina_str_free(buf);
        return ina_err_get_rc();
    }
    ina_str_free(buf);

    for (t = now - now % 60; t > 0; t -= 60) {
        struct tm *tp = localtime(&t);
        if (tp == NULL) {
            return INA_OS_ERROR(INA_ES_TIME | INA_ERR_INVALID);
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


INA_API(ina_rc_t) ina_cron_try_pull(ina_cron_ctx_t *ctx, uint32_t *key, void **user_data)
{
    ina_hashtable_iter_t *iter;
    ina_cron_event_t *e;

    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(key);

    *key = 0;
    ina_hashtable_iter_new(ctx->events, &iter);
    while (INA_SUCCEED(ina_hashtable_iter_next(iter, (void**)&e))) {
        if ((e->cf&INA_CRON_CF_PULL) && e->callable) {
            e->callable = 0;
            *key = e->key;
            if (user_data != NULL) {
                *user_data = e->pull_data;
            }
            break;
        }
    }
    ina_hashtable_iter_free(&iter);
    return INA_SUCCESS;
}
