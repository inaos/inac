/*
 * Copyright (c) 2013, INAOS GmbH
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

static ina_iscp_ctx_t *__iscp = NULL;
static int __running = 0;

static int __cleanup_handler(const int sig, const int error) 
{
    ina_iscp_destroy(&__iscp);
    return EXIT_SUCCESS;
}

static ina_rc_t __command_1_handler(int cmd_id, int count, ina_iscp_param_t *params)
{
    return INA_SUCCESS;
}

static ina_rc_t __command_2_handler(int cmd_id, int count, ina_iscp_param_t *params)
{
    __running = 0;
    return INA_SUCCESS;
}

INA_TEST_HELPER(iscp, tcp_server) {

    INA_ISCP_CMDS(cmds,
           INA_ISCP_SENDRECV_CMD(1, 3, __command_1_handler),
           INA_ISCP_SENDRECV_CMD(2, 1, __command_2_handler));

     ina_set_cleanup_handler(__cleanup_handler);

     if (!INA_SUCCEED(ina_iscp_create_tcp(&__iscp, "127.0.0.1", 9999))) {
         *retval = ina_err_peek();
         return;
     }

    if (!INA_SUCCEED(ina_iscp_register_ex(__iscp, cmds))) {
        *retval = ina_err_peek();
        return;
    }

    __running = 1;

    while (__running) {
        ina_iscp_recv(__iscp, 1, 0);
        ina_time_sleep(10);
    }
    *retval = INA_SUCCESS;
}