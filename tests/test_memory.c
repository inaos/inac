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

INA_TEST_DATA(memory){};
INA_TEST(memory, zero_size)
{
    INA_UNUSED(data);
    INA_TEST_ASSERT_NULL(ina_mem_alloc(0));
    INA_TEST_ASSERT_SUCCEED(ina_err_get_rc());
    INA_TEST_ASSERT_NULL(ina_mem_alloc_aligned(sizeof(void*), 0));
    INA_TEST_ASSERT_SUCCEED(ina_err_get_rc());
}

INA_TEST(memory, realloc)
{
    uint8_t i;
    uint8_t *pc;
    uint8_t *p1 = ina_mem_alloc(128);
    INA_UNUSED(data);

    INA_TEST_ASSERT_NOT_NULL(p1);
    ina_mem_set(p1, 0, 128);
    pc = p1;
    for (i = 0; i < 128; ++i) {
        pc += 1;
        *pc = i;
    }
    uint8_t *p2 = ina_mem_realloc(p1, 64);
    pc = p2;
    for (i = 0; i < 64; ++i) {
        pc += 1;
        INA_TEST_ASSERT_EQUAL_UINT(i, *pc);
    }
    INA_TEST_ASSERT_SAME(p1, p2);
    p2 = ina_mem_realloc(p2, 256);
    pc = p2;
    for (i = 0; i < 64; ++i) {
        pc += 1;
        INA_TEST_ASSERT_EQUAL_UINT(i, *pc);
    }
    INA_TEST_ASSERT_NULL(ina_mem_realloc(p2, 0));
}

INA_TEST(memory, invalid_alignment)
{
    INA_UNUSED(data);
    INA_TEST_ASSERT_NULL(ina_mem_alloc_aligned(0, 16));
    INA_TEST_ASSERT_FAILED(ina_err_get_rc());
    INA_TEST_ASSERT_EQUAL_UINT(INA_ERR_INVALID_ARGUMENT, INA_RC_ERRMSG(ina_err_get_rc()));
}

INA_TEST(memory, memory_align)
{
    INA_UNUSED(data);

    INA_TEST_ASSERT_EQUAL_INT(16, INA_MEM_ALIGN(1));
    INA_TEST_ASSERT_EQUAL_INT(16, INA_MEM_ALIGN(10));
    INA_TEST_ASSERT_EQUAL_INT(32, INA_MEM_ALIGN(17));
    INA_TEST_ASSERT_EQUAL_INT(48, INA_MEM_ALIGN(33));
    INA_TEST_ASSERT_EQUAL_INT(6, INA_MEM_ALIGN_N(6,1));
    INA_TEST_ASSERT_EQUAL_INT(0, INA_MEM_ALIGN_N(6,0));
}

INA_TEST(memory, memory_alloc_aligned)
{
    void *p;
    INA_UNUSED(data);

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
    INA_UNUSED(data);

#ifdef INA_OS_WINDOWS
    SYSTEM_INFO si;
#endif

    INA_TEST_ASSERT_SUCCEED(ina_mem_get_pagesize(&size));
#ifndef INA_OS_WINDOWS
    INA_TEST_ASSERT_TRUE((size_t)sysconf(_SC_PAGESIZE) == size);
#else
    GetSystemInfo(&si);
    INA_TEST_ASSERT_EQUAL_SIZE_T((size_t)si.dwPageSize, size);
#endif
}

INA_TEST(memory, invalid_arguments)
{
    INA_UNUSED(data);
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_mem_get_pagesize(NULL));
}
