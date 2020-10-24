/*
 * Copyright 2012-2020 INAOS GmbH, Thalwil
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
    if (INA_FAILED(ina_app_init(argc, argv, NULL))) {
        return EXIT_FAILURE;
    }
    /* Use default configuration */
    if (INA_FAILED(ina_log_init(NULL))) {
        return EXIT_FAILURE;
    }

    INA_LOG_DEBUG("this is a simple DEBUG message to stdout");
    INA_LOG_INFO("this is a simple INFO message to the stdout");
    INA_LOG_WARNING("this is a simple WARNING message to stdout");
    INA_LOG_ERROR("this is a simple ERROR message to stderr ");

    INA_LOG_ERROR("this is a ERROR message with var args (p1=%d) to stderr", 100);


    ina_log_destroy();

    return EXIT_SUCCESS;
}
