/*
 * Copyright INAOS GmbH, Thalwil, 2019. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <stdio.h>
#include <libinac/lib.h>

INA_LJIT_PACKAGE(idoc);
INA_LJIT_IMPORT(idoc,lidoc);

int main(int argc,  char** argv) 
{
    ina_ljit_ctx_t *ctx = NULL;
    ina_str_t output_dir = NULL;
    ina_str_t filter = NULL;

    INA_OPTS(opt,
        INA_OPT_STRING("o", "output-dir", NULL, "Output directory"),
        INA_OPT_STRING("f", "filter", "*.h", "File filter, default is *.h"));

    if (!INA_SUCCEED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }


    if (!INA_SUCCEED(ina_ljit_ctx_new(&ctx))) {
        return EXIT_FAILURE;
    }

    ina_opt_get_string("o", &output_dir);
    ina_opt_get_string("f", &filter);


    if (INA_FAILED(ina_ljit_dostring(ctx, "idoc = require(\"lidoc\")\n"))) {
        printf("%s", ina_ljit_last_error(ctx));
        return EXIT_FAILURE;
    }
    if (INA_FAILED(ina_ljit_call(ctx, "idoc.run", "ssd", output_dir, filter, (double)0))) {
        printf("%s", ina_ljit_last_error(ctx));
        return EXIT_FAILURE;
    }
    ina_ljit_ctx_free(&ctx);

    return EXIT_SUCCESS;
}
