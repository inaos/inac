/*
 * Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include "test_ullc.h"

static ina_ipc_flags_t *flags = NULL;



static void ina_test_helper_cleanup(int error, int *exitcode) {
    INA_UNUSED(error);
    INA_UNUSED(exitcode);
    ina_ipc_flags_free(&flags);
}

/* Create a single */
INA_TEST_HELPER(ipc, set_unset_flag) {
    
    char *name;
    ina_set_cleanup_handler(ina_test_helper_cleanup);

    INA_TEST_HELPER_CHECK_ARGC(1);
    name = INA_TEST_HELPER_CARG(0);

  
    if (!INA_SUCCEED(ina_ipc_flags_open(name, &flags))) {
        INA_TEST_HELPER_EXIT(ina_err_get_rc());
    }
    if (!INA_SUCCEED(ina_ipc_flags_set(flags, INA_IPC_FLAGS_13))) {
        INA_TEST_HELPER_EXIT(ina_err_get_rc());
    }

    ina_time_sleep(2000);

    if (!INA_SUCCEED(ina_ipc_flags_unset(flags, INA_IPC_FLAGS_13))) {
        INA_TEST_HELPER_EXIT(ina_err_get_rc());
    }

    INA_TEST_HELPER_SET_RC(INA_SUCCESS);
 }

