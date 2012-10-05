/*
 * Copyright (c) 2012, INAOS GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
<<<<<<< HEAD
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
=======
 *     * Neither the name of the INAOS GmbH nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
 */
#include <libinac/lib.h>
#include "config.h"


INA_API(ina_rc_t) ina_mempool_init(void)
{
<<<<<<< HEAD
    return INA_SUCCESS;
=======
    return INA_RC_OK;
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
}

INA_API(ina_rc_t) ina_mempool_create(ina_mempool_t **pool)
{
<<<<<<< HEAD
    return INA_SUCCESS;
=======
    return INA_RC_OK;
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
}

INA_API(ina_rc_t) ina_mempool_destroy(ina_mempool_t *pool)
{
<<<<<<< HEAD
    return INA_SUCCESS;
=======
    return INA_RC_OK;
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
}

INA_API(ina_rc_t) ina_mempool_reset(ina_mempool_t *pool)
{
<<<<<<< HEAD
    return INA_SUCCESS;
=======
    return INA_RC_OK;
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
}

INA_API(void *) ina_mempool_palloc(ina_mempool_t *pool, size_t size)
{
    return NULL;
}

INA_API(ina_rc_t) ina_mempool_pfree(ina_mempool_t *pool, void *p)
{
<<<<<<< HEAD
    return INA_SUCCESS;
=======
    return INA_RC_OK;
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
}

