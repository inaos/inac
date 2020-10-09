/*
 * Copyright 2014-2020 INAOS GmbH, Thalwil
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

int main(int argc,  char** argv) 
{
    ina_rc_t rc;
    ina_ljit_ctx_t *ctx = NULL;
    ina_str_t bootstrp = NULL;

    INA_OPTS(opt,
        INA_OPT_STRING("b", "bootstrap", "<bootstrap>", "Full path to the bootstrap file"));

    if (!INA_SUCCEED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }

    if (!INA_SUCCEED(ina_ljit_ctx_new(&ctx))) {
        return EXIT_FAILURE;
    }

    /* execute the bootstrap if present */
    ina_opt_get_string("bootstrap", &bootstrp);
    if (strcmp("<bootstrap>", ina_str_cstr(bootstrp))!=0) {
        if (luaL_dofile(ctx->lstate, ina_str_cstr(bootstrp)) != 0) {
            printf("%s", luaL_checkstring(ctx->lstate, 1));
            ina_str_free(bootstrp);
            return EXIT_FAILURE;
        }
        else {
            ina_str_free(bootstrp);
        }
    }

    rc = ina_test_run(argc, argv, ctx);

    ina_ljit_ctx_free(&ctx);

    return rc;
}
