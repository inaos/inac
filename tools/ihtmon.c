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

typedef struct ina_ht_info_s {
    int active;
    ina_hashtable_hash_type_t hash_type;
    ina_hashtable_key_type_t  key_type;
    int buckets;
    int items;
    uint64_t del_ops;
    uint64_t del_low_ns;
    uint64_t del_hig_ns;
    uint64_t del_tot_ns;
    uint64_t del_lst_ns;
    uint64_t del_ts1_ns;
    uint64_t set_low_ns;
    uint64_t set_hig_ns;
    uint64_t set_tot_ns;
    uint64_t set_lst_ns;
    uint64_t set_ts1_ns;
    uint64_t set_ops;
    uint64_t get_low_ns;
    uint64_t get_hig_ns;
    uint64_t get_tot_ns;
    uint64_t get_lst_ns;
    uint64_t get_ts1_ns;
    uint64_t get_ops;
    uint64_t get_misses;
    uint64_t del_misses;
    uint64_t collisions;
    int      mc;
    int      ex;
} ina_ht_info_t;

static ina_ht_info_t info[INA_HASHTABLE_MAX_STAT_TABLES];

static ina_file_ctx_t *file_ctx = NULL;
static ina_ullc_ctx_t *ullc_ctx = NULL;

static void ina_cleanup_handler(int error, int *exitcode)
{
    ina_cio_clear();
    ina_cio_reset();
    if (ullc_ctx != NULL) {
        ina_ullc_consumer_destroy(&ullc_ctx);
    }
    if (file_ctx != NULL) {
        ina_file_destroy(&file_ctx);
    }
}

static ina_str_t ihtm_format_number(int number)
{
    if (number < 10000) {
        return ina_str_sprintf("%d", number);
    } else if (number < 10000000) {
        number = number / 1000;
        return ina_str_sprintf("%dK", number);
    } else {
        number = number / 1000000;
        return ina_str_sprintf("%dM", number);
    }
}
static void ihtm_update(int ht)
{
    ina_str_t state;
    ina_str_t nb;
    ina_str_t ni;
    ina_str_t nc;
    ina_str_t ne;
    int ideal = 0;
    int misses = 0;

    if (info[ht].active) {
        state = ina_str_new_fromcstr("*");
    } else {
        state = ina_str_new_fromcstr(" ");
    }
    if ((info[ht].get_ops+info[ht].del_ops > 0)) {
        misses = (int) ((info[ht].get_misses + info[ht].del_misses) / (info[ht].get_ops + info[ht].del_ops));
    }
    nb = ihtm_format_number(info[ht].buckets);
    ni = ihtm_format_number(info[ht].items);
    nc = ihtm_format_number((int)info[ht].collisions);
    ne = ihtm_format_number(info[ht].ex);

    ina_cio_printf(ht+2, 0, INA_CIO_COLOR_BLUE,INA_CIO_COLOR_WHITE ,
                   "[%02d] %s    %#2d%%  %5s    %5s   %5s   %5s   %#2d%%  %3u %3u  %3u  %3u  %3u  %3u",
                   ht, state, ideal, nb, ni, nc, ne,
                   misses,
                   (uint32_t)info[ht].set_low_ns,
                   (uint32_t)info[ht].set_hig_ns,
                   (uint32_t)info[ht].get_low_ns,
                   (uint32_t)info[ht].get_hig_ns,
                   (uint32_t)info[ht].del_low_ns,
                   (uint32_t)info[ht].del_hig_ns);

    ina_str_free(nb);
    ina_str_free(ni);
    ina_str_free(nc);
    ina_str_free(ne);
}

static void ihtm_init(void)
{
    int i;
    ina_mem_set(&info, 0, sizeof(ina_ht_info_t)*INA_HASHTABLE_MAX_STAT_TABLES);
    ina_cio_clear();

    ina_cio_printf(0, 0, INA_CIO_COLOR_WHITE, INA_CIO_COLOR_BLUE, "%s",
            "IHTMON                                                                          ");
    ina_cio_printf(1, 0, INA_CIO_COLOR_WHITE, INA_CIO_COLOR_BLUE, "%s",
            "     A  ideal   #bkt   #items     #cln     #ex  miss   sl   sh   gl   gh   dl dh");

    for (i = 0; i < 16; ++i) {
        ihtm_update(i);
    }
    ina_cio_printf(18, 0, INA_CIO_COLOR_WHITE, INA_CIO_COLOR_BLUE, "%s",
            "  CTRL-C = end  t= time on/off                                                  ");
    ina_cio_printf(19, 0, INA_CIO_COLOR_WHITE, INA_CIO_COLOR_BLUE, "%s",
            "> connecting...                                                                 ");
    fflush(stdout);
}


int main(int argc,  char** argv)
{

    int core;
    ina_hashtable_event_t *event;

    INA_OPTS(opt,
             INA_OPT_INT("c", "core", 0, "core to pin")
    );

    if (!INA_SUCCEED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }
    ina_set_cleanup_handler(ina_cleanup_handler);

    ina_opt_get_int("c", &core);
    if (INA_FAILED(ina_cpu_pin_to_core(core))) {
        printf("failed to pint on core %d", core);
        return EXIT_FAILURE;
    }

    ihtm_init();

    while (INA_FAILED(INA_ULLC_CONSUMER_CREATE(ina_hashtable_event_t, 1, 4096, INA_HASHTABLE_MAX_STAT_TABLES, INA_HASHTABLE_MAX_STAT_TABLES, "/ina_htmon", &ullc_ctx))){
        ina_time_sleep(10);
    }

    ina_cio_printf(19, 0, INA_CIO_COLOR_WHITE, INA_CIO_COLOR_BLUE, "%s",
                   ">connected!                                                              ");

    while (1) {
        event = INA_ULLC_GET(ina_hashtable_event_t, ullc_ctx);
        if (event != NULL) {
            int ht = (int)event->hashtable_id - 1;
            switch (event->event_id) {
                case INA_HASHTABLE_EVENT_NEW: {
                    info[ht].active = 1;
                    info[ht].hash_type = event->data1;
                    info[ht].buckets = event->data2;
                    break;
                }
                case INA_HASHTABLE_EVENT_SET_BEGIN: {
                    info[ht].set_ts1_ns = event->ts;
                    break;
                }
                case INA_HASHTABLE_EVENT_SET_END: {
                    info[ht].set_lst_ns = event->ts - info[ht].set_ts1_ns;
                    if (info[ht].set_hig_ns < info[ht].set_lst_ns) {
                        info[ht].set_hig_ns = info[ht].set_lst_ns;
                    }
                    if (info[ht].set_low_ns > info[ht].set_lst_ns || info[ht].set_low_ns == 0) {
                        info[ht].set_low_ns = info[ht].set_lst_ns;
                    }
                    info[ht].items = (int)event->data2;
                    ++info[ht].set_ops;
                    info[ht].set_tot_ns += info[ht].set_lst_ns;
                    break;
                }
                case INA_HASHTABLE_EVENT_GET_BEGIN: {
                    info[ht].get_ts1_ns = event->ts;
                    break;
                }
                case INA_HASHTABLE_EVENT_GET_END: {
                    info[ht].get_lst_ns = event->ts - info[ht].get_ts1_ns;
                    if (info[ht].get_hig_ns < info[ht].get_lst_ns) {
                        info[ht].get_hig_ns = info[ht].get_lst_ns;
                    }
                    if (info[ht].get_low_ns > info[ht].get_lst_ns || info[ht].get_low_ns == 0) {
                        info[ht].get_low_ns = info[ht].get_lst_ns;
                    }
                    info[ht].get_misses += event->data2;
                    ++info[ht].get_ops;
                    info[ht].get_tot_ns += info[ht].get_lst_ns;
                    break;
                }
                case INA_HASHTABLE_EVENT_REMOVE_BEGIN: {
                    info[ht].del_ts1_ns = event->ts;
                    break;
                }
                case INA_HASHTABLE_EVENT_REMOVE_END: {
                    info[ht].del_lst_ns = event->ts - info[ht].del_ts1_ns;
                    if (info[ht].del_hig_ns < info[ht].del_lst_ns) {
                        info[ht].del_hig_ns = info[ht].del_lst_ns;
                    }
                    if (info[ht].del_low_ns > info[ht].del_lst_ns || info[ht].del_low_ns == 0) {
                        info[ht].del_low_ns = info[ht].del_lst_ns;
                    }
                    info[ht].get_misses += event->data1;
                    info[ht].items = (int)event->data2;
                    ++info[ht].del_ops;
                    info[ht].del_tot_ns += info[ht].del_lst_ns;
                    break;
                }
                case INA_HASHTABLE_EVENT_EXPANSION: {
                    ++info[ht].ex;
                    break;
                }
                case INA_HASHTABLE_EVENT_FREE: {
                    info[ht].active = 0;
                    break;
                }
            }
            ihtm_update(ht);
            /*printf("ts: %llu table: %u  event: %d  data: %llu:%llu\n",
                    event->ts,
                    event->hashtable_id,
                    event->event_id,
                    event->data1,
                    event->data2);*/
            fflush(stdout);
        }
    }
    return EXIT_SUCCESS;
}