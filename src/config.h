/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_CONIFG_H_
#define _LIBINAC_CONIFG_H_

/* Enabled logging */
#ifndef INA_LOG_ENABLED
#define INA_LOG_ENABLED 1
#endif

/* Define time code/library to use */
#ifndef INA_TIME_DEFINED
#define INA_OSTIME_ENABLED 1
#endif

/* Define default mem pool size */
#ifndef INA_MEMPOOL_SIZE
#define INA_MEMPOOL_SIZE  8*1024*1024
#endif

/* Define break message on assert for windows platform */
#ifndef INA_DGBMSG_ASSERT
#define INA_DGBMSG_ASSERT 1
#endif



#endif