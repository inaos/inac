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
#include <stdio.h>
#include <libinac/lib.h>

void test_mempool_syspool() 
{
    void *p;
    ina_mempool_info_t mi;

    INA_TRACE("test_mempool_syspool");
    
    INA_ASSERT_EQUAL(INA_SUCCESS, ina_err_clear(INA_ERR_STATE_CLEAR));    
    INA_ASSERT_EQUAL(INA_SUCCESS, ina_mempool_destroy());
    INA_ASSERT_EQUAL(INA_SUCCESS, ina_mempool_init(0));
    INA_ASSERT_EQUAL(INA_SUCCESS, ina_mempool_getinfo(NULL, &mi));
    INA_ASSERT_EQUAL(0, mi.children);
    INA_ASSERT_EQUAL(8*1024*1024, mi.size);
    INA_ASSERT_EQUAL(INA_SUCCESS, ina_mempool_release(NULL, 0));
    INA_ASSERT_EQUAL(INA_SUCCESS, ina_mempool_init(10*1024*1024));
    INA_ASSERT_EQUAL(INA_SUCCESS, ina_mempool_getinfo(NULL, &mi));
    INA_ASSERT_EQUAL(0, mi.children);
    INA_ASSERT_EQUAL(10*1024*1024, mi.size);
    
    p = ina_mem_alloc(2*1024*1024);
    INA_ASSERT_NOTNULL(p);
    INA_ASSERT_EQUAL(INA_SUCCESS, ina_err_peek());
    INA_ASSERT_EQUAL(INA_SUCCESS, ina_mempool_getinfo(NULL, &mi));
    INA_ASSERT_EQUAL(0, mi.children);
    INA_ASSERT_EQUAL(10*1024*1024, mi.size);
    printf("mi.used= %lu", mi.used);
    INA_ASSERT_EQUAL(2*1024*1042, mi.used);
} 
