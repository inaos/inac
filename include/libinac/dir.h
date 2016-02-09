/*
 * Copyright (c) 2016, INAOS GmbH
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
#ifndef _LIBINAC_DIR_H_
#define _LIBINAC_DIR_H_

#include <libinac/lib.h>


#ifdef __cplusplus
extern "C" {
#endif

/* Opaque directory walker handle */
typedef struct ina_dir_walker_s ina_dir_walker_t;


/* Directory sort order */
typedef enum ina_dir_sort_order_e {
    INA_DIR_SORT_ORDER_NONE = 0,
    INA_DIR_SORT_ORDER_ASCEND,
    INA_DIR_SORT_ORDER_DESCEND,
} ina_dir_sort_order_t;

/* Directory sort attrib */
typedef enum ina_dir_sort_attrib_e {
    INA_DIR_SORT_ATTRIB_DFT = 0,
    INA_DIR_SORT_ATTRIB_NAME,
    INA_DIR_SORT_ATTRIB_TYPE
} ina_dir_sort_attrib_t;

/* Directory entry typ */
typedef enum ina_dir_entry_type_e {
    INA_DIR_ENTRY_TYPE_FILE = 1,
    INA_DIR_ENTRY_TYPE_DIR
} ina_dir_entry_type_t;

/* Directory entry */
typedef struct ina_dir_entry_s {
    ina_str_t name;
    ina_dir_entry_type_t type;
    size_t size;
} INA_ALIGNED64 ina_dir_entry_t;

/*
 * Create a new directory walker.
 */
INA_API(ina_rc_t) ina_dir_walker_new(const char *basedir, ina_dir_walker_t **walker);

/*
 * Enable recursive directory walking.
 */
INA_API(ina_rc_t) ina_dir_walker_enable_recursive(ina_dir_walker_t *walker);

/*
 * Disable recursive directory walking.
 */
INA_API(ina_rc_t) ina_dir_walker_disable_recursive(ina_dir_walker_t *walker);

/*
 * Get current sort order for walker.
 */
INA_API(ina_rc_t) ina_dir_walker_get_sort_order(const ina_dir_walker_t *walker,
                                                ina_dir_sort_order_t *sort_order);

/*
 * Set directory sort order for walker.
 */
INA_API(ina_rc_t) ina_dir_walker_set_sort_order(ina_dir_walker_t *walker,
                                                ina_dir_sort_order_t sort_order);

/*
 * Get current directory sort attrib for walker.
 */
INA_API(ina_rc_t) ina_dir_walker_get_sort_attrib(const ina_dir_walker_t *walker,
                                                 ina_dir_sort_attrib_t *sort_attrib);

/*
 * Set directory sort attrib for walker.
 */
INA_API(ina_rc_t) ina_dir_walker_set_sort_attrib(ina_dir_walker_t *walker,
                                                 ina_dir_sort_attrib_t sort_attrib);

/*
 * Get next directory entry of walker.
 */
INA_API(ina_rc_t) ina_dir_walker_get_next_entry(ina_dir_walker_t *walker,
                                                 const ina_dir_entry_t **entry);

/*
 * Reset walker. Does not reflect FS changes, sort order and sort attrib.
 */
INA_API(ina_rc_t) ina_dir_walker_reset(ina_dir_walker_t *walker);

/*
 * Reload walker. Reflect FS changes, sort order and sort attrib.
 */
INA_API(ina_rc_t) ina_dir_walker_reload(ina_dir_walker_t *walker);

/*
 * Free directory walker.
 */
INA_API(ina_rc_t) ina_dir_walker_free(ina_dir_walker_t **walker);



#ifdef __cplusplus
}
#endif

#endif
