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