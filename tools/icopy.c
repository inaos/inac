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

static void ina_cleanup_handler(int error, int *exitcode)
{
    if (file_ctx != NULL) {
        ina_file_ctx_free(&file_ctx);
    }
}

int main(int argc,  char** argv) 
{
    ina_str_t src;
    ina_str_t dst;
    ina_file_t *src_file;
    ina_file_t *dst_file;
    ina_file_stat_t *stat;
    unsigned char buf[4096];
    uint64_t buf_size = 4096;
    int64_t nread = 0;
    int64_t nwrote = 0;
    int mode = 0;

    INA_OPTS(opt,
        INA_OPT_STRING("s", "source", NULL, "Source file"),
        INA_OPT_STRING("d", "destination", NULL, "Destination file"),
        INA_OPT_INT("m", "mode", (S_IRWXU), "Mode")
    );

    if (!INA_SUCCEED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }
    ina_set_cleanup_handler(ina_cleanup_handler);

    ina_opt_get_string("s", &src);
    ina_opt_get_string("d", &dst);
    ina_opt_get_int("m", &mode);

    if (INA_FAILED(ina_file_ctx_new(&file_ctx, 0))) {
        return EXIT_FAILURE;
    }

    if (INA_FAILED(ina_file_new(file_ctx, ina_str_cstr(src),
            INA_FILE_ACCESS_MODE_READ,
            INA_FILE_CREATE_MODE_OPEN,
            INA_FILE_SHARE_MODE_READ,
            0,
            &src_file))) {
        return EXIT_FAILURE;
    }
    if (INA_FAILED(ina_file_stat_new(src_file, &stat))) {
        printf("Can't stat %s\n", ina_str_cstr(src));
        return EXIT_FAILURE;
    }


    if (INA_FAILED(ina_file_new(file_ctx, ina_str_cstr(dst),
            INA_FILE_ACCESS_MODE_WRITE,
            INA_FILE_CREATE_MODE_CREATE,
            INA_FILE_SHARE_MODE_EXCLUSIVE,
            0,
            &dst_file))) {
        return EXIT_FAILURE;
    }
    if (INA_FAILED(ina_file_set_mode(dst_file, (mode_t )mode))) {
        printf("Can't set mode for %s\n", ina_str_cstr(dst));
        ina_file_stat_free(&stat);
        return EXIT_FAILURE;
    }

    while ((INA_SUCCEED(ina_file_read(src_file, (unsigned char*)buf, buf_size, &nread)) && nread > 0)) {
        if (INA_FAILED(ina_file_write(dst_file, (unsigned char*)buf, nread, &nwrote))) {
            break;
        }
    }
    return EXIT_SUCCESS;
}