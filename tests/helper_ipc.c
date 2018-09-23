/*
 * Copyright (c) 2014-2018, INAOS GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the INAOS GmbH nor the names of its contributors
 *       may be used to endorse or promote products derived from this software 
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <libinac/lib.h>
#include "test_ullc.h"

static ina_ipc_flags_t *flags = NULL;



static void ina_test_helper_cleanup(int error, int *exitcode) {
    ina_ipc_flags_free(&flags);
}

/* Create a single */
INA_TEST_HELPER(ipc, set_unset_flag) {
    
    char *name;
    ina_set_cleanup_handler(ina_test_helper_cleanup);

    INA_TEST_HELPER_CHECK_ARGC(1);
    name = INA_TEST_HELPER_CARG(0);

  
    if (!INA_SUCCEED(ina_ipc_flags_open(name, &flags))) {
        INA_TEST_HELPER_EXIT(ina_err_get_last_rc());
    }
    if (!INA_SUCCEED(ina_ipc_flags_set(flags, INA_IPC_FLAGS_13))) {
        INA_TEST_HELPER_EXIT(ina_err_get_last_rc());
    }

    ina_time_sleep(2000);

    if (!INA_SUCCEED(ina_ipc_flags_unset(flags, INA_IPC_FLAGS_13))) {
        INA_TEST_HELPER_EXIT(ina_err_get_last_rc());
    }

    INA_TEST_HELPER_SET_RC(INA_SUCCESS);
 }

