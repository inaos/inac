/*
 * Copyright 2013-2020 INAOS GmbH, Thalwil
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
#ifndef _LIBINAC_LJIT_H_
#define _LIBINAC_LJIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>

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
    INA_LJIT_EXTERN const char *luaJIT_BC_##module;                     \
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
 * Initialize LuaJIT context
 * 
 * Parameters
 *  ctx  Pointer to LuaJIT state pointer
 *
 * Return
 *  INA_SUCCESS if no error occurred.
 */
INA_API(ina_rc_t) ina_ljit_ctx_new(ina_ljit_ctx_t **ctx);

/*
 * Destroy LuaJIT context.
 *
 * Parameters
 *   ctx  Pointer to LuaJIT state pointer to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(void) ina_ljit_ctx_free(ina_ljit_ctx_t **ctx);
     
/*
 * Call a Lua function.
 *
 * Parameter
 * ctx        LuaJIT state
 * fname      Function name. For methods use the object.method() syntax.
 * signature  Function signature. Each parameter is represented by a char
 *            indicating the type.
 *            s = string
 *            d = double
 *            i = int
 *            c = cdata (const void pointer)
 *            Return values are defined by the same way. After after the
 *            < char.
 *            Example of signature taking 2 strings, 1 double, 1 int and
 *            returning an int:
 *            "ssdi<i"
 * ...        Function arguments.
 *
 * Return
 *  INA_SUCCESS if function called without any error.
 */
INA_API(ina_rc_t) ina_ljit_call(ina_ljit_ctx_t *ctx,
                                const char* fname,
                                const char *signature,
                                ...);

/*
 * Return last error message.
 *
 * Parameters
 *  ctx  LuaJIT context
 *
 * Return
 *  Error message of last occurred error.
 */
INA_API(const char*) ina_ljit_last_error(ina_ljit_ctx_t *ctx);

/* 
 * Load lua code an execute it.
 *
 * Parameters
 *  cxt   LuaJIT state
 *  code  Lua code to execute
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_ljit_dostring(ina_ljit_ctx_t *ctx, const char* code);

/*
 * Printout LUA stack to the stdout.
 *
 * Parameters
 *  ctx  LuaJIT context
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_ljit_dump_stack(ina_ljit_ctx_t *ctx);

/*
 * Check C data type.
 *
 * Parameters
 *  ctx   LuaJIT context
 *  narg  Stack index
 *
 * Return
 *  Return underling C pointer for CDATA type
 */
INA_API(const void*) ina_ljit_checkcdata(ina_ljit_ctx_t *ctx, int narg);

/*
 * Calculate SBDM hash
 *
 * Parameters
 *  str   The string to be calculated
 *
 * Return
 *  Calculated hash
 */
INA_API(unsigned long) ina_ljit_hash_sbdm(const char *str);



#ifdef __cplusplus
}
#endif 

#endif