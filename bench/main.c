/*
 * Copyright 2018-2020 INAOS GmbH, Thalwil
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

int main(int argc,  char** argv) 
{
    INA_OPTS(opt,
             INA_OPT_STRING("r", "report-path", "."INA_PATH_SEPARATOR_STR, "Directory for report output"),
             INA_OPT_INT(NULL, "x-repeat", INA_NUM2STR(0), "Override number of repetitions"),
             INA_OPT_INT(NULL, "x-iter", INA_NUM2STR(0), "Override number of iteration"),
             INA_OPT_INT(NULL, "x-warm-up", INA_NUM2STR(3), "Warm-up iterations"),
             INA_OPT_INT(NULL, "cache-size", INA_NUM2STR(0), "L1/L2/L3 cache size"),
             INA_OPT_INT("c", "core", INA_NUM2STR(-1), "Pin core"),
             INA_OPT_FLAG(NULL, "disable-aggregation", "Disable result aggregation"),
             INA_OPT_STRING("n", "name", "", "Benchmark name"));

    if (INA_FAILED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }
    return ina_bench_run();
}
