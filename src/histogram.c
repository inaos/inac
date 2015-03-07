/*
 * Copyright (c) 2015, INAOS GmbH
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

#include <contribs/hdr-histogram/hdr_histogram.h>
#include <contribs/hdr-histogram/hdr_histogram_log.h>

struct ina_histogram_recorder_s {
    ina_str_t id;
    ina_ullc_ctx_t *producer;
    ina_timer_t *timer;
    ina_time_event_t *phase; 
};

INA_API(ina_rc_t) ina_histogram_recorder_new(ina_histogram_recorder_t **recorder,
                                             ina_str_t id,
                                             int64_t highest_trackable_value,
                                             int significant_figures,
                                             int sample_interval_ms)
{
    *recorder = (ina_histogram_recorder_t*)ina_mem_alloc(sizeof(ina_histogram_recorder_t));

    ina_str_cpy((*recorder)->id, id);
    
    if (!INA_SUCCEED(ina_timer_init(&(*recorder)->timer))) {
        return INA_ERR_PUSH_LAST;
    }

    

    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_recorder_free(ina_histogram_recorder_t **recorder)
{
    ina_timer_destroy(&(*recorder)->timer);
    ina_mem_free(*recorder);
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_recorder_record(ina_histogram_recorder_t *recorder, int64_t value)
{
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_recorder_process(ina_histogram_recorder_t *recorder, int64_t time)
{
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_serializer_new(ina_histogram_serializer_t **serializer,
                                               ina_str_t id,
                                               int64_t highest_trackable_value,
                                               int significant_figures)
{
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_serializer_free(ina_histogram_serializer_t **serializer)
{
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_serializer_serialize(ina_histogram_serializer_t *serializer, 
                                                     ina_histogram_record_t *histogram,
                                                     ina_str_t *record)
{
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_reporter_new(ina_histogram_reporter_t **reporter)
{
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_reporter_free(ina_histogram_reporter_t **reporter)
{
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_reporter_try_next_percentile(ina_histogram_reporter_t *reporter, 
                                                             const ina_str_t record,
                                                             ina_str_t *percentile)
{
    return INA_SUCCESS;
}
