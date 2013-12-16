/*
 * Copyright (c) 2013, INAOS GmbH
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
#ifndef _LIBINAC_LJIT_H_
#define _LIBINAC_LJIT_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* LuaJIT/Lua context */
typedef struct ina_ljit_ctx_s {
    lua_State *lstate;
} ina_ljit_ctx_t;

/* Cast Lua raw cdata pointer to a const char* */
#define INA_LJIT_TOCSTRING(ctx, index)                               \
    INA_LJIT_TOPOINTER(ctx, index, const char*)
/* Cast raw cdata pointer to a typed pointer */
#define INA_LJIT_TOPOINTER(ctx, index, type)                         \
    *(type*)ina_ljit_checkcdata(ctx, index)
/* Cast Lua number to an int */
#define INA_LJIT_TOINTEGER(ctx, index)                               \
    lua_tointeger(cxt->lstate, index)
/* Cast Lua number to double */
#define INA_LJIT_TODOUBLE(ctx, index)                                \
    lua_tonumber(ctx->lstate, index)
/* Cast Lua value to an int */ 
#define INA_LJIT_TOBOOLEAN(ctx, index)                               \
    lua_toboolean(ctx->lstate, index)

/*
 * Expose C Symbol.
 */
#define INA_LJIT_EXPORT(package, symbol)                               \
INA_API(const void) *__ina_ljit_export_##symbol (void) {               \
    __ina_ljit_##package = (const char*)(size_t) symbol;               \
    return  __ina_ljit_##package;                                      \
}
/*
 * Import a LuaJIT module.
 */
#ifdef __cplusplus
#define INA_LIJT_EXTERN extern "C"
#else
#define INA_LJIT_EXTERN extern
#endif
#define INA_LJIT_IMPORT(package, module)                                \
    INA_LJIT_EXTERN const char *luaJIT_BC_##module;              \
    INA_API(const void) *__ina_ljit_import_##module (void) {            \
        __ina_ljit_##package = (const char*)(size_t)luaJIT_BC_##module; \
        return  __ina_ljit_##package;                                   \
    }

/* 
 * Import LuaJIT Bytecode. Works only for modules generated using 
 * standard naming convention.
 */
#define INA_LJIT_PACKAGE(package)                                      \
    const void  *__ina_ljit_##package = NULL;
/*
 * Initalize LuaJIT contex.t
 * 
 * Parameters
 * ctx      Pointer to LuaJIT state pointer
 *
 * Return Value
 * INA_SUCCESS if no error occured. 
 */
INA_API(ina_rc_t) ina_ljit_init(ina_ljit_ctx_t **ctx);

/*
 * Destroy LuaJIT context.
 *
 * Parameters
 * ctx      Pointer to LuaJIT state pointer
 *
 * Return Value
 * INA_SUCCESS 
 */
INA_API(ina_rc_t) ina_ljit_destroy(ina_ljit_ctx_t **ctx);
     
/*
 * Call a Lua function.
 *
 * Parameter
 * ctx          LuaJIT state
 * fname        Function name. For methods use the object.method() syntax.
 * signature    Function signature. Each parameter is represente by a char 
 *              indicating the type. 
 *              s = string
 *              d = double
 *              i = int
 *              Return values are defined by the same way. After after the
 *              < char.
 *              Example of signature taking 2 strings, 1 double, 1 int and 
 *              returning an int: 
 *              "ssdi<i" 
 * ...          Function arguments.
 *
 * Return Value
 * INA_SUCCEES if function called without any error.
 */
INA_API(ina_rc_t) ina_ljit_call(ina_ljit_ctx_t *ctx, const char* fname, const char *signatur, ...);

/* 
 * Load lua code an execute it.
 */
INA_API(ina_rc_t) ina_ljit_dostring(ina_ljit_ctx_t *ctx, const char* code);

/*
 * Printout lua stack
 */
INA_API(ina_rc_t) ina_ljit_dump_stack(ina_ljit_ctx_t *ctx);

/*
 * Check C data type.
 */
INA_API(const void*) ina_ljit_checkcdata(ina_ljit_ctx_t *ctx, int narg);

/*
 * 
 */
INA_API(unsigned long) ina_ljit_hash_sbdm(const char *str);

/*
 *
 */
INA_API(void) ina_ljit_dbl_to_decimal(double dbl, ina_decimal_t *dec);
    
/*
 *
 */
INA_API(double) ina_ljit_dbl_from_decimal(const ina_decimal_t *dec);

#ifdef __cplusplus
}
#endif 

#endif