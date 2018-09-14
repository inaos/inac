/*
 * Copyright (c) 2018, INAOS GmbH
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

static ina_file_ctx_t *file_ctx = NULL;
static ina_ullc_ctx_t *ullc_ctx = NULL;

static void ina_cleanup_handler(int error, int *exitcode)
{
    if (ullc_ctx != NULL) {
        ina_ullc_consumer_destroy(&ullc_ctx);
    }
    if (file_ctx != NULL) {
        ina_file_destroy(&file_ctx);
    }
}

int main(int argc,  char** argv)
{

    ina_hashtable_event_t *event;

    INA_OPTS(opt,
             INA_OPT_STRING("s", "source", "t", "Source file")
    );

    if (!INA_SUCCEED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }
    ina_set_cleanup_handler(ina_cleanup_handler);

    ina_err_set_log_file(">2");

    printf("try to connect...\n");
    while (!INA_SUCCEED(INA_ULLC_CONSUMER_CREATE(ina_hashtable_event_t, 1, 4096, 32, 32, "/ina_htmon", &ullc_ctx))){
        ina_time_sleep(5);
    }
    printf("connected!\n");
    printf("press CTRL-C to stop");
    fflush(stdout);

    while (1) {
        event = INA_ULLC_GET(ina_hashtable_event_t, ullc_ctx);
        if (event != NULL) {
            printf("table: %u  event: %d  data: %lu\n", event->hashtable_id, event->event_id, event->data);
            fflush(stdout);
        }
    }
    return EXIT_SUCCESS;
}