/*
 * Copyright (c) 2014, INAOS GmbH
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
    ina_str_t name;
    ina_str_t action;
 
    INA_OPTS(opt,
        INA_OPT_STRING("n", "name", NULL, "Service name for start,stop,status or binary path for install/uninstall"),
        INA_OPT_STRING("a", "action", "status", "Action: status, start, stop, install, uninstall"));

    if (!INA_SUCCEED(ina_app_init(argc, argv, 0, opt))) {
        return EXIT_FAILURE;
    }
    
    ina_opt_get_string("name", &name);
    ina_opt_get_string("action", &action);

    if (strcasecmp("start", ina_str_cstr(action)) == 0) {
        if (INA_SUCCEED(ina_service_mgnt_start(ina_str_cstr(name)))) {
            printf("Service %s started.\n", ina_str_cstr(name));
            return EXIT_SUCCESS;            
        }
        printf("Error starting service %s\n", ina_str_cstr(name));

    } else if (strcasecmp("stop", ina_str_cstr(action)) == 0) {
        if (INA_SUCCEED(ina_service_mgnt_stop(ina_str_cstr(name)))) {
            printf("Service %s stopped.\n", ina_str_cstr(name));
            return EXIT_SUCCESS;            
        }
        printf("Error stopping service %s\n", ina_str_cstr(name));
    
    } else if (strcasecmp("install", ina_str_cstr(action)) == 0) {
        if (INA_SUCCEED(ina_service_mgnt_install(ina_str_cstr(name), NULL))) {
            printf("Service %s installed.\n", ina_str_cstr(name));
            return EXIT_SUCCESS;            
        }
        printf("Error installing service %s\n", ina_str_cstr(name));
    
    } else if (strcasecmp("uninstall", ina_str_cstr(action)) == 0) {
        if (INA_SUCCEED(ina_service_mgnt_uninstall(ina_str_cstr(name)))) {
            printf("Service %s uninstalled.\n", ina_str_cstr(name));
            return EXIT_SUCCESS;            
        }
        printf("Error uninstalling service %s\n", ina_str_cstr(name));
    
    } else {
        ina_service_status_t status;
        ina_service_mgnt_status(ina_str_cstr(name), &status);
        switch (status) {
            case INA_SERVICE_STATUS_RUN:
                printf("Service %s is runnning.\n", ina_str_cstr(name));
                break;
            case INA_SERVICE_STATUS_STOP:
                printf("Service %s is stopped.\n", ina_str_cstr(name));
                break;
            default:
                printf("Could not query status of service %s\n", ina_str_cstr(name));
                break;
        }
    }
    return EXIT_SUCCESS;
}
