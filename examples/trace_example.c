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

    INA_TRACE(*, "current trace filter=%s", getenv("INAC_TRACE"));

    INA_TRACE(example.main, "trace example.main category");
    INA_TRACE(inac.examples, "trace message for category inac.examples.*");
    INA_TRACE(inac.examples, "trace message for category inac.examples.* val=%d", 1);
    INA_TRACE(inac.examples.trace, "trace message for category inac.examples.trace");

    return EXIT_SUCCESS;
}
