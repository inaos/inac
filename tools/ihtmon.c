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
    ina_hashtable_type_t type;
    ina_hash_type_t hash_type;
    ina_hashtable_key_type_t  key_type;
    size_t key_len;
    uint32_t cf;
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
static ina_file_t     *file;
static ina_ullc_ctx_t *ullc_ctx = NULL;
static ina_ullc_ctx_t *ullc_ctx2 = NULL;
static ina_hashtable_event_t  *records = NULL;
static ina_hashtable_event_t  *current_record = NULL;
static int max_records = 1024*10;

static int panel = -1;

static void ihtm_write_stats(int);

static void ina_cleanup_handler(int error, int *exitcode)
{
    INA_UNUSED(error);
    INA_UNUSED(exitcode);

    ina_cio_clear();
    ina_cio_reset();
    ihtm_write_stats(1);
    if (records != NULL) {
        ina_mem_free(records);
    }
    if (file != NULL) {
        ina_file_free(&file);
    }
    if (file_ctx != NULL) {
        ina_file_destroy(&file_ctx);
    }
    if (ullc_ctx2 != NULL) {
        ina_ullc_producer_destroy(&ullc_ctx2);
    }
    if (ullc_ctx != NULL) {
        ina_ullc_consumer_destroy(&ullc_ctx);
    }
    if (file_ctx != NULL) {
        ina_file_destroy(&file_ctx);
    }
}

static ina_str_t ihtm_format_number(int number)
{
    if (number == 0) {
        return ina_str_new_fromcstr("n/a");
    } else if (number < 1000) {
        return ina_str_sprintf("%d", number);
    } else if (number < 10000000) {
        number = number / 1000;
        return ina_str_sprintf("%dK", number);
    } else {
        number = number / 1000000;
        return ina_str_sprintf("%dM", number);
    }
}

static ina_str_t ihtm_format_time(int number)
{
    if (number == 0) {
        return ina_str_new_fromcstr("n/a");
    } else if (number < 1000) {
        return ina_str_sprintf("%n", number);
    } else if (number < 10000000) {
        number = number / 1000;
        return ina_str_sprintf("%du", number);
    } else {
        number = number / 1000000;
        return ina_str_sprintf("%dm", number);
    }
}

static void ihtm_write_stats(int force)
{
    static char buf[1024];
    int64_t wrote;

    ina_hashtable_event_t *next;
    if (records == NULL) {
        return;
    }
    if (current_record-records < max_records-1 &&  !force) {
        return;
    }
    next = records;
    while (next != current_record+1) {
        sprintf(buf, "%lu,%d,%d,%lu,%lu\n",
                next->ts,
                next->hashtable_id,
                next->event_id,
                next->data1,
                next->data2);
        ina_file_write(file, (unsigned char*)buf, strlen(buf), &wrote);
        next++;
    }
}
static void ihtm_update_stats(int ht)
{
    ina_str_t state;
    int ideal = 0;

    if (info[ht].active) {
        state = ina_str_new_fromcstr("*");
    } else {
        state = ina_str_new_fromcstr(" ");
    }

    if (panel == 0) {
        ina_str_t nb;
        ina_str_t ni;
        ina_str_t nc;
        ina_str_t ne;
        ina_str_t st;
        ina_str_t gt;
        ina_str_t dt;

        int misses = 0;


        if ((info[ht].get_ops + info[ht].del_ops > 0)) {
            misses = (int) ((info[ht].get_misses + info[ht].del_misses) / (info[ht].get_ops + info[ht].del_ops));
        }
        nb = ihtm_format_number(info[ht].buckets);
        ni = ihtm_format_number(info[ht].items);
        nc = ihtm_format_number((int) info[ht].collisions);
        ne = ihtm_format_number(info[ht].ex);
        st = ihtm_format_number((int) info[ht].set_ops),
        gt = ihtm_format_number((int) info[ht].get_ops),
        dt = ihtm_format_number((int) info[ht].del_ops),

                ina_cio_printf(ht + 2, 0, INA_CIO_COLOR_BLUE, INA_CIO_COLOR_WHITE,
                               "[%02d] %s      %#2d%%  %5s    %5s    %5s   %5s   %#2d%%        %3s    %3s     %3s",
                               ht, state, ideal, nb, ni, nc, ne,
                               misses,
                               st,
                               gt,
                               dt);

        ina_str_free(nb);
        ina_str_free(ni);
        ina_str_free(nc);
        ina_str_free(ne);
        ina_str_free(st);
        ina_str_free(gt);
        ina_str_free(dt);
    } else if (panel == 1) {
        ina_str_t nb;
        ina_str_t ni;
        ina_str_t kt;
        ina_str_t fl;

        nb = ihtm_format_number(info[ht].buckets);
        ni = ihtm_format_number(info[ht].items);
        switch (info[ht].key_type) {
            case INA_HASHTABLE_STR_KEY: {
                kt = ina_str_new_fromcstr("str");
                break;
            }
            case INA_HASHTABLE_PTR_KEY: {
                kt = ina_str_new_fromcstr("ptr");
                break;
            }
            case INA_HASHTABLE_INT32_KEY: {
                kt = ina_str_new_fromcstr("i32");
                break;
            }
            case INA_HASHTABLE_UINT32_KEY: {
                kt = ina_str_new_fromcstr("u32");
                break;
            }
            case INA_HASHTABLE_INT64_KEY: {
                kt = ina_str_new_fromcstr("i64");
                break;
            }
            case INA_HASHTABLE_UINT64_KEY: {
                kt = ina_str_new_fromcstr("u64");
                break;
            }
            default:
                kt = ina_str_new_fromcstr("n/a");
        }
        fl = ina_str_new_fromcstr("--");
        if (info[ht].cf&INA_HASHTABLE_CF_PREALLOCATED) {
            fl = ina_str_catcstr(fl, "P");
        } else {
            fl = ina_str_catcstr(fl, "-");
        }
        if (info[ht].cf&INA_HASHTABLE_CF_STAT) {
            fl = ina_str_catcstr(fl, "S");
        } else {
            fl = ina_str_catcstr(fl, "-");
        }
        ina_cio_printf(ht + 2, 0, INA_CIO_COLOR_BLUE, INA_CIO_COLOR_WHITE,
                       "[%02d] %s      %#2d%%  %5s    %5s      %-10s    %ss         %d      %s     ",
                       ht, state, ideal, nb, ni, ina_hash_name(info[ht].hash_type), kt,
                               (int)info[ht].key_len, fl);

        ina_str_free(nb);
        ina_str_free(ni);
        ina_str_free(kt);
        ina_str_free(fl);
    }
    ina_str_free(state);
}

static void ihtm_update_status(const char* status_msg)
{
    ina_cio_printf(18, 0, INA_CIO_COLOR_WHITE, INA_CIO_COLOR_BLACK, "> %-78s", status_msg);
    fflush(stdout);
}

static void ihtm_update_menu(const char* commands)
{
    ina_cio_printf(19, 0, INA_CIO_COLOR_WHITE, INA_CIO_COLOR_BLACK, "  %-78s", commands);
    fflush(stdout);
}
static void ihtm_update_event(ina_hashtable_event_t *event)
{
    ina_cio_printf(18, 0, INA_CIO_COLOR_WHITE, INA_CIO_COLOR_BLACK,
            "> ts: %llu table: %u  event: %d  data: %llu:%llu                             ",
                   event->ts,
                   event->hashtable_id,
                   event->event_id,
                   event->data1,
                   event->data2);
    fflush(stdout);
}


static void ihtm_switch_panel(int p)
{
    int i;
    if (p == panel) {
        return;
    }
    panel = p;
    ina_cio_move_to_row_and_col(0,0);
    switch (panel) {
        case 0: {
            ina_cio_printf(0, 0, INA_CIO_COLOR_WHITE, INA_CIO_COLOR_BLUE, "%-80s", "IHTMON - Main");
            ina_cio_printf(1, 0, INA_CIO_COLOR_WHITE, INA_CIO_COLOR_BLUE, "%s",
                           "     A |  ideal   #bkt   #items     #cln     #ex  miss  |    #set   #get    #del");
            break;
        }
        case 1: {
            ina_cio_printf(0, 0, INA_CIO_COLOR_WHITE, INA_CIO_COLOR_BLUE, "%-80s", "IHTMON - Meta data");
            ina_cio_printf(1, 0, INA_CIO_COLOR_WHITE, INA_CIO_COLOR_BLUE, "%s",
                           "     A |  ideal   #bkt   #items   |  hash        key-type   key-len    flags    ");
            break;
        }
        default:
            break;
    }
    for (i = 0; i < 16; ++i) {
        ihtm_update_stats(i);
    }
    fflush(stdout);
}

static void ihtm_init(void)
{

    ina_mem_set(&info, 0, sizeof(ina_ht_info_t)*INA_HASHTABLE_MAX_STAT_TABLES);
    ina_cio_clear();
    ihtm_switch_panel(0);
    ihtm_update_status("connecting...");
}


int main(int argc,  char** argv)
{

    int core;
    int ilde_count = 0;
    ina_str_t filepath;
    ina_hashtable_event_t *event;

    INA_OPTS(opt,
             INA_OPT_INT("c", "core", 0, "core to pin"),
             INA_OPT_STRING("o", "output", NULL, "file path for stats event data")
    );

    if (!INA_SUCCEED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }
    ina_set_cleanup_handler(ina_cleanup_handler);

    ina_opt_get_int("c", &core);
    if (INA_FAILED(ina_cpu_pin_to_core(core))) {
        printf("failed to pin on core %d", core);
        return EXIT_FAILURE;
    }
    ina_opt_get_string("o", &filepath);
    if (strcmp(filepath, "-") != 0) {

        size_t size;
        if (INA_FAILED(ina_file_init(&file_ctx, 0))) {
            return EXIT_FAILURE;
        }
        if (INA_FAILED(ina_file_new(file_ctx, filepath,
                                    INA_FILE_ACCESS_MODE_READWRITE,
                                    INA_FILE_CREATE_MODE_CREATE,
                                    INA_FILE_SHARE_MODE_EXCLUSIVE,
                                    0,
                                    &file))) {
            return EXIT_FAILURE;
        }
        size = (max_records+1)* sizeof(ina_hashtable_event_t);
        records = ina_mem_alloc(size);
        ina_mem_set(records, 0, size);
        current_record = records;
    }

    ihtm_init();

    while (INA_FAILED(INA_ULLC_PRODUCER_CREATE(ina_hashtable_event_t,
                                               1, 4096,
                                               INA_HASHTABLE_MAX_STAT_TABLES,
                                               INA_HASHTABLE_MAX_STAT_TABLES,
                                               "/ina_htmon", INA_ULLC_WS_SIGNAL_WAIT,
                                               &ullc_ctx2))){
        ina_time_sleep(10);
    }
    while (INA_FAILED(INA_ULLC_CONSUMER_CREATE(ina_hashtable_event_t,
            1, 4096,
            INA_HASHTABLE_MAX_STAT_TABLES,
            INA_HASHTABLE_MAX_STAT_TABLES,
            "/ina_htmon", &ullc_ctx))){
        ina_time_sleep(10);
    }

    ihtm_update_status("connected! waiting for data...");
    ihtm_update_menu("   [q] quit [s] panel switch ");

    while (1) {
        ++ilde_count;
        event = INA_ULLC_GET(ina_hashtable_event_t, ullc_ctx);
        if (event != NULL) {
            int ht = (int)event->hashtable_id - 1;
            if (records != NULL) {
                ina_mem_cpy(current_record, event, sizeof(ina_hashtable_event_t));
                current_record++;
                ihtm_write_stats(0);
            }
            switch (event->event_id) {
                case INA_HASHTABLE_EVENT_IDLE:
                    continue;
                case INA_HASHTABLE_EVENT_META: {
                    switch (event->data1) {
                        case 1: {
                            info[ht].hash_type = (ina_hash_type_t) event->data2;
                            break;
                        }
                        case 2: {
                            info[ht].key_len = (size_t) event->data2;
                            break;
                        }
                        case 3: {
                            info[ht].type = (ina_hashtable_type_t) event->data2;
                            break;
                        }
                        case 4: {
                            info[ht].cf = (uint32_t )event->data2;
                            break;
                        }
                        default:
                            continue;
                    }
                    break;

                }
                case INA_HASHTABLE_EVENT_NEW: {
                    ina_mem_set(&info[ht], 0, sizeof(ina_ht_info_t));
                    info[ht].active = 1;
                    info[ht].hash_type = (ina_hash_type_t)event->data1;
                    info[ht].buckets =  (int)event->data2;
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
                default:
                    break;
            }
            ihtm_update_stats(ht);
            ihtm_update_event(event);
        } else {
            char c = 0;
            ina_time_sleep(5);
            if (!(ilde_count % 1000)) {
                ilde_count = 0;
                ihtm_update_status("waiting for data...");
            }
            ina_cio_read_char_non_block(&c);
            if (c == 'q') {
                break;
            } else if (c == 's') {
                ihtm_switch_panel((panel==0)?1:0);
            }
        }
    }
    return EXIT_SUCCESS;
}