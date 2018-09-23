/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>


INA_TEST_HELPER(process, spawn_and_wait)  {
    int exit_code = INA_TEST_HELPER_IARG(0);
    printf("HELPER spawn_and_wait STARTED, exit code = %d", exit_code);
    ina_time_sleep(2000);
    INA_TEST_HELPER_SET_RC(exit_code);
}
