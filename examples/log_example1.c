/*
 * Copyright INAOS GmbH, Thalwil, 2012-2019. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <stdio.h>
#include <libinac/lib.h>

int main(int argc,  char** argv)
{
    if (INA_FAILED(ina_app_init(argc, argv, NULL))) {
        return EXIT_FAILURE;
    }
    if (INA_FAILED(ina_log_init(NULL))) {
        return EXIT_FAILURE;
    }

    INA_LOG_DEBUG("this is a simple DEBUG message to stdout");
    INA_LOG_INFO("this is a simple INFO message to the stdout");
    INA_LOG_WARNING("this is a simple WARNING message to stdout");
    INA_LOG_ERROR("this is a simple ERROR message to stderr ");

    INA_LOG_ERROR("this is a ERROR message with var args (p1=%d) to stderr", 100);


    ina_log_destroy();

    return EXIT_SUCCESS;
}
