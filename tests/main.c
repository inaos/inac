/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <stdio.h>
#include <libinac/lib.h>

#define INAC_TEST_INT_PARAM 121


int main(int argc,  char** argv) 
{
    INA_OPTS(opt,
        INA_OPT_FLAG("h", "helper", "Start a helper"),
        INA_OPT_INT("t", "testint", INAC_TEST_INT_PARAM, "Test intargument"),
        INA_OPT_INT("x", "repeat", 1, "Test int argument"),
        INA_OPT_FLOAT("f", "float", 1.02, "Test float argument"),
        INA_OPT_STRING("r", "run", "all", "Test string argument"),
        INA_OPT_STRING(NULL, "long-option", "long", "This is a long option without short option"),
        INA_OPT_STRING(NULL, "format", "inac", "Format: tap=Test Anything Protocol, junit=JUnit"));

    if (INA_FAILED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }
    return ina_test_run(argc, argv, NULL);
}
