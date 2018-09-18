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
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <libinac/lib.h>

#define __INA_MBS(tot_nb_read, us) ((((double)(tot_nb_read))/1024)/(((double)us)/1000/1000))

INA_BENCH_DATA(file)
{
    ina_str_t          filepath;
    ina_file_ctx_t    *file_ctx;
    ina_mmap_ctx_t    *mmap_ctx;
    unsigned char     *read_buf;
    uint64_t           tot_nb_read;
    int                buffer_size;
    ina_file_t        *file;
    ina_file_cursor_t *cursor;
};

INA_BENCH_SETUP(file) {
    ina_bench_set_scale_label("buffer_size_kb");
    ina_bench_set_precision(2);
    ina_mem_set(data, 0, sizeof(struct file_data));
    INA_MUST_SUCCEED(ina_file_ctx_new(&data->file_ctx, 0));
    INA_MUST_SUCCEED(ina_mmap_init(&data->mmap_ctx));

    int i;
    FILE *fp;

    if ((fp = fopen("file_bench.txt", "r")) == NULL) {
        fp = fopen("file_bench.txt", "w");
        for (i = 0; i < (1024 * 10); i++) {
            fseek(fp, (1024 * 10), SEEK_CUR);
            fprintf(fp, "C");
        }
        fclose(fp);
    } else {
        fclose(fp);
    }
    data->filepath = ina_str_new_fromcstr("file_bench.txt");

}

INA_BENCH_TEARDOWN(file) {
    if (data->filepath != NULL) {
        ina_str_free(data->filepath);
        data->filepath = NULL;
    }
    if (data->cursor != NULL) {
        ina_file_cursor_free(&data->cursor);
    }
    if (data->file_ctx != NULL) {
        ina_file_ctx_free(&data->file_ctx);
    }
    if (data->mmap_ctx != NULL) {
        ina_mmap_destroy(&data->mmap_ctx);
    }
}

INA_BENCH_SCALE(file) {
    data->buffer_size = 4096 * ina_bench_get_iteration();
    ina_bench_set_scale(data->buffer_size);
}

INA_BENCH_BEGIN(file, bf_read) {}
INA_BENCH_END(file, bf_read) {}
INA_BENCH(file, bf_read, 4)
{
    int64_t nb_read = -1;
    data->tot_nb_read = 0;

    INA_MUST_SUCCEED(ina_file_new(data->file_ctx, ina_str_cstr(data->filepath),
            INA_FILE_ACCESS_MODE_READ,
            INA_FILE_CREATE_MODE_OPEN,
            INA_FILE_SHARE_MODE_READ,
            0,
            &data->file));

    data->read_buf = ina_mem_alloc(data->buffer_size);

    ina_bench_stopwatch_start();
    while (INA_SUCCEED(ina_file_read(data->file, data->read_buf, data->buffer_size, &nb_read)) && nb_read) {
        data->tot_nb_read += nb_read;
    }
    ina_bench_set_double(__INA_MBS(data->tot_nb_read, ina_bench_stopwatch_stop()));
    INA_BENCH_MSG("bytes read : %"INA_INT64_T_FMT, data->tot_nb_read);

    ina_file_free(&data->file);
    ina_mem_free(data->read_buf);
}


INA_BENCH_BEGIN(file, bf_read_seq) {}
INA_BENCH_END(file, bf_read_seq) {}
INA_BENCH(file, bf_read_seq, 4)
{
    int64_t nb_read = -1;
    data->tot_nb_read = 0;

    INA_MUST_SUCCEED(ina_file_new(data->file_ctx, data->filepath,
            INA_FILE_ACCESS_MODE_READ,
            INA_FILE_CREATE_MODE_OPEN,
            INA_FILE_SHARE_MODE_READ,
            INA_FILE_FLAG_SEQUENTIAL_ACCESS,
            &data->file));

    data->read_buf = ina_mem_alloc(data->buffer_size);

    ina_bench_stopwatch_start();
    while (INA_SUCCEED(ina_file_read(data->file, data->read_buf, data->buffer_size, &nb_read)) && nb_read) {
        data->tot_nb_read += nb_read;
    }
    ina_bench_set_double(__INA_MBS(data->tot_nb_read, ina_bench_stopwatch_stop()));
    INA_BENCH_MSG("bytes read : %"INA_INT64_T_FMT, data->tot_nb_read);

    ina_file_free(&data->file);
    ina_mem_free(data->read_buf);
    data->read_buf = NULL;
}

INA_BENCH_BEGIN(file, bf_read_direct) {}
INA_BENCH_END(file, bf_read_direct) {}
INA_BENCH(file, bf_read_direct, 4)
{
    int64_t nb_read = -1;
    data->tot_nb_read = 0;
    unsigned char* buf;
#ifndef INA_OS_WIN32
    INA_MUST_SUCCEED(ina_file_new(data->file_ctx, data->filepath,
            INA_FILE_ACCESS_MODE_READ,
            INA_FILE_CREATE_MODE_OPEN,
            INA_FILE_SHARE_MODE_READ,
            INA_FILE_FLAG_POSIX_DIRECT|INA_FILE_FLAG_SEQUENTIAL_ACCESS,
            &data->file));
#else
    INA_MUST_SUCCEED(ina_file_new(data->file_ctx, data->filepath,
                                  INA_FILE_ACCESS_MODE_READ,
                                  INA_FILE_CREATE_MODE_OPEN,
                                  INA_FILE_SHARE_MODE_READ,
                                  INA_FILE_FLAG_SEQUENTIAL_ACCESS,
                                  &data->file));
#endif
    data->read_buf = ina_mem_alloc_aligned(4096 * 2, data->buffer_size + 4096);
    buf = data->read_buf;
    data->read_buf += 4096;

    ina_bench_stopwatch_start();
    while (INA_SUCCEED(ina_file_read(data->file, data->read_buf, data->buffer_size, &nb_read)) && nb_read) {
        data->tot_nb_read += nb_read;
    }
    ina_bench_set_double(__INA_MBS(data->tot_nb_read, ina_bench_stopwatch_stop()));
    INA_BENCH_MSG("bytes read : %"INA_INT64_T_FMT, data->tot_nb_read);

    ina_file_free(&data->file);
    ina_mem_free(buf);
    data->read_buf = NULL;
}

INA_BENCH_BEGIN(file, bf_read_cursor) {}
INA_BENCH_END(file, bf_read_cursor) {}
INA_BENCH(file, bf_read_cursor, 4)
{
    size_t nb_read = -1;
    data->tot_nb_read = 0;
    const unsigned char *buf;

    INA_MUST_SUCCEED(ina_file_new(data->file_ctx, data->filepath,
            INA_FILE_ACCESS_MODE_READ,
            INA_FILE_CREATE_MODE_OPEN,
            INA_FILE_SHARE_MODE_READ,
            INA_FILE_FLAG_SEQUENTIAL_ACCESS,
            &data->file));
        
    INA_MUST_SUCCEED(ina_file_cursor_new(data->file,
                INA_FILE_CURSOR_TYPE_FILEIO,
                INA_FILE_CURSOR_MODE_READ_BINARY,
                (size_t)data->buffer_size,
                &data->cursor,
                data->mmap_ctx));


    ina_bench_stopwatch_start();
    while (INA_SUCCEED(ina_file_cursor_binary_read_chunk(data->cursor,
                            data->buffer_size, &nb_read, &buf)) && nb_read) {
        data->tot_nb_read += nb_read;
    }
    ina_bench_set_double(__INA_MBS(data->tot_nb_read, ina_bench_stopwatch_stop()));
    INA_BENCH_MSG("bytes read : %"INA_INT64_T_FMT, data->tot_nb_read);

    ina_file_free(&data->file);
}

INA_BENCH_BEGIN(file, bf_read_mmap_cursor) {}
INA_BENCH_END(file, bf_read_mmap_cursor) {}
INA_BENCH_SKIP(file, bf_read_mmap_cursor, 4)
{
    size_t nb_read = -1;
    data->tot_nb_read = 0;
    const unsigned char *buf;

    INA_MUST_SUCCEED(ina_file_new(data->file_ctx, data->filepath,
        INA_FILE_ACCESS_MODE_READ,
        INA_FILE_CREATE_MODE_OPEN,
        INA_FILE_SHARE_MODE_READ,
        INA_FILE_FLAG_SEQUENTIAL_ACCESS,
        &data->file));
        
    INA_MUST_SUCCEED(ina_file_cursor_new(data->file,
        INA_FILE_CURSOR_TYPE_MMAP,
        INA_FILE_CURSOR_MODE_READ_BINARY,
        (size_t)data->buffer_size,
        &data->cursor,
        data->mmap_ctx));

    ina_bench_stopwatch_start();
    while (INA_SUCCEED(ina_file_cursor_binary_read_chunk(data->cursor,
                            (size_t)data->buffer_size, &nb_read, &buf)) && nb_read) {
        data->tot_nb_read += nb_read;
    }
    ina_bench_set_double(__INA_MBS(data->tot_nb_read, ina_bench_stopwatch_stop()));
    INA_BENCH_MSG("bytes read : %"INA_INT64_T_FMT, data->tot_nb_read);
    ina_file_free(&data->file);
}


