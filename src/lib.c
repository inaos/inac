/*
 * Copyright (c) 2012, INAOS GmbH
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
#include <libinac/lib.h>
#include "config.h"

static int32_t initialized = 0;
 
INA_API(ina_rc_t) ina_appinit(const int argc,  const char *argv[]) 
{
    return ina_libinit();
}

INA_API(ina_rc_t) ina_libinit(void)
{
    if (initialized++) {
        return INA_SUCCESS;
    }
    
    /* initalize global memory functons */
    ina_mem_set_fn(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    
    /* initalize error state */
    ina_err_clear(INA_ERR_CLEAR_ALL);
    
    /* TODO: initialize memory pool */
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_exit(void)
{
    while (!initialized--) {
        ina_exit();
    }
    
    ina_err_clear(INA_ERR_CLEAR_ALL);
    
    /* TODO: tear down memory pool */
    return INA_SUCCESS;
}
 