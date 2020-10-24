/*
 * Copyright 2016-2020 INAOS GmbH, Thalwil
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

static ina_dir_stat_t *__ds = NULL;

static void ina_cleanup_handler(int error, int *exitcode)
{
	INA_UNUSED(error);
    if (__ds != NULL) {
        ina_dir_stat_free(&__ds);
    }
	*exitcode = 0;
}

int main(int argc,  char** argv) 
{
    ina_str_t dir;
    int hr = 0;

    INA_OPTS(opt,
        INA_OPT_STRING("d", "dir", NULL, "Directory to stat"),
        INA_OPT_FLAG("h", "human-readable", "Display the human readable stat")
    );

    if (!INA_SUCCEED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }
    ina_set_cleanup_handler(ina_cleanup_handler);
    
    ina_opt_get_string("d", &dir);
    if (INA_SUCCEED(ina_opt_isset("h"))) {
        hr = 1;
    }

    if (INA_SUCCEED(ina_dir_stat_new(ina_str_cstr(dir), &__ds))) {
        size_t b_total;
        size_t b_free;
        ina_dir_stat_bytes_capacity(__ds, &b_total);
        ina_dir_stat_bytes_free(__ds, &b_free);
        if (hr) {
            double mb_total;
            double mb_free;
            double gb_total;
            double gb_free;
            double d_total;
            double d_free;
            int used;
            char hdu[3];
            mb_total = (double)b_total/1024/1024;
            gb_total = mb_total/1024;
            mb_free = (double)b_free/1024/1024;
            gb_free = mb_free/1024;
            d_total = (double)b_total/1024;
            strncpy(hdu, "KB", 3);
            if (b_total > 1024*1024) {
                d_total = mb_total;
                strncpy(hdu, "MB", 3);
            }
            if (b_total > 1024UL*1024UL*1024UL) {
                d_total = gb_total;
                strncpy(hdu, "GB", 3);
            }
            d_free = (double)b_free/1024;
            if (b_free > 1024*1024) {
                d_free = mb_free;
            }
            if (b_free > 1024UL*1024UL*1024UL) {
                d_free = gb_free;
            }
            ina_dir_stat_pct_used(__ds, &used);
            printf("Total: %.2f %s, Free: %.2f %s - Percentage used: %d%%\n", d_total, hdu, d_free, hdu, used);
        }
        else {
            printf("Total Kbytes: %.2f, Free Kbytes: %.2f\n", ((double)b_total/1024.0), ((double)b_free/1024.0));
        }
    }

    return EXIT_SUCCESS;
}
