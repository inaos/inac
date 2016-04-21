/*
 * Copyright (c) 2016 INAOS GmbH
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
  	270919,291812,278014,344222,303965,263408,328418,758164,
	293400,261438,276039,487205,292468,268941,309731,290728,
	272495,385159,302925,268589,284899,336305,313902,288263,
	317853,268829,288691,276141,311811,288089,287207,267362,
	333445,355059,320249,261990,272730,291386,313317,269409,
	279987,255697,483655,618533,287671,267270,362711,264866,
	294784,284082,308582,308437,276707,287734,271856,272858,
	286571,340023,501338,273194,285953,870960,342777,482570,
	1507240,401264,354430,294495,273148,258347,276013,260629,
	276336,627073,354531,259990,285908,1043509,968356,250967,
	298714,310196,287334,265627,290655,280329,287576,285775,
	292190,301599,304254,273842,330485,330266,484109,435444,
	363542,284190,288732,313206,283362,1274899,283601,270792,
	286783,285412,318426,264900,298408,272237,294909,1413854,
	1993843,1213858,297172,273579,305633,310359,271169,273758,
	430661,614023,294362,261202,291101,264220,287524,274138,
	321801,272527,292836,258325,289942,258040,289663,1462132,
	290728,285350,688629,264657,381524,313542,454162,1051379,
	287519,296564,302740,291228,312370,910589,321491,274529,
	306307,258245,303081,257884,366780,1034799,352741,266791,
	274581,289501,280638,326654,315804,270056,282051,271361,
	303637,309921,775853,280767,313582,305971,287146,286304,
	290649,270637,298135,254138,281250,291806,282220,271832,
	287993,532174,268451,1084772,305206,308922,334738,353142,
	297288,288795,334404,266854,305081,300544,401103,401103
};

INA_TEST_SKIP(histogram, roundtrip_remote_system) {
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
    
    ina_mem_set(&meta, 0, sizeof(ina_histogram_meta_t));

    /*INA_TEST_ASSERT_SUCCEED(ina_histogram_reporter_new(&reporter));
    INA_TEST_ASSERT_SUCCEED(ina_histogram_serializer_new(&serializer, 
        id, 
        INT64_C(1*1000*1000*1000),
        5));*/
    INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_new(&recorder,
        id,
        INT64_C(1*1000*1000*1000),
        //INT64_MAX,
        3,
        1000));

    for (i = 0; i < 5; i++) {
        INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_record(recorder, __test_histogram_samples[i]));
    }
    now_ns += one_sec;
    INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_process(recorder, now_ns));

    for (i = 5; i < 15; i++) {
        INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_record(recorder, __test_histogram_samples[i]));
    }
    now_ns += one_sec;
    INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_process(recorder, now_ns));

    for (i = 15; i < 35; i++) {
        INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_record(recorder, __test_histogram_samples[i]));
    }
    now_ns += one_sec;
    INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_process(recorder, now_ns));

    for (i = 35; i < 50; i++) {
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

    ina_histogram_serializer_serialize(serializer, &record, &meta);
    INA_TEST_ASSERT_NOT_NULL(record);
    while (record != NULL) {
        ina_histogram_serializer_serialize(serializer, &record, &meta);
    }

    INA_TEST_ASSERT_SUCCEED(ina_histogram_recorder_free(&recorder));
    INA_TEST_ASSERT_SUCCEED(ina_histogram_serializer_free(&serializer));
    INA_TEST_ASSERT_SUCCEED(ina_histogram_reporter_free(&reporter));
    ina_str_free(id);
}

