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


int main(int argc,  char** argv)
{
    if (INA_FAILED(ina_app_init(argc, argv, NULL))) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
