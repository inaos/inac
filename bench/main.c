/*
 * Copyright INAOS GmbH, Thalwil, 2018-2019. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

int main(int argc,  char** argv) 
{
    INA_OPTS(opt,
             INA_OPT_STRING("r", "report-path", "."INA_PATH_SEPARATOR_STR, "Directory for report output"),
             INA_OPT_INT(NULL, "x-repeat", INA_NUM2STR(0), "Override number of repetitions"),
             INA_OPT_INT(NULL, "x-iter", INA_NUM2STR(0), "Override number of iteration"),
             INA_OPT_INT(NULL, "x-warm-up", INA_NUM2STR(0), "Warm-up iteration"),
             INA_OPT_INT(NULL, "cache-size", INA_NUM2STR(0), "L1/L2/L3 cache size"),
             INA_OPT_INT("c", "core", INA_NUM2STR(-1), "Pin core"),
             INA_OPT_STRING("n", "name", "", "Benchmark name"));

    if (INA_FAILED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }
    return ina_bench_run();
}
