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
    INA_UNUSED(data);
}
INA_BENCH_BEGIN(test, test1)
{
    INA_UNUSED(data);
}
INA_BENCH_SCALE(test)
{
    ina_bench_set_scale(ina_bench_get_repetition());
    data->c = 10 * ina_bench_get_repetition();
}
INA_BENCH(test, test1, 100, 10)
{
    int k;
    for (k = 0; k < data->c; k++) {
        ina_bench_get_value();
    }
    ina_bench_set_value(k);

}
INA_BENCH_END(test, test1)
{
    INA_UNUSED(data);
}
INA_BENCH_TEARDOWN(test)
{
    INA_UNUSED(data);
}