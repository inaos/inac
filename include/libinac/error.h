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
#ifndef _LIBINAC_ERROR_H_
#define _LIBINAC_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>

/* Indicate no errors */
#define INA_SUCCESS  (0ULL)

/* Global return code */
extern INA_TLS(ina_rc_t) __rc;

/* Bit-shifts */
#define INA_RC_BIT_E 63U
#define INA_RC_BIT_V 56U
#define INA_RC_BIT_R 48U
#define INA_RC_BIT_O 32U
#define INA_RC_BIT_U 24U
#define INA_RC_BIT_C 16U
#define INA_RC_BIT_N 15U
#define INA_RC_BIT_S 00U

/* Accessors */
#define INA_RC_EFLAG(rc)   ((uint32_t)(((rc) >> INA_RC_BIT_E) & 0x1UL))
#define INA_RC_VER(rc)     ((uint32_t)(((rc) >> INA_RC_BIT_V) & 0x7UL))
#define INA_RC_REV(rc)     ((uint32_t)(((rc) >> INA_RC_BIT_R) & 0xFFUL))
#define INA_RC_ERRNO(rc)   ((uint32_t)(((rc) >> INA_RC_BIT_O) & 0xFFFFUL))
#define INA_RC_UBITS(rc)   ((uint32_t)(((rc) >> INA_RC_BIT_U) & 0xFFUL))
#define INA_RC_NFLAG(rc)   ((uint32_t)(((rc) >> INA_RC_BIT_N) & 0x1UL))
#define INA_RC_CODE(rc)    ((uint32_t)((rc)&( 0xFFULL << INA_RC_BIT_C)))
#define INA_RC_ADJ(rc)     ((uint32_t)(((rc) >> INA_RC_BIT_C) & 0xFFUL))
#define INA_RC_SUBJECT(rc) ((uint32_t)(((rc) >> INA_RC_BIT_S) & 0x7FFFUL))
#define INA_RC_ERROR(rc)   ((uint32_t)((INA_MID_BITS((rc), INA_RC_BIT_U-INA_RC_BIT_N, INA_RC_BIT_N)<<(INA_RC_BIT_N-1UL))))
#define INA_RC_ERRMSG(rc)  ((uint32_t)((rc) & 0xFFFFFFUL))

/* Flags */
#define INA_ERR_ERROR               (  1ULL << INA_RC_BIT_E) /* Error-bit  */
#define INA_ERR_NOT                 (  1ULL << INA_RC_BIT_N) /* Negate-bit */

/* Errors codes */
#define INA_ERR_A                   (  1ULL << INA_RC_BIT_C)
#define INA_ERR_ACK                 (  2ULL << INA_RC_BIT_C)
#define INA_ERR_ACTIVE              (  3ULL << INA_RC_BIT_C)
#define INA_ERR_ALIGNED             (  4ULL << INA_RC_BIT_C)
#define INA_ERR_ALLOWED             (  5ULL << INA_RC_BIT_C)
#define INA_ERR_ASSIGNED            (  6ULL << INA_RC_BIT_C)
#define INA_ERR_ATTACHED            (  7ULL << INA_RC_BIT_C)
#define INA_ERR_ATTEMPTED           (  8ULL << INA_RC_BIT_C)
#define INA_ERR_AUTHORIZED          (  9ULL << INA_RC_BIT_C)
#define INA_ERR_AVAILABLE           ( 10ULL << INA_RC_BIT_C)
#define INA_ERR_BAD                 ( 11ULL << INA_RC_BIT_C)
#define INA_ERR_BLOCKED             ( 12ULL << INA_RC_BIT_C)
#define INA_ERR_BROKEN              ( 13ULL << INA_RC_BIT_C)
#define INA_ERR_BUILT               ( 14ULL << INA_RC_BIT_C)
#define INA_ERR_BUSY                ( 15ULL << INA_RC_BIT_C)
#define INA_ERR_CLOSED              ( 16ULL << INA_RC_BIT_C)
#define INA_ERR_COLLIDED            ( 17ULL << INA_RC_BIT_C)
#define INA_ERR_COMPILED            ( 18ULL << INA_RC_BIT_C)
#define INA_ERR_COMPLETE            ( 19ULL << INA_RC_BIT_C)
#define INA_ERR_CONFLICTED          ( 20ULL << INA_RC_BIT_C)
#define INA_ERR_CONNECTED           ( 21ULL << INA_RC_BIT_C)
#define INA_ERR_CONSTRUCTED         ( 22ULL << INA_RC_BIT_C)
#define INA_ERR_CREATED             ( 23ULL << INA_RC_BIT_C)
#define INA_ERR_DEFINED             ( 24ULL << INA_RC_BIT_C)
#define INA_ERR_DENIED              ( 25ULL << INA_RC_BIT_C)
#define INA_ERR_DEPARTED            ( 26ULL << INA_RC_BIT_C)
#define INA_ERR_DESTRUCTED          ( 27ULL << INA_RC_BIT_C)
#define INA_ERR_DETACHED            ( 28ULL << INA_RC_BIT_C)
#define INA_ERR_DETECTED            ( 29ULL << INA_RC_BIT_C)
#define INA_ERR_DISABLED            ( 30ULL << INA_RC_BIT_C)
#define INA_ERR_DOWN                ( 31ULL << INA_RC_BIT_C)
#define INA_ERR_DOWNLOADED          ( 32ULL << INA_RC_BIT_C)
#define INA_ERR_EMPTY               ( 33ULL << INA_RC_BIT_C)
#define INA_ERR_ENABLED             ( 34ULL << INA_RC_BIT_C)
#define INA_ERR_ENHANCED            ( 35ULL << INA_RC_BIT_C)
#define INA_ERR_ENOUGH              ( 36ULL << INA_RC_BIT_C)
#define INA_ERR_EXCEEDED            ( 37ULL << INA_RC_BIT_C)
#define INA_ERR_EXCHANGED           ( 38ULL << INA_RC_BIT_C)
#define INA_ERR_EXECUTABLE          ( 39ULL << INA_RC_BIT_C)
#define INA_ERR_EXISTS              ( 40ULL << INA_RC_BIT_C)
#define INA_ERR_EXPIRED             ( 41ULL << INA_RC_BIT_C)
#define INA_ERR_EXTENDED            ( 42ULL << INA_RC_BIT_C)
#define INA_ERR_FAILED              ( 43ULL << INA_RC_BIT_C)
#define INA_ERR_FALSE               ( 44ULL << INA_RC_BIT_C)
#define INA_ERR_FATAL               ( 45ULL << INA_RC_BIT_C)
#define INA_ERR_FORBIDDEN           ( 46ULL << INA_RC_BIT_C)
#define INA_ERR_FORMATTED           ( 47ULL << INA_RC_BIT_C)
#define INA_ERR_FOUND               ( 48ULL << INA_RC_BIT_C)
#define INA_ERR_FULL                ( 49ULL << INA_RC_BIT_C)
#define INA_ERR_GONE                ( 50ULL << INA_RC_BIT_C)
#define INA_ERR_GOOD                ( 51ULL << INA_RC_BIT_C)
#define INA_ERR_HALTED              ( 52ULL << INA_RC_BIT_C)
#define INA_ERR_HIDDEN              ( 53ULL << INA_RC_BIT_C)
#define INA_ERR_HOLD                ( 54ULL << INA_RC_BIT_C)
#define INA_ERR_IDLE                ( 55ULL << INA_RC_BIT_C)
#define INA_ERR_ILLEGAL             ( 56ULL << INA_RC_BIT_C)
#define INA_ERR_IMPLEMENTED         ( 57ULL << INA_RC_BIT_C)
#define INA_ERR_IN_PROGRESS         ( 58ULL << INA_RC_BIT_C)
#define INA_ERR_IN_USE              ( 59ULL << INA_RC_BIT_C)
#define INA_ERR_INITIALIZED         ( 60ULL << INA_RC_BIT_C)
#define INA_ERR_INSERTED            ( 61ULL << INA_RC_BIT_C)
#define INA_ERR_INSTALLED           ( 62ULL << INA_RC_BIT_C)
#define INA_ERR_INTERRUPTED         ( 63ULL << INA_RC_BIT_C)
#define INA_ERR_JOINED              ( 64ULL << INA_RC_BIT_C)
#define INA_ERR_KNOWN               ( 65ULL << INA_RC_BIT_C)
#define INA_ERR_LINKED              ( 66ULL << INA_RC_BIT_C)
#define INA_ERR_LOADED              ( 67ULL << INA_RC_BIT_C)
#define INA_ERR_LOCAL               ( 68ULL << INA_RC_BIT_C)
#define INA_ERR_LOCKED              ( 69ULL << INA_RC_BIT_C)
#define INA_ERR_LOOPED              ( 70ULL << INA_RC_BIT_C)
#define INA_ERR_LOST                ( 71ULL << INA_RC_BIT_C)
#define INA_ERR_MERGED              ( 72ULL << INA_RC_BIT_C)
#define INA_ERR_MISSING             ( 73ULL << INA_RC_BIT_C)
#define INA_ERR_MOUNTED             ( 74ULL << INA_RC_BIT_C)
#define INA_ERR_NEEDED              ( 75ULL << INA_RC_BIT_C)
#define INA_ERR_NO                  ( 76ULL << INA_RC_BIT_C)
#define INA_ERR_NO_SUCH             ( 77ULL << INA_RC_BIT_C)
#define INA_ERR_OFF                 ( 78ULL << INA_RC_BIT_C)
#define INA_ERR_ON                  ( 79ULL << INA_RC_BIT_C)
#define INA_ERR_ONLINE              ( 80ULL << INA_RC_BIT_C)
#define INA_ERR_OPEN                ( 81ULL << INA_RC_BIT_C)
#define INA_ERR_ORDERED             ( 82ULL << INA_RC_BIT_C)
#define INA_ERR_OUT_OF              ( 83ULL << INA_RC_BIT_C)
#define INA_ERR_OUT_OF_RANGE        ( 84ULL << INA_RC_BIT_C)
#define INA_ERR_OVERFLOW            ( 85ULL << INA_RC_BIT_C)
#define INA_ERR_PADDED              ( 86ULL << INA_RC_BIT_C)
#define INA_ERR_PARTED              ( 87ULL << INA_RC_BIT_C)
#define INA_ERR_PERMITTED           ( 88ULL << INA_RC_BIT_C)
#define INA_ERR_POPPED              ( 89ULL << INA_RC_BIT_C)
#define INA_ERR_PRELOADED           ( 90ULL << INA_RC_BIT_C)
#define INA_ERR_PROCESSABLE         ( 91ULL << INA_RC_BIT_C)
#define INA_ERR_PROVIDED            ( 92ULL << INA_RC_BIT_C)
#define INA_ERR_PUSHED              ( 93ULL << INA_RC_BIT_C)
#define INA_ERR_REACHABLE           ( 94ULL << INA_RC_BIT_C)
#define INA_ERR_READABLE            ( 95ULL << INA_RC_BIT_C)
#define INA_ERR_RECEIVED            ( 96ULL << INA_RC_BIT_C)
#define INA_ERR_REFUSED             ( 97ULL << INA_RC_BIT_C)
#define INA_ERR_REGISTERED          ( 98ULL << INA_RC_BIT_C)
#define INA_ERR_REJECTED            ( 99ULL << INA_RC_BIT_C)
#define INA_ERR_RELEASED            (100ULL << INA_RC_BIT_C)
#define INA_ERR_REMOTE              (101ULL << INA_RC_BIT_C)
#define INA_ERR_REMOVED             (102ULL << INA_RC_BIT_C)
#define INA_ERR_RENDERABLE          (103ULL << INA_RC_BIT_C)
#define INA_ERR_RESERVED            (104ULL << INA_RC_BIT_C)
#define INA_ERR_RESET               (105ULL << INA_RC_BIT_C)
#define INA_ERR_RESPONDING          (106ULL << INA_RC_BIT_C)
#define INA_ERR_RETRIED             (107ULL << INA_RC_BIT_C)
#define INA_ERR_RIGHT               (108ULL << INA_RC_BIT_C)
#define INA_ERR_RUNNING             (109ULL << INA_RC_BIT_C)
#define INA_ERR_SENT                (110ULL << INA_RC_BIT_C)
#define INA_ERR_SHARED              (111ULL << INA_RC_BIT_C)
#define INA_ERR_SORTED              (112ULL << INA_RC_BIT_C)
#define INA_ERR_SPECIFIED           (113ULL << INA_RC_BIT_C)
#define INA_ERR_SPLITTED            (114ULL << INA_RC_BIT_C)
#define INA_ERR_STALLED             (115ULL << INA_RC_BIT_C)
#define INA_ERR_STOPPED             (116ULL << INA_RC_BIT_C)
#define INA_ERR_SUCEEDED            (117ULL << INA_RC_BIT_C)
#define INA_ERR_SUITABLE            (118ULL << INA_RC_BIT_C)
#define INA_ERR_SUPPORTED           (119ULL << INA_RC_BIT_C)
#define INA_ERR_SYNCHRONIZED        (120ULL << INA_RC_BIT_C)
#define INA_ERR_TERMINATED          (121ULL << INA_RC_BIT_C)
#define INA_ERR_THROWN              (122ULL << INA_RC_BIT_C)
#define INA_ERR_TIMED_OUT           (123ULL << INA_RC_BIT_C)
#define INA_ERR_TOO_COMPLEX         (124ULL << INA_RC_BIT_C)
#define INA_ERR_TOO_FEW             (125ULL << INA_RC_BIT_C)
#define INA_ERR_TOO_LARGE           (126ULL << INA_RC_BIT_C)
#define INA_ERR_TOO_LONG            (127ULL << INA_RC_BIT_C)
#define INA_ERR_TOO_MANY            (128ULL << INA_RC_BIT_C)
#define INA_ERR_TOO_MUCH            (129ULL << INA_RC_BIT_C)
#define INA_ERR_TOO_SIMPLE          (130ULL << INA_RC_BIT_C)
#define INA_ERR_TOO_SMALL           (131ULL << INA_RC_BIT_C)
#define INA_ERR_TRIGGERED           (132ULL << INA_RC_BIT_C)
#define INA_ERR_TRUE                (133ULL << INA_RC_BIT_C)
#define INA_ERR_UNBLOCKED           (134ULL << INA_RC_BIT_C)
#define INA_ERR_UNDERFLOW           (135ULL << INA_RC_BIT_C)
#define INA_ERR_UNINITIALIZED       (136ULL << INA_RC_BIT_C)
#define INA_ERR_UNINSTALLED         (137ULL << INA_RC_BIT_C)
#define INA_ERR_UNIQUE              (138ULL << INA_RC_BIT_C)
#define INA_ERR_UNLOADED            (139ULL << INA_RC_BIT_C)
#define INA_ERR_UNLOCKED            (140ULL << INA_RC_BIT_C)
#define INA_ERR_UNSORTED            (141ULL << INA_RC_BIT_C)
#define INA_ERR_UP                  (142ULL << INA_RC_BIT_C)
#define INA_ERR_UPDATED             (143ULL << INA_RC_BIT_C)
#define INA_ERR_UPGRADED            (144ULL << INA_RC_BIT_C)
#define INA_ERR_UPLOADED            (145ULL << INA_RC_BIT_C)
#define INA_ERR_USED                (146ULL << INA_RC_BIT_C)
#define INA_ERR_VALID               (147ULL << INA_RC_BIT_C)
#define INA_ERR_VISIBLE             (148ULL << INA_RC_BIT_C)
#define INA_ERR_WORKING             (149ULL << INA_RC_BIT_C)
#define INA_ERR_WRITABLE            (150ULL << INA_RC_BIT_C)
#define INA_ERR_WRONG               (151ULL << INA_RC_BIT_C)
#define INA_ERR_END_OF              (152ULL << INA_RC_BIT_C)
#define INA_ERR_RESOLVED            (153ULL << INA_RC_BIT_C)
#define INA_ERR_MATCH               (154ULL << INA_RC_BIT_C)
#define INA_ERR_TRY_AGAIN           (155ULL << INA_RC_BIT_C)
#define INA_ERR_PARSED              (156ULL << INA_RC_BIT_C)
#define INA_ERR_CHANGED             (157ULL << INA_RC_BIT_C)

/* Error codes (negate forms) */
#define INA_ERR_NOT_A               (INA_ERR_NOT | INA_ERR_A)
#define INA_ERR_NOT_ACK             (INA_ERR_NOT | INA_ERR_ACK)
#define INA_ERR_NOT_ACTIVE          (INA_ERR_NOT | INA_ERR_ACTIVE)
#define INA_ERR_NOT_ALIGNED         (INA_ERR_NOT | INA_ERR_ALIGNED)
#define INA_ERR_NOT_ALLOWED         (INA_ERR_NOT | INA_ERR_ALLOWED)
#define INA_ERR_NOT_ASSIGNED        (INA_ERR_NOT | INA_ERR_ASSIGNED)
#define INA_ERR_NOT_ATTACHED        (INA_ERR_NOT | INA_ERR_ATTACHED)
#define INA_ERR_NOT_ATTEMPTED       (INA_ERR_NOT | INA_ERR_ATTEMPTED)
#define INA_ERR_NOT_AUTHORIZED      (INA_ERR_NOT | INA_ERR_AUTHORIZED)
#define INA_ERR_NOT_AVAILABLE       (INA_ERR_NOT | INA_ERR_AVAILABLE)
#define INA_ERR_NOT_BAD             (INA_ERR_NOT | INA_ERR_BAD)
#define INA_ERR_NOT_BLOCKED         (INA_ERR_NOT | INA_ERR_BLOCKED)
#define INA_ERR_NOT_BROKEN          (INA_ERR_NOT | INA_ERR_BROKEN)
#define INA_ERR_NOT_BUILT           (INA_ERR_NOT | INA_ERR_BUILT)
#define INA_ERR_NOT_BUSY            (INA_ERR_NOT | INA_ERR_BUSY)
#define INA_ERR_NOT_CLOSED          (INA_ERR_NOT | INA_ERR_CLOSED)
#define INA_ERR_NOT_COLLIDED        (INA_ERR_NOT | INA_ERR_COLLIDED)
#define INA_ERR_NOT_COMPILED        (INA_ERR_NOT | INA_ERR_COMPILED)
#define INA_ERR_NOT_COMPLETE        (INA_ERR_NOT | INA_ERR_COMPLETE)
#define INA_ERR_NOT_CONFLICTED      (INA_ERR_NOT | INA_ERR_CONFLICTED)
#define INA_ERR_NOT_CONNECTED       (INA_ERR_NOT | INA_ERR_CONNECTED)
#define INA_ERR_NOT_CONSTRUCTED     (INA_ERR_NOT | INA_ERR_CONSTRUCTED)
#define INA_ERR_NOT_CREATED         (INA_ERR_NOT | INA_ERR_CREATED)
#define INA_ERR_NOT_DEFINED         (INA_ERR_NOT | INA_ERR_DEFINED)
#define INA_ERR_NOT_DENIED          (INA_ERR_NOT | INA_ERR_DENIED)
#define INA_ERR_NOT_DEPARTED        (INA_ERR_NOT | INA_ERR_DEPARTED)
#define INA_ERR_NOT_DESTRUCTED      (INA_ERR_NOT | INA_ERR_DESTRUCTED)
#define INA_ERR_NOT_DETACHED        (INA_ERR_NOT | INA_ERR_DETACHED)
#define INA_ERR_NOT_DETECTED        (INA_ERR_NOT | INA_ERR_DETECTED)
#define INA_ERR_NOT_DISABLED        (INA_ERR_NOT | INA_ERR_DISABLED)
#define INA_ERR_NOT_DOWN            (INA_ERR_NOT | INA_ERR_DOWN)
#define INA_ERR_NOT_DOWNLOADED      (INA_ERR_NOT | INA_ERR_DOWNLOADED)
#define INA_ERR_NOT_EMPTY           (INA_ERR_NOT | INA_ERR_EMPTY)
#define INA_ERR_NOT_ENABLED         (INA_ERR_NOT | INA_ERR_ENABLED)
#define INA_ERR_NOT_ENHANCED        (INA_ERR_NOT | INA_ERR_ENHANCED)
#define INA_ERR_NOT_ENOUGH          (INA_ERR_NOT | INA_ERR_ENOUGH)
#define INA_ERR_NOT_EXCEEDED        (INA_ERR_NOT | INA_ERR_EXCEEDED)
#define INA_ERR_NOT_EXCHANGED       (INA_ERR_NOT | INA_ERR_EXCHANGED)
#define INA_ERR_NOT_EXECUTABLE      (INA_ERR_NOT | INA_ERR_EXECUTABLE)
#define INA_ERR_NOT_EXISTS          (INA_ERR_NOT | INA_ERR_EXISTS)
#define INA_ERR_NOT_EXPIRED         (INA_ERR_NOT | INA_ERR_EXPIRED)
#define INA_ERR_NOT_EXTENDED        (INA_ERR_NOT | INA_ERR_EXTENDED)
#define INA_ERR_NOT_FAILED          (INA_ERR_NOT | INA_ERR_FAILED)
#define INA_ERR_NOT_FALSE           (INA_ERR_NOT | INA_ERR_FALSE)
#define INA_ERR_NOT_FATAL           (INA_ERR_NOT | INA_ERR_FATAL)
#define INA_ERR_NOT_FORBIDDEN       (INA_ERR_NOT | INA_ERR_FORBIDDEN)
#define INA_ERR_NOT_FORMATTED       (INA_ERR_NOT | INA_ERR_FORMATTED)
#define INA_ERR_NOT_FOUND           (INA_ERR_NOT | INA_ERR_FOUND)
#define INA_ERR_NOT_FULL            (INA_ERR_NOT | INA_ERR_FULL)
#define INA_ERR_NOT_GONE            (INA_ERR_NOT | INA_ERR_GONE)
#define INA_ERR_NOT_GOOD            (INA_ERR_NOT | INA_ERR_GOOD)
#define INA_ERR_NOT_HALTED          (INA_ERR_NOT | INA_ERR_HALTED)
#define INA_ERR_NOT_HIDDEN          (INA_ERR_NOT | INA_ERR_HIDDEN)
#define INA_ERR_NOT_HOLD            (INA_ERR_NOT | INA_ERR_HOLD)
#define INA_ERR_NOT_IDLE            (INA_ERR_NOT | INA_ERR_IDLE)
#define INA_ERR_NOT_ILLEGAL         (INA_ERR_NOT | INA_ERR_ILLEGAL)
#define INA_ERR_NOT_IMPLEMENTED     (INA_ERR_NOT | INA_ERR_IMPLEMENTED)
#define INA_ERR_NOT_IN_PROGRESS     (INA_ERR_NOT | INA_ERR_IN_PROGRESS)
#define INA_ERR_NOT_IN_USE          (INA_ERR_NOT | INA_ERR_IN_USE)
#define INA_ERR_NOT_INITIALIZED     (INA_ERR_NOT | INA_ERR_INITIALIZED)
#define INA_ERR_NOT_INSERTED        (INA_ERR_NOT | INA_ERR_INSERTED)
#define INA_ERR_NOT_INSTALLED       (INA_ERR_NOT | INA_ERR_INSTALLED)
#define INA_ERR_NOT_INTERRUPTED     (INA_ERR_NOT | INA_ERR_INTERRUPTED)
#define INA_ERR_NOT_JOINED          (INA_ERR_NOT | INA_ERR_JOINED)
#define INA_ERR_NOT_KNOWN           (INA_ERR_NOT | INA_ERR_KNOWN)
#define INA_ERR_NOT_LINKED          (INA_ERR_NOT | INA_ERR_LINKED)
#define INA_ERR_NOT_LOADED          (INA_ERR_NOT | INA_ERR_LOADED)
#define INA_ERR_NOT_LOCAL           (INA_ERR_NOT | INA_ERR_LOCAL)
#define INA_ERR_NOT_LOCKED          (INA_ERR_NOT | INA_ERR_LOCKED)
#define INA_ERR_NOT_LOOPED          (INA_ERR_NOT | INA_ERR_LOOPED)
#define INA_ERR_NOT_LOST            (INA_ERR_NOT | INA_ERR_LOST)
#define INA_ERR_NOT_MERGED          (INA_ERR_NOT | INA_ERR_MERGED)
#define INA_ERR_NOT_MISSING         (INA_ERR_NOT | INA_ERR_MISSING)
#define INA_ERR_NOT_MOUNTED         (INA_ERR_NOT | INA_ERR_MOUNTED)
#define INA_ERR_NOT_NEEDED          (INA_ERR_NOT | INA_ERR_NEEDED)
#define INA_ERR_NOT_NO              (INA_ERR_NOT | INA_ERR_NO)
#define INA_ERR_NOT_NO_SUCH         (INA_ERR_NOT | INA_ERR_NO_SUCH)
#define INA_ERR_NOT_OFF             (INA_ERR_NOT | INA_ERR_OFF)
#define INA_ERR_NOT_ON              (INA_ERR_NOT | INA_ERR_ON)
#define INA_ERR_NOT_ONLINE          (INA_ERR_NOT | INA_ERR_ONLINE)
#define INA_ERR_NOT_OPEN            (INA_ERR_NOT | INA_ERR_OPEN)
#define INA_ERR_NOT_ORDERED         (INA_ERR_NOT | INA_ERR_ORDERED)
#define INA_ERR_NOT_OUT_OF          (INA_ERR_NOT | INA_ERR_OUT_OF)
#define INA_ERR_NOT_OUT_OF_RANGE    (INA_ERR_NOT | INA_ERR_OUT_OF_RANGE)
#define INA_ERR_NOT_OVERFLOW        (INA_ERR_NOT | INA_ERR_OVERFLOW)
#define INA_ERR_NOT_PADDED          (INA_ERR_NOT | INA_ERR_PADDED)
#define INA_ERR_NOT_PARTED          (INA_ERR_NOT | INA_ERR_PARTED)
#define INA_ERR_NOT_PERMITTED       (INA_ERR_NOT | INA_ERR_PERMITTED)
#define INA_ERR_NOT_POPPED          (INA_ERR_NOT | INA_ERR_POPPED)
#define INA_ERR_NOT_PRELOADED       (INA_ERR_NOT | INA_ERR_PRELOADED)
#define INA_ERR_NOT_PROCESSABLE     (INA_ERR_NOT | INA_ERR_PROCESSABLE)
#define INA_ERR_NOT_PROVIDED        (INA_ERR_NOT | INA_ERR_PROVIDED)
#define INA_ERR_NOT_PUSHED          (INA_ERR_NOT | INA_ERR_PUSHED)
#define INA_ERR_NOT_REACHABLE       (INA_ERR_NOT | INA_ERR_REACHABLE)
#define INA_ERR_NOT_READABLE        (INA_ERR_NOT | INA_ERR_READABLE)
#define INA_ERR_NOT_RECEIVED        (INA_ERR_NOT | INA_ERR_RECEIVED)
#define INA_ERR_NOT_REFUSED         (INA_ERR_NOT | INA_ERR_REFUSED)
#define INA_ERR_NOT_REGISTERED      (INA_ERR_NOT | INA_ERR_REGISTERED)
#define INA_ERR_NOT_REJECTED        (INA_ERR_NOT | INA_ERR_REJECTED)
#define INA_ERR_NOT_RELEASED        (INA_ERR_NOT | INA_ERR_RELEASED)
#define INA_ERR_NOT_REMOTE          (INA_ERR_NOT | INA_ERR_REMOTE)
#define INA_ERR_NOT_REMOVED         (INA_ERR_NOT | INA_ERR_REMOVED)
#define INA_ERR_NOT_RENDERABLE      (INA_ERR_NOT | INA_ERR_RENDERABLE)
#define INA_ERR_NOT_RESERVED        (INA_ERR_NOT | INA_ERR_RESERVED)
#define INA_ERR_NOT_RESET           (INA_ERR_NOT | INA_ERR_RESET)
#define INA_ERR_NOT_RESPONDING      (INA_ERR_NOT | INA_ERR_RESPONDING)
#define INA_ERR_NOT_RETRIED         (INA_ERR_NOT | INA_ERR_RETRIED)
#define INA_ERR_NOT_RIGHT           (INA_ERR_NOT | INA_ERR_RIGHT)
#define INA_ERR_NOT_RUNNING         (INA_ERR_NOT | INA_ERR_RUNNING)
#define INA_ERR_NOT_SENT            (INA_ERR_NOT | INA_ERR_SENT)
#define INA_ERR_NOT_SHARED          (INA_ERR_NOT | INA_ERR_SHARED)
#define INA_ERR_NOT_SORTED          (INA_ERR_NOT | INA_ERR_SORTED)
#define INA_ERR_NOT_SPECIFIED       (INA_ERR_NOT | INA_ERR_SPECIFIED)
#define INA_ERR_NOT_SPLITTED        (INA_ERR_NOT | INA_ERR_SPLITTED)
#define INA_ERR_NOT_STALLED         (INA_ERR_NOT | INA_ERR_STALLED)
#define INA_ERR_NOT_STOPPED         (INA_ERR_NOT | INA_ERR_STOPPED)
#define INA_ERR_NOT_SUCEEDED        (INA_ERR_NOT | INA_ERR_SUCEEDED)
#define INA_ERR_NOT_SUITABLE        (INA_ERR_NOT | INA_ERR_SUITABLE)
#define INA_ERR_NOT_SUPPORTED       (INA_ERR_NOT | INA_ERR_SUPPORTED)
#define INA_ERR_NOT_SYNCHRONIZED    (INA_ERR_NOT | INA_ERR_SYNCHRONIZED)
#define INA_ERR_NOT_TERMINATED      (INA_ERR_NOT | INA_ERR_TERMINATED)
#define INA_ERR_NOT_THROWN          (INA_ERR_NOT | INA_ERR_THROWN)
#define INA_ERR_NOT_TIMED_OUT       (INA_ERR_NOT | INA_ERR_TIMED_OUT)
#define INA_ERR_NOT_TOO_COMPLEX     (INA_ERR_NOT | INA_ERR_TOO_COMPLEX)
#define INA_ERR_NOT_TOO_FEW         (INA_ERR_NOT | INA_ERR_TOO_FEW)
#define INA_ERR_NOT_TOO_LARGE       (INA_ERR_NOT | INA_ERR_TOO_LARGE)
#define INA_ERR_NOT_TOO_LONG        (INA_ERR_NOT | INA_ERR_TOO_LONG)
#define INA_ERR_NOT_TOO_MANY        (INA_ERR_NOT | INA_ERR_TOO_MANY)
#define INA_ERR_NOT_TOO_MUCH        (INA_ERR_NOT | INA_ERR_TOO_MUCH)
#define INA_ERR_NOT_TOO_SIMPLE      (INA_ERR_NOT | INA_ERR_TOO_SIMPLE)
#define INA_ERR_NOT_TOO_SMALL       (INA_ERR_NOT | INA_ERR_TOO_SMALL)
#define INA_ERR_NOT_TRIGGERED       (INA_ERR_NOT | INA_ERR_TRIGGERED)
#define INA_ERR_NOT_TRUE            (INA_ERR_NOT | INA_ERR_TRUE)
#define INA_ERR_NOT_UNBLOCKED       (INA_ERR_NOT | INA_ERR_UNBLOCKED)
#define INA_ERR_NOT_UNDERFLOW       (INA_ERR_NOT | INA_ERR_UNDERFLOW)
#define INA_ERR_NOT_UNINITIALIZED   (INA_ERR_NOT | INA_ERR_UNINITIALIZED)
#define INA_ERR_NOT_UNINSTALLED     (INA_ERR_NOT | INA_ERR_UNINSTALLED)
#define INA_ERR_NOT_UNIQUE          (INA_ERR_NOT | INA_ERR_UNIQUE)
#define INA_ERR_NOT_UNLOADED        (INA_ERR_NOT | INA_ERR_UNLOADED)
#define INA_ERR_NOT_UNLOCKED        (INA_ERR_NOT | INA_ERR_UNLOCKED)
#define INA_ERR_NOT_UNSORTED        (INA_ERR_NOT | INA_ERR_UNSORTED)
#define INA_ERR_NOT_UP              (INA_ERR_NOT | INA_ERR_UP)
#define INA_ERR_NOT_UPDATED         (INA_ERR_NOT | INA_ERR_UPDATED)
#define INA_ERR_NOT_UPGRADED        (INA_ERR_NOT | INA_ERR_UPGRADED)
#define INA_ERR_NOT_UPLOADED        (INA_ERR_NOT | INA_ERR_UPLOADED)
#define INA_ERR_NOT_USED            (INA_ERR_NOT | INA_ERR_USED)
#define INA_ERR_NOT_VALID           (INA_ERR_NOT | INA_ERR_VALID)
#define INA_ERR_NOT_VISIBLE         (INA_ERR_NOT | INA_ERR_VISIBLE)
#define INA_ERR_NOT_WORKING         (INA_ERR_NOT | INA_ERR_WORKING)
#define INA_ERR_NOT_WRITABLE        (INA_ERR_NOT | INA_ERR_WRITABLE)
#define INA_ERR_NOT_WRONG           (INA_ERR_NOT | INA_ERR_WRONG)
#define INA_ERR_NOT_END_OF          (INA_ERR_NOT | INA_ERR_END_OF)
#define INA_ERR_NOT_RESOLVED        (INA_ERR_NOT | INA_ERR_RESOLVED)
#define INA_ERR_NOT_MATCH           (INA_ERR_NOT | INA_ERR_MATCH)
#define INA_ERR_NOT_TRY_AGAIN       (INA_ERR_NOT | INA_ERR_TRY_AGAIN)
#define INA_ERR_NOT_PARSED          (INA_ERR_NOT | INA_ERR_PARSED)
#define INA_ERR_NOT_CHANGED         (INA_ERR_NOT | INA_ERR_CHANGED)

/* Error codes aliases */
#define INA_ERR_UNDEFINED           (INA_ERR_NOT_DEFINED)
#define INA_ERR_UNUSED              (INA_ERR_NOT_USED)
#define INA_ERR_UNORDERED           (INA_ERR_NOT_ORDERED)
#define INA_ERR_INVALID             (INA_ERR_NOT_VALID)
#define INA_ERR_INACTIVE            (INA_ERR_NOT_ACTIVE)
#define INA_ERR_ERASED              (INA_ERR_REMOVED)
#define INA_ERR_DELETED             (INA_ERR_REMOVED)
#define INA_ERR_OFFLINE             (INA_ERR_NOT_ONLINE)
#define INA_ERR_UNAVAILABLE         (INA_ERR_NOT_AVERS)

/* Error subjects */
#define INA_ES_NONE                 (0U)
#define INA_ES_ACCESS               (1U)
#define INA_ES_ARGUMENT             (2U)
#define INA_ES_COMPRESSION          (3U)
#define INA_ES_DECOMPRESSION        (5U)
#define INA_ES_DESCRIPTOR           (4U)
#define INA_ES_DEVICE               (6U)
#define INA_ES_DIRECTORY            (7U)
#define INA_ES_FILE                 (8U)
#define INA_ES_FUNCTION             (9U)
#define INA_ES_HANDLE               (10U)
#define INA_ES_INPUT                (11U)
#define INA_ES_IO                   (12U)
#define INA_ES_LIMIT                (13U)
#define INA_ES_MEMORY               (14U)
#define INA_ES_OBJECT               (15U)
#define INA_ES_POOL                 (16U)
#define INA_ES_POSITION             (17U)
#define INA_ES_SCRIPT               (18U)
#define INA_ES_SIZE                 (19U)
#define INA_ES_SOCKET               (20U)
#define INA_ES_STRING               (21U)
#define INA_ES_TYPE                 (22U)
#define INA_ES_VERSION              (23U)
#define INA_ES_STATE                (24U)
#define INA_ES_CONFIGURATION        (25U)
#define INA_ES_SECTION              (26U)
#define INA_ES_KEY                  (27U)
#define INA_ES_ENUMERATION          (28U)
#define INA_ES_READ                 (29U)
#define INA_ES_WRITE                (30U)
#define INA_ES_OPTION               (31U)
#define INA_ES_BUFFER               (32U)
#define INA_ES_ADDRESS              (33U)
#define INA_ES_MAC                  (34U)
#define INA_ES_PROCESS              (35U)
#define INA_ES_PATTERN              (36U)
#define INA_ES_SEMAPHORE            (37U)
#define INA_ES_OPERATION            (38U)
#define INA_ES_TIME                 (39U)
#define INA_ES_HOST                 (40U)
#define INA_ES_TEXT                 (41U)
#define INA_ES_API                  (42U)

/* Start of user defined error subjects */
#define INA_ES_USER_DEFINED         (1024UL)


/*
 * Global error messages
 */
#define INA_ERR_INVALID_ARGUMENT  (INA_ERR_INVALID|INA_ES_ARGUMENT)
#define INA_ERR_OUT_OF_MEMORY     (INA_ERR_OUT_OF|INA_ES_MEMORY)
#define INA_ERR_INVALID_PATTERN   (INA_ERR_INVALID|INA_ES_PATTERN)
#define INA_ERR_POOL_FULL         (INA_ERR_FULL|INA_ES_POOL)
#define INA_ERR_OPERATION_INVALID (INA_ES_OPERATION|INA_ERR_INVALID)
#define INA_ERR_OPERATION_FAILED  (INA_ES_OPERATION|INA_ERR_FAILED)
#define INA_ERR_WRITE_FAILED      (INA_ES_WRITE|INA_ERR_FAILED)
#define INA_ERR_READ_FAILED       (INA_ES_READ|INA_ERR_FAILED)
#define INA_ERR_NOT_A_DIRECTORY   (INA_ES_DIRECTORY|INA_ERR_NOT_A)
#define INA_ERR_FILE_OPEN         (INA_ES_FILE|INA_ERR_OPEN)
#define INA_ERR_OPERATION_INVALID (INA_ES_OPERATION|INA_ERR_INVALID)

/*
 * Subject dictionary callback
 */
typedef const char* (*ina_err_subject_cb_t)(int);


/*
 * Initialize the error module. This function should never be called,
 * because is part of the INAC initialization and  called in ina_init().
 *
 * Return
 *  INA_SUCCESS if all went well.
 *
 *  Error messages
 *   INA_ERR_OUT_OF_MEMORY
 */
INA_API(ina_rc_t) ina_err_init(void);

/*
 * Destroy error module. This function is called at program exit and
 * should never called directly.
 */
INA_API(void) ina_err_destroy(void);

/*
 * Register an error subject dictionary callback.
 *
 * Parameters
 *  cb  Dictionary callback
 *
 * Return
 *  Previously registered dictionary callback or NULL
 */
INA_API(ina_err_subject_cb_t) ina_err_register_dict(ina_err_subject_cb_t cb);

/*
 * Set global return code. Use INA_ERROR() or INA_OS_ERROR() as shortcut or use
 * INA_RC_PACK to generate an valid RC.
 *
 * Parameters
 *  rc   New return code to set
 *
 * Return
 *  Current RC
 */
INA_INLINE ina_rc_t ina_err_set_rc(ina_rc_t rc)
{
    __rc = rc;
    return __rc;
}

/*
 * Return current global return code.
 *
 * Return
 *  Current RC
 */
INA_INLINE ina_rc_t ina_err_get_rc(void)
{
    return __rc;
}

/*
 * Mark a RC as handled.
 *
 * Parameters
 *  rc  Valid RC to mark as handled. If a error was already marked as handled
 *      no error occurs.
 *
 * Return
 *  Ceared RC
 */
INA_INLINE ina_rc_t ina_err_clear_rc(ina_rc_t rc)
{
    return (rc & ~(INA_ERR_ERROR));
}

/*
 * Reset the global RC by marking it as handled.
 *
 * Return
 *  Current global RC
 */
INA_INLINE ina_rc_t ina_err_reset(void)
{
    __rc = ina_err_clear_rc(__rc);
    return __rc;
}

/*
 * Set user bit on the global RC.
 *
 * Parameters
 *  ubits  User defined bits
 *
 * Return
 *  Current global RC
 */
INA_INLINE ina_rc_t ina_err_set_ubits(uint8_t ubits)
{
    __rc = ((__rc) & (~ (((1ULL << (8)) - 1ULL) << (INA_RC_BIT_U)))) | (((ina_rc_t)(ubits)) << (INA_RC_BIT_U));
    return __rc;
}

/*
 * Return user defined bit of an given RC.
 *
 * Parameters
 *  rc   Valid RC where to extract user defined bits
 *
 * Return
 *  User defined bits
 */
INA_INLINE uint8_t ina_err_get_ubits(ina_rc_t rc)
{
    return (uint8_t)INA_RC_UBITS(rc);
}

/*
 * Format the error message for a given RC.
 *
 * Parameters
 *  rc   Valid RC
 *
 * Return
 *  Error message
 */
INA_API(const char*) ina_err_strerror(ina_rc_t rc);

/* Pack a RC */
#ifdef INA_LIB
#  define INA_RC_PACK(x, e) (INA_ERR_ERROR | (((ina_rc_t)INA_VERSION_HEX) << INA_RC_BIT_R) | (((ina_rc_t)(e)) << INA_RC_BIT_O) | (x))
#else
#  ifndef INA_ERROR_VER
#    define INA_ERROR_VER (0)
#  endif
#  ifndef INA_ERROR_REV
#    define INA_ERROR_REV (0)
#  endif
#  define INA_RC_PACK(x, e) (INA_ERR_ERROR | (((ina_rc_t)INA_ERROR_VER) << INA_RC_BIT_V) | (((ina_rc_t)INA_ERROR_REV) << INA_RC_BIT_R) | (((ina_rc_t)(e)) << INA_RC_BIT_O) | ((x)))
#endif

/* Check return code: failure */
#define INA_FAILED(rc) ((rc)&(INA_ERR_ERROR))
/* Check return code: successful or handled */
#define INA_SUCCEED(rc) (!INA_FAILED((rc)))

/* Checkpoint must succeed */
#define INA_MUST_SUCCEED(rc) do { if (INA_UNLIKELY(INA_FAILED((rc)))) abort(); } while(0)
#define INA_FAIL_IF(cond) do { if ((cond)) goto fail; } while(0)
#define INA_FAIL_IF_ERROR(rc) INA_FAIL_IF(INA_FAILED((rc)))

/* Set global RC */
#define INA_ERROR(x) ina_err_set_rc(INA_RC_PACK((x), 0))
/* Set global RC and capture errno */
#ifndef INA_OS_WINDOWS
#define INA_OS_ERROR(x) ina_err_set_rc(INA_RC_PACK((x), errno))
#else
#define INA_OS_ERROR(x) ina_err_set_rc(INA_RC_PACK((x), GetLastError()))
#endif
typedef void*  CExceptionHandler;
INA_API(CExceptionHandler) ina_err_init_coredump(const char* dump_dir);
#ifdef __cplusplus
}
#endif

#endif
