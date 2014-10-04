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

#define MSGBUFSIZE 1024*1024

static int __fd = 0;
static ina_str_t __address = NULL, __group = NULL;

static void 
ina_cleanup_handler(int error, int *exitcode)
{
    if (__fd > 0) {
        ina_net_leave_group(__fd, ina_str_cstr(__address), ina_str_cstr(__group));
        ina_net_close(__fd);
    }
}

int main(int argc,  char** argv) 
{
    int port;
    unsigned char msgbuf[MSGBUFSIZE];

    INA_OPTS(opt,
        INA_OPT_STRING("a", "address", "0.0.0.0", "Interface to join on"),
        INA_OPT_STRING("g", "group", NULL, "Multicast group to join"),
        INA_OPT_INT("p", "port", NULL, "Multicast port to join"));

    if (!INA_SUCCEED(ina_app_init(argc, argv, 0, opt))) {
        return EXIT_FAILURE;
    }
    ina_set_cleanup_handler(ina_cleanup_handler);

    ina_opt_get_string("a", &__address);
    ina_opt_get_string("g", &__group);
    ina_opt_get_int("p", &port);

    if (!INA_SUCCEED(ina_net_udp_bind(&__fd, ina_str_cstr(__address), port))) {
        return EXIT_FAILURE;
    }
    if (!INA_SUCCEED(ina_net_nonblock(__fd))) {
        return EXIT_FAILURE;
    }

    if (!INA_SUCCEED(ina_net_join_group(__fd, ina_str_cstr(__address), ina_str_cstr(__group)))) {
        return EXIT_FAILURE;
    }

    printf("Start listen on %s:%d: ...\n", ina_str_cstr(__group), port);
    fflush(stdout);

    while (1) {
        int i;
        int nbytes;

        ina_mem_set(msgbuf, 0, MSGBUFSIZE);
        if (!INA_SUCCEED(ina_net_read(__fd, msgbuf, MSGBUFSIZE, &nbytes))) {
            EXIT_FAILURE;
        }
      
        if (nbytes > 0) {
            printf("Incoming message size = %d\n", nbytes);

            for (i=0; i < nbytes; i++) {
                printf("%02x ", msgbuf[i]);
            }

            printf("\n");
            fflush(stdout);
        }

        ina_time_sleep(1);
    }

    return EXIT_SUCCESS;
}
