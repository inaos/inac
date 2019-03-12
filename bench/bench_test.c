/*
 * Copyright INAOS GmbH, Thalwil, 2019. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

INA_BENCH_DATA(test) {
    int c;
};

INA_BENCH_SETUP(test)
{
    INA_BENCH_MSG("%s", "INA_BENCH_SETUP");
    INA_BENCH_INIT("scale", 0, 10, 10);
}

INA_BENCH_SCALE(test)
{
    INA_BENCH_MSG("%s", "INA_BENCH_SCALE");
    ina_bench_set_scale(ina_bench_get_repetition());
    data->c = 10 * ina_bench_get_repetition();
}

INA_BENCH_BEGIN(test, test1)
{
    INA_BENCH_MSG("%s", "INA_BENCH_BEGIN");
}

INA_BENCH(test, test1)
{
    int k;
    for (k = 0; k < data->c; k++) {
        ina_bench_get_value();
    }
    ina_bench_set_value(k);

}

INA_BENCH_END(test, test1)
{
    INA_BENCH_MSG("%s", "INA_BENCH_END");

}

INA_BENCH_BEGIN(test, test2)
{
    INA_BENCH_MSG("%s", "INA_BENCH_BEGIN");
}

INA_BENCH(test, test2)
{
    int k;
    for (k = 0; k < data->c; k++) {
        ina_bench_get_value();
    }
    ina_bench_set_value(k);

}

INA_BENCH_END(test, test2)
{
    INA_BENCH_MSG("%s", "INA_BENCH_END");

}


INA_BENCH_TEARDOWN(test)
{
    INA_BENCH_MSG("%s", "INA_BENCH_TEARDOWN");

}