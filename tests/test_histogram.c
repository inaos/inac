/*
 * Copyright (c) 2016-2017 INAOS GmbH
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

static int64_t __test_histogram_samples[200] = {
  	22220,21924,22117,22313,21970,22315,27235,
    24301,22148,21938,22391,128511,22771,24856,
    22339,22214,21936,22099,22013,23445,22333,
    25448,22176,22312,22007,26503,22105,22131,
    22428,22627,21991,23628,22069,22842,22083,
    23075,21966,22771,22060,22320,22713,22066,
    22204,22195,22219,24509,22241,21922,25240,
    23569,22016,22362,22907,21860,24221,23531,
    22314,25964,22083,22753,21991,22864,22876,
    22400,22638,23046,22065,26561,22064,22160,
    22012,106464,22130,22136,22521,22723,22053,
    22138,21959,22266,22042,22589,22068,23685,
    22016,22363,32323,21943,33482,23898,22427,
    23290,22984,22237,22291,22464,22051,22355,
    27171,22815,22188,22232,22362,21878,22048,
    21975,23136,22804,22731,22387,52520,22666,
    22360,22459,22706,22452,21974,21992,22421,
    23352,22038,22905,22441,22187,22022,22517,
    22759,23805,22406,22524,22181,22112,22131,
    22684,21922,22201,22018,22272,22459,22011,
    22786,21925,22085,23033,22014,21961,22134,
    23839,22381,24390,22667,21877,22104,22620,
    22114,22402,22344,22090,22149,22086,23529,
    22345,22669,22062,22713,22635,22760,25581,
    22669,22331,22632,22008,22349,22117,23557,
    22409,21977,22936,22051,22283,22044,26639,
    22158,22608,22081,22453,22656,21998,23159,
    22162,22028,22212,22433,22057,26686,22164,
    22038,21986,22275
};

INA_TEST(histogram, roundtrip_remote_system) {
    int i;
    ina_histogram_recorder_t *recorder = NULL;
    ina_histogram_serializer_t *serializer = NULL;
    ina_histogram_reporter_t *reporter = NULL;
    time_t now = time(NULL);
    int64_t now_ns = now*INT64_C(1000*1000*1000);
    int64_t one_sec = INT64_C(1*1100*1000*1000);
    ina_str_t record = NULL;
    ina_histogram_meta_t meta;
    ina_str_t id = ina_str_new_fromcstr("ina_test_recorder");
    FILE *f = fopen("test_histogram.txt", "w");
    
    ina_mem_set(&meta, 0, sizeof(ina_histogram_meta_t));

    INA_TEST_ASSERT_SUCCEED(ina_histogram_reporter_new(&reporter));
    INA_TEST_ASSERT_SUCCEED(ina_histogram_serializer_new(&serializer, 
        id, 
        INT64_C(24) * 60 * 60 * 1000000,
        3));
    INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_new(&recorder,
        id,
        INT64_C(24) * 60 * 60 * 1000000,
        3,
        1000));

    ina_histogram_reporter_write_header(reporter, f, now_ns);
    ina_histogram_recorder_start(recorder, now_ns);
    for (i = 0; i < 50; i++) {
        INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_record(recorder, __test_histogram_samples[i]));
    }
    now_ns += one_sec;
    INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_process(recorder, now_ns));

    for (i = 50; i < 51; i++) {
        INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_record(recorder, __test_histogram_samples[i]));
    }
    now_ns += one_sec;
    INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_process(recorder, now_ns));

    for (i = 51; i < 60; i++) {
        INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_record(recorder, __test_histogram_samples[i]));
    }
    now_ns += one_sec;
    INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_process(recorder, now_ns));

    for (i = 60; i < 100; i++) {
        INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_record(recorder, __test_histogram_samples[i]));
    }
    now_ns += one_sec;
    INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_process(recorder, now_ns));

    for (i = 100; i < 120; i++) {
        INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_record(recorder, __test_histogram_samples[i]));
    }
    now_ns += one_sec;
    INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_process(recorder, now_ns));

    for (i = 120; i < 160; i++) {
        INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_record(recorder, __test_histogram_samples[i]));
    }
    now_ns += one_sec;
    INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_process(recorder, now_ns));

    for (i = 160; i < 200; i++) {
        INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_record(recorder, __test_histogram_samples[i]));
    }
    now_ns += one_sec;
    INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_process(recorder, now_ns));

    fprintf(stdout, "\n");
    ina_histogram_serializer_serialize(serializer, &record, &meta);
    INA_TEST_ASSERT_NOT_NULL(record);
    while (record != NULL) {
        //ina_histogram_reporter_print_percentile(reporter, record, stdout, 5, 1.0);
        ina_histogram_reporter_write(reporter, f, record);
        ina_histogram_serializer_serialize(serializer, &record, &meta);
    }

    INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_free(&recorder));
    INA_TEST_ASSERT_SUCCEED(ina_histogram_serializer_free(&serializer));
    INA_TEST_ASSERT_SUCCEED(ina_histogram_reporter_free(&reporter));
    ina_str_free(id);

    fclose(f);
}

