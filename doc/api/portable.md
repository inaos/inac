
```C
#ifndef _LIBINAC_PORTABLE_H_
```

Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
#if defined __ECC || defined __ICC || defined __INTEL_COMPILER
```

Determine compilation environment

```C
#  define INA_COMPILER_APPLECC 1
```
we don't define the compiler string here, let it be GNU
```C
#if defined linux || defined __linux__
```

Determine target operating system

```C
#if defined __WATCOMC__  && defined __386__ && defined __DOS__
```
NOTE: make sure you use /bt=DOS if compiling for 32-bit DOS,
otherwise Watcom assumes host=target
```C
#if defined GEKKO
```

Determine target CPU

```C
#if !defined INA_OS_STRING
```

Attempt to autodetect building for embedded on Sony PS2

```C
#if defined INA_CPU_X86 && !defined INA_CPU_X86_64
```

Handle cdecl, stdcall, fastcall, etc.

```C
#ifndef __dead
```

Handle some useful macros

```C
#ifdef __cplusplus
```
If your compiler supports the inline keyword in C, INA_INLINE is
defined to `inline', otherwise empty. In C++, the inline is always
supported.
```C
#if defined INA_EXPORT
```

Define INA_EXPORT signature based on INA_DLL and INA_LIB (only Windows)

```C
#      if defined __GNUC__ || defined __WATCOMC__ || defined __MWERKS__
```
for all other compilers, we're just making a blanket assumption
```C
#if !defined INA_EXPORT
```
On pretty much everything else, we can thankfully just ignore this
```C
#ifdef INA_API
```

(Re)define INA_API export signature

```C
#if defined INA_CPU_X86 || defined INA_CPU_AXP || defined INA_CPU_STRONGARM || defined INA_OS_WIN32 || defined INA_OS_WINCE || defined __MIPSEL__
```

Try to infer endianess.  Basically we just go through the CPUs we know are
little endian, and assume anything that isn't one of those is big endian.
As a sanity check, we also do this with operating systems we know are
little endian, such as Windows.  Some processors are bi-endian, such as
the MIPS series, so we have to be careful about those.

```C
#define INA_CASSERT(name, x) typedef int _INA_dummy_## name[(x) ? 1 : -1 ]
```

----------------------------------------------------------------------------
Cross-platform compile time assertion macro
----------------------------------------------------------------------------

```C
#if ((defined(__STDC__) && __STDC__ && __STDC_VERSION__ >= 199901L) || (defined (__WATCOMC__) && (defined (_STDINT_H_INCLUDED) || __WATCOMC__ >= 1250)) || (defined(__GNUC__) && (defined(_STDINT_H) || defined(_STDINT_H_) || defined (__UINT_FAST64_TYPE__)) )) && !defined (_PSTDINT_H_INCLUDED)
```

----------------------------------------------------------------------------
Cross-platform numeric types
----------------------------------------------------------------------------

```C
# if defined (__WATCOMC__) && __WATCOMC__ >= 1250
```

Something really weird is going on with Open Watcom.  Just pull some of
these duplicated definitions from Open Watcom's stdint.h file for now.

```C
#ifndef UINT8_MAX
```

Deduce the type assignments from limits.h under the assumption that
integer sizes in bits are powers of 2, and follow the ANSI
definitions.

```C
#ifndef INT32_C
```
typedef signed long int32_t;
```C
#  ifndef UINT64_C
```
__extension__ typedef long long int64_t;
__extension__ typedef unsigned long long uint64_t;
```C
#ifndef PRINTF_INT64_HEX_WIDTH
```

Width of hexadecimal for number field.

```C
/*
```

Because this file currently only supports platforms which have
precise powers of 2 as bit sizes for the default integers, the
least definitions are all trivial.  Its possible that a future
version of this file could have different definitions.

```C
# ifdef _STDINT_H_INCLUDED
```

The ANSI C committee pretending to know or specify anything about
performance is the epitome of misguided arrogance.  The mandate of
this file is to ONLY ever support that absolute minimum
definition of the fast integer types, for compatibility purposes.
No extensions, and no attempt to suggest what may or may not be a
faster integer type will ever be made in this file.  Developers are
warned to stay away from these types when using this or any other
stdint.h.

```C
#if defined(__WATCOMC__) || defined(_MSC_VER) || defined (__GNUC__)
```

Whatever piecemeal, per compiler thing we can do about the wchar_t
type limits.

```C
#if defined (_MSC_VER) && defined (_UINTPTR_T_DEFINED)
```

Whatever piecemeal, per compiler/platform thing we can do about the
(u)intptr_t types and limits.

```C
# endif
```
TODO -- what did Intel do about x86-64?
```C
# else
```
typedef stdint_intptr_glue3(uint,stdint_intptr_bits,_t) uintptr_t;
typedef stdint_intptr_glue3( int,stdint_intptr_bits,_t)  intptr_t;
```C
typedef ptrdiff_t intptr_t;# endif
```
TODO -- This following is likely wrong for some platforms, and does
nothing for the definition of uintptr_t.
```C
#ifndef SIG_ATOMIC_MAX
```

Assumes sig_atomic_t is signed and we have a 2s complement machine.

```C
#ifdef INA_OS_WIN32
```
Pack
```C
#ifdef INA_OS_WIN32
```
Atomic operations
```C
#ifdef INA_OS_WIN32
```
Branch prediction hints
```C
#ifdef INA_OS_WIN32
```
C99 restrict a.k.a. pointer aliasing hint
```C
#ifdef INA_OS_WIN32
```
byte swapping
```C
#ifdef INA_OS_WIN32
```
FD for net.h
```C
#define S_ISUID      0x08000000   /* does nothing  */
```
If STRICT_UGO_PERMISSIONS is not defined, then setting Read for any
of User, Group, or Other will set Read for User and setting Write
will set Write for User.  Otherwise, Read and Write for Group and
Other are ignored.

For the POSIX modes that do not have a Windows equivalent, the modes
defined here use the POSIX values left shifted 16 bits.

```C
#ifndef INA_TLS
```

thread-local-safe variable
