/* Copyright (c) 2013, INAOS GmbH
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


INA_TEST_HELPER(mempool_ipc, mempool_create_and_fill_int32_values) {
    const char *label;
    size_t size;
    int32_t *v;
    size_t c;
    ina_mempool_t *mp = NULL;

    INA_TEST_HELPER_CHECK_ARGC(2);
    label = INA_TEST_HELPER_CARG(0);
    size = INA_TEST_HELPER_IARG(1);

    if (!INA_SUCCEED(ina_mempool_create(&mp, size, 
        INA_MEM_SHARED|INA_MEM_SHARED_CREATE, ina_str_fromcstr(label)))) {
            INA_TEST_HELPER_SET_RC(ina_err_peek());
            return;
    }

    c = 0;
    v = (int32_t*)ina_mempool_dalloc(mp, size);
    while (c  < (size/sizeof(int32_t))) {
        *v = c++;
        v++;
    }
    
    /* Run until kill signal */
    while (1) {
        ina_time_sleep(1000);
    }
    INA_TEST_HELPER_SET_RC(INA_SUCCESS);
}
