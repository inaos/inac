/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_ERROR_H_
#define _LIBINAC_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>

/* Indicate no errors */
#define INA_SUCCESS  (0)


static INA_TLS(ina_rc_t) __rc = INA_SUCCESS;

/* Bit-shifts */
#define INA_RC_BIT_E 63U
#define INA_RC_BIT_V 56U
#define INA_RC_BIT_R 40U
#define INA_RC_BIT_O 24U
#define INA_RC_BIT_N 23U
#define INA_RC_BIT_C 15U
#define INA_RC_BIT_S 00U

#define INA_RC_EFLAG(rc)   ((uint32_t)(((rc) >> INA_RC_BIT_E) & 0x1UL))
#define INA_RC_APIVER(rc)  ((uint32_t)(((rc) >> INA_RC_BIT_V) & 0x7FUL))
#define INA_RC_APIREV(rc)  ((uint32_t)(((rc) >> INA_RC_BIT_R) & 0xFFFFUL))
#define INA_RC_ERRNO(rc)   ((uint32_t)(((rc) >> INA_RC_BIT_O) & 0xFFFFUL))
#define INA_RC_NFLAG(rc)   ((uint32_t)(((rc) >> INA_RC_BIT_N) & 0x1UL))
#define INA_RC_ERRCDE(rc)  ((uint32_t)(((rc) >> INA_RC_BIT_C) & 0xFFUL))
#define INA_RC_ERROR(rc)   ((uint32_t)(((rc) >> INA_RC_BIT_C) & 0xFFUL))
#define INA_RC_SUBJECT(rc) ((uint32_t)(((rc) >> INA_RC_BIT_S) & 0x7FFFUL))
#define INA_RC_ERRMSG(rc)  ((uint32_t)((INA_MID_BITS((rc), INA_RC_BIT_C, INA_RC_BIT_N+1))<<INA_RC_BIT_C))


/* Flags */
#define INA_ERR_ERROR               (  1ULL << INA_RC_BIT_E) /* Error-bit  */
#define INA_ERR_NOT                 (  1ULL << INA_RC_BIT_N) /* Negate-bit */

/* Error attributes */
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

/* Error attributes (negate forms) */
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

/* Attribute aliases */
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
#define INA_ES_ACCESS               (2U)
#define INA_ES_ADMINISTRATOR        (3U)
#define INA_ES_API                  (4U)
#define INA_ES_APPLICATION          (5U)
#define INA_ES_ARGUMENT             (6U)
#define INA_ES_ARRAY                (200U)
#define INA_ES_AUTHENTICATION       (7U)
#define INA_ES_BINARY               (8U)
#define INA_ES_BROADCAST            (10U)
#define INA_ES_CLIENT               (11U)
#define INA_ES_CHARSET              (13U)
#define INA_ES_CODE                 (14U)
#define INA_ES_COMMIT               (15U)
#define INA_ES_COMPILATION          (16U)
#define INA_ES_COMPILER             (17U)
#define INA_ES_COMPRESSION          (18U)
#define INA_ES_CONSOLE              (19U)
#define INA_ES_CRC                  (19U)
#define INA_ES_DAEMON               (20U)
#define INA_ES_DATA                 (21U)
#define INA_ES_DEPENDENCY           (22U)
#define INA_ES_DESCRIPTOR           (23U)
#define INA_ES_DEVICE               (24U)
#define INA_ES_DIRECTORY            (25U)
#define INA_ES_DISK                 (26U)
#define INA_ES_DRIVE                (26U)
#define INA_ES_DLL                  (27U)
#define INA_ES_DOMAIN               (28U)
#define INA_ES_DRIVER               (30U)
#define INA_ES_ENDPOINT             (32U)
#define INA_ES_ENGINE               (33U)
#define INA_ES_EVALUATION           (34U)
#define INA_ES_EVENT                (35U)
#define INA_ES_EXCEPTION            (36U)
#define INA_ES_EXPECTATION          (37U)
#define INA_ES_FETCH                (38U)
#define INA_ES_FILE                 (39U)
#define INA_ES_FLOAT                (40U)
#define INA_ES_FORMAT               (42U)
#define INA_ES_FUNCTION             (43U)
#define INA_ES_GATEWAY              (44U)
#define INA_ES_GROUP                (45U)
#define INA_ES_LONG                 (46U)
#define INA_ES_HANDLE               (47U)
#define INA_ES_HARDWARE             (48U)
#define INA_ES_HEADER               (49U)
#define INA_ES_HASH                 (50U)
#define INA_ES_HOST                 (51U)
#define INA_ES_IDENTIFIER           (52U)
#define INA_ES_INDEX                (53U)
#define INA_ES_INPUT                (54U)
#define INA_ES_INTEGER              (55U)
#define INA_ES_INTERFACE            (56U)
#define INA_ES_INTERVAL             (57U)
#define INA_ES_IO                   (58U)
#define INA_ES_KEYBOARD             (59U)
#define INA_ES_LENGTH               (60U)
#define INA_ES_LEVEL                (61U)
#define INA_ES_LIBRARY              (62U)
#define INA_ES_LIMIT                (63U)
#define INA_ES_LINK                 (64U)
#define INA_ES_LINKAGE              (65U)
#define INA_ES_LINKER               (66U)
#define INA_ES_LOCATION             (67U)
#define INA_ES_LOGIN                (68U)
#define INA_ES_LOOP                 (69U)
#define INA_ES_MACHINE              (70U)
#define INA_ES_MEDIA                (71U)
#define INA_ES_MEMORY               (72U)
#define INA_ES_MESSAGE              (73U)
#define INA_ES_METHOD               (74U)
#define INA_ES_MODULE               (75U)
#define INA_ES_MONITOR              (76U)
#define INA_ES_NETWORK              (77U)
#define INA_ES_NODE                 (78U)
#define INA_ES_NOTHING              (79U)
#define INA_ES_NUMBER               (80U)
#define INA_ES_OBJECT               (81U)
#define INA_ES_OPERATION            (82U)
#define INA_ES_OPERATOR             (83U)
#define INA_ES_PACKAGE              (84U)
#define INA_ES_PACKET               (85U)
#define INA_ES_PASSWORD             (86U)
#define INA_ES_PATH                 (87U)
#define INA_ES_PEER                 (88U)
#define INA_ES_PERMISSION           (89U)
#define INA_ES_PLATFORM             (90U)
#define INA_ES_POOL                 (91U)
#define INA_ES_POSITION             (92U)
#define INA_ES_PROFILER             (93U)
#define INA_ES_PROTOCOL             (94U)
#define INA_ES_PROXY                (95U)
#define INA_ES_RANGE                (97U)
#define INA_ES_RATIO                (98U)
#define INA_ES_RECORD               (99U)
#define INA_ES_REPOSITORY           (100U)
#define INA_ES_REQUEST              (101U)
#define INA_ES_RESOURCE             (102U)
#define INA_ES_REVISION             (103U)
#define INA_ES_ROUTE                (104U)
#define INA_ES_RUNTIME              (105U)
#define INA_ES_SCALE                (106U)
#define INA_ES_SCREEN               (107U)
#define INA_ES_SCRIPT               (108U)
#define INA_ES_SEQUENCE             (109U)
#define INA_ES_SERIALIZATION        (110U)
#define INA_ES_SERVER               (111U)
#define INA_ES_SERVICE              (112U)
#define INA_ES_SEEK                 (112U)
#define INA_ES_SIZE                 (113U)
#define INA_ES_SOCKET               (114U)
#define INA_ES_SOFTWARE             (115U)
#define INA_ES_SOURCE               (116U)
#define INA_ES_SPACE                (117U)
#define INA_ES_STACK                (118U)
#define INA_ES_STACKTRACE           (119U)
#define INA_ES_STREAM               (120U)
#define INA_ES_STREAMING            (121U)
#define INA_ES_STRING               (122U)
#define INA_ES_STRUCT               (123U)
#define INA_ES_SUBSYSTEM            (124U)
#define INA_ES_SYSTEM               (125U)
#define INA_ES_TEXT                 (126U)
#define INA_ES_TIME                 (127U)
#define INA_ES_TRANSLATION          (128U)
#define INA_ES_TRANSPORT            (129U)
#define INA_ES_TYPE                 (131U)
#define INA_ES_USER                 (134U)
#define INA_ES_USERNAME             (135U)
#define INA_ES_VALUE                (146U)
#define INA_ES_VECTOR               (147U)
#define INA_ES_VERSION              (148U)
#define INA_ES_DECOMPRESSION        (149U)
#define INA_ES_STATE                (150U)
#define INA_ES_DUMP                 (151U)
#define INA_ES_CHAR                 (152U)
#define INA_ES_CONFIGURATION        (153U)
#define INA_ES_SECTION              (154U)
#define INA_ES_KEY                  (155U)
#define INA_ES_ENUMERATION          (156U)
#define INA_ES_READ                 (157U)
#define INA_ES_WRITE                (158U)
#define INA_ES_OPTION               (159U)
#define INA_ES_BUFFER               (160U)
#define INA_ES_ADDRESS              (161U)
#define INA_ES_NAME                 (162U)
#define INA_ES_MAC                  (163U)
#define INA_ES_PROCESS              (164U)
#define INA_ES_PATTERN              (165U)
#define INA_ES_MUTEX                (166U)
#define INA_ES_SEMAPHORE            (167U)
#define INA_ES_THREAD               (168U)
#define INA_ES_CRON                 (169U)
#define INA_ES_VIOLATION            (170U)

#define INA_ES_USER_DEFINED         (1024UL)

typedef const char* (*ina_err_subject_cb_t)(int);
typedef struct ina_log_s ina_log_t;

/*
 * Initialize error module.
 *
 * Returns
 *  INA_SUCCESS if all went well.
 */
INA_API(ina_rc_t) ina_err_init(void);

/*
 * Destroy error module.
 */
INA_API(void) ina_err_destroy(void);

/*
 * Register an error subject dictionary.
 *
 * Parameters
 *  cb  Dictionary callback
 *
 * Returns
 *  Previously registered dictionary callback
 */
INA_API(ina_err_subject_cb_t) ina_err_register_dict(ina_err_subject_cb_t cb);

/*
 * Set RC
 *
 * Parameters
 *   rc         Return code
 *   location   source location
 *
 * Return
 *   Last RC
 */
INA_INLINE ina_rc_t ina_err_set_last_rc(ina_rc_t rc)
{
    __rc = rc;
    return __rc;
}

/*
 * Return the last RC
 *
 * Return
 *  Last RC
 */
INA_INLINE ina_rc_t ina_err_get_last_rc(void)
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
 *  Returns cleared RC
 */
INA_INLINE ina_rc_t ina_err_clear_rc(ina_rc_t rc)
{
    return (rc & ~(INA_ERR_ERROR));
}

/*
 * Mark an error as handled.
 *
 * Parameters
 *  rc  Valid RC to mark as handled. If a error was already marked as handled
 *      no error occurs.
 *
 * Return
 *  Returns INA_SUCCESS when the complete error state was cleared successfully
 *  otherwise
 */
INA_INLINE ina_rc_t ina_err_reset(void)
{
    __rc = ina_err_clear_rc(__rc);
    return __rc;
}

/*
 * Set log file.
 *
 * Parameters
 *  log   log context
 */
INA_API(void) ina_err_set_log(ina_log_t *log);

/*
 * Write to the error log
 *
 *  Parameters
 *   fmt  format
 *   ...  arguments
 */
INA_API(void) ina_err_log(const char *fmt, ...);
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
#  define INA_RC_PACK(x, e) (INA_ERR_ERROR | (((ina_rc_t)INA_VERSION_HEX) << INA_RC_BIT_R) | (((e)) << INA_RC_BIT_O) | (x))
#else
#  ifndef INA_ERROR_VER
#    define INA_ERROR_VER (0)
#  endif
#  ifndef INA_ERROR_REV
#    define INA_ERROR_REV (0)
#  endif
#  define INA_RC_PACK(x, e) (INA_ERR_ERROR | (((ina_rc_t)INA_ERROR_VER) << INA_RC_BIT_V) | (((ina_rc_t)INA_ERROR_REV) << INA_RC_BIT_R) | (((uint32_t)(e)) << INA_RC_BIT_O) | (x))
#endif

/* Check return code: failure */
#define INA_FAILED(rc) ((rc)&INA_ERR_ERROR)
/* Check return code: successful or handled */
#define INA_SUCCEED(rc) (!(INA_FAILED((rc))))

/* Set last RC */
#define INA_ERROR(x) ina_err_set_last_rc(INA_RC_PACK((x), 0UL))
/* Set last RC and capture errno */
#ifndef INA_OS_WIN32
#define INA_OS_ERROR(x) ina_err_set_last_rc(INA_RC_PACK((x), errno))
#else
#define INA_OS_ERROR(x) ina_err_set_last_rc(INA_RC_PACK((x), GetLastError()))
#endif
/* Set last RC and set user defined errno */
#define INA_USR_ERROR(x,e) ina_err_set_last_rc(INA_RC_PACK((x), (e)))

/* Return with last rc if condition x fails */
#define INA_RETURN_IF(x) do {if ((x)) return ina_err_get_last_rc(); } while(0)
/* Return with last rc if x == NULL */
#define INA_RETURN_IF_NULL(x) do {if ((x) == NULL) return ina_err_get_last_rc();} while(0)
/* Return with last rc if failed */
#define INA_RETURN_IF_FAILED(rc) if (INA_FAILED((rc))) return ina_err_get_last_rc()
/* Return with last rc if succeed */
#define INA_RETURN_IF_SUCCEED(rc) if (INA_SUCCEED((rc))) return ina_err_get_last_rc()
/* Checkpoint must succeed */
#define INA_MUST_SUCCEED(rc) do { if (INA_UNLIKELY(INA_FAILED(rc))) abort(); } while(0)

#ifndef INA_VERIFY_DISABLED
#define INA_VERIFY(x) do { if (INA_UNLIKELY((x))) return INA_ERROR(INA_ES_ARGUMENT|INA_ERR_INVALID); } while (0)
#define INA_VERIFY_NOT_NULL(x) INA_VERIFY((x) == NULL)
#else
#define INA_VERIFY_NOT_NULL(x) INA_ASSERT_NOTNULL((x))
#define INA_VERIFY(x) INA_ASSERT_TRUE((x))
#endif

#ifdef __cplusplus
}
#endif

#endif
