/*
 * Copyright INAOS GmbH, Thalwil, 2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
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
    INA_MUST_SUCCEED(ina_mmap_ctx_new(&data->mmap_ctx));

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
        ina_mmap_ctx_free(&data->mmap_ctx);
    }
}

INA_BENCH_SCALE(file) {
    data->buffer_size = 4096 * ina_bench_get_iteration();
    ina_bench_set_scale(data->buffer_size);
}

INA_BENCH_BEGIN(file, bf_read) { INA_UNUSED(data); }
INA_BENCH_END(file, bf_read) { INA_UNUSED(data); }
INA_BENCH(file, bf_read, 4, 1)
{
    size_t nb_read = 0;
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
    ina_bench_set_value(__INA_MBS(data->tot_nb_read, ina_bench_stopwatch_stop()));
    INA_BENCH_MSG("bytes read : %"INA_INT64_T_FMT, data->tot_nb_read);

    ina_file_free(&data->file);
    ina_mem_free(data->read_buf);
}


INA_BENCH_BEGIN(file, bf_read_seq) { INA_UNUSED(data);}
INA_BENCH_END(file, bf_read_seq) { INA_UNUSED(data);}
INA_BENCH(file, bf_read_seq, 4, 1)
{
    size_t nb_read = 0;
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
    ina_bench_set_value(__INA_MBS(data->tot_nb_read, ina_bench_stopwatch_stop()));
    INA_BENCH_MSG("bytes read : %"INA_INT64_T_FMT, data->tot_nb_read);

    ina_file_free(&data->file);
    ina_mem_free(data->read_buf);
    data->read_buf = NULL;
}

INA_BENCH_BEGIN(file, bf_read_direct) { INA_UNUSED(data);}
INA_BENCH_END(file, bf_read_direct) { INA_UNUSED(data); }
INA_BENCH(file, bf_read_direct, 4, 1)
{
    size_t nb_read = 0;
    data->tot_nb_read = 0;
    unsigned char* buf;
#ifndef INA_OS_WINDOWS
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
    ina_bench_set_value(__INA_MBS(data->tot_nb_read, ina_bench_stopwatch_stop()));
    INA_BENCH_MSG("bytes read : %"INA_INT64_T_FMT, data->tot_nb_read);

    ina_file_free(&data->file);
    ina_mem_free(buf);
    data->read_buf = NULL;
}

INA_BENCH_BEGIN(file, bf_read_cursor) { INA_UNUSED(data);}
INA_BENCH_END(file, bf_read_cursor) { INA_UNUSED(data); }
INA_BENCH(file, bf_read_cursor, 4, 1)
{
    size_t nb_read = 0;
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
    ina_bench_set_value(__INA_MBS(data->tot_nb_read, ina_bench_stopwatch_stop()));
    INA_BENCH_MSG("bytes read : %"INA_INT64_T_FMT, data->tot_nb_read);

    ina_file_free(&data->file);
}

INA_BENCH_BEGIN(file, bf_read_mmap_cursor) { INA_UNUSED(data);}
INA_BENCH_END(file, bf_read_mmap_cursor) { INA_UNUSED(data); }
INA_BENCH_SKIP(file, bf_read_mmap_cursor, 4, 1)
{
    size_t nb_read = 0;
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
    ina_bench_set_value(__INA_MBS(data->tot_nb_read, ina_bench_stopwatch_stop()));
    INA_BENCH_MSG("bytes read : %"INA_INT64_T_FMT, data->tot_nb_read);
    ina_file_free(&data->file);
}


