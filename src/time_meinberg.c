/*
 * Copyright (c) 2013, INAOS GmbH
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

#ifdef INA_MBTIME_ENABLED

#define _C99_BIT_TYPES_DEFINED 1

#include <mbgdevio.h>

struct ina_time_s {
    MBG_DEV_HANDLE dh;
    PCPS_HR_TIME t;
    int32_t hns_latency;
} ina_time_s;

static ina_rc_t __ina_time_init_dev(MBG_DEV_HANDLE *dh)
{
    int devices_found;

    if (mbgdevio_check_version(MBGDEVIO_VERSION) != PCPS_SUCCESS) {
        return INA_TIME_EHWDRV;
    }

    devices_found = mbg_find_devices();
    
    if (devices_found != 1) {
        if (devices_found == 0) {
            return INA_TIME_ENODEV;
        }
        else {
            return INA_TIME_ETMDEV;
        }
    }

    *dh = mbg_open_device(0);
    
    if (*dh == MBG_INVALID_DEV_HANDLE) {
        return INA_TIME_EHWERR;
    }

    return INA_SUCCESS;
}

static ina_rc_t __ina_time_close_dev(MBG_DEV_HANDLE *dh)
{
    mbg_close_device(dh);
    if (*dh != MBG_INVALID_DEV_HANDLE) {
        return INA_TIME_EHWERR;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_sys_backend_info(ina_str_t *info)
{
    PCPS_DEV dev;
    MBG_DEV_HANDLE dh;
    int rc;
    
    if (!INA_SUCCEED(__ina_time_init_dev(&dh))) {
        return INA_ERR_PUSH_LAST;
    }

    rc = mbg_get_device_info(dh, &dev);
    if (rc != PCPS_SUCCESS) {
        return INA_TIME_EHWERR;
    }

    *info = ina_str_vsprintf("HW backend: %s", dev.cfg.fw_id);

    if (!INA_SUCCEED(__ina_time_close_dev(&dh))) {
        return INA_ERR_PUSH_LAST;
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_sys_new(ina_time_t **time)
{
    int has_fast_hr_support = 0;

    *time = (ina_time_t*)ina_mem_alloc(sizeof(ina_time_t));

    if (!INA_SUCCEED(__ina_time_init_dev(&(*time)->dh))) {
        return INA_ERR_PUSH_LAST;
    }

    if (mbg_dev_has_fast_hr_timestamp((*time)->dh, &has_fast_hr_support) != MBG_SUCCESS) {
        return INA_TIME_EHWERR;
    }

    if (!has_fast_hr_support) {
        return INA_TIME_EHWMISSF;
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_sys_free(ina_time_t **time)
{
    if (!INA_SUCCEED(__ina_time_close_dev(&(*time)->dh))) {
        return ina_err_peek();
    }

    ina_mem_free(*time);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_read_sys_clock(ina_time_t* time)
{
    if (mbg_get_fast_hr_timestamp_comp(time->dh, &time->t.tstamp, &time->hns_latency) != MBG_SUCCESS) {
        return INA_TIME_EHWERR;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_sys_seconds_micros(ina_time_t* time, time_t *secs, 
						long *micros)
{
    double frac;
    double frac_scale = 0x100000000;

    *secs = time->t.tstamp.sec;
    frac = time->t.tstamp.frac;
    frac = frac / frac_scale;
    frac = frac * 1000000;
    *micros = (long)frac;

	return INA_SUCCESS;
}

#endif
