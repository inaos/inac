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

#define MSGBUFSIZE 1024*1024

static ina_fd_t __fd = 0;
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
    int port = 0;
    unsigned char msgbuf[MSGBUFSIZE];

    INA_OPTS(opt,
        INA_OPT_STRING("a", "address", "0.0.0.0", "Interface to join on"),
        INA_OPT_STRING("g", "group", NULL, "Multicast group to join"),
        INA_OPT_INT("p", "port", NULL, "Multicast port to join"));

    if (!INA_SUCCEED(ina_app_init(argc, argv, opt))) {
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
