/*
 * Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>

INA_TEST_SKIP(cio, get_limits)
{
    ina_cio_pos_t pos = {0,0};
    INA_UNUSED(data);

    INA_TEST_ASSERT_SUCCEED(ina_cio_get_limits(&pos));
    INA_TEST_ASSERT_TRUE(0 < pos.col);
    INA_TEST_ASSERT_TRUE(0 < pos.row);
}

INA_TEST(cio, invalid_arguments)
{
    INA_UNUSED(data);
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cio_set_attribs(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cio_get_attribs(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cio_get_pos(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cio_move_to_pos(NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cio_printf(1,1,
            INA_CIO_COLOR_BLACK, INA_CIO_COLOR_BLUE, NULL));
    INA_TEST_ASSERT_ERRMSG(INA_ERR_INVALID_ARGUMENT, ina_cio_read_line(NULL));
}
