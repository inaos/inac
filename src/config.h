/*
 * Copyright 2012-2020 INAOS GmbH, Thalwil
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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