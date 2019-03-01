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
    ina_str_t output = NULL;
    ina_str_t config_file = NULL;
    int single =  INA_NO;
    int quiet = INA_NO;
    double retval = 0.0;

    INA_OPTS(opt,
        INA_OPT_STRING("o", "output", NULL, "Output directory or output file"),
        INA_OPT_FLAG("s", "single-files", "Generate single files"),
        INA_OPT_FLAG("q", "quiet", "Quiet mode"),
        INA_OPT_STRING("c", "config-file", ".idoc", "Configuration file"));

    if (!INA_SUCCEED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }


    INA_FAIL_IF_ERROR(ina_ljit_ctx_new(&ctx));

    ina_opt_get_string("o", &output);
    ina_opt_get_string("c", &config_file);
    if (INA_SUCCEED(ina_opt_isset("s"))) {
        single = INA_YES;
    }
    if (INA_SUCCEED(ina_opt_isset("q"))) {
        quiet= INA_YES;
    }

    INA_FAIL_IF_ERROR(ina_ljit_dostring(ctx, "idoc = require(\"lidoc\")\n"));
    INA_FAIL_IF_ERROR(ina_ljit_call(ctx, "idoc.run", "ssdd<d",
            output, config_file, (double)single, (double)quiet, &retval));
    ina_ljit_ctx_free(&ctx);
    return (int)retval;

fail:
    if (ctx != NULL) {
        printf("%s", ina_ljit_last_error(ctx));
        ina_ljit_ctx_free(&ctx);
    }
    return EXIT_FAILURE;
}
