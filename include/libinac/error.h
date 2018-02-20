/*
 * Copyright (c) 2012-2018, INAOS GmbH
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
#ifndef _LIBINAC_ERROR_H_
#define _LIBINAC_ERROR_H_

#include <time.h>
#include <errno.h>

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Return code */
typedef int64_t ina_rc_t;


/* Indicate no errors */
#define INA_SUCCESS  (0)
/* Check return code: successful or handled */
#define INA_SUCCEED(rc) (rc >= 0)
/* Check return code: failure */
#define INA_FAILED(rc) (rc < 0)

#ifdef INA_VERIFY_ENABLED
#define INA_VERIFY_NOT_NULL(x) INA_VERIFY(x != NULL)
#define INA_VERIFY(x) if (INA_UNLIKELY((x))) return INA_ERROR(INA_NN_ARGUMENT|INA_ERR_INVALID)
#else
#define INA_VERIFY_NOT_NULL(x) INA_ASSERT_NOTNULL((x))
#define INA_VERIFY(x) INA_ASSERT_TRUE((x))
#endif

#define INA_RETURN_IF(x) if ((x)) return ina_err_get_last_rc()
#define INA_RETURN_IF_FAILED(x) if (INA_FAILED((x))) return ina_err_get_last_rc()
#define INA_RETURN_IF_SUCCEED(x) if (INA_SUCCEED((x))) return ina_err_get_last_rc()

/* Checkpoint must succeed */
#define INA_MUST_SUCCEED(rc) if (INA_UNLIKELY(INA_FAILED(rc))) abort()
/* Set last RC */
#define INA_ERROR(x) ina_err_set_last_rc(INA_RC_PACK((x), 0LL), __FILE__ ":" INA_NUM2STR(__LINE__))
/* Set last RC and capture errno */
#define INA_OS_ERROR(x) ina_err_set_last_rc(INA_RC_PACK((x), errno),  __FILE__ ":" INA_NUM2STR(__LINE__))
/* Set last RC and ser user defined errno */
#define INA_USR_ERROR(x,e) ina_err_set_last_rc(INA_RC_PACK((x), (e)),  __FILE__ ":" INA_NUM2STR(__LINE__))


/* Pack a RC */
#define INA_RC_PACK(x, e) (INA_ERR_ERROR | (((ina_rc_t)(e)) << INA_RC_BIT_L) | (x))

/* Extract bits from error code */
#define INA_RC_E(rc)   ( (int32_t)((rc >> INA_RC_BIT_E) & 0x1) )
#define INA_RC_V(rc)   ( (int32_t)((rc >> INA_RC_BIT_V) & 0x7f) )
#define INA_RC_R(rc)   ( (int32_t)((rc >> INA_RC_BIT_R) & 0xffff) )
#define INA_RC_L(rc)   ( (int32_t)((rc >> INA_RC_BIT_L) & 0xffff) )
#define INA_RC_N(rc)   ( (int32_t)((rc >> INA_RC_BIT_N) & 0x1) )
#define INA_RC_A(rc)   ( (int32_t)((rc >> INA_RC_BIT_A) & 0xff) )
#define INA_RC_U(rc)   ( (int32_t)((rc >> INA_RC_BIT_U) & 0x7fff) )

/* Bit-shifts */
#define INA_RC_BIT_E                63
#define INA_RC_BIT_V                56
#define INA_RC_BIT_R                40
#define INA_RC_BIT_L                24
#define INA_RC_BIT_N                23
#define INA_RC_BIT_A                15
#define INA_RC_BIT_U                00

/* Flags */
#define INA_ERR_ERROR               (  1LL << INA_RC_BIT_E) /*Error-bit*/
#define INA_ERR_NOT                 (  1LL << INA_RC_BIT_N) /*Negate-bit*/
/* Error attributes */
#define INA_ERR_A                   (  1LL << INA_RC_BIT_A)
#define INA_ERR_ACK                 (  2LL << INA_RC_BIT_A)
#define INA_ERR_ACTIVE              (  3LL << INA_RC_BIT_A)
#define INA_ERR_ALIGNED             (  4LL << INA_RC_BIT_A)
#define INA_ERR_ALLOWED             (  5LL << INA_RC_BIT_A)
#define INA_ERR_ASSIGNED            (  6LL << INA_RC_BIT_A)
#define INA_ERR_ATTACHED            (  7LL << INA_RC_BIT_A)
#define INA_ERR_ATTEMPTED           (  8LL << INA_RC_BIT_A)
#define INA_ERR_AUTHORIZED          (  9LL << INA_RC_BIT_A)
#define INA_ERR_AVAILABLE           ( 10LL << INA_RC_BIT_A)
#define INA_ERR_BAD                 ( 11LL << INA_RC_BIT_A)
#define INA_ERR_BLOCKED             ( 12LL << INA_RC_BIT_A)
#define INA_ERR_BROKEN              ( 13LL << INA_RC_BIT_A)
#define INA_ERR_BUILT               ( 14LL << INA_RC_BIT_A)
#define INA_ERR_BUSY                ( 15LL << INA_RC_BIT_A)
#define INA_ERR_CLOSED              ( 16LL << INA_RC_BIT_A)
#define INA_ERR_COLLIDED            ( 17LL << INA_RC_BIT_A)
#define INA_ERR_COMPILED            ( 18LL << INA_RC_BIT_A)
#define INA_ERR_COMPLETE            ( 19LL << INA_RC_BIT_A)
#define INA_ERR_CONFLICTED          ( 20LL << INA_RC_BIT_A)
#define INA_ERR_CONNECTED           ( 21LL << INA_RC_BIT_A)
#define INA_ERR_CONSTRUCTED         ( 22LL << INA_RC_BIT_A)
#define INA_ERR_CREATED             ( 23LL << INA_RC_BIT_A)
#define INA_ERR_DEFINED             ( 24LL << INA_RC_BIT_A)
#define INA_ERR_DENIED              ( 25LL << INA_RC_BIT_A)
#define INA_ERR_DEPARTED            ( 26LL << INA_RC_BIT_A)
#define INA_ERR_DESTRUCTED          ( 27LL << INA_RC_BIT_A)
#define INA_ERR_DETACHED            ( 28LL << INA_RC_BIT_A)
#define INA_ERR_DETECTED            ( 29LL << INA_RC_BIT_A)
#define INA_ERR_DISABLED            ( 30LL << INA_RC_BIT_A)
#define INA_ERR_DOWN                ( 31LL << INA_RC_BIT_A)
#define INA_ERR_DOWNLOADED          ( 32LL << INA_RC_BIT_A)
#define INA_ERR_EMPTY               ( 33LL << INA_RC_BIT_A)
#define INA_ERR_ENABLED             ( 34LL << INA_RC_BIT_A)
#define INA_ERR_ENHANCED            ( 35LL << INA_RC_BIT_A)
#define INA_ERR_ENOUGH              ( 36LL << INA_RC_BIT_A)
#define INA_ERR_EXCEEDED            ( 37LL << INA_RC_BIT_A)
#define INA_ERR_EXCHANGED           ( 38LL << INA_RC_BIT_A)
#define INA_ERR_EXECUTABLE          ( 39LL << INA_RC_BIT_A)
#define INA_ERR_EXISTS              ( 40LL << INA_RC_BIT_A)
#define INA_ERR_EXPIRED             ( 41LL << INA_RC_BIT_A)
#define INA_ERR_EXTENDED            ( 42LL << INA_RC_BIT_A)
#define INA_ERR_FAILED              ( 43LL << INA_RC_BIT_A)
#define INA_ERR_FALSE               ( 44LL << INA_RC_BIT_A)
#define INA_ERR_FATAL               ( 45LL << INA_RC_BIT_A)
#define INA_ERR_FORBIDDEN           ( 46LL << INA_RC_BIT_A)
#define INA_ERR_FORMATTED           ( 47LL << INA_RC_BIT_A)
#define INA_ERR_FOUND               ( 48LL << INA_RC_BIT_A)
#define INA_ERR_FULL                ( 49LL << INA_RC_BIT_A)
#define INA_ERR_GONE                ( 50LL << INA_RC_BIT_A)
#define INA_ERR_GOOD                ( 51LL << INA_RC_BIT_A)
#define INA_ERR_HALTED              ( 52LL << INA_RC_BIT_A)
#define INA_ERR_HIDDEN              ( 53LL << INA_RC_BIT_A)
#define INA_ERR_HOLD                ( 54LL << INA_RC_BIT_A)
#define INA_ERR_IDLE                ( 55LL << INA_RC_BIT_A)
#define INA_ERR_ILLEGAL             ( 56LL << INA_RC_BIT_A)
#define INA_ERR_IMPLEMENTED         ( 57LL << INA_RC_BIT_A)
#define INA_ERR_IN_PROGRESS         ( 58LL << INA_RC_BIT_A)
#define INA_ERR_IN_USE              ( 59LL << INA_RC_BIT_A)
#define INA_ERR_INITIALIZED         ( 60LL << INA_RC_BIT_A)
#define INA_ERR_INSERTED            ( 61LL << INA_RC_BIT_A)
#define INA_ERR_INSTALLED           ( 62LL << INA_RC_BIT_A)
#define INA_ERR_INTERRUPTED         ( 63LL << INA_RC_BIT_A)
#define INA_ERR_JOINED              ( 64LL << INA_RC_BIT_A)
#define INA_ERR_KNOWN               ( 65LL << INA_RC_BIT_A)
#define INA_ERR_LINKED              ( 66LL << INA_RC_BIT_A)
#define INA_ERR_LOADED              ( 67LL << INA_RC_BIT_A)
#define INA_ERR_LOCAL               ( 68LL << INA_RC_BIT_A)
#define INA_ERR_LOCKED              ( 69LL << INA_RC_BIT_A)
#define INA_ERR_LOOPED              ( 70LL << INA_RC_BIT_A)
#define INA_ERR_LOST                ( 71LL << INA_RC_BIT_A)
#define INA_ERR_MERGED              ( 72LL << INA_RC_BIT_A)
#define INA_ERR_MISSING             ( 73LL << INA_RC_BIT_A)
#define INA_ERR_MOUNTED             ( 74LL << INA_RC_BIT_A)
#define INA_ERR_NEEDED              ( 75LL << INA_RC_BIT_A)
#define INA_ERR_NO                  ( 76LL << INA_RC_BIT_A)
#define INA_ERR_NO_SUCH             ( 77LL << INA_RC_BIT_A)
#define INA_ERR_OFF                 ( 78LL << INA_RC_BIT_A)
#define INA_ERR_ON                  ( 79LL << INA_RC_BIT_A)
#define INA_ERR_ONLINE              ( 80LL << INA_RC_BIT_A)
#define INA_ERR_OPEN                ( 81LL << INA_RC_BIT_A)
#define INA_ERR_ORDERED             ( 82LL << INA_RC_BIT_A)
#define INA_ERR_OUT_OF              ( 83LL << INA_RC_BIT_A)
#define INA_ERR_OUT_OF_RANGE        ( 84LL << INA_RC_BIT_A)
#define INA_ERR_OVERFLOW            ( 85LL << INA_RC_BIT_A)
#define INA_ERR_PADDED              ( 86LL << INA_RC_BIT_A)
#define INA_ERR_PARTED              ( 87LL << INA_RC_BIT_A)
#define INA_ERR_PERMITTED           ( 88LL << INA_RC_BIT_A)
#define INA_ERR_POPPED              ( 89LL << INA_RC_BIT_A)
#define INA_ERR_PRELOADED           ( 90LL << INA_RC_BIT_A)
#define INA_ERR_PROCESSABLE         ( 91LL << INA_RC_BIT_A)
#define INA_ERR_PROVIDED            ( 92LL << INA_RC_BIT_A)
#define INA_ERR_PUSHED              ( 93LL << INA_RC_BIT_A)
#define INA_ERR_REACHABLE           ( 94LL << INA_RC_BIT_A)
#define INA_ERR_READABLE            ( 95LL << INA_RC_BIT_A)
#define INA_ERR_RECEIVED            ( 96LL << INA_RC_BIT_A)
#define INA_ERR_REFUSED             ( 97LL << INA_RC_BIT_A)
#define INA_ERR_REGISTERED          ( 98LL << INA_RC_BIT_A)
#define INA_ERR_REJECTED            ( 99LL << INA_RC_BIT_A)
#define INA_ERR_RELEASED            (100LL << INA_RC_BIT_A)
#define INA_ERR_REMOTE              (101LL << INA_RC_BIT_A)
#define INA_ERR_REMOVED             (102LL << INA_RC_BIT_A)
#define INA_ERR_RENDERABLE          (103LL << INA_RC_BIT_A)
#define INA_ERR_RESERVED            (104LL << INA_RC_BIT_A)
#define INA_ERR_RESET               (105LL << INA_RC_BIT_A)
#define INA_ERR_RESPONDING          (106LL << INA_RC_BIT_A)
#define INA_ERR_RETRIED             (107LL << INA_RC_BIT_A)
#define INA_ERR_RIGHT               (108LL << INA_RC_BIT_A)
#define INA_ERR_RUNNING             (109LL << INA_RC_BIT_A)
#define INA_ERR_SENT                (110LL << INA_RC_BIT_A)
#define INA_ERR_SHARED              (111LL << INA_RC_BIT_A)
#define INA_ERR_SORTED              (112LL << INA_RC_BIT_A)
#define INA_ERR_SPECIFIED           (113LL << INA_RC_BIT_A)
#define INA_ERR_SPLITTED            (114LL << INA_RC_BIT_A)
#define INA_ERR_STALLED             (115LL << INA_RC_BIT_A)
#define INA_ERR_STOPPED             (116LL << INA_RC_BIT_A)
#define INA_ERR_SUCEEDED            (117LL << INA_RC_BIT_A)
#define INA_ERR_SUITABLE            (118LL << INA_RC_BIT_A)
#define INA_ERR_SUPPORTED           (119LL << INA_RC_BIT_A)
#define INA_ERR_SYNCHRONIZED        (120LL << INA_RC_BIT_A)
#define INA_ERR_TERMINATED          (121LL << INA_RC_BIT_A)
#define INA_ERR_THROWN              (122LL << INA_RC_BIT_A)
#define INA_ERR_TIMED_OUT           (123LL << INA_RC_BIT_A)
#define INA_ERR_TOO_COMPLEX         (124LL << INA_RC_BIT_A)
#define INA_ERR_TOO_FEW             (125LL << INA_RC_BIT_A)
#define INA_ERR_TOO_LARGE           (126LL << INA_RC_BIT_A)
#define INA_ERR_TOO_LONG            (127LL << INA_RC_BIT_A)
#define INA_ERR_TOO_MANY            (128LL << INA_RC_BIT_A)
#define INA_ERR_TOO_MUCH            (129LL << INA_RC_BIT_A)
#define INA_ERR_TOO_SIMPLE          (130LL << INA_RC_BIT_A)
#define INA_ERR_TOO_SMALL           (131LL << INA_RC_BIT_A)
#define INA_ERR_TRIGGERED           (132LL << INA_RC_BIT_A)
#define INA_ERR_TRUE                (133LL << INA_RC_BIT_A)
#define INA_ERR_UNBLOCKED           (134LL << INA_RC_BIT_A)
#define INA_ERR_UNDERFLOW           (135LL << INA_RC_BIT_A)
#define INA_ERR_UNINITIALIZED       (136LL << INA_RC_BIT_A)
#define INA_ERR_UNINSTALLED         (137LL << INA_RC_BIT_A)
#define INA_ERR_UNIQUE              (138LL << INA_RC_BIT_A)
#define INA_ERR_UNLOADED            (139LL << INA_RC_BIT_A)
#define INA_ERR_UNLOCKED            (140LL << INA_RC_BIT_A)
#define INA_ERR_UNSORTED            (141LL << INA_RC_BIT_A)
#define INA_ERR_UP                  (142LL << INA_RC_BIT_A)
#define INA_ERR_UPDATED             (143LL << INA_RC_BIT_A)
#define INA_ERR_UPGRADED            (144LL << INA_RC_BIT_A)
#define INA_ERR_UPLOADED            (145LL << INA_RC_BIT_A)
#define INA_ERR_USED                (146LL << INA_RC_BIT_A)
#define INA_ERR_VALID               (147LL << INA_RC_BIT_A)
#define INA_ERR_VISIBLE             (148LL << INA_RC_BIT_A)
#define INA_ERR_WORKING             (149LL << INA_RC_BIT_A)
#define INA_ERR_WRITABLE            (150LL << INA_RC_BIT_A)
#define INA_ERR_WRONG               (151LL << INA_RC_BIT_A)
#define INA_ERR_END_OF              (152LL << INA_RC_BIT_A)
#define INA_ERR_RESOLVED            (153LL << INA_RC_BIT_A)
#define INA_ERR_MATCH               (154LL << INA_RC_BIT_A)
#define INA_ERR_TRY_AGAIN           (155LL << INA_RC_BIT_A)
#define INA_ERR_PARSED              (156LL << INA_RC_BIT_A)

/* Error attributes (negate forms) */
#define INA_ERR_NOT_A               (INA_ERR_NOT | INA_ERR_A )
#define INA_ERR_NOT_ACK             (INA_ERR_NOT | INA_ERR_ACK )
#define INA_ERR_NOT_ACTIVE          (INA_ERR_NOT | INA_ERR_ACTIVE )
#define INA_ERR_NOT_ALIGNED         (INA_ERR_NOT | INA_ERR_ALIGNED )
#define INA_ERR_NOT_ALLOWED         (INA_ERR_NOT | INA_ERR_ALLOWED )
#define INA_ERR_NOT_ASSIGNED        (INA_ERR_NOT | INA_ERR_ASSIGNED )
#define INA_ERR_NOT_ATTACHED        (INA_ERR_NOT | INA_ERR_ATTACHED )
#define INA_ERR_NOT_ATTEMPTED       (INA_ERR_NOT | INA_ERR_ATTEMPTED )
#define INA_ERR_NOT_AUTHORIZED      (INA_ERR_NOT | INA_ERR_AUTHORIZED )
#define INA_ERR_NOT_AVAILABLE       (INA_ERR_NOT | INA_ERR_AVAILABLE )
#define INA_ERR_NOT_BAD             (INA_ERR_NOT | INA_ERR_BAD )
#define INA_ERR_NOT_BLOCKED         (INA_ERR_NOT | INA_ERR_BLOCKED )
#define INA_ERR_NOT_BROKEN          (INA_ERR_NOT | INA_ERR_BROKEN )
#define INA_ERR_NOT_BUILT           (INA_ERR_NOT | INA_ERR_BUILT )
#define INA_ERR_NOT_BUSY            (INA_ERR_NOT | INA_ERR_BUSY )
#define INA_ERR_NOT_CLOSED          (INA_ERR_NOT | INA_ERR_CLOSED )
#define INA_ERR_NOT_COLLIDED        (INA_ERR_NOT | INA_ERR_COLLIDED )
#define INA_ERR_NOT_COMPILED        (INA_ERR_NOT | INA_ERR_COMPILED )
#define INA_ERR_NOT_COMPLETE        (INA_ERR_NOT | INA_ERR_COMPLETE )
#define INA_ERR_NOT_CONFLICTED      (INA_ERR_NOT | INA_ERR_CONFLICTED )
#define INA_ERR_NOT_CONNECTED       (INA_ERR_NOT | INA_ERR_CONNECTED )
#define INA_ERR_NOT_CONSTRUCTED     (INA_ERR_NOT | INA_ERR_CONSTRUCTED )
#define INA_ERR_NOT_CREATED         (INA_ERR_NOT | INA_ERR_CREATED )
#define INA_ERR_NOT_DEFINED         (INA_ERR_NOT | INA_ERR_DEFINED )
#define INA_ERR_NOT_DENIED          (INA_ERR_NOT | INA_ERR_DENIED )
#define INA_ERR_NOT_DEPARTED        (INA_ERR_NOT | INA_ERR_DEPARTED )
#define INA_ERR_NOT_DESTRUCTED      (INA_ERR_NOT | INA_ERR_DESTRUCTED )
#define INA_ERR_NOT_DETACHED        (INA_ERR_NOT | INA_ERR_DETACHED )
#define INA_ERR_NOT_DETECTED        (INA_ERR_NOT | INA_ERR_DETECTED )
#define INA_ERR_NOT_DISABLED        (INA_ERR_NOT | INA_ERR_DISABLED )
#define INA_ERR_NOT_DOWN            (INA_ERR_NOT | INA_ERR_DOWN )
#define INA_ERR_NOT_DOWNLOADED      (INA_ERR_NOT | INA_ERR_DOWNLOADED )
#define INA_ERR_NOT_EMPTY           (INA_ERR_NOT | INA_ERR_EMPTY )
#define INA_ERR_NOT_ENABLED         (INA_ERR_NOT | INA_ERR_ENABLED )
#define INA_ERR_NOT_ENHANCED        (INA_ERR_NOT | INA_ERR_ENHANCED )
#define INA_ERR_NOT_ENOUGH          (INA_ERR_NOT | INA_ERR_ENOUGH )
#define INA_ERR_NOT_EXCEEDED        (INA_ERR_NOT | INA_ERR_EXCEEDED )
#define INA_ERR_NOT_EXCHANGED       (INA_ERR_NOT | INA_ERR_EXCHANGED )
#define INA_ERR_NOT_EXECUTABLE      (INA_ERR_NOT | INA_ERR_EXECUTABLE )
#define INA_ERR_NOT_EXISTS          (INA_ERR_NOT | INA_ERR_EXISTS )
#define INA_ERR_NOT_EXPIRED         (INA_ERR_NOT | INA_ERR_EXPIRED )
#define INA_ERR_NOT_EXTENDED        (INA_ERR_NOT | INA_ERR_EXTENDED )
#define INA_ERR_NOT_FAILED          (INA_ERR_NOT | INA_ERR_FAILED )
#define INA_ERR_NOT_FALSE           (INA_ERR_NOT | INA_ERR_FALSE )
#define INA_ERR_NOT_FATAL           (INA_ERR_NOT | INA_ERR_FATAL )
#define INA_ERR_NOT_FORBIDDEN       (INA_ERR_NOT | INA_ERR_FORBIDDEN )
#define INA_ERR_NOT_FORMATTED       (INA_ERR_NOT | INA_ERR_FORMATTED )
#define INA_ERR_NOT_FOUND           (INA_ERR_NOT | INA_ERR_FOUND )
#define INA_ERR_NOT_FULL            (INA_ERR_NOT | INA_ERR_FULL )
#define INA_ERR_NOT_GONE            (INA_ERR_NOT | INA_ERR_GONE )
#define INA_ERR_NOT_GOOD            (INA_ERR_NOT | INA_ERR_GOOD )
#define INA_ERR_NOT_HALTED          (INA_ERR_NOT | INA_ERR_HALTED )
#define INA_ERR_NOT_HIDDEN          (INA_ERR_NOT | INA_ERR_HIDDEN )
#define INA_ERR_NOT_HOLD            (INA_ERR_NOT | INA_ERR_HOLD )
#define INA_ERR_NOT_IDLE            (INA_ERR_NOT | INA_ERR_IDLE )
#define INA_ERR_NOT_ILLEGAL         (INA_ERR_NOT | INA_ERR_ILLEGAL )
#define INA_ERR_NOT_IMPLEMENTED     (INA_ERR_NOT | INA_ERR_IMPLEMENTED )
#define INA_ERR_NOT_IN_PROGRESS     (INA_ERR_NOT | INA_ERR_IN_PROGRESS )
#define INA_ERR_NOT_IN_USE          (INA_ERR_NOT | INA_ERR_IN_USE )
#define INA_ERR_NOT_INITIALIZED     (INA_ERR_NOT | INA_ERR_INITIALIZED )
#define INA_ERR_NOT_INSERTED        (INA_ERR_NOT | INA_ERR_INSERTED )
#define INA_ERR_NOT_INSTALLED       (INA_ERR_NOT | INA_ERR_INSTALLED )
#define INA_ERR_NOT_INTERRUPTED     (INA_ERR_NOT | INA_ERR_INTERRUPTED )
#define INA_ERR_NOT_JOINED          (INA_ERR_NOT | INA_ERR_JOINED )
#define INA_ERR_NOT_KNOWN           (INA_ERR_NOT | INA_ERR_KNOWN )
#define INA_ERR_NOT_LINKED          (INA_ERR_NOT | INA_ERR_LINKED )
#define INA_ERR_NOT_LOADED          (INA_ERR_NOT | INA_ERR_LOADED )
#define INA_ERR_NOT_LOCAL           (INA_ERR_NOT | INA_ERR_LOCAL )
#define INA_ERR_NOT_LOCKED          (INA_ERR_NOT | INA_ERR_LOCKED )
#define INA_ERR_NOT_LOOPED          (INA_ERR_NOT | INA_ERR_LOOPED )
#define INA_ERR_NOT_LOST            (INA_ERR_NOT | INA_ERR_LOST )
#define INA_ERR_NOT_MERGED          (INA_ERR_NOT | INA_ERR_MERGED )
#define INA_ERR_NOT_MISSING         (INA_ERR_NOT | INA_ERR_MISSING )
#define INA_ERR_NOT_MOUNTED         (INA_ERR_NOT | INA_ERR_MOUNTED )
#define INA_ERR_NOT_NEEDED          (INA_ERR_NOT | INA_ERR_NEEDED )
#define INA_ERR_NOT_NO              (INA_ERR_NOT | INA_ERR_NO )
#define INA_ERR_NOT_NO_SUCH         (INA_ERR_NOT | INA_ERR_NO_SUCH )
#define INA_ERR_NOT_OFF             (INA_ERR_NOT | INA_ERR_OFF )
#define INA_ERR_NOT_ON              (INA_ERR_NOT | INA_ERR_ON )
#define INA_ERR_NOT_ONLINE          (INA_ERR_NOT | INA_ERR_ONLINE )
#define INA_ERR_NOT_OPEN            (INA_ERR_NOT | INA_ERR_OPEN )
#define INA_ERR_NOT_ORDERED         (INA_ERR_NOT | INA_ERR_ORDERED )
#define INA_ERR_NOT_OUT_OF          (INA_ERR_NOT | INA_ERR_OUT_OF )
#define INA_ERR_NOT_OUT_OF_RANGE    (INA_ERR_NOT | INA_ERR_OUT_OF_RANGE )
#define INA_ERR_NOT_OVERFLOW        (INA_ERR_NOT | INA_ERR_OVERFLOW )
#define INA_ERR_NOT_PADDED          (INA_ERR_NOT | INA_ERR_PADDED )
#define INA_ERR_NOT_PARTED          (INA_ERR_NOT | INA_ERR_PARTED )
#define INA_ERR_NOT_PERMITTED       (INA_ERR_NOT | INA_ERR_PERMITTED )
#define INA_ERR_NOT_POPPED          (INA_ERR_NOT | INA_ERR_POPPED )
#define INA_ERR_NOT_PRELOADED       (INA_ERR_NOT | INA_ERR_PRELOADED )
#define INA_ERR_NOT_PROCESSABLE     (INA_ERR_NOT | INA_ERR_PROCESSABLE )
#define INA_ERR_NOT_PROVIDED        (INA_ERR_NOT | INA_ERR_PROVIDED )
#define INA_ERR_NOT_PUSHED          (INA_ERR_NOT | INA_ERR_PUSHED )
#define INA_ERR_NOT_REACHABLE       (INA_ERR_NOT | INA_ERR_REACHABLE )
#define INA_ERR_NOT_READABLE        (INA_ERR_NOT | INA_ERR_READABLE )
#define INA_ERR_NOT_RECEIVED        (INA_ERR_NOT | INA_ERR_RECEIVED )
#define INA_ERR_NOT_REFUSED         (INA_ERR_NOT | INA_ERR_REFUSED )
#define INA_ERR_NOT_REGISTERED      (INA_ERR_NOT | INA_ERR_REGISTERED )
#define INA_ERR_NOT_REJECTED        (INA_ERR_NOT | INA_ERR_REJECTED )
#define INA_ERR_NOT_RELEASED        (INA_ERR_NOT | INA_ERR_RELEASED )
#define INA_ERR_NOT_REMOTE          (INA_ERR_NOT | INA_ERR_REMOTE )
#define INA_ERR_NOT_REMOVED         (INA_ERR_NOT | INA_ERR_REMOVED )
#define INA_ERR_NOT_RENDERABLE      (INA_ERR_NOT | INA_ERR_RENDERABLE )
#define INA_ERR_NOT_RESERVED        (INA_ERR_NOT | INA_ERR_RESERVED )
#define INA_ERR_NOT_RESET           (INA_ERR_NOT | INA_ERR_RESET )
#define INA_ERR_NOT_RESPONDING      (INA_ERR_NOT | INA_ERR_RESPONDING )
#define INA_ERR_NOT_RETRIED         (INA_ERR_NOT | INA_ERR_RETRIED )
#define INA_ERR_NOT_RIGHT           (INA_ERR_NOT | INA_ERR_RIGHT )
#define INA_ERR_NOT_RUNNING         (INA_ERR_NOT | INA_ERR_RUNNING )
#define INA_ERR_NOT_SENT            (INA_ERR_NOT | INA_ERR_SENT )
#define INA_ERR_NOT_SHARED          (INA_ERR_NOT | INA_ERR_SHARED )
#define INA_ERR_NOT_SORTED          (INA_ERR_NOT | INA_ERR_SORTED )
#define INA_ERR_NOT_SPECIFIED       (INA_ERR_NOT | INA_ERR_SPECIFIED )
#define INA_ERR_NOT_SPLITTED        (INA_ERR_NOT | INA_ERR_SPLITTED )
#define INA_ERR_NOT_STALLED         (INA_ERR_NOT | INA_ERR_STALLED )
#define INA_ERR_NOT_STOPPED         (INA_ERR_NOT | INA_ERR_STOPPED )
#define INA_ERR_NOT_SUCEEDED        (INA_ERR_NOT | INA_ERR_SUCEEDED )
#define INA_ERR_NOT_SUITABLE        (INA_ERR_NOT | INA_ERR_SUITABLE )
#define INA_ERR_NOT_SUPPORTED       (INA_ERR_NOT | INA_ERR_SUPPORTED )
#define INA_ERR_NOT_SYNCHRONIZED    (INA_ERR_NOT | INA_ERR_SYNCHRONIZED )
#define INA_ERR_NOT_TERMINATED      (INA_ERR_NOT | INA_ERR_TERMINATED )
#define INA_ERR_NOT_THROWN          (INA_ERR_NOT | INA_ERR_THROWN )
#define INA_ERR_NOT_TIMED_OUT       (INA_ERR_NOT | INA_ERR_TIMED_OUT )
#define INA_ERR_NOT_TOO_COMPLEX     (INA_ERR_NOT | INA_ERR_TOO_COMPLEX )
#define INA_ERR_NOT_TOO_FEW         (INA_ERR_NOT | INA_ERR_TOO_FEW )
#define INA_ERR_NOT_TOO_LARGE       (INA_ERR_NOT | INA_ERR_TOO_LARGE )
#define INA_ERR_NOT_TOO_LONG        (INA_ERR_NOT | INA_ERR_TOO_LONG )
#define INA_ERR_NOT_TOO_MANY        (INA_ERR_NOT | INA_ERR_TOO_MANY )
#define INA_ERR_NOT_TOO_MUCH        (INA_ERR_NOT | INA_ERR_TOO_MUCH )
#define INA_ERR_NOT_TOO_SIMPLE      (INA_ERR_NOT | INA_ERR_TOO_SIMPLE )
#define INA_ERR_NOT_TOO_SMALL       (INA_ERR_NOT | INA_ERR_TOO_SMALL )
#define INA_ERR_NOT_TRIGGERED       (INA_ERR_NOT | INA_ERR_TRIGGERED )
#define INA_ERR_NOT_TRUE            (INA_ERR_NOT | INA_ERR_TRUE )
#define INA_ERR_NOT_UNBLOCKED       (INA_ERR_NOT | INA_ERR_UNBLOCKED )
#define INA_ERR_NOT_UNDERFLOW       (INA_ERR_NOT | INA_ERR_UNDERFLOW )
#define INA_ERR_NOT_UNINITIALIZED   (INA_ERR_NOT | INA_ERR_UNINITIALIZED )
#define INA_ERR_NOT_UNINSTALLED     (INA_ERR_NOT | INA_ERR_UNINSTALLED )
#define INA_ERR_NOT_UNIQUE          (INA_ERR_NOT | INA_ERR_UNIQUE )
#define INA_ERR_NOT_UNLOADED        (INA_ERR_NOT | INA_ERR_UNLOADED )
#define INA_ERR_NOT_UNLOCKED        (INA_ERR_NOT | INA_ERR_UNLOCKED )
#define INA_ERR_NOT_UNSORTED        (INA_ERR_NOT | INA_ERR_UNSORTED )
#define INA_ERR_NOT_UP              (INA_ERR_NOT | INA_ERR_UP )
#define INA_ERR_NOT_UPDATED         (INA_ERR_NOT | INA_ERR_UPDATED )
#define INA_ERR_NOT_UPGRADED        (INA_ERR_NOT | INA_ERR_UPGRADED )
#define INA_ERR_NOT_UPLOADED        (INA_ERR_NOT | INA_ERR_UPLOADED )
#define INA_ERR_NOT_USED            (INA_ERR_NOT | INA_ERR_USED )
#define INA_ERR_NOT_VALID           (INA_ERR_NOT | INA_ERR_VALID )
#define INA_ERR_NOT_VISIBLE         (INA_ERR_NOT | INA_ERR_VISIBLE )
#define INA_ERR_NOT_WORKING         (INA_ERR_NOT | INA_ERR_WORKING )
#define INA_ERR_NOT_WRITABLE        (INA_ERR_NOT | INA_ERR_WRITABLE )
#define INA_ERR_NOT_WRONG           (INA_ERR_NOT | INA_ERR_WRONG )
#define INA_ERR_NOT_END_OF          (INA_ERR_NOT | INA_ERR_END_OF)
#define INA_ERR_NOT_RESOLVED        (INA_ERR_NOT | INA_ERR_RESOLVED)
#define INA_ERR_NOT_MATCH           (INA_ERR_NOT | INA_ERR_MATCH)
#define INA_ERR_NOT_TRY_AGAIN       (INA_ERR_NOT | INA_ERR_TRY_AGAIN)
#define INA_ERR_NOT_PARSED          (INA_ERR_NOT | INA_ERR_PARSED)

/* Attribute aliases */
#define INA_ERR_UNDEFINED           (INA_ERR_NOT_DEFINED)
#define INA_ERR_UNUSED              (INA_ERR_NOT_USED)
#define INA_ERR_UNORDERED           (INA_ERR_NOT_ORDERED)
#define INA_ERR_INVALID             (INA_ERR_NOT_VALID)
#define INA_ERR_INACTIVE            (INA_ERR_NOT_ACTIVE)
#define INA_ERR_ERASED              (INA_ERR_REMOVED)
#define INA_ERR_DELETED             (INA_ERR_REMOVED)
#define INA_ERR_OFFLINE             (INA_ERR_NOT_ONLINE)
#define INA_ERR_UNAVAILABLE         (INA_ERR_NOT_AVAILABLE)

/* Nouns */
#define INA_NN_NONE                 (1)
#define INA_NN_ACCESS               (2)
#define INA_NN_ADMINISTRATOR        (3)
#define INA_NN_API                  (4)
#define INA_NN_APPLICATION          (5)
#define INA_NN_ARGUMENT             (6)
#define INA_NN_ARRAY                (200)
#define INA_NN_AUTHENTICATION       (7)
#define INA_NN_BINARY               (8)
#define INA_NN_BLOB                 (9)
#define INA_NN_BROADCAST            (10)
#define INA_NN_CLIENT               (11)
#define INA_NN_CLOUD                (12)
#define INA_NN_CHARSET              (13)
#define INA_NN_CODE                 (14)
#define INA_NN_COMMIT               (15)
#define INA_NN_COMPILATION          (16)
#define INA_NN_COMPILER             (17)
#define INA_NN_COMPRESSION          (18)
#define INA_NN_CONSOLE              (19)
#define INA_NN_DAEMON               (20)
#define INA_NN_DATA                 (21)
#define INA_NN_DEPENDENCY           (22)
#define INA_NN_DESCRIPTOR           (23)
#define INA_NN_DEVICE               (24)
#define INA_NN_DIRECTORY            (25)
#define INA_NN_DISK                 (26)
#define INA_NN_DLL                  (27)
#define INA_NN_DOMAIN               (28)
#define INA_NN_DOWNLOAD             (29)
#define INA_NN_DRIVER               (30)
#define INA_NN_EDITOR               (31)
#define INA_NN_ENDPOINT             (32)
#define INA_NN_ENGINE               (33)
#define INA_NN_EVALUATION           (34)
#define INA_NN_EVENT                (35)
#define INA_NN_EXCEPTION            (36)
#define INA_NN_EXPECTATION          (37)
#define INA_NN_FETCH                (38)
#define INA_NN_FILE                 (39)
#define INA_NN_FLOAT                (40)
#define INA_NN_FOLDER               (41)
#define INA_NN_FORMAT               (42)
#define INA_NN_FUNCTION             (43)
#define INA_NN_GATEWAY              (44)
#define INA_NN_GROUP                (45)
#define INA_NN_LONG                 (46)
#define INA_NN_HANDLE               (47)
#define INA_NN_HARDWARE             (48)
#define INA_NN_HEADER               (49)
#define INA_NN_HASH                 (50)
#define INA_NN_HOST                 (51)
#define INA_NN_IDENTIFIER           (52)
#define INA_NN_INDEX                (53)
#define INA_NN_INPUT                (54)
#define INA_NN_INTEGER              (55)
#define INA_NN_INTERFACE            (56)
#define INA_NN_INTERVAL             (57)
#define INA_NN_IO                   (58)
#define INA_NN_KEYBOARD             (59)
#define INA_NN_LENGTH               (60)
#define INA_NN_LEVEL                (61)
#define INA_NN_LIBRARY              (62)
#define INA_NN_LIMIT                (63)
#define INA_NN_LINK                 (64)
#define INA_NN_LINKAGE              (65)
#define INA_NN_LINKER               (66)
#define INA_NN_LOCATION             (67)
#define INA_NN_LOGIN                (68)
#define INA_NN_LOOP                 (69)
#define INA_NN_MACHINE              (70)
#define INA_NN_MEDIA                (71)
#define INA_NN_MEMORY               (72)
#define INA_NN_MESSAGE              (73)
#define INA_NN_METHOD               (74)
#define INA_NN_MODULE               (75)
#define INA_NN_MONITOR              (76)
#define INA_NN_NETWORK              (77)
#define INA_NN_NODE                 (78)
#define INA_NN_NOTHING              (79)
#define INA_NN_NUMBER               (80)
#define INA_NN_OBJECT               (81)
#define INA_NN_OPERATION            (82)
#define INA_NN_OPERATOR             (83)
#define INA_NN_PACKAGE              (84)
#define INA_NN_PACKET               (85)
#define INA_NN_PASSWORD             (86)
#define INA_NN_PATH                 (87)
#define INA_NN_PEER                 (88)
#define INA_NN_PERMISSION           (89)
#define INA_NN_PLATFORM             (90)
#define INA_NN_POOL                 (91)
#define INA_NN_POSITION             (92)
#define INA_NN_PROFILER             (93)
#define INA_NN_PROTOCOL             (94)
#define INA_NN_PROXY                (95)
#define INA_NN_QUERY                (96)
#define INA_NN_RANGE                (97)
#define INA_NN_RATIO                (98)
#define INA_NN_RECORD               (99)
#define INA_NN_REPOSITORY           (100)
#define INA_NN_REQUEST              (101)
#define INA_NN_RESOURCE             (102)
#define INA_NN_REVISION             (103)
#define INA_NN_ROUTE                (104)
#define INA_NN_RUNTIME              (105)
#define INA_NN_SCALE                (106)
#define INA_NN_SCREEN               (107)
#define INA_NN_SCRIPT               (108)
#define INA_NN_SEQUENCE             (109)
#define INA_NN_SERIALIZATION        (110)
#define INA_NN_SERVER               (111)
#define INA_NN_SERVICE              (112)
#define INA_NN_SIZE                 (113)
#define INA_NN_SOCKET               (114)
#define INA_NN_SOFTWARE             (115)
#define INA_NN_SOURCE               (116)
#define INA_NN_SPACE                (117)
#define INA_NN_STACK                (118)
#define INA_NN_STACKTRACE           (119)
#define INA_NN_STREAM               (120)
#define INA_NN_STREAMING            (121)
#define INA_NN_STRING               (122)
#define INA_NN_STRUCT               (123)
#define INA_NN_SUBSYSTEM            (124)
#define INA_NN_SYSTEM               (125)
#define INA_NN_TEXT                 (126)
#define INA_NN_TIME                 (127)
#define INA_NN_TRANSLATION          (128)
#define INA_NN_TRANSPORT            (129)
#define INA_NN_TRIGGER              (130)
#define INA_NN_TYPE                 (131)
#define INA_NN_UPGRADE              (132)
#define INA_NN_UPLOAD               (133)
#define INA_NN_USER                 (134)
#define INA_NN_USERNAME             (135)
#define INA_NN_VALUE                (146)
#define INA_NN_VECTOR               (147)
#define INA_NN_VERSION              (148)
#define INA_NN_DECOMPRESSION        (149)
#define INA_NN_STATE                (150)
#define INA_NN_DUMP                 (151)
#define INA_NN_CHAR                 (152)
#define INA_NN_CONFIGURATION        (153)
#define INA_NN_SECTION              (154)
#define INA_NN_KEY                  (155)
#define INA_NN_ENUMERATION          (156)
#define INA_NN_READ                 (157)
#define INA_NN_WRITE                (158)
#define INA_NN_OPTION               (159)
#define INA_NN_BUFFER               (160)
#define INA_NN_ADDRESS              (161)
#define INA_NN_NAME                 (162)
#define INA_NN_MAC                  (163)
#define INA_NN_PROCESS              (164)
#define INA_NN_PATTERN              (165)
#define INA_NN_MUTEX                (166)
#define INA_NN_SEMAPHORE            (167)
#define INA_NN_THREAD               (168)
#define INA_NN_CRON                 (169)

/* Error message length */
#define INA_ERR_MSGLEN  512


/*
 * Set RC
 *
 * Parameters
 *   rc         Return code
 *   location   source location
 *
 * Return
 *   INA_SUCCESS
 */
INA_API(ina_rc_t) ina_err_set_last_rc(ina_rc_t rc, const char* location);

/*
 * Return the last RC
 *
 * Return
 *  Last RC or INA_SUCCESS of no error occurred
 */
INA_API(ina_rc_t) ina_err_get_last_rc(void);

/*
 * Mark an error as handled. All errors pushed before this one are removed
 * from the state.
 *
 * Parameters
 *  rc  Valid RC to mark as handled. If a error was already maked as handled
 *      no error occurs.
 *
 * Return
 *  Returns INA_SUCCESS when the complete error state was cleared successfully
 *  otherwise returns INA_FAILURE. A marked
 */
INA_API(ina_rc_t) ina_err_clear_last_rc(void);

/*
 * Set log file.
 *
 * Parameters
 *  file_path   path to the log file
 *
 * Return
 *  INA_SUCCESS if no error occurred
 */
INA_API(ina_rc_t) ina_err_set_log_file(const char *file_path);

/*
 * Write to the error log
 *
 *  Parameters
 *   fmt  format
 *   ...  arguments
 *
 *  Return
 *   INA_SUCCESS
 */
INA_API(ina_rc_t) ina_err_log(const char *fmt, ...);
/*
 * Format the error message for a given RC.
 *
 * Parameters
 *  rc   Valid RC
 *  buf  String buffer to hold the message
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(const char*) ina_err_strerror(ina_rc_t rc, char buf[INA_ERR_MSGLEN]);

/*
 * Makes a backtrace to the stderr
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_err_backtrace(void *data);


#ifdef __cplusplus
}
#endif

#endif
