/*
 * Copyright INAOS GmbH, Thalwil, 2019. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

INA_BENCH_DATA(test1) {
    int c;
};

INA_BENCH_SETUP(test1)
{
    INA_BENCH_MSG("%s", "INA_BENCH_SETUP");
    INA_BENCH_INIT("scale", 0, 12, 10);
}

INA_BENCH_SCALE(test1)
{
    INA_BENCH_MSG("%s", "INA_BENCH_SCALE");
    ina_bench_set_scale(ina_bench_get_repetition());
    data->c = 10 * ina_bench_get_repetition() ;
}

INA_BENCH_BEGIN(test1, series1)
{
    INA_BENCH_MSG("%s", "INA_BENCH_BEGIN");
}

INA_BENCH(test1, series1)
{
    ina_bench_set_value(data->c);

}

INA_BENCH_END(test1, series1)
{
    INA_BENCH_MSG("%s", "INA_BENCH_END");

}

INA_BENCH_BEGIN(test1, series2)
{
    INA_BENCH_MSG("%s", "INA_BENCH_BEGIN");
}

INA_BENCH(test1, series2)
{
    ina_bench_set_value(data->c);

}

INA_BENCH_END(test1, series2)
{
    INA_BENCH_MSG("%s", "INA_BENCH_END");

}


INA_BENCH_TEARDOWN(test1)
{
    INA_BENCH_MSG("%s", "INA_BENCH_TEARDOWN");

}

INA_BENCH_DATA(test2) {
    int c;
};

INA_BENCH_SETUP(test2)
{
    INA_BENCH_MSG("%s", "INA_BENCH_SETUP");
    INA_BENCH_INIT("scale", 0, 10, 10);
}

INA_BENCH_SCALE(test2)
{
    INA_BENCH_MSG("%s", "INA_BENCH_SCALE");
    ina_bench_set_scale(ina_bench_get_repetition());
    data->c = 10 * ina_bench_get_repetition();
}

INA_BENCH_BEGIN(test2, series1)
{
    INA_BENCH_MSG("%s", "INA_BENCH_BEGIN");
}

INA_BENCH(test2, series1)
{
    ina_bench_set_value(data->c);

}

INA_BENCH_END(test2, series1)
{
    INA_BENCH_MSG("%s", "INA_BENCH_END");

}

INA_BENCH_BEGIN(test2, series2)
{
    INA_BENCH_MSG("%s", "INA_BENCH_BEGIN");
}

INA_BENCH(test2, series2)
{
    ina_bench_set_value(data->c);

}

INA_BENCH_END(test2, series2)
{
    INA_BENCH_MSG("%s", "INA_BENCH_END");

}


INA_BENCH_TEARDOWN(test2)
{
    INA_BENCH_MSG("%s", "INA_BENCH_TEARDOWN");

}