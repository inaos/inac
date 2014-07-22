/*
 * Copyright (c) 2014, INAOS GmbH
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

static ina_rc_t __ina_service_fn(const ina_service_ctx_t *ctx, ina_service_status_t status, void *user_data)
{
    int *value;
    ina_service_descriptor_t *ds;
    
    if (INA_SUCCEED(ina_service_get_descriptor(ctx, &ds))) {
        return INA_ERR_PUSH_LAST;
    }

    switch (status) {
        case INA_SERVICE_STATUS_INIT:
            value = (int*)ina_mem_alloc(sizeof(int));
            ina_service_set_data(ctx, (void*)value);
        case INA_SERVICE_STATUS_START:
            *((int*)user_data) = 1;
            break;
        case INA_SERVICE_STATUS_RUN:
            *((int*)user_data) = 2;
            break;
        case INA_SERVICE_STATUS_SHUTDOWN:
            *((int*)user_data) = 3;
            break;
        case INA_SERVICE_STATUS_STOP:
            ina_mem_free(user_data);
            ina_service_set_data(ctx, NULL);
            break;
        case INA_SERVICE_STATUS_INSTALL:
            break;
        case INA_SERVICE_STATUS_UNINSTALL:
            break;
        case INA_SERVICE_STATUS_REPORT:
            break;
        case INA_SERVICE_STATUS_ERROR:
            break;
    }
    return INA_SUCCESS;
}

INA_SERVICE_DESCRIPTOR("test",
    "Test Daemon",
    "Simple test deamon", 
    "root", 
    "password", 
    "-h service simple_deamon 127.0.0.1 9998", 
    "/opt/test",
    "1234 99 10",
    __ina_service_fn,
    INA_SERVICE_STARTUP_TYPE_AUTO, 
    INA_YES);


