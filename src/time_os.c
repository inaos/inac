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

#ifdef INA_OSTIME_ENABLED

struct ina_time_s {
#ifdef WIN32
    FILETIME systime;
#else
    struct timeval systime;
#endif
} ina_time_s;

#ifdef INA_OS_WIN32
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
#endif

INA_API(ina_rc_t) ina_time_sys_backend_info(ina_time_sys_info_t *info)
{
    INA_VERIFY_NOT_NULL(info);
#ifdef INA_OS_WIN32
    strncpy(info->backend_name, "OS backend: GetSystemTimeAsFileTime()", INA_TIME_BACKEND_NAME_MAXLEN);
#else
    strncpy(info->backend_name, "OS backend: gettimeofday()", INA_TIME_BACKEND_NAME_MAXLEN);
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_sys_new(ina_time_t **time)
{
    INA_VERIFY_NOT_NULL(time);
    *time = (ina_time_t*)ina_mem_alloc(sizeof(ina_time_t));
    INA_RETURN_IF_NULL(*time);
    return INA_SUCCESS;
}

INA_API(void) ina_time_sys_free(ina_time_t **time)
{
    INA_VERIFY_FREE(time);
    INA_MEM_FREE_SAFE(*time);
}

INA_API(ina_rc_t) ina_time_read_sys_clock(ina_time_t* time)
{
    INA_VERIFY_NOT_NULL(time);
#ifdef INA_OS_WIN32
    GetSystemTimeAsFileTime(&time->systime);
#else
    if (gettimeofday(&time->systime, NULL) == -1) {
        return INA_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_sys_seconds_micros(const ina_time_t* time, time_t *secs, 
						long *micros)
{
#ifdef INA_OS_WIN32
    unsigned __int64 tmpres = 0;
    INA_VERIFY_NOT_NULL(time);
    INA_VERIFY_NOT_NULL(secs);
    INA_VERIFY_NOT_NULL(micros);
    tmpres |= time->systime.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= time->systime.dwLowDateTime;
    tmpres /= 10;  /*convert into microseconds*/
    /*converting file time to unix epoch*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    *secs = (long)(tmpres / 1000000UL);
    *micros = (long)(tmpres % 1000000UL);
#else
    INA_VERIFY_NOT_NULL(time);
    INA_VERIFY_NOT_NULL(secs);
    INA_VERIFY_NOT_NULL(micros);
    *secs = time->systime.tv_sec;
    *micros = time->systime.tv_usec;
#endif
	return INA_SUCCESS;
}

#endif
