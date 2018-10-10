/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <stdio.h>
#include <libinac/lib.h>
 
INA_TEST(memory, memory_memfn)
{
}

INA_TEST(memory, zero_size)
{
    INA_TEST_ASSERT_NULL(ina_mem_alloc(0));
    INA_TEST_ASSERT_SUCCEED(ina_err_get_rc());
    INA_TEST_ASSERT_NULL(ina_mem_alloc_aligned(sizeof(void*), 0));
    INA_TEST_ASSERT_SUCCEED(ina_err_get_rc());
}

INA_TEST(memory, invalid_alignment)
{
    INA_TEST_ASSERT_NULL(ina_mem_alloc_aligned(0, 16));
    INA_TEST_ASSERT_FAILED(ina_err_get_rc());
    INA_TEST_ASSERT_EQUAL_INT64(INA_ERR_INVALID, INA_RC_ERROR(ina_err_get_rc()));
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
    INA_TEST_ASSERT_NOT_NULL(p);
    INA_TEST_ASSERT_TRUE(INA_MEM_IS_ALIGNED(p, 2));
    ina_mem_free(p);
    p = NULL;

    p = ina_mem_alloc_aligned(4, 128);
    INA_TEST_ASSERT_NOT_NULL(p);
    INA_TEST_ASSERT_TRUE(INA_MEM_IS_ALIGNED(p, 4));
    ina_mem_free(p);
    p = NULL;

    p = ina_mem_alloc_aligned(16, 128);
    INA_TEST_ASSERT_NOT_NULL(p);
    INA_TEST_ASSERT_TRUE(INA_MEM_IS_ALIGNED(p, 16));
    ina_mem_free(p);
    p = NULL;

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
