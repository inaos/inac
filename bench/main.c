/*
 * Copyright INAOS GmbH, Thalwil, 2018. All rights reserved
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
             INA_OPT_STRING("r", "report-path", NULL, "Directory for report output"));

    if (INA_FAILED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }
    return ina_bench_run(argc, argv);
}
