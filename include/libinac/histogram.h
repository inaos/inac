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
#ifndef _LIBINAC_HISTOGRAM_H_
#define _LIBINAC_HISTOGRAM_H_

/*
 * http://en.wikipedia.org/wiki/Histogram
 * 
 * Some applications process millions of events per second, meaning that millions
 * of measurements will be taken every second. You might be asking yourself: is
 * it possible to store millions of measurements efficiently without incurring
 * in significant memory and CPU overhead? Yes, it is possible, thanks to the
 * HdrHistogram developed by Gil Tene, and it is backing our Histogram
 * implementation.
 * 
 * The HdrHistogram mixes linear and exponential bucket systems to produce a
 * unique data structure capable of recording measurements with configurable
 * precision and with fixed memory and cpu costs, regardless of the number of
 * measurements recorded.
 *
 * ---- From the HdrHistogram documentation ----
 * Internally, data in HdrHistogram variants is maintained using a concept 
 * somewhat similar to that of floating point number representation: Using an
 * exponent a (non-normalized) mantissa to support a wide dynamic range at a
 * high but varying (by exponent value) resolution. AbstractHistogram uses
 * exponentially increasing bucket value ranges (the parallel of the exponent
 * portion of a floating point number) with each bucket containing a fixed
 * number (per bucket) set of linear sub-buckets (the parallel of a
 * non-normalized mantissa portion of a floating point number).
 * Both dynamic range and resolution are configurable, with
 * highestTrackableValue controlling dynamic range, and
 * numberOfSignificantValueDigits controlling resolution.
 * ----
 * 
 * We have three components:
 * - Recorder
 * - Serializer
 * - Reporter
 *
 * Our implementation uses the ULLC rings to transfer the samples from the
 * recorder to the serializer component. The recorder is a producer and the
 * serializer is dummy producer because it has to be created first and a
 * consumer to consume and serialize the samples.
 *
 */


#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* opaque histogram recorder */
typedef struct ina_histogram_recorder_s ina_histogram_recorder_t;

/* opaque histogram serializer */
typedef struct ina_histogram_serializer_s ina_histogram_serializer_t;

/* opaque histogram reporter */
typedef struct ina_histogram_reporter_s ina_histogram_reporter_t;

typedef struct ina_histogram_record_s {
    int64_t start_ts_ns;
    int64_t end_ts_ns;
    char data[128]; /* size of the hdr_histogram = 96, but rounding-up */
    char free_text1[128];
    char free_text2[128];
    char free_text3[128];
    char free_text4[128];
} ina_histogram_record_t;

typedef struct ina_histogram_meta_s {
    char free_text1[128];
    char free_text2[128];
    char free_text3[128];
    char free_text4[128];
} ina_histogram_meta_t;

/*
 * Create an new histogram recoder.
 *
 * Parameters
 *  recorder                 Where to store the newly created recorder
 *  id                       Recorder id
 *  highest_trackable_value  Configure dynamic range by defining highest
 *                           trackable value
 *  significant_figures      Configure the resolution by defining the number
 *                           of significant figures
 *  sample_interval_ms       Define interval of sample recording in milliseconds
 *
 * Return
 * INA_SUCCESS if all went well.
 */
INA_API(ina_rc_t) ina_histogram_recorder_new(ina_histogram_recorder_t **recorder,
                                             ina_str_t id,
                                             int64_t highest_trackable_value,
                                             int significant_figures,
                                             int sample_interval_ms);

/*
 * Destroy a histogram recorder.
 *
 * Parameters
 *  recorder  Recorder to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_histogram_recorder_free(ina_histogram_recorder_t **recorder);

/*
 * Set a free text 1
 *
 * Parameters
 *  recorder  Histogram recorder
 *  text      Free text 1
 *
 * Return
 * INA_SUCCESS
 */
INA_API(ina_rc_t) ina_histogram_recorder_set_free_text1(
                                            ina_histogram_recorder_t *recorder,
                                            const char *text);

/*
 * Set a free text 2
 *
 * Parameters
 *  recorder  Histogram recorder
 *  text      Free text 2
 *
 * Return
 * INA_SUCCESS
 */
INA_API(ina_rc_t) ina_histogram_recorder_set_free_text2(
                                            ina_histogram_recorder_t *recorder,
                                            const char *text);

/*
 * Set a free text 3
 *
 * Parameters
 *  recorder  Histogram recorder
 *  text      Free text 3
 *
 * Return
 * INA_SUCCESS
 */
INA_API(ina_rc_t) ina_histogram_recorder_set_free_text3(
                                            ina_histogram_recorder_t *recorder,
                                            const char *text);

/*
 * Set a free text 4
 *
 * Parameters
 *  recorder  Histogram recorder
 *  text      Free text 4
 *
 * Return
 * INA_SUCCESS
 */
INA_API(ina_rc_t) ina_histogram_recorder_set_free_text4(
                                            ina_histogram_recorder_t *recorder,
                                            const char *text);

/*
 * Record a histogram value.
 *
 * Parameters
 *  recorder  Histogram recorder
 *  value     Value to record
 *
 * Return
 * INA_SUCCESS
 */
INA_API(ina_rc_t) ina_histogram_recorder_record(
                                            ina_histogram_recorder_t *recorder,
                                            int64_t value);

/*
 * Process the recorded data by copying into a ring buffer for serialization.
 *
 * Parameters
 *  recorder  Histogram recorder
 *  time_ns   Time in nanoseconds to be elapsed before write to the ring.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_histogram_recorder_process(
                                            ina_histogram_recorder_t *recorder,
                                            int64_t time_ns);

/*
 * Create a new histogram serializer.
 *
 * Parameters
 *  serializer               Where to store the newly created serializer
 *  id                       Serializer identifier
 *  highest_trackable_value  Configure dynamic range by defining highest
 *                           trackable value
 *  significant_figures      Configure the resolution by defining the number
 *                           of significant figures
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_histogram_serializer_new(
                                        ina_histogram_serializer_t **serializer,
                                        ina_str_t id,
                                        int64_t highest_trackable_value,
                                        int significant_figures);

/*
 * Destroy a histogram serializer.
 *
 * Parameters
 *  serializer  Histogram serializer to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_histogram_serializer_free(
                                        ina_histogram_serializer_t **serializer);

/*
 * Serialize histogram data previously recorded.
 *
 * Parameters
 *  serializer   Histogram serializer
 *  record       Histogram data
 *  meta         Histogram meta data
 *
 * Return
 *  INA_SUCCESS if all went well
 *  INA_EAGAIN  if no data is currently available.
 */
INA_API(ina_rc_t) ina_histogram_serializer_serialize(
                                        ina_histogram_serializer_t *serializer,
                                        ina_str_t *record,
                                        ina_histogram_meta_t *meta);

/*
 * Create a new histogram reporter.
 *
 * Parameters
 *  reporter  Where to store the newly created reporter
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_histogram_reporter_new(ina_histogram_reporter_t **reporter);

/*
 * Destroy a histogram reporter.
 *
 * Parameters
 *  reporter  Histogram reporter to destroy
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_histogram_reporter_free(ina_histogram_reporter_t **reporter);

/*
 * Print percentile to a file stream.
 *
 * Parameters
 *  reporter                 Histogram reporter
 *  record                   Histogram data
 *  stream                   File to write to
 *  ticks_per_half_distance  Granularity of printed values
 *  value_scale              Multiplier for results
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_histogram_reporter_print_percentile(
                                            ina_histogram_reporter_t *reporter,
                                            const ina_str_t record,
                                            FILE *stream,
                                            int32_t ticks_per_half_distance,
                                            double value_scale);

#ifdef __cplusplus
}
#endif

#endif
