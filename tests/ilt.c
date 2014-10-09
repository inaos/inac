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
 * CAUSED AND ON ANYs THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <libinac/lib.h>

int main(int argc,  char** argv) 
{
    ina_rc_t rc;
    ina_ljit_ctx_t *ctx = NULL;
    ina_str_t bootstrp;

    INA_OPTS(opt,
        INA_OPT_STRING("b", "bootstrap", "<bootstrap>", "Full path to the bootstrap file"));

    if (!INA_SUCCEED(ina_app_init(argc, argv, 0, opt))) {
        return EXIT_FAILURE;
    }

    if (!INA_SUCCEED(ina_ljit_init(&ctx))) {
        return EXIT_FAILURE;
    }

    /* execute the bootstrap if present */
    ina_opt_get_string("bootstrap", &bootstrp);
    if (strcmp("<bootstrap>", ina_str_cstr(bootstrp))!=0) {
        if (luaL_dofile(ctx->lstate, ina_str_cstr(bootstrp)) != 0) {
            printf("%s", luaL_checkstring(ctx->lstate, 1));
            ina_str_free(bootstrp);
            return EXIT_FAILURE;
        }
        else {
            ina_str_free(bootstrp);
        }
    }

    rc = ina_test_run(argc, argv, ctx);

    ina_ljit_destroy(&ctx);

    return rc;
}
