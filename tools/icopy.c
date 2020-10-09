/*
 * Copyright 2018-2020 INAOS GmbH, Thalwil
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <libinac/lib.h>

static ina_file_ctx_t *file_ctx = NULL;

static void ina_cleanup_handler(int error, int *exitcode)
{
	INA_UNUSED(error);
    if (file_ctx != NULL) {
        ina_file_ctx_free(&file_ctx);
    }
	*exitcode = 0;
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
    size_t nread = 0;
    size_t nwrote = 0;
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