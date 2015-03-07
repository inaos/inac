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

#define __INA_HISTOGRAM_ULLC_VERSION    1
#define __INA_HISTOGRAM_ULLC_SOLTS     16
#define __INA_HISTOGRAM_ULLC_PRODUCERS  2
#define __INA_HISTOGRAM_ULLC_CONSUMERS  1

struct ina_histogram_recorder_s {
    ina_str_t id;
    ina_ullc_ctx_t *producer;
    ina_timer_t *timer;
    ina_time_event_t *phase;
    int64_t start_ns;
    ina_str_t free_text1;
    ina_str_t free_text2;
    ina_str_t free_text3;
    ina_str_t free_text4;
    struct hdr_histogram *hist;
};

struct ina_histogram_serializer_s {
    ina_str_t id;
    ina_ullc_ctx_t *dummy;
    ina_ullc_ctx_t *consumer;
    struct hdr_log_writer *writer;
    struct hdr_histogram *hist;
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

    if (!INA_SUCCEED(INA_ULLC_PRODUCER_CREATE(
        ina_histogram_record_t,
        __INA_HISTOGRAM_ULLC_VERSION,
        __INA_HISTOGRAM_ULLC_SOLTS,
        __INA_HISTOGRAM_ULLC_PRODUCERS,
        __INA_HISTOGRAM_ULLC_CONSUMERS,
        ina_str_cstr(id),
        INA_ULLC_WS_BUSY_WAIT,
        &(*recorder)->producer))) {
            return INA_ERR_PUSH_LAST;
    }

    if (hdr_init(1, highest_trackable_value, significant_figures, &(*recorder)->hist) != 0) {
        /* FIXME: return error */
    }

    (*recorder)->phase = ina_timer_create_event((*recorder)->timer, sample_interval_ms);

    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_recorder_free(ina_histogram_recorder_t **recorder)
{
    /* FIXME: how to free hdr_histogram? */
    ina_timer_delete_event((*recorder)->timer, (*recorder)->phase);
    ina_timer_destroy(&(*recorder)->timer);
    ina_ullc_producer_destroy(&(*recorder)->producer);
    ina_mem_free(*recorder);
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_recorder_set_free_text1(ina_histogram_recorder_t *recorder, const char *text)
{
    if (recorder->free_text1 != NULL) {
        ina_str_free(recorder->free_text1);
    }
    recorder->free_text1 = ina_str_new_fromcstr(text);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_histogram_recorder_set_free_text2(ina_histogram_recorder_t *recorder, const char *text)
{
    if (recorder->free_text2 != NULL) {
        ina_str_free(recorder->free_text2);
    }
    recorder->free_text2 = ina_str_new_fromcstr(text);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_histogram_recorder_set_free_text3(ina_histogram_recorder_t *recorder, const char *text)
{
    if (recorder->free_text3 != NULL) {
        ina_str_free(recorder->free_text3);
    }
    recorder->free_text3 = ina_str_new_fromcstr(text);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_histogram_recorder_set_free_text4(ina_histogram_recorder_t *recorder, const char *text)
{
    if (recorder->free_text4 != NULL) {
        ina_str_free(recorder->free_text4);
    }
    recorder->free_text4 = ina_str_new_fromcstr(text);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_histogram_recorder_record(ina_histogram_recorder_t *recorder, int64_t value)
{
    hdr_record_value(recorder->hist, value);
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_recorder_process(ina_histogram_recorder_t *recorder, int64_t time_ns)
{
    ina_time_event_t *e;
    time_t time_ms = (time_t)(time_ns/1000/1000);
    e = ina_timer_next_event_with_time(recorder->timer, time_ms);
    /* phase change */
    if (e->id == recorder->phase->id) {
        ina_histogram_record_t *r = INA_ULLC_CLAIM(ina_histogram_record_t, recorder->producer);
        ina_mem_cpy(r->data, recorder->hist, sizeof(recorder->hist));
        strcpy(r->free_text1, ina_str_cstr(recorder->free_text1));
        strcpy(r->free_text2, ina_str_cstr(recorder->free_text2));
        strcpy(r->free_text3, ina_str_cstr(recorder->free_text3));
        strcpy(r->free_text4, ina_str_cstr(recorder->free_text4));
        r->start_ts_ns = recorder->start_ns;
        r->end_ts_ns = time_ns;
        INA_ULLC_COMMIT(recorder->producer);
        recorder->start_ns = time_ns;
        hdr_reset(recorder->hist);
    }
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_serializer_new(ina_histogram_serializer_t **serializer,
                                               ina_str_t id,
                                               int64_t highest_trackable_value,
                                               int significant_figures)
{
    //hdr_log_writer_init(
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_serializer_free(ina_histogram_serializer_t **serializer)
{
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_serializer_serialize(ina_histogram_serializer_t *serializer, 
                                                     ina_str_t *record,
                                                     ina_histogram_meta_t *meta)
{
    ina_histogram_record_t *r = INA_ULLC_GET(ina_histogram_record_t, serializer->consumer);
    
    //hdr_log_write_str(serializer->writer, 
    strcpy(meta->free_text1, r->free_text1);
    strcpy(meta->free_text2, r->free_text2);
    strcpy(meta->free_text3, r->free_text3);
    strcpy(meta->free_text4, r->free_text4);

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
