/*
 * Copyright (c) 2012-2018, INAOS GmbH
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
 
INA_TEST(memory, memory_memfn)
{
}

INA_TEST(memory, memory_align)
{
    INA_TEST_ASSERT_EQUAL_INT(16, INA_MEM_ALIGN(1));
    INA_TEST_ASSERT_EQUAL_INT(16, INA_MEM_ALIGN(10));
    INA_TEST_ASSERT_EQUAL_INT(32, INA_MEM_ALIGN(17));
    INA_TEST_ASSERT_EQUAL_INT(48, INA_MEM_ALIGN(33));
}

INA_TEST(memory, memory_alloc_aligned)
{
    void *p;
    p = ina_mem_alloc_aligned(2, 128);
    INA_TEST_ASSERT_TRUE(INA_MEM_IS_ALIGNED(p, 2));
    ina_mem_free_aligned(p);

    p = ina_mem_alloc_aligned(4, 128);
    INA_TEST_ASSERT_TRUE(INA_MEM_IS_ALIGNED(p, 4));
    ina_mem_free_aligned(p);

    p = ina_mem_alloc_aligned(16, 128);
    INA_TEST_ASSERT_TRUE(INA_MEM_IS_ALIGNED(p, 16));
    ina_mem_free_aligned(p);
}

INA_TEST(memory, pagesize)
{
    size_t size = 0;
#ifdef INA_OS_WIN32
    SYSTEM_INFO si;
#endif

    INA_TEST_ASSERT_SUCCEED(ina_mem_get_pagesize(&size));
#ifndef INA_OS_WIN32
    INA_TEST_ASSERT_TRUE((size_t)sysconf(_SC_PAGESIZE) == size);
#else
    GetSystemInfo(&si);
    INA_TEST_ASSERT_EQUAL_SIZE_T((size_t)si.dwPageSize, size);
#endif
}
