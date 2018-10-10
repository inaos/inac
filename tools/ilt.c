/*
 * Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved
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
    ina_rc_t rc;
    ina_ljit_ctx_t *ctx = NULL;
    ina_str_t bootstrp = NULL;

    INA_OPTS(opt,
        INA_OPT_STRING("b", "bootstrap", "<bootstrap>", "Full path to the bootstrap file"));

    if (!INA_SUCCEED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }

    if (!INA_SUCCEED(ina_ljit_ctx_new(&ctx))) {
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

    ina_ljit_ctx_free(&ctx);

    return rc;
}
