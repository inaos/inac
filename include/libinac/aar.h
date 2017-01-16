/*
* Copyright (c) 2017, INAOS GmbH
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* * Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of the INAOS GmbH nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
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
#ifndef _LIBINAC_AAR_H_
#define _LIBINAC_AAR_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * AAR - Application Archive
 *
 * - File extension will be .iar (INAOS application archive)
 *
 * WARING: In this first release only file-loading is supported
 *
 */

/* opaque context */
typedef struct ina_aar_ctx_s ina_aar_ctx_t;

/* opaque context */
typedef struct ina_aar_app_s ina_aar_app_t;

/*
 * Initialize a context
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_aar_init(ina_aar_ctx_t **ctx, const char *working_directory);

/*
 *
 */
INA_API(ina_rc_t) ina_aar_app_new(ina_aar_ctx_t *ctx, const char *id, ina_aar_app_t **app);

/*
 *
 */
INA_API(ina_rc_t) ina_aar_app_archive_load(ina_aar_app_t *app, const char *full_path);

/*
 *
 */
INA_API(ina_rc_t) ina_aar_app_meta_str_get(ina_aar_app_t *app, const char *key, ina_str_t *value);

/*
 *
 */
INA_API(ina_rc_t) ina_aar_app_free(ina_aar_ctx_t *ctx, ina_aar_app_t **app);

/*
 * Destroy a context
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_aar_destroy(ina_aar_ctx_t **ctx);


#ifdef __cplusplus
}
#endif

#endif
