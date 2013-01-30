/*
 * Copyright (c) 2012, INAOS GmbH
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
 * CAUSED AND ON ANYs THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <libinac/lib.h>
#include "suites.h"

#define INAC_ERROR_TEST_TRACE INA_ERR_PUSH(129,1,2,"Test Trace")

static int __cleanup_called = 0;
static int __ina_cleanup_handler(const int sig, const int error) 
{
    ++__cleanup_called;
    INA_TRACE("Cleanup called = %d", __cleanup_called);
    return EXIT_SUCCESS;
}

int main(int argc,  char** argv) 
{ 
    INA_TRACE_MSG("TEST START");
    
    ina_opt_t opt[] = {
        {"r", "run", INA_OPT_TYPE_STRING, "all", "fork a test"},
        {NULL, NULL, 0, NULL, NULL}
    };
    
    if (INA_SUCCEED(ina_appinit(argc, argv, 0, opt))) {
        ina_str_t run = NULL;
        if (INA_SUCCEED(ina_opt_get_string("run", &run))) {
            INA_TRACE("run=%s", ina_str_cstr(run));
        } 
        runtests();
        ina_set_cleanup_handler(__ina_cleanup_handler);

        /* this test program should alway exits with a
        failure */
        INAC_ERROR_TEST_TRACE;
    }

    INA_TRACE_MSG("TEST END");
    
    return EXIT_SUCCESS;
}
