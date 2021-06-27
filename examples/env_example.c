/*
 * Copyright 2012-2021 INAOS GmbH, Thalwil
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
    INA_OPTS(opt,
        INA_OPT_STRING("u","username", "$ENV:USERNAME", "Username" ),
        INA_OPT_INT("i", "integer", "$ENV:ENV_EXAMPLE_INT", "Integer value"),
        INA_OPT_FLOAT("f", "float", "$ENV:ENV_EXAMPLE_FLOAT", "Float value"));

    if (INA_FAILED(ina_app_init(argc, argv, opt))) {
        return EXIT_FAILURE;
    }
    ina_str_t username = NULL;
    float float_value = 0;
    int int_value = 0;
    ina_opt_get_string("username", &username);
    ina_opt_get_float("float", &float_value);
    ina_opt_get_int("integer", &int_value);

    printf("Username: %s\n", username);
    printf("Float value: %f\n", float_value);
    printf("Integer value: %d\n", int_value);

    ina_str_free(username);

    return EXIT_SUCCESS;
}
