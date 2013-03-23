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

 void test_string_simple_allocation_with_pool() 
 {
	 ina_str_t str1;
     ina_str_t str2;
     ina_mempool_t *pool;

     INA_TRACE_MSG("test_string_simple_allocation_with_pool");

     INA_ASSERT_SUCCEED(ina_mempool_create(&pool, 1024, 0, NULL));

     str1 = ina_str_pfromcstr("hallo", pool);
     INA_ASSERT_NOTNULL(str1);
     INA_ASSERT_EQUAL(strlen("hallo"), ina_str_len(str1));
     str2 = ina_str_pdup(str1, pool);
     INA_ASSERT_NOTNULL(str2);
     ina_str_destroy(str1);
     ina_str_destroy(str2);
}

void test_string_simple_allocation_without_pool() 
{
	ina_str_t str1;
    ina_str_t str2;

    INA_TRACE_MSG("test_string_simple_allocation_without_pool");
    
    str1 = ina_str_fromcstr("hallo");
    INA_ASSERT_NOTNULL(str1);
    INA_ASSERT_EQUAL(strlen("hallo"), ina_str_len(str1));
    str2 = ina_str_dup(str1);
    INA_ASSERT_NOTNULL(str2);
    ina_str_destroy(str1);
    ina_str_destroy(str2);
}
