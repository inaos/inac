/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include "config.h"

#ifdef INA_OS_WIN32
#define __INA_TIME_INC(vv_ptr) InterlockedExchangeAdd64(vv_ptr, 1)
INA_INLINE double __ina_lit_to_secs(const double freq_sec, const LARGE_INTEGER * L)
{
    return ((double)L->QuadPart / freq_sec);
}
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
#else
#define __INA_TIME_INC(vv_ptr) __sync_fetch_and_add(vv_ptr, 1)
#endif


/* Stopwatch  data */
typedef struct ina_stopwatch_tv_s {
    ina_time_tsc_t start;          /* start time */
    ina_time_tsc_t stop;           /* stop time */
    size_t max_stamps;             /* max stamps, readonly */
    volatile int64_t next_stamp;   /* next free stamp slot */
    double duration;               /* duration  */
    ina_time_resolution_t resolution;
    char pad[4];                   /* padding */
    ina_stopwatch_ts_t stamps;     /* stamp records */
} ina_stopwatch_tv_t;

/* Stopwatch time values */
struct ina_stopwatch_s {
    int32_t id;               /* stop watch id */
    char pad[4];              /* padding */
    ina_mempool_t *mp;        /* memory pool */
    ina_stopwatch_tv_t *tv;   /* stopwatch data */
    ina_stopwatch_ts_t *ts;   /* current time stamp */
#ifdef INA_OS_WIN32
    double freq_sec;          /* WIN32: tick count per second */
#endif
};


static ina_rc_t __ina_stopwatch_init(int, ina_stopwatch_t **, int, size_t);



INA_API(ina_rc_t) ina_stopwatch_new(int id, int max_stamps, ina_stopwatch_t **stopwatch)
{
    size_t size = INA_STOPWATCH_MAX_STAMPS;
    INA_VERIFY_NOT_NULL(stopwatch);
    *stopwatch = NULL;
    if (max_stamps == -1) {
        if (id > 0) {
            size = (size_t) INA_STOPWATCH_MAX_STAMPS;
        } else {
            size = 0;
        }
    }
    return __ina_stopwatch_init(id, stopwatch, 1, size);
}

INA_API(ina_rc_t) ina_stopwatch_open(int id, ina_stopwatch_t **stopwatch)
{
    return __ina_stopwatch_init(id, stopwatch, 0, INA_STOPWATCH_MAX_STAMPS);
}

INA_API(ina_rc_t) ina_stopwatch_started(const ina_stopwatch_t *stopwatch)
{
    INA_VERIFY_NOT_NULL(stopwatch);
    if (stopwatch->tv->duration == 0.0) {
        return INA_SUCCESS;
    }
    return INA_ERROR(INA_ERR_NOT_RUNNING);
}

INA_API(ina_rc_t) ina_stopwatch_valid(const ina_stopwatch_t *stopwatch)
{
    INA_VERIFY_NOT_NULL(stopwatch);
#ifdef INA_OS_WIN32
    if (INA_UNLIKELY(stopwatch->tv->stop.tp.QuadPart < stopwatch->tv->start.tp.QuadPart)) {
        return INA_ERROR(INA_ERR_INVALID);
    }
#elif defined(INA_OS_OSX)
    if (INA_UNLIKELY(stopwatch->tv->stop.tp < stopwatch->tv->start.tp)) {
        return INA_ERROR(INA_ERR_INVALID);
    }
#else   
    if (INA_UNLIKELY(stopwatch->tv->stop.tp.tv_sec <  stopwatch->tv->start.tp.tv_sec)) {
        return INA_ERROR(INA_ERR_INVALID);
    }
    if (stopwatch->tv->stop.tp.tv_sec == stopwatch->tv->start.tp.tv_sec) {
        if (INA_UNLIKELY(stopwatch->tv->stop.tp.tv_nsec < stopwatch->tv->start.tp.tv_nsec)) {
            return INA_ERROR(INA_ERR_INVALID);
        }
    }
#endif
    return INA_SUCCESS; 
}


INA_API(ina_rc_t) ina_stopwatch_free(ina_stopwatch_t **stopwatch)
{
    INA_VERIFY_NOT_NULL(stopwatch);
    INA_VERIFY_NOT_NULL(*stopwatch);

    if ((*stopwatch)->mp != NULL) {
        ina_mempool_free(&(*stopwatch)->mp);
    }
    ina_mem_free(*stopwatch);
    *stopwatch = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_stopwatch_start(ina_stopwatch_t* stopwatch,
                                           ina_time_tsc_t *start)
{
    INA_VERIFY_NOT_NULL(stopwatch);
    /* Duration = 0, indicate stopwwatch is running */
    stopwatch->tv->duration  = 0.0;
    /* Reset timestamp index, clear all timestamps */
    stopwatch->tv->next_stamp = 0;
    ina_mem_set(&stopwatch->tv->stamps, 0,
        (sizeof(ina_stopwatch_ts_t)*stopwatch->tv->max_stamps));
    
    /* Override start if passed */
    if (start != NULL) {
        ina_mem_cpy(&stopwatch->tv->start, start, sizeof(ina_time_tsc_t));
        return INA_SUCCESS;
    }

    /* Read clock */
    return ina_time_read_tsc_clock(&stopwatch->tv->start);
}

INA_API(ina_rc_t) ina_stopwatch_start_time(const ina_stopwatch_t *stopwatch, ina_time_tsc_t **time)
{
    INA_VERIFY_NOT_NULL(stopwatch);
    INA_VERIFY_NOT_NULL(time);
    *time = &stopwatch->tv->start;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_stopwatch_stop_time(const ina_stopwatch_t *stopwatch, ina_time_tsc_t **time)
{
    INA_VERIFY_NOT_NULL(stopwatch);
    INA_VERIFY_NOT_NULL(time);
    *time = &stopwatch->tv->stop;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_stopwatch_duration(const ina_stopwatch_t *stopwatch, double *duration)
{
    INA_VERIFY_NOT_NULL(stopwatch);
    INA_VERIFY_NOT_NULL(duration);
    INA_VERIFY(INA_FAILED(ina_stopwatch_started(stopwatch)));
    *duration = stopwatch->tv->duration;
    return INA_SUCCESS;

}

INA_API(ina_rc_t) ina_stopwatch_read_stamp(ina_stopwatch_t* stopwatch,
                                                int64_t *stamp_index,
                                                ina_stopwatch_ts_t **ts)
{
    INA_VERIFY_NOT_NULL(stopwatch);

    /* reset current timestamp */
    stopwatch->ts = NULL;

    /* Return if there arent any timestamp */
    if (stopwatch->tv->max_stamps == 0) {
        return INA_ERROR(INA_ERR_EMPTY);
    }

    /* Get the timestamp depending in stamp index */
    if (stamp_index == NULL) {
        stopwatch->ts = &stopwatch->tv->stamps;
    } else if (*stamp_index >= stopwatch->tv->next_stamp) {
        return INA_ERR_END_OF;
    } else if (*stamp_index == -1) {
        *stamp_index = stopwatch->tv->next_stamp;
    }
    if (stopwatch->ts == NULL) {
        stopwatch->ts = (&(stopwatch->tv->stamps))+(*stamp_index);
    }
 
    /* Calculate duration if not yet done */
    if (stopwatch->ts->duration == 0) {
#ifdef INA_OS_WIN32
        LARGE_INTEGER elapsed;
        if (*stamp_index == 0) {
            elapsed.QuadPart = stopwatch->ts->stamp.tp.QuadPart - 
		    stopwatch->tv->start.tp.QuadPart; 
        } else {
           *ts = (&(stopwatch->tv->stamps))+(*stamp_index-1);
           elapsed.QuadPart = stopwatch->ts->stamp.tp.QuadPart - 
                   (*ts)->stamp.tp.QuadPart;
        }
        stopwatch->ts->duration = __ina_lit_to_secs(stopwatch->freq_sec, &elapsed);
#elif defined(INA_OS_OSX)
        if (*stamp_index == 0) {
            stopwatch->ts->duration += ((stopwatch->ts->stamp.tp -
				    stopwatch->tv->start.tp) / 10000000.0);
        } else {
            ina_stopwatch_ts_t *ts = (&(stopwatch->tv->stamps))+(*stamp_index)-1;
            stopwatch->ts->duration += ((stopwatch->ts->stamp.tp -
				    ts->stamp.tp) / 10000000.0);         
        } 
#else
        if (stamp_index && *stamp_index == 0) {

            ina_time_tsc_seconds_nanos(&stopwatch->tv->start,
                &stopwatch->tv->start.tp.tv_sec,
                &stopwatch->tv->start.tp.tv_nsec);

            stopwatch->ts->stamp.ref = stopwatch->tv->start.ref;
            stopwatch->ts->stamp.refhpet = stopwatch->tv->start.refhpet;

            ina_time_tsc_seconds_nanos(&stopwatch->ts->stamp, 
                &stopwatch->ts->stamp.tp.tv_sec,
                &stopwatch->ts->stamp.tp.tv_nsec);
             
            stopwatch->ts->duration = (stopwatch->ts->stamp.tp.tv_sec -
			    stopwatch->tv->start.tp.tv_sec);
            stopwatch->ts->duration += ((stopwatch->ts->stamp.tp.tv_nsec -
				    stopwatch->tv->start.tp.tv_nsec)/1000000000.0);
   
        } else {
            ina_stopwatch_ts_t *ts = (&(stopwatch->tv->stamps))+(*stamp_index-1);

            stopwatch->ts->stamp.ref = stopwatch->tv->start.ref;
            stopwatch->ts->stamp.refhpet = stopwatch->tv->start.refhpet;
   
            ina_time_tsc_seconds_nanos(&stopwatch->ts->stamp, 
                &stopwatch->ts->stamp.tp.tv_sec,
                &stopwatch->ts->stamp.tp.tv_nsec);

            stopwatch->ts->duration = (stopwatch->ts->stamp.tp.tv_sec
			    - ts->stamp.tp.tv_sec);
            stopwatch->ts->duration += ((stopwatch->ts->stamp.tp.tv_nsec -
				    ts->stamp.tp.tv_nsec) / 1000000000.0);
        } 
#endif
    }
    *ts = stopwatch->ts;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_stopwatch_foreach_stamp(ina_stopwatch_t *stopwatch,
                                              ina_foreach_fn_t foreach_fn)
{
    int64_t index = 0;
    ina_stopwatch_ts_t *ts;
    INA_VERIFY_NOT_NULL(stopwatch);
    INA_VERIFY_NOT_NULL(foreach_fn);

    while (INA_SUCCEED(ina_stopwatch_read_stamp(stopwatch, &index, &ts))) {
        if (INA_FAILED(foreach_fn(ts))) {
            break;
        }
    }
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_stopwatch_stamp(ina_stopwatch_t* stopwatch,
			const char* user_data1, const char* user_data2)
{
    uint64_t si = 0;
    ina_stopwatch_ts_t *ts = NULL;

    INA_VERIFY_NOT_NULL(stopwatch);
    if (INA_UNLIKELY(stopwatch->tv->max_stamps == 0)) {
        return INA_ERROR(INA_ES_STATE|INA_ERR_INVALID);
    }

    si = __INA_TIME_INC(&stopwatch->tv->next_stamp);
    if (INA_UNLIKELY(si > stopwatch->tv->max_stamps)) {
        return INA_ERROR(INA_ERR_OVERFLOW);
    }

    ts = (&(stopwatch->tv->stamps))+si;
 
    ina_time_read_tsc_clock(&ts->stamp);

    if (user_data1 != NULL) {
        if (strlen(user_data1)+1 < INA_STOPWATCH_MAX_USERDATA_LEN) {
            strcpy(ts->user_data1, user_data1);
        }
    }
    if (user_data2 != NULL) {
        if (strlen(user_data2)+1 < INA_STOPWATCH_MAX_USERDATA_LEN) {
            strcpy(ts->user_data2, user_data2); 
        }
    }
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_stopwatch_stop(ina_stopwatch_t* stopwatch)
{
#ifdef INA_OS_WIN32
    LARGE_INTEGER elapsed;
    INA_VERIFY_NOT_NULL(stopwatch);
    ina_time_read_tsc_clock(&stopwatch->tv->stop);
    elapsed.QuadPart = stopwatch->tv->stop.tp.QuadPart - stopwatch->tv->start.tp.QuadPart; 
    stopwatch->tv->duration = __ina_lit_to_secs(stopwatch->freq_sec, &elapsed);
#elif defined(INA_OS_OSX)
    INA_VERIFY_NOT_NULL(stopwatch);
    ina_time_read_tsc_clock(&stopwatch->tv->stop);
    stopwatch->tv->duration = (stopwatch->tv->stop.tp - stopwatch->tv->stop.tp) / 1000000000;
#else
    INA_VERIFY_NOT_NULL(stopwatch);
    ina_time_read_tsc_clock(&stopwatch->tv->stop);
    
    stopwatch->tv->stop.ref = stopwatch->tv->start.ref;
    stopwatch->tv->stop.refhpet = stopwatch->tv->start.refhpet;
   
    ina_time_tsc_seconds_nanos(&stopwatch->tv->start, 
        &stopwatch->tv->start.tp.tv_sec, 
        &stopwatch->tv->start.tp.tv_nsec);

    ina_time_tsc_seconds_nanos(&stopwatch->tv->stop, 
        &stopwatch->tv->stop.tp.tv_sec, 
        &stopwatch->tv->stop.tp.tv_nsec);

    stopwatch->tv->duration = (stopwatch->tv->stop.tp.tv_sec -
		    stopwatch->tv->start.tp.tv_sec);
    stopwatch->tv->duration += ((stopwatch->tv->stop.tp.tv_nsec -
			    stopwatch->tv->start.tp.tv_nsec) / 1000000000.0); 
#endif
    return ina_stopwatch_valid(stopwatch);
}


static ina_rc_t 
__ina_stopwatch_init(int id, ina_stopwatch_t **stopwatch, int create, 
			size_t max_stamps) {
    size_t size;
    uint32_t cf = 0;
    char name[100];

    sprintf(name, "/ina_stopwatch_%d", id);

    *stopwatch = (ina_stopwatch_t *) ina_mem_alloc(sizeof(ina_stopwatch_t));
    if (*stopwatch == NULL) {
        return ina_err_get_last_rc();
    }
    ina_mem_set(*stopwatch, 0, sizeof(ina_stopwatch_t));

    if (id > 0) {
        cf = INA_MEM_SHARED;
        if (create == 1) {
            cf = cf | INA_MEM_SHARED_CREATE | INA_MEM_SHARED_EXCL;
        }
    } else {
        cf = INA_MEM_DYNAMIC;
    }

    size = sizeof(ina_stopwatch_t) + (max_stamps * sizeof(ina_stopwatch_ts_t));
    if (INA_FAILED(ina_mempool_new(
            size,
            name,
            cf, &(*stopwatch)->mp))) {
        ina_mem_free(*stopwatch);
        *stopwatch = NULL;
        return ina_err_get_last_rc();
    }

    (*stopwatch)->tv = (ina_stopwatch_tv_t *) ina_mempool_dalloc(
            (*stopwatch)->mp,
            size);

    if ((*stopwatch)->tv == NULL) {
        ina_mempool_free(&(*stopwatch)->mp);
        ina_mem_free(*stopwatch);
        *stopwatch = NULL;
        return ina_err_get_last_rc();
    }

    if (create) {
        int stamps = 0;
        if (size > INT32_MAX) {
            return INA_ERROR(INA_ERR_OVERFLOW);
        }
        stamps = (int) size;
        ina_mem_set(&(*stopwatch)->tv, stamps, 0);
        (*stopwatch)->tv->max_stamps = max_stamps;
        (*stopwatch)->tv->duration = -1.0;
    }
    return INA_SUCCESS;
}