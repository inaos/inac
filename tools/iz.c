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
 * CAUSED AND ON ANYs THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <libinac/lib.h>

INA_NO_SERVICE;


int main(int argc,  char** argv) 
{ 
    ina_gzip_file_t *gzf = NULL;
    ina_file_ctx_t *fctx = NULL;
    ina_file_t *of = NULL;
    ina_str_t in_filepath;
    ina_str_t out_filepath;
    size_t read;
    int64_t wrote;
    unsigned char buf[1024];
    unsigned char *chunk;
 
    INA_OPTS(opt,
        INA_OPT_STRING("f", "infile", NULL, "Input filepath"),
        INA_OPT_STRING("o", "outfile", NULL, "Outpuf filepath"));

    if (!INA_SUCCEED(ina_app_init(argc, argv, 0, opt))) {
        return EXIT_FAILURE;
    }
    
    ina_opt_get_string("f", &in_filepath);
    ina_opt_get_string("o", &out_filepath);
  
    chunk = buf;

    if (!INA_SUCCEED(ina_gzip_open(ina_str_cstr(in_filepath), 4*1024, &gzf))) {
        return EXIT_FAILURE;
    }
    if (!INA_SUCCEED(ina_file_init(&fctx))) {
        ina_gzip_close(&gzf);
        return EXIT_FAILURE;
    }
    if (!INA_SUCCEED(ina_file_new(fctx, ina_str_cstr(out_filepath), 
        INA_FILE_ACCESS_MODE_READWRITE,
        INA_FILE_CREATE_MODE_CREATE,
        INA_FILE_SHARE_MODE_WRITE,0, &of))) {
        ina_gzip_close(&gzf);
        ina_file_destroy(&fctx);
        return EXIT_FAILURE;
    }
  
    while (INA_SUCCEED(ina_gzip_read_next_block(gzf, 1024, &read, &chunk)) && read > 0) {
        ina_file_write(of, chunk, (int64_t)read, &wrote);
        chunk = buf;
    }
    ina_gzip_close(&gzf);
    ina_file_free(fctx, &of);
    ina_file_destroy(&fctx);
    return EXIT_SUCCESS;
}
