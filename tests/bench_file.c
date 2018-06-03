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
#include <malloc.h>

static ina_file_ctx_t    *file_ctx = NULL;
static ina_mmap_ctx_t    *mmap_ctx = NULL;
static ina_file_t        *file = NULL;
static ina_file_cursor_t *cursor = NULL;
static ina_stopwatch_t   *stopwatch = NULL;
static unsigned char     *read_buf = NULL;
static uint64_t           tot_nb_read = 0;
static int                buffer_size = 0;
static int                buffer_size_o = 0;
static ina_str_t          size_unit = NULL;
static ina_str_t          benchmark = NULL;


static void bf_cleanup_handler(int sig, int *error)
{
    if (read_buf != NULL) {
        ina_mem_free(read_buf);
    }
    if (cursor != NULL) {
        ina_file_cursor_free(&cursor);
    }
    if (file != NULL) {
        ina_file_free(file_ctx, &file);
    }
    if (file_ctx != NULL) {
        ina_file_destroy(&file_ctx);
    }
    if (mmap_ctx != NULL) {
        ina_mmap_destroy(&mmap_ctx);
    }
    if (stopwatch != NULL) {
        double mb_sec;
        
        INA_TIME_STOPWATCH_STOP(stopwatch);

        mb_sec = tot_nb_read/stopwatch->tv->sec_duration/1024/1024;

        printf("%s: Average speed %f MB/s, Duration %f seconds, Total bytes read: %lu, buffer size: %d %s \n", 
            benchmark,
            mb_sec, 
            stopwatch->tv->sec_duration, 
            tot_nb_read,
            (int)buffer_size_o,
            size_unit);
        ina_time_stopwatch_destroy(&stopwatch);
    }

    if (benchmark) {
        ina_str_free(benchmark);
    }
    if (size_unit) {
        ina_str_free(size_unit);
    }
}

static void bf_read(int buffer_size, const char *filepath)
{
    int64_t nb_read = -1;

    if (INA_SUCCEED(ina_file_new(file_ctx, filepath, 
            INA_FILE_ACCESS_MODE_READ,
            INA_FILE_CREATE_MODE_OPEN,
            INA_FILE_SHARE_MODE_READ,
            0,
            &file))) {

        read_buf = ina_mem_alloc(buffer_size);

        while (INA_SUCCEED(ina_file_read(file, read_buf, buffer_size, &nb_read)) && nb_read) {
            tot_nb_read += nb_read;
        }
    }
}

static void bf_read_seq(int buffer_size, const char *filepath)
{
    int64_t nb_read = -1;

    if (INA_SUCCEED(ina_file_new(file_ctx, filepath, 
            INA_FILE_ACCESS_MODE_READ,
            INA_FILE_CREATE_MODE_OPEN,
            INA_FILE_SHARE_MODE_READ,
            INA_FILE_FLAG_SEQUENTIAL_ACCESS,
            &file))) {
        read_buf = ina_mem_alloc(buffer_size);

        while (INA_SUCCEED(ina_file_read(file, read_buf, buffer_size, &nb_read)) && nb_read) {
            tot_nb_read += nb_read;
        }
    }
}

static void bf_read_direct(int buffer_size, const char *filepath)
{
    int64_t nb_read = -1;

    if (INA_SUCCEED(ina_file_new(file_ctx, filepath, 
            INA_FILE_ACCESS_MODE_READ,
            INA_FILE_CREATE_MODE_OPEN,
            INA_FILE_SHARE_MODE_READ,
            INA_FILE_FLAG_POSIX_DIRECT|INA_FILE_FLAG_SEQUENTIAL_ACCESS,
            &file))) {

        read_buf = memalign(4096 * 2, buffer_size + 4096);
        read_buf += 4096;

        while (INA_SUCCEED(ina_file_read(file, read_buf, buffer_size, &nb_read)) && nb_read) {
            tot_nb_read += nb_read;
        }
    }
}

static void bf_read_cursor(int buffer_size, const char *filepath)
{
    size_t nb_read = -1;

    if (INA_SUCCEED(ina_file_new(file_ctx, filepath, 
            INA_FILE_ACCESS_MODE_READ,
            INA_FILE_CREATE_MODE_OPEN,
            INA_FILE_SHARE_MODE_READ,
            INA_FILE_FLAG_SEQUENTIAL_ACCESS,
            &file))) {
        
        if (INA_SUCCEED(ina_file_cursor_new(file,
                INA_FILE_CURSOR_TYPE_FILEIO,
                INA_FILE_CURSOR_MODE_READ_BINARY,
                (size_t)buffer_size,
                &cursor,
                mmap_ctx))) {

            const unsigned char *buf;

            while (INA_SUCCEED(ina_file_cursor_binary_read_chunk(cursor, 
                                    buffer_size, &nb_read, &buf)) && nb_read) {
                tot_nb_read += nb_read;
            }
        }
    }
}

static void bf_read_mmap_cursor(int buffer_size, const char *filepath)
{
    size_t nb_read = -1;

    if (INA_SUCCEED(ina_file_new(file_ctx, filepath, 
            INA_FILE_ACCESS_MODE_READ,
            INA_FILE_CREATE_MODE_OPEN,
            INA_FILE_SHARE_MODE_READ,
            INA_FILE_FLAG_SEQUENTIAL_ACCESS,
            &file))) {
        
        if (INA_SUCCEED(ina_file_cursor_new(file,
                INA_FILE_CURSOR_TYPE_MMAP,
                INA_FILE_CURSOR_MODE_READ_BINARY,
                (size_t)buffer_size,
                &cursor,
                mmap_ctx))) {
            const unsigned char *buf;

            while (INA_SUCCEED(ina_file_cursor_binary_read_chunk(cursor, 
                                    (size_t)buffer_size, &nb_read, &buf)) && nb_read) {
                tot_nb_read += nb_read;
            }
        } else {
            ina_err_trace();
        }
    }
}

int main(int argc, char **argv)
{
    ina_str_t filepath;

    INA_OPTS(opt,
        INA_OPT_INT("f", "file", NULL, "Input file"),
        INA_OPT_FLAG(NULL, "read", "Bare c read"),
        INA_OPT_FLAG(NULL, "read-seq", "Bare c read using INA_FILE_FLAG_SEQUENTIAL_ACCESS"),
        INA_OPT_FLAG(NULL, "read-direct", "Bare c read using INA_FILE_FLAG_POSIX_DIRECT"),
        INA_OPT_FLAG(NULL, "read-cursor", "Squential read using cursor"),
        INA_OPT_FLAG(NULL, "read-mmap-cursor", "Squential read using mmap cursor"),
        INA_OPT_INT("b", "buffer-size", 4, "Buffer size (default 4)"),
        INA_OPT_STRING("u", "buffer-size-unit", "kb", "Buffer size unit [KB/MB/GB]. Default KB")
    );

    if (!INA_SUCCEED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }
    ina_set_cleanup_handler(bf_cleanup_handler);

    if (!INA_SUCCEED(ina_opt_get_string("f", &filepath))) {
        return EXIT_FAILURE;
    }
    if (!INA_SUCCEED(ina_opt_get_int("b", &buffer_size_o))) {
        return EXIT_FAILURE;
    }
    if (!INA_SUCCEED(ina_opt_get_string("u", &size_unit))) {
        return EXIT_FAILURE;
    }

    if (!INA_SUCCEED(INA_TIME_STOPWATCH_CREATE(&stopwatch, 1, -1))) {
        return EXIT_FAILURE;
    }

    if (!INA_SUCCEED(ina_file_init(&file_ctx, 0))) {
        return EXIT_FAILURE;
    }

    if (!INA_SUCCEED(ina_mmap_init(&mmap_ctx))) {
        return EXIT_FAILURE;
    }

    buffer_size = buffer_size_o;
    if (INA_CSTR_CASECMP(size_unit, "kb") ==0 ) {
        buffer_size *= 1024;
    } else if (INA_CSTR_CASECMP(size_unit, "mb") == 0) {
        buffer_size *= 1024*1024;
    } else if (INA_CSTR_CASECMP(size_unit, "gb") == 0) {
        buffer_size *= 1024*1024*1024;
    } else {
        printf("Invalid size unit!\n");
        return EXIT_FAILURE;
    }

    INA_TIME_STOPWATCH_START(stopwatch);

    if (INA_SUCCEED(ina_opt_isset("read"))) {
        benchmark = ina_str_new_fromcstr("bf_read");
        bf_read(buffer_size, ina_str_cstr(filepath));
    }  else if (INA_SUCCEED(ina_opt_isset("read-seq"))) {
        benchmark = ina_str_new_fromcstr("read_seq");
        bf_read_seq(buffer_size, ina_str_cstr(filepath));
    }  else if (INA_SUCCEED(ina_opt_isset("read-direct"))) {
        benchmark = ina_str_new_fromcstr("read_direct");
        bf_read_direct(buffer_size, ina_str_cstr(filepath));
    } else if (INA_SUCCEED(ina_opt_isset("read-cursor"))) {
        benchmark = ina_str_new_fromcstr("bf_read_cursor");
        bf_read_cursor(buffer_size, ina_str_cstr(filepath));
    } else if (INA_SUCCEED(ina_opt_isset("read-mmap-cursor"))) {
        benchmark = ina_str_new_fromcstr("bf_read_mmap_cursor");
        bf_read_mmap_cursor(buffer_size, ina_str_cstr(filepath));
    } else {
        printf("Invalid benchmark!\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

