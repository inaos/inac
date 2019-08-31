/*
 * Copyright INAOS GmbH, Thalwil, 2012-2019. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <stdio.h>
#include <libinac/lib.h>

static const char* cfg = "global { buffer_size = 4096 } rule \"*.INFO\" { target = \">stdout\" } rule \"app.*\" { truncate=\"true\", target = \"app.log\"}";

int main(int argc,  char** argv)
{
    ina_log_ctx_t *ctx = NULL;

    if (INA_FAILED(ina_app_init(argc, argv, NULL))) {
        return EXIT_FAILURE;
    }
    /* Override default configuration */
    if (INA_FAILED(ina_log_init(cfg))) {
        return EXIT_FAILURE;
    }
    /* Create an additional log context for category app */
    if (INA_FAILED(ina_log_ctx_new("app", &ctx))) {
        ina_log_destroy();
        return EXIT_FAILURE;
    }

    INA_LOG_DEBUG("this is a simple DEBUG message is not printed");
    INA_LOG_INFO("this is a simple INFO message to the stdout");
    INA_LOG_WARNING("this is a simple WARNING message not printed");
    INA_LOG_ERROR("this is a simple ERROR message is not printed ");
    INA_LOG_ERROR("this is a ERROR message with var args (p1=%d) is not printed", 100);

    INA_LOG_CTX_DEBUG(ctx, "this message goes to app.log");
    INA_LOG_CTX_INFO(ctx, "this message goes also to app.log and to the stdout");

    ina_log_ctx_free(&ctx);
    ina_log_destroy();

    return EXIT_SUCCESS;
}
