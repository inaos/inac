/*
 * Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
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

    if (INA_FAILED(ina_mempool_new(size, ina_str_new_fromcstr(label),
                                   INA_MEM_SHARED | INA_MEM_SHARED_CREATE | INA_MEM_SHARED_EXCL, &mp))) {
            INA_TEST_HELPER_SET_RC(ina_err_get_last_rc());
            return;
    }

    c = 0;
    v = (int32_t*)ina_mempool_dalloc(mp, size);
    INA_ASSERT_NOTNULL(v);
    
    while (c  < (size/sizeof(int32_t))) {
        v[c] = (int)c;
        c++;
    }
    
    /* Run until kill signal */
    while (1) {
        ina_time_sleep(1000);
    }
    INA_TEST_HELPER_SET_RC(INA_SUCCESS);
}
