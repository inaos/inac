/*
 * Copyright (c) 2015-2016, INAOS GmbH
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
#include <contribs/hdr-histogram/hdr_encoding.h>

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
    struct hdr_log_writer writer;
};

struct ina_histogram_reporter_s {
    struct hdr_log_reader reader;
};

#define FAIL_AND_CLEANUP(label, error_name, error) \
    do                      \
    {                       \
        error_name = error; \
        goto label;         \
    }                       \
    while (0)

static int realloc_buffer(
    void** buffer, size_t nmemb, ssize_t size)
{
    size_t len = nmemb * size;
    if (NULL == *buffer)
    {
        *buffer = malloc(len);
    }
    else
    {
        *buffer = realloc(*buffer, len);
    }

    if (NULL == *buffer)
    {
        return ENOMEM;
    }
    else
    {
        memset(*buffer, 0, len);
        return 0;
    }
}

static void update_timespec(hdr_timespec* ts, int time_s, int time_ms)
{
    if (NULL == ts)
    {
        return;
    }

    ts->tv_sec = time_s;
    ts->tv_nsec = time_ms * 1000000;
}

int __hdr_log_read_str(
    struct hdr_log_reader* reader, ina_str_t str_line, struct hdr_histogram** histogram,
    struct timespec* timestamp, struct timespec* interval)
{
    const char* format = "%d.%d,%d.%d,%d.%d,%s";
    char* base64_histogram = NULL;
    uint8_t* compressed_histogram = NULL;
    int result = 0;

    int begin_s = 0;
    int begin_ms = 0;
    int end_s = 0;
    int end_ms = 0;
    int interval_max_s = 0;
    int interval_max_ms = 0;
    size_t base64_len;
    size_t compressed_len;
    int r;
    int num_tokens;

    ssize_t read = ina_str_len(str_line);

    r = realloc_buffer(
        (void**)&base64_histogram, sizeof(char), read);
    if (r != 0)
    {
        FAIL_AND_CLEANUP(cleanup, result, ENOMEM);
    }

    r = realloc_buffer(
        (void**)&compressed_histogram, sizeof(uint8_t), read);
    if (r != 0)
    {
        FAIL_AND_CLEANUP(cleanup, result, ENOMEM);
    }

    num_tokens = sscanf(
        ina_str_cstr(str_line), format, &begin_s, &begin_ms, &end_s, &end_ms,
        &interval_max_s, &interval_max_ms, base64_histogram);

    if (num_tokens != 7)
    {
        FAIL_AND_CLEANUP(cleanup, result, EINVAL);
    }

    base64_len = strlen(base64_histogram);
    compressed_len = hdr_base64_decoded_len(base64_len);

    r = hdr_base64_decode(
        base64_histogram, base64_len, compressed_histogram, compressed_len);

    if (r != 0)
    {
        FAIL_AND_CLEANUP(cleanup, result, r);
    }

    r = hdr_decode_compressed(compressed_histogram, compressed_len, histogram);
    if (r != 0)
    {
        FAIL_AND_CLEANUP(cleanup, result, r);
    }

    update_timespec(timestamp, begin_s, begin_ms);
    update_timespec(interval, end_s, end_ms);

cleanup:
    free(base64_histogram);
    free(compressed_histogram);

    return result;
}

static int __hdr_log_write_str(struct hdr_log_writer* writer,
    ina_str_t *str,
    const struct timespec* start_timestamp,
    const struct timespec* end_timestamp,
    struct hdr_histogram* histogram)
{
    uint8_t* compressed_histogram = NULL;
    size_t compressed_len = 0;
    char* encoded_histogram = NULL;
    int rc = 0;
    int result = 0;
    size_t encoded_len;

    rc = hdr_encode_compressed(histogram, &compressed_histogram, &compressed_len);
    if (rc != 0)
    {
        FAIL_AND_CLEANUP(cleanup, result, rc);
    }

    encoded_len = hdr_base64_encoded_len(compressed_len);
    encoded_histogram = (char*)calloc(encoded_len + 1, sizeof(char));

    rc = hdr_base64_encode(
        compressed_histogram, compressed_len, encoded_histogram, encoded_len);
    if (rc != 0)
    {
        FAIL_AND_CLEANUP(cleanup, result, rc);
    }

    *str = ina_str_new(encoded_len+256);
    if (ina_str_snprintf(&(*str), encoded_len+256,
        "%d.%d,%d.%d,%"PRIu64".0,%s\n",
        (int) start_timestamp->tv_sec, (int) (start_timestamp->tv_nsec / 1000000),
        (int) end_timestamp->tv_sec, (int) (end_timestamp->tv_nsec / 1000000),
        hdr_max(histogram),
        encoded_histogram) < 0)
    {
        result = EIO;
    }

cleanup:
    free(compressed_histogram);
    free(encoded_histogram);

    return result;
}

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
    ina_str_free((*recorder)->id);
    if ((*recorder)->free_text1 != NULL) {
        ina_str_free((*recorder)->free_text1);
    }
    if ((*recorder)->free_text2 != NULL) {
        ina_str_free((*recorder)->free_text2);
    }
    if ((*recorder)->free_text3 != NULL) {
        ina_str_free((*recorder)->free_text3);
    }
    if ((*recorder)->free_text4 != NULL) {
        ina_str_free((*recorder)->free_text4);
    }
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
    ina_time_event_t *e = NULL;
    time_t time_ms = (time_t)(time_ns/1000/1000);
    e = ina_timer_next_event_with_time(recorder->timer, time_ms);
    /* phase change */
    if (e && e->id == recorder->phase->id) {
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
    *serializer = (ina_histogram_serializer_t*)ina_mem_alloc(sizeof(ina_histogram_serializer_t));

    ina_str_cpy((*serializer)->id, id);

    if (!INA_SUCCEED(INA_ULLC_PRODUCER_CREATE(
        ina_histogram_record_t,
        __INA_HISTOGRAM_ULLC_VERSION,
        __INA_HISTOGRAM_ULLC_SOLTS,
        __INA_HISTOGRAM_ULLC_PRODUCERS,
        __INA_HISTOGRAM_ULLC_CONSUMERS,
        ina_str_cstr(id),
        INA_ULLC_WS_BUSY_WAIT,
        &(*serializer)->dummy))) {
            return INA_ERR_PUSH_LAST;
    }

    if (!INA_SUCCEED(INA_ULLC_CONSUMER_CREATE(
        ina_histogram_record_t,
        __INA_HISTOGRAM_ULLC_VERSION,
        __INA_HISTOGRAM_ULLC_SOLTS,
        __INA_HISTOGRAM_ULLC_PRODUCERS,
        __INA_HISTOGRAM_ULLC_CONSUMERS,
        ina_str_cstr(id),
        &(*serializer)->consumer))) {
            return INA_ERR_PUSH_LAST;
    }

    hdr_log_writer_init(&(*serializer)->writer);

    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_serializer_free(ina_histogram_serializer_t **serializer)
{
    ina_ullc_consumer_destroy(&(*serializer)->consumer);
    ina_ullc_producer_destroy(&(*serializer)->dummy);
    ina_str_free((*serializer)->id);
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_serializer_serialize(ina_histogram_serializer_t *serializer, 
                                                     ina_str_t *record,
                                                     ina_histogram_meta_t *meta)
{
    ina_histogram_record_t *r = INA_ULLC_GET(ina_histogram_record_t, serializer->consumer);
    
    if (r != NULL) {
        struct hdr_histogram *h = (struct hdr_histogram*)r->data;
        struct timespec st, et;

        st.tv_sec = (time_t)r->start_ts_ns/1000;
        st.tv_nsec = r->start_ts_ns % 1000;
        et.tv_sec = (time_t)r->end_ts_ns/1000;
        et.tv_nsec = r->end_ts_ns % 1000;
    
        __hdr_log_write_str(&serializer->writer, record, &st, &et, h);
    
        strcpy(meta->free_text1, r->free_text1);
        strcpy(meta->free_text2, r->free_text2);
        strcpy(meta->free_text3, r->free_text3);
        strcpy(meta->free_text4, r->free_text4);
    }
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_reporter_new(ina_histogram_reporter_t **reporter)
{
    *reporter = (ina_histogram_reporter_t*)ina_mem_alloc(sizeof(ina_histogram_reporter_t));

    hdr_log_reader_init(&(*reporter)->reader);

    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_reporter_free(ina_histogram_reporter_t **reporter)
{
    ina_mem_free(*reporter);
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_histogram_reporter_print_percentile(ina_histogram_reporter_t *reporter, 
                                                          const ina_str_t record,
                                                          FILE *stream,
                                                          int32_t ticks_per_half_distance,
                                                          double value_scale)
{
    struct hdr_histogram *h;
    struct timespec ts, interval;

    __hdr_log_read_str(&reporter->reader, record, &h, &ts, &interval);
    hdr_percentiles_print(h, stream, ticks_per_half_distance, value_scale, CLASSIC);

    return INA_SUCCESS;
}
