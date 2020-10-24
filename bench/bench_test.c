/*
 * Copyright 2019-2020 INAOS GmbH, Thalwil
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <libinac/lib.h>

INA_BENCH_DATA(test1) {
    int c;
};

INA_BENCH_SETUP(test1)
{
    INA_UNUSED(data);
    INA_BENCH_MSG("%s", "INA_BENCH_SETUP");
    INA_BENCH_INIT("scale", 0, 10, 10);
}

INA_BENCH_SCALE(test1)
{
    INA_UNUSED(data);
    INA_BENCH_MSG("INA_BENCH_SCALE(%d)", ina_bench_get_repetition());
    ina_bench_set_scale(ina_bench_get_repetition());
}

INA_BENCH_BEGIN(test1, series1)
{
    data->c = 10 * ina_bench_get_repetition() ;
    INA_BENCH_MSG("%s", "INA_BENCH_BEGIN");
}

INA_BENCH(test1, series1)
{
    ina_bench_set_value(data->c);

}

INA_BENCH_END(test1, series1)
{
    INA_UNUSED(data);
    INA_BENCH_MSG("%s", "INA_BENCH_END");

}

INA_BENCH_BEGIN(test1, series2)
{
    data->c = 10 * ina_bench_get_repetition() ;
    INA_BENCH_MSG("%s", "INA_BENCH_BEGIN");
}

INA_BENCH(test1, series2)
{
    ina_bench_set_value(data->c);

}

INA_BENCH_END(test1, series2)
{
    INA_UNUSED(data);
    INA_BENCH_MSG("%s", "INA_BENCH_END");

}


INA_BENCH_TEARDOWN(test1)
{
    INA_UNUSED(data);
    INA_BENCH_MSG("%s", "INA_BENCH_TEARDOWN");

}

INA_BENCH_DATA(test2) {
    int c;
};

INA_BENCH_SETUP(test2)
{
    INA_UNUSED(data);
    INA_BENCH_MSG("%s", "INA_BENCH_SETUP");
    INA_BENCH_INIT("scale", 0, 10, 10);
}

INA_BENCH_SCALE(test2)
{
    INA_UNUSED(data);
    INA_BENCH_MSG("INA_BENCH_SCALE(%d)", ina_bench_get_repetition());
    ina_bench_set_scale(ina_bench_get_repetition());
}

INA_BENCH_BEGIN(test2, series1)
{
    INA_BENCH_MSG("%s", "INA_BENCH_BEGIN");
    data->c = 10 * ina_bench_get_repetition() ;

}

INA_BENCH(test2, series1)
{
    ina_bench_set_value(data->c);

}

INA_BENCH_END(test2, series1)
{
    INA_UNUSED(data);
    INA_BENCH_MSG("%s", "INA_BENCH_END");

}

INA_BENCH_BEGIN(test2, series2)
{
    INA_BENCH_MSG("%s", "INA_BENCH_BEGIN");
    data->c = 10 * ina_bench_get_repetition() ;
}

INA_BENCH(test2, series2)
{
    ina_bench_set_value(data->c);

}

INA_BENCH_END(test2, series2)
{
    INA_UNUSED(data);
    INA_BENCH_MSG("%s", "INA_BENCH_END");

}


INA_BENCH_TEARDOWN(test2)
{
    INA_UNUSED(data);
    INA_BENCH_MSG("%s", "INA_BENCH_TEARDOWN");

}


INA_BENCH_DATA(test_too_slow) {
    int c;
};

INA_BENCH_SETUP(test_too_slow)
{
    INA_UNUSED(data);
    INA_BENCH_MSG("%s", "INA_BENCH_SETUP");
    INA_BENCH_INIT("scale", 0, 10, 10);
    ina_bench_set_max_duration(1.0);
}

INA_BENCH_SCALE(test_too_slow)
{
    INA_UNUSED(data);
    INA_BENCH_MSG("INA_BENCH_SCALE(%d)", ina_bench_get_repetition());
    ina_bench_set_scale(ina_bench_get_repetition());
}

INA_BENCH_BEGIN(test_too_slow, series_fast)
{
    INA_BENCH_MSG("%s", "INA_BENCH_BEGIN");
    data->c = 10 * ina_bench_get_repetition() ;

}

INA_BENCH(test_too_slow, series_fast)
{
    ina_bench_set_value(data->c);

}

INA_BENCH_END(test_too_slow, series_fast)
{
    INA_UNUSED(data);
    INA_BENCH_MSG("%s", "INA_BENCH_END");

}

INA_BENCH_BEGIN(test_too_slow, series_slow)
{
    INA_BENCH_MSG("%s", "INA_BENCH_BEGIN");
    data->c = 10 * ina_bench_get_repetition() ;
}

INA_BENCH(test_too_slow, series_slow)
{
    if (ina_bench_get_iteration() == 2) {
        ina_time_sleep(2000);
    }
    ina_bench_set_value(data->c);
}

INA_BENCH_END(test_too_slow, series_slow)
{
    INA_UNUSED(data);
    INA_BENCH_MSG("%s", "INA_BENCH_END");
}


INA_BENCH_TEARDOWN(test_too_slow)
{
    INA_UNUSED(data);
    INA_BENCH_MSG("%s", "INA_BENCH_TEARDOWN");
}