/*
 * Copyright (c) 2015, INAOS GmbH
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
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <libinac/lib.h>

static ina_file_ctx_t  *ctx = NULL;
static ina_file_t      *file = NULL;
static ina_stopwatch_t *stopwatch = NULL;
static unsigned char *read_buf = NULL;


static void bf_cleanup_handler(int sig, int *error)
{
    if (read_buf != NULL) {
        ina_mem_free(read_buf);
    }
    if (file != NULL) {
        ina_file_free(ctx, &file);
    }
    if (file != NULL) {
        ina_file_destroy(&ctx);
    }
    if (stopwatch != NULL) {
        ina_time_stopwatch_destroy(&stopwatch);
    }
}

static void bf_barre_io(void)
{

}


int main(int argc, char **argv)
{
    ina_str_t filepath;
    int buffer_size;
    uint64_t size;
    uint64_t nb_read;
    uint64_t tot_nb_read = 0;
    double mb_sec;

    INA_OPTS(opt,
        INA_OPT_INT("f", "file", NULL, "Input file"),
        INA_OPT_INT("b", "buffer-size", 4, "Buffer size in KB")
    );

    if (!INA_SUCCEED(ina_app_init(argc, argv, 0, opt))) {
        return EXIT_FAILURE;
    }
    ina_set_cleanup_handler(bf_cleanup_handler);

    if (!INA_SUCCEED(ina_opt_get_string("f", &filepath))) {
        return EXIT_FAILURE;
    }
    if (!INA_SUCCEED(ina_opt_get_int("b", &buffer_size))) {
        return EXIT_FAILURE;
    }

    if (!INA_SUCCEED(INA_TIME_STOPWATCH_CREATE(&stopwatch, 1, -1))) {
        return EXIT_FAILURE;
    }

    if (!INA_SUCCEED(ina_file_init(&ctx))) {
        return EXIT_FAILURE;
    }

    if (!INA_SUCCEED(ina_file_new(ctx, ina_str_cstr(filepath), 
            INA_FILE_ACCESS_MODE_READWRITE,
            INA_FILE_CREATE_MODE_OPEN,
            INA_FILE_SHARE_MODE_EXCLUSIVE,
            0,
            &file))) {
        return EXIT_FAILURE;
    }

    size = buffer_size*1024;
    nb_read = 1;
    read_buf = ina_mem_alloc(size);

    INA_TIME_STOPWATCH_START(stopwatch);
    while (INA_SUCCEED(ina_file_read(file, read_buf, size, &nb_read)) && nb_read) {
        tot_nb_read += nb_read;
        //printf("%lu\n", nb_read);
    }
    INA_TIME_STOPWATCH_STOP(stopwatch);


    mb_sec = tot_nb_read/stopwatch->tv->sec_duration/1024/1024;

    printf("Average speed %f MB/s, Duration %f seconds, MB %lu \n", mb_sec, stopwatch->tv->sec_duration, tot_nb_read/1024/1024 );

    return EXIT_SUCCESS;
}

