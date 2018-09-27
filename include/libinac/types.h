/*
 * Copyright INAOS GmbH, Thalwil, 2016-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_TYPES_H_
#define _LIBINAC_TYPES_H_


#ifdef __cplusplus
extern "C" {
#endif


/* Return code */
typedef uint64_t ina_rc_t;

typedef ina_rc_t (*ina_foreach_fn_t)(void *data);
typedef ina_rc_t (*ina_foreach_arg_fn_t)(void *arg, void *data);

typedef int (*ina_compare_fn_t)(const void *lhs, const void *lhd);
typedef int (*ina_find_fn_t)(const void *data, const void *find_arg);


#ifdef __cplusplus
}
#endif

#endif
