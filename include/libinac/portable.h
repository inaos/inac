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
#ifndef _LIBINAC_PORTABLE_H_
#define _LIBINAC_PORTABLE_H_

#include <stddef.h>
#include <limits.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Determine compilation environment
 */
#if defined __ECC || defined __ICC || defined __INTEL_COMPILER
#  define INA_COMPILER_STRING "Intel C/C++"
#  define INA_COMPILER_INTEL 1
#endif

#if ( defined __host_mips || defined __sgi ) && !defined __GNUC__
#  define INA_COMPILER_STRING    "MIPSpro C/C++"
#  define INCA_COMPILER_MIPSPRO 1 
#endif

#if defined __hpux && !defined __GNUC__
#  define INA_COMPILER_STRING "HP-UX CC"
#  define INA_COMPILER_HPCC 1 
#endif

#if defined __GNUC__
#  define INA_COMPILER_STRING "Gnu GCC"
#  define INA_COMPILER_GCC 1
#endif

#if defined __APPLE_CC__
   /* we don't define the compiler string here, let it be GNU */
#  define INA_COMPILER_APPLECC 1
#endif

#if defined __IBMC__ || defined __IBMCPP__
#  define INA_COMPILER_STRING "IBM C/C++"
#  define INA_COMPILER_IBM 1
#endif

#if defined _MSC_VER
#  define INA_COMPILER_STRING "Microsoft Visual C++"
#  define INA_COMPILER_MSVC 1
#endif

#if defined __SUNPRO_C
#  define INA_COMPILER_STRING "Sun Pro" 
#  define INA_COMPILER_SUN 1
#endif

#if defined __BORLANDC__
#  define INA_COMPILER_STRING "Borland C/C++"
#  define INA_COMPILER_BORLAND 1
#endif

#if defined __MWERKS__
#  define INA_COMPILER_STRING     "MetroWerks CodeWarrior"
#  define INA_COMPILER_METROWERKS 1
#endif

#if defined __DECC || defined __DECCXX
#  define INA_COMPILER_STRING "Compaq/DEC C/C++"
#  define INA_COMPILER_DEC 1
#endif

#if defined __WATCOMC__
#  define INA_COMPILER_STRING "Watcom C/C++"
#  define INA_COMPILER_WATCOM 1
#endif

#if !defined INA_COMPILER_STRING
#  define INA_COMPILER_STRING "Unknown compiler"
#  warning "Unkown compiler"
#endif

/*
 * Determine target operating system
 */
#if defined linux || defined __linux__
#  define INA_OS_LINUX 1 
#  define INA_OS_STRING "Linux"
#endif

#if defined __CYGWIN32__
#  define INA_OS_CYGWIN32 1
#  define INA_OS_STRING "Cygwin"
#endif

#if defined GEKKO
#  define INA_OS_GAMECUBE
#  define __powerpc__
#  define INA_OS_STRING "GameCube"
#endif

#if defined __MINGW32__
#  define INA_OS_MINGW 1
#  define INA_OS_STRING "MinGW"
#endif

#if defined GO32 && defined DJGPP && defined __MSDOS__ 
#  define INA_OS_GO32 1
#  define INA_OS_STRING "GO32/MS-DOS"
#endif

/* NOTE: make sure you use /bt=DOS if compiling for 32-bit DOS,
   otherwise Watcom assumes host=target */
#if defined __WATCOMC__  && defined __386__ && defined __DOS__
#  define INA_OS_DOS32 1
#  define INA_OS_STRING "DOS/32-bit"
#endif

#if defined _UNICOS
#  define INA_OS_UNICOS 1
#  define INA_OS_STRING "UNICOS"
#endif

#if ( defined __MWERKS__ && defined __powerc && !defined macintosh ) || defined __APPLE_CC__ || defined macosx
#  define INA_OS_OSX 1
#  define INA_OS_STRING "MacOS X"
#endif

#if defined __sun__ || defined sun || defined __sun || defined __solaris__
#  if defined __SVR4 || defined __svr4__ || defined __solaris__
#     define INA_OS_STRING "Solaris"
#     define INA_OS_SOLARIS 1
#  endif
#  if !defined INA_OS_STRING
#     define INA_OS_STRING "SunOS"
#     define INA_OS_SUNOS 1
#  endif
#endif

#if defined __sgi__ || defined sgi || defined __sgi
#  define INA_OS_IRIX 1
#  define INA_OS_STRING "Irix"
#endif

#if defined __hpux__ || defined __hpux
#  define INA_OS_HPUX 1
#  define INA_OS_STRING "HP-UX"
#endif

#if defined _AIX
#  define INA_OS_AIX 1
#  define INA_OS_STRING "AIX"
#endif

#if ( defined __alpha && defined __osf__ )
#  define INA_OS_TRU64 1
#  define INA_OS_STRING "Tru64"
#endif

#if defined __BEOS__ || defined __beos__
#  define INA_OS_BEOS 1
#  define INA_OS_STRING "BeOS"
#endif

#if defined amiga || defined amigados || defined AMIGA || defined _AMIGA
#  define INA_OS_AMIGA 1
#  define INA_OS_STRING "Amiga"
#endif

#if defined __unix__
#  define INA_OS_UNIX 1 
#  if !defined INA_OS_STRING
#     define INA_OS_STRING "Unix-like(generic)"
#  endif
#endif

#if defined _WIN32_WCE
#  define INA_OS_WINCE 1
#  define INA_OS_STRING "Windows CE"
#endif

#if defined _XBOX
#  define INA_OS_XBOX 1
#  define INA_OS_STRING "XBOX"
#endif

#if defined _WIN32 || defined WIN32 || defined __NT__ || defined __WIN32__
#  define INA_OS_WIN32 1
#  if !defined INA_OS_XBOX
#     if defined _WIN64
#        define INA_OS_WIN64 1
#        define INA_OS_STRING "Win64"
#     else
#        if !defined INA_OS_STRING
#           define INA_OS_STRING "Win32"
#        endif
#     endif
#  endif
#endif

#if defined __palmos__
#  define INA_OS_PALM 1
#  define INA_OS_STRING "PalmOS"
#endif

#if defined THINK_C || defined macintosh
#  define INA_OS_MACOS 1
#  define INA_OS_STRING "MacOS"
#endif

/*
 * Determine target CPU
 */
#if defined GEKKO
#  define INA_CPU_PPC750 1
#  define INA_CPU_STRING "IBM PowerPC 750 (NGC)"
#endif

#if defined mc68000 || defined m68k || defined __MC68K__ || defined m68000
#  define INA_CPU_68K 1
#  define INA_CPU_STRING "MC68000"
#endif

#if defined __PPC__ || defined __POWERPC__  || defined powerpc || defined _POWER || defined __ppc__ || defined __powerpc__
#  define INA_CPU_PPC 1
#  if !defined INA_CPU_STRING
#    if defined __powerpc64__
#       define INA_CPU_STRING "PowerPC64"
#    else
#       define INA_CPU_STRING "PowerPC"
#    endif
#  endif
#endif

#if defined _CRAYT3E || defined _CRAYMPP
#  define INA_CPU_CRAYT3E 1 /* target processor is a DEC Alpha 21164 used in a Cray T3E*/
#  define INA_CPU_STRING "Cray T3E (Alpha 21164)"
#endif

#if defined CRAY || defined _CRAY && !defined _CRAYT3E
#  error Non-AXP Cray systems not supported
#endif

#if defined _SH3
#  define INA_CPU_SH3 1
#  define INA_CPU_STRING "Hitachi SH-3"
#endif

#if defined __sh4__ || defined __SH4__
#  define INA_CPU_SH3 1
#  define INA_CPU_SH4 1
#  define INA_CPU_STRING "Hitachi SH-4"
#endif

#if defined __sparc__ || defined __sparc
#  if defined __arch64__ || defined __sparcv9 || defined __sparc_v9__
#     define INA_CPU_SPARC64 1 
#     define INA_CPU_STRING "Sparc/64"
#  else
#     define INA_CPU_STRING "Sparc/32"
#  endif
#  define INA_CPU_SPARC 1
#endif

#if defined ARM || defined __arm__ || defined _ARM
#  define INA_CPU_STRONGARM 1
#  define INA_CPU_STRING "ARM"
#endif

#if defined mips || defined __mips__ || defined __MIPS__ || defined _MIPS
#  define INA_CPU_MIPS 1 
#  if defined _R5900
#    define INA_CPU_STRING "MIPS R5900 (PS2)"
#  else
#    define INA_CPU_STRING "MIPS"
#  endif
#endif

#if defined __ia64 || defined _M_IA64 || defined __ia64__ 
#  define INA_CPU_IA64 1
#  define INA_CPU_STRING "IA64"
#endif

#if defined __X86__ || defined __i386__ || defined i386 || defined _M_IX86 || defined __386__ || defined __x86_64__ || defined _M_X64
#  define INA_CPU_X86 1
#  if defined __x86_64__ || defined _M_X64
#     define INA_CPU_X86_64 1 
#  endif
#  if defined INA_CPU_X86_64
#     define INA_CPU_STRING "AMD x86-64"
#  else
#     define INA_CPU_STRING "Intel 386+"
#  endif
#endif

#if defined __alpha || defined alpha || defined _M_ALPHA || defined __alpha__
#  define INA_CPU_AXP 1
#  define INA_CPU_STRING "AXP"
#endif

#if defined __hppa || defined hppa
#  define INA_CPU_HPPA 1
#  define INA_CPU_STRING "PA-RISC"
#endif

#if !defined INA_CPU_STRING
#  error INA cannot determine target CPU
#  define INA_CPU_STRING "Unknown" /* this is here for Doxygen's benefit */
#endif

/*
 * Attempt to autodetect building for embedded on Sony PS2
 */
#if !defined INA_OS_STRING
#  define INA_OS_EMBEDDED 1 
#  if defined _R5900
#     define INA_OS_STRING "Sony PS2(embedded)"
#  else
#     define INA_OS_STRING "Embedded/Unknown"
#  endif
#endif

/*
 * Handle cdecl, stdcall, fastcall, etc.
 */
#if defined INA_CPU_X86 && !defined INA_CPU_X86_64
#  if defined __GNUC__
#     define INA_CDECL __attribute__((cdecl))
#     define INA_STDCALL __attribute__((stdcall))
#     define INA_FASTCALL __attribute__((fastcall))
#  elif ( defined _MSC_VER || defined __WATCOMC__ || defined __BORLANDC__ || defined __MWERKS__ )
#     define INA_CDECL    __cdecl
#     define INA_STDCALL  __stdcall
#     define INA_FASTCALL __fastcall
#  endif
#else
#  define INA_CDECL
#  define INA_STDCALL
#  define INA_FASTCALL 
#endif

/*
 * Handle some useful macros
 */
#ifndef __dead
#  define __dead
#endif

/* If your compiler supports the inline keyword in C, INA_INLINE is
   defined to `inline', otherwise empty. In C++, the inline is always
   supported. */
#ifdef __cplusplus
#  ifdef INA_COMPILER_MSVC
#    define INA_INLINE __inline
#  else
#   define INA_INLINE inline
#  endif
#else
#  ifdef INA_COMPILER_MSVC
#    define INA_INLINE __inline
#  else
#    ifdef INLINE_ENABLED
#       define INA_INLINE inline
#    endif
#  endif
#endif
#ifndef INA_INLINE
#define INA_INLINE
#endif

/*
 * Define INA_EXPORT signature based on INA_DLL and INA_LIB (only Windows)
 */
#if defined INA_EXPORT
#  undef INA_EXPORT
#endif

#if defined INA_DLL
#   if defined INA_OS_WIN32
#      if defined _MSC_VER 
#         if ( _MSC_VER >= 800 )
#            if defined INA_LIB
#               define INA_EXPORT __declspec( dllexport )
#            else
#               define INA_EXPORT __declspec( dllimport )
#            endif
#         else
#            if defined INA_LIB
#               define INA_EXPORT __export
#            else
#               define INA_EXPORT 
#            endif
#         endif
#      endif  /* defined _MSC_VER */
#      if defined __BORLANDC__
#         if ( __BORLANDC__ >= 0x500 )
#            if defined INA_LIB 
#               define INA_EXPORT __declspec( dllexport )
#            else
#               define INA_EXPORT __declspec( dllimport )
#            endif
#         else
#            if defined INA_LIB
#               define INA_EXPORT __export
#            else
#               define INA_EXPORT 
#            endif
#         endif
#      endif /* defined __BORLANDC__ */
       /* for all other compilers, we're just making a blanket assumption */
#      if defined __GNUC__ || defined __WATCOMC__ || defined __MWERKS__
#         if defined INA_LIB
#            define INA_EXPORT __declspec( dllexport )
#         else
#            define INA_EXPORT __declspec( dllimport )
#         endif
#      endif /* all other compilers */
#      if !defined INA_EXPORT
#         error Building DLLs not supported on this compiler
#      endif
#   endif /* defined INA_OS_WIN32 */
#endif

/* On pretty much everything else, we can thankfully just ignore this */
#if !defined INA_EXPORT
#  define INA_EXPORT
#endif

/*
 * (Re)define INA_API export signature 
 */
#ifdef INA_API
#  undef INA_API
#endif

#if ( ( defined _MSC_VER ) && ( _MSC_VER < 800 ) ) || ( defined __BORLANDC__ && ( __BORLANDC__ < 0x500 ) )
#  if defined( __cplusplus__ )
#    define INA_API(rtype) extern "C" rtype INA_EXPORT
#  else
#    define INA_API(rtype) extern rtype INA_EXPORT 
#  endif
#else
#  ifdef __cplusplus__
#    define INA_API(rtype) extern "C" INA_EXPORT rtype
#  else
#    define INA_API(rtype) extern rtype INA_EXPORT 
#  endif
#endif

/*
 * Try to infer endianess.  Basically we just go through the CPUs we know are
 * little endian, and assume anything that isn't one of those is big endian.
 * As a sanity check, we also do this with operating systems we know are
 * little endian, such as Windows.  Some processors are bi-endian, such as 
 * the MIPS series, so we have to be careful about those.
*/
#if defined INA_CPU_X86 || defined INA_CPU_AXP || defined INA_CPU_STRONGARM || defined INA_OS_WIN32 || defined INA_OS_WINCE || defined __MIPSEL__
#  define INA_ENDIAN_STRING "little"
#  define INA_LITTLE_ENDIAN 1
#else
#  define INA_ENDIAN_STRING "big"
#  define INA_BIG_ENDIAN 1
#endif

/*
 * ----------------------------------------------------------------------------
 * Cross-platform compile time assertion macro
 * ----------------------------------------------------------------------------
 */
#define INA_CASSERT(name, x) typedef int _INA_dummy_## name[(x) ? 1 : -1 ]

/*
 * ----------------------------------------------------------------------------
 * Cross-platform numeric types
 * ----------------------------------------------------------------------------
 */
#if ((defined(__STDC__) && __STDC__ && __STDC_VERSION__ >= 199901L) || (defined (__WATCOMC__) && (defined (_STDINT_H_INCLUDED) || __WATCOMC__ >= 1250)) || (defined(__GNUC__) && (defined(_STDINT_H) || defined(_STDINT_H_) || defined (__UINT_FAST64_TYPE__)) )) && !defined (_PSTDINT_H_INCLUDED)
# ifndef PRINTF_INT64_MODIFIER
#  define PRINTF_INT64_MODIFIER "ll"
# endif
# ifndef PRINTF_INT32_MODIFIER
#  define PRINTF_INT32_MODIFIER "l"
# endif
# ifndef PRINTF_INT16_MODIFIER
#  define PRINTF_INT16_MODIFIER "h"
# endif
# ifndef PRINTF_INTMAX_MODIFIER
#  define PRINTF_INTMAX_MODIFIER PRINTF_INT64_MODIFIER
# endif
# ifndef PRINTF_INT64_HEX_WIDTH
#  define PRINTF_INT64_HEX_WIDTH "16"
# endif
# ifndef PRINTF_INT32_HEX_WIDTH
#  define PRINTF_INT32_HEX_WIDTH "8"
# endif
# ifndef PRINTF_INT16_HEX_WIDTH
#  define PRINTF_INT16_HEX_WIDTH "4"
# endif
# ifndef PRINTF_INT8_HEX_WIDTH
#  define PRINTF_INT8_HEX_WIDTH "2"
# endif
# ifndef PRINTF_INT64_DEC_WIDTH
#  define PRINTF_INT64_DEC_WIDTH "20"
# endif
# ifndef PRINTF_INT32_DEC_WIDTH
#  define PRINTF_INT32_DEC_WIDTH "10"
# endif
# ifndef PRINTF_INT16_DEC_WIDTH
#  define PRINTF_INT16_DEC_WIDTH "5"
# endif
# ifndef PRINTF_INT8_DEC_WIDTH
#  define PRINTF_INT8_DEC_WIDTH "3"
# endif
# ifndef PRINTF_INTMAX_HEX_WIDTH
#  define PRINTF_INTMAX_HEX_WIDTH PRINTF_INT64_HEX_WIDTH
# endif
# ifndef PRINTF_INTMAX_DEC_WIDTH
#  define PRINTF_INTMAX_DEC_WIDTH PRINTF_INT64_DEC_WIDTH
# endif

/*
 *  Something really weird is going on with Open Watcom.  Just pull some of
 *  these duplicated definitions from Open Watcom's stdint.h file for now.
 */
# if defined (__WATCOMC__) && __WATCOMC__ >= 1250
#  if !defined (INT64_C)
#   define INT64_C(x)   (x + (INT64_MAX - INT64_MAX))
#  endif
#  if !defined (UINT64_C)
#   define UINT64_C(x)  (x + (UINT64_MAX - UINT64_MAX))
#  endif
#  if !defined (INT32_C)
#   define INT32_C(x)   (x + (INT32_MAX - INT32_MAX))
#  endif
#  if !defined (UINT32_C)
#   define UINT32_C(x)  (x + (UINT32_MAX - UINT32_MAX))
#  endif
#  if !defined (INT16_C)
#   define INT16_C(x)   (x)
#  endif
#  if !defined (UINT16_C)
#   define UINT16_C(x)  (x)
#  endif
#  if !defined (INT8_C)
#   define INT8_C(x)   (x)
#  endif
#  if !defined (UINT8_C)
#   define UINT8_C(x)  (x)
#  endif
#  if !defined (UINT64_MAX)
#   define UINT64_MAX  18446744073709551615ULL
#  endif
#  if !defined (INT64_MAX)
#   define INT64_MAX  9223372036854775807LL
#  endif
#  if !defined (UINT32_MAX)
#   define UINT32_MAX  4294967295UL
#  endif
#  if !defined (INT32_MAX)
#   define INT32_MAX  2147483647L
#  endif
#  if !defined (INTMAX_MAX)
#   define INTMAX_MAX INT64_MAX
#  endif
#  if !defined (INTMAX_MIN)
#   define INTMAX_MIN INT64_MIN
#  endif
# endif
#endif

#ifndef SIZE_MAX
# define SIZE_MAX (~(size_t)0)
#endif

/*
 *  Deduce the type assignments from limits.h under the assumption that
 *  integer sizes in bits are powers of 2, and follow the ANSI
 *  definitions.
 */
/*
#ifndef UINT8_MAX
# define UINT8_MAX 0xff
#endif
#ifndef uint8_t
# if (UCHAR_MAX == UINT8_MAX) || defined (S_SPLINT_S)
    typedef unsigned char uint8_t;
#   define UINT8_C(v) ((uint8_t) v)
# else
#   error "Platform not supported"
# endif
#endif

#ifndef INT8_MAX
# define INT8_MAX 0x7f
#endif
#ifndef INT8_MIN
# define INT8_MIN INT8_C(0x80)
#endif
#ifndef int8_t
# if (SCHAR_MAX == INT8_MAX) || defined (S_SPLINT_S)
    typedef signed char int8_t;
#   define INT8_C(v) ((int8_t) v)
# else
#   error "Platform not supported"
# endif
#endif

#ifndef UINT16_MAX
# define UINT16_MAX 0xffff
#endif
#ifndef uint16_t
#if (UINT_MAX == UINT16_MAX) || defined (S_SPLINT_S)
  typedef unsigned int uint16_t;
# ifndef PRINTF_INT16_MODIFIER
#  define PRINTF_INT16_MODIFIER ""
# endif
# define UINT16_C(v) ((uint16_t) (v))
#elif (USHRT_MAX == UINT16_MAX)
  typedef unsigned short uint16_t;
# define UINT16_C(v) ((uint16_t) (v))
# ifndef PRINTF_INT16_MODIFIER
#  define PRINTF_INT16_MODIFIER "h"
# endif
#else
#error "Platform not supported"
#endif
#endif

#ifndef INT16_MAX
# define INT16_MAX 0x7fff
#endif
#ifndef INT16_MIN
# define INT16_MIN INT16_C(0x8000)
#endif
#ifndef int16_t
#if (INT_MAX == INT16_MAX) || defined (S_SPLINT_S)
  typedef signed int int16_t;
# define INT16_C(v) ((int16_t) (v))
# ifndef PRINTF_INT16_MODIFIER
#  define PRINTF_INT16_MODIFIER ""
# endif
#elif (SHRT_MAX == INT16_MAX)
  typedef signed short int16_t;
# define INT16_C(v) ((int16_t) (v))
# ifndef PRINTF_INT16_MODIFIER
#  define PRINTF_INT16_MODIFIER "h"
# endif
#else
#error "Platform not supported"
#endif
#endif

#ifndef UINT32_MAX
# define UINT32_MAX (0xffffffffUL)
#endif
#ifndef uint32_t
#if (ULONG_MAX == UINT32_MAX) || defined (S_SPLINT_S)
  typedef unsigned long uint32_t;
# define UINT32_C(v) v ## UL
# ifndef PRINTF_INT32_MODIFIER
#  define PRINTF_INT32_MODIFIER "l"
# endif
#elif (UINT_MAX == UINT32_MAX)
  typedef unsigned int uint32_t;
# ifndef PRINTF_INT32_MODIFIER
#  define PRINTF_INT32_MODIFIER ""
# endif
# define UINT32_C(v) v ## U
#elif (USHRT_MAX == UINT32_MAX)
  typedef unsigned short uint32_t;
# define UINT32_C(v) ((unsigned short) (v))
# ifndef PRINTF_INT32_MODIFIER
#  define PRINTF_INT32_MODIFIER ""
# endif
#else
#error "Platform not supported"
#endif
#endif

#ifndef INT32_MAX
# define INT32_MAX (0x7fffffffL)
#endif
#ifndef INT32_MIN
# define INT32_MIN INT32_C(0x80000000)
#endif
#ifndef int32_t
#if (LONG_MAX == INT32_MAX) || defined (S_SPLINT_S)
  typedef signed long int32_t;
# define INT32_C(v) v ## L
# ifndef PRINTF_INT32_MODIFIER
#  define PRINTF_INT32_MODIFIER "l"
# endif
#elif (INT_MAX == INT32_MAX)
  typedef signed int int32_t;
# define INT32_C(v) v
# ifndef PRINTF_INT32_MODIFIER
#  define PRINTF_INT32_MODIFIER ""
# endif
#elif (SHRT_MAX == INT32_MAX)
  typedef signed short int32_t;
# define INT32_C(v) ((short) (v))
# ifndef PRINTF_INT32_MODIFIER
#  define PRINTF_INT32_MODIFIER ""
# endif
#else
#error "Platform not supported"
#endif
#endif
*/
/*
 *  The macro stdint_int64_defined is temporarily used to record
 *  whether or not 64 integer support is available.  It must be
 *  defined for any 64 integer extensions for new platforms that are
 *  added.
 */
 /*
#undef stdint_int64_defined
#if (defined(__STDC__) && defined(__STDC_VERSION__)) || defined (S_SPLINT_S)
# if (__STDC__ && __STDC_VERSION__ >= 199901L) || defined (S_SPLINT_S)
#  define stdint_int64_defined
   typedef long long int64_t;
   typedef unsigned long long uint64_t;
#  define UINT64_C(v) v ## ULL
#  define  INT64_C(v) v ## LL
#  ifndef PRINTF_INT64_MODIFIER
#   define PRINTF_INT64_MODIFIER "ll"
#  endif
# endif
#endif*/

/*#if !defined (stdint_int64_defined)
# if defined(__GNUC__)
#  define stdint_int64_defined
   __extension__ typedef long long int64_t;
   __extension__ typedef unsigned long long uint64_t;
#  ifndef UINT64_C
#    define UINT64_C(v) v ## ULL
#  endif
#  ifndef INT64_C
#    define  INT64_C(v) v ## LL
#  endif
#  ifndef PRINTF_INT64_MODIFIER
#   define PRINTF_INT64_MODIFIER "ll"
#  endif
# elif defined(__MWERKS__) || defined (__SUNPRO_C) || defined (__SUNPRO_CC) || defined (__APPLE_CC__) || defined (_LONG_LONG) || defined (_CRAYC) || defined (S_SPLINT_S)
#  define stdint_int64_defined
   typedef long long int64_t;
   typedef unsigned long long uint64_t;
#  ifndef UINT64_C
#    define UINT64_C(v) v ## ULL
#  endif
#  ifndef INT64_C
#    define  INT64_C(v) v ## LL
#  endif
#  ifndef PRINTF_INT64_MODIFIER
#   define PRINTF_INT64_MODIFIER "ll"
#  endif
# elif (defined(__WATCOMC__) && defined(__WATCOM_INT64__)) || (defined(_MSC_VER) && _INTEGRAL_MAX_BITS >= 64) || (defined (__BORLANDC__) && __BORLANDC__ > 0x460) || defined (__alpha) || defined (__DECC)
#  define stdint_int64_defined
   typedef __int64 int64_t;
   typedef unsigned __int64 uint64_t;
#  ifndef UINT64_C
#    define UINT64_C(v) v ## UI64
#  endif
#  ifndef INT64_C
#    define  INT64_C(v) v ## I64
#  endif
#  ifndef PRINTF_INT64_MODIFIER
#   define PRINTF_INT64_MODIFIER "I64"
#  endif
# endif
#endif

#if !defined (LONG_LONG_MAX) && defined (INT64_C)
# define LONG_LONG_MAX INT64_C (9223372036854775807)
#endif
#ifndef ULONG_LONG_MAX
# define ULONG_LONG_MAX UINT64_C (18446744073709551615)
#endif

#if !defined (INT64_MAX) && defined (INT64_C)
# define INT64_MAX INT64_C (9223372036854775807)
#endif
#if !defined (INT64_MIN) && defined (INT64_C)
# define INT64_MIN INT64_C (-9223372036854775808)
#endif
#if !defined (UINT64_MAX) && defined (INT64_C)
# define UINT64_MAX UINT64_C (18446744073709551615)
#endif
*/
/*
 *  Width of hexadecimal for number field.
 */

#ifndef PRINTF_INT64_HEX_WIDTH
# define PRINTF_INT64_HEX_WIDTH "16"
#endif
#ifndef PRINTF_INT32_HEX_WIDTH
# define PRINTF_INT32_HEX_WIDTH "8"
#endif
#ifndef PRINTF_INT16_HEX_WIDTH
# define PRINTF_INT16_HEX_WIDTH "4"
#endif
#ifndef PRINTF_INT8_HEX_WIDTH
# define PRINTF_INT8_HEX_WIDTH "2"
#endif

#ifndef PRINTF_INT64_DEC_WIDTH
# define PRINTF_INT64_DEC_WIDTH "20"
#endif
#ifndef PRINTF_INT32_DEC_WIDTH
# define PRINTF_INT32_DEC_WIDTH "10"
#endif
#ifndef PRINTF_INT16_DEC_WIDTH
# define PRINTF_INT16_DEC_WIDTH "5"
#endif
#ifndef PRINTF_INT8_DEC_WIDTH
# define PRINTF_INT8_DEC_WIDTH "3"
#endif

/*
 *  Ok, lets not worry about 128 bit integers for now.  Moore's law says
 *  we don't need to worry about that until about 2040 at which point
 *  we'll have bigger things to worry about.
 */
/*
#ifdef stdint_int64_defined
  typedef int64_t intmax_t;
  typedef uint64_t uintmax_t;
# define  INTMAX_MAX   INT64_MAX
# define  INTMAX_MIN   INT64_MIN
# define UINTMAX_MAX  UINT64_MAX
# define UINTMAX_C(v) UINT64_C(v)
# define  INTMAX_C(v)  INT64_C(v)
# ifndef PRINTF_INTMAX_MODIFIER
#   define PRINTF_INTMAX_MODIFIER PRINTF_INT64_MODIFIER
# endif
# ifndef PRINTF_INTMAX_HEX_WIDTH
#  define PRINTF_INTMAX_HEX_WIDTH PRINTF_INT64_HEX_WIDTH
# endif
# ifndef PRINTF_INTMAX_DEC_WIDTH
#  define PRINTF_INTMAX_DEC_WIDTH PRINTF_INT64_DEC_WIDTH
# endif
#else
  typedef int32_t intmax_t;
  typedef uint32_t uintmax_t;
# define  INTMAX_MAX   INT32_MAX
# define UINTMAX_MAX  UINT32_MAX
# define UINTMAX_C(v) UINT32_C(v)
# define  INTMAX_C(v)  INT32_C(v)
# ifndef PRINTF_INTMAX_MODIFIER
#   define PRINTF_INTMAX_MODIFIER PRINTF_INT32_MODIFIER
# endif
# ifndef PRINTF_INTMAX_HEX_WIDTH
#  define PRINTF_INTMAX_HEX_WIDTH PRINTF_INT32_HEX_WIDTH
# endif
# ifndef PRINTF_INTMAX_DEC_WIDTH
#  define PRINTF_INTMAX_DEC_WIDTH PRINTF_INT32_DEC_WIDTH
# endif
#endif
*/

/*
 *  Because this file currently only supports platforms which have
 *  precise powers of 2 as bit sizes for the default integers, the
 *  least definitions are all trivial.  Its possible that a future
 *  version of this file could have different definitions.
 */

/*#ifndef stdint_least_defined
  typedef   int8_t   int_least8_t;
  typedef  uint8_t  uint_least8_t;
  typedef  int16_t  int_least16_t;
  typedef uint16_t uint_least16_t;
  typedef  int32_t  int_least32_t;
  typedef uint32_t uint_least32_t;
# define PRINTF_LEAST32_MODIFIER PRINTF_INT32_MODIFIER
# define PRINTF_LEAST16_MODIFIER PRINTF_INT16_MODIFIER
# define  UINT_LEAST8_MAX  UINT8_MAX
# define   INT_LEAST8_MAX   INT8_MAX
# define UINT_LEAST16_MAX UINT16_MAX
# define  INT_LEAST16_MAX  INT16_MAX
# define UINT_LEAST32_MAX UINT32_MAX
# define  INT_LEAST32_MAX  INT32_MAX
# define   INT_LEAST8_MIN   INT8_MIN
# define  INT_LEAST16_MIN  INT16_MIN
# define  INT_LEAST32_MIN  INT32_MIN
# ifdef stdint_int64_defined
    typedef  int64_t  int_least64_t;
    typedef uint64_t uint_least64_t;
#   define PRINTF_LEAST64_MODIFIER PRINTF_INT64_MODIFIER
#   define UINT_LEAST64_MAX UINT64_MAX
#   define  INT_LEAST64_MAX  INT64_MAX
#   define  INT_LEAST64_MIN  INT64_MIN
# endif
#endif
#undef stdint_least_defined
*/
/*
 *  The ANSI C committee pretending to know or specify anything about
 *  performance is the epitome of misguided arrogance.  The mandate of
 *  this file is to *ONLY* ever support that absolute minimum
 *  definition of the fast integer types, for compatibility purposes.
 *  No extensions, and no attempt to suggest what may or may not be a
 *  faster integer type will ever be made in this file.  Developers are
 *  warned to stay away from these types when using this or any other
 *  stdint.h.
 */

/*typedef   int_least8_t   int_fast8_t;
typedef  uint_least8_t  uint_fast8_t;
typedef  int_least16_t  int_fast16_t;
typedef uint_least16_t uint_fast16_t;
typedef  int_least32_t  int_fast32_t;
typedef uint_least32_t uint_fast32_t;
#define  UINT_FAST8_MAX  UINT_LEAST8_MAX
#define   INT_FAST8_MAX   INT_LEAST8_MAX
#define UINT_FAST16_MAX UINT_LEAST16_MAX
#define  INT_FAST16_MAX  INT_LEAST16_MAX
#define UINT_FAST32_MAX UINT_LEAST32_MAX
#define  INT_FAST32_MAX  INT_LEAST32_MAX
#define   INT_FAST8_MIN   INT_LEAST8_MIN
#define  INT_FAST16_MIN  INT_LEAST16_MIN
#define  INT_FAST32_MIN  INT_LEAST32_MIN
#ifdef stdint_int64_defined
  typedef  int_least64_t  int_fast64_t;
  typedef uint_least64_t uint_fast64_t;
# define UINT_FAST64_MAX UINT_LEAST64_MAX
# define  INT_FAST64_MAX  INT_LEAST64_MAX
# define  INT_FAST64_MIN  INT_LEAST64_MIN
#endif*/

#undef stdint_int64_defined

/*
 *  Whatever piecemeal, per compiler thing we can do about the wchar_t
 *  type limits.
 */

#if defined(__WATCOMC__) || defined(_MSC_VER) || defined (__GNUC__)
# include <wchar.h>
# ifndef WCHAR_MIN
#  define WCHAR_MIN 0
# endif
# ifndef WCHAR_MAX
#  define WCHAR_MAX ((wchar_t)-1)
# endif
#endif

/*
 *  Whatever piecemeal, per compiler/platform thing we can do about the
 *  (u)intptr_t types and limits.
 */

#if defined (_MSC_VER) && defined (_UINTPTR_T_DEFINED)
# define STDINT_H_UINTPTR_T_DEFINED
#endif

#ifndef STDINT_H_UINTPTR_T_DEFINED
# if defined (__alpha__) || defined (__ia64__) || defined (__x86_64__) || defined (_WIN64)
#  define stdint_intptr_bits 64
# elif defined (__WATCOMC__) || defined (__TURBOC__)
#  if defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__)
#    define stdint_intptr_bits 16
#  else
#    define stdint_intptr_bits 32
#  endif
# elif defined (__i386__) || defined (_WIN32) || defined (WIN32)
#  define stdint_intptr_bits 32
# elif defined (__INTEL_COMPILER)
/* TODO -- what did Intel do about x86-64? */
# endif

# ifdef stdint_intptr_bits
#  define stdint_intptr_glue3_i(a,b,c)  a##b##c
#  define stdint_intptr_glue3(a,b,c)    stdint_intptr_glue3_i(a,b,c)
#  ifndef PRINTF_INTPTR_MODIFIER
#    define PRINTF_INTPTR_MODIFIER      stdint_intptr_glue3(PRINTF_INT,stdint_intptr_bits,_MODIFIER)
#  endif
#  ifndef PTRDIFF_MAX
#    define PTRDIFF_MAX                 stdint_intptr_glue3(INT,stdint_intptr_bits,_MAX)
#  endif
#  ifndef PTRDIFF_MIN
#    define PTRDIFF_MIN                 stdint_intptr_glue3(INT,stdint_intptr_bits,_MIN)
#  endif
#  ifndef UINTPTR_MAX
#    define UINTPTR_MAX                 stdint_intptr_glue3(UINT,stdint_intptr_bits,_MAX)
#  endif
#  ifndef INTPTR_MAX
#    define INTPTR_MAX                  stdint_intptr_glue3(INT,stdint_intptr_bits,_MAX)
#  endif
#  ifndef INTPTR_MIN
#    define INTPTR_MIN                  stdint_intptr_glue3(INT,stdint_intptr_bits,_MIN)
#  endif
#  ifndef INTPTR_C
#    define INTPTR_C(x)                 stdint_intptr_glue3(INT,stdint_intptr_bits,_C)(x)
#  endif
#  ifndef UINTPTR_C
#    define UINTPTR_C(x)                stdint_intptr_glue3(UINT,stdint_intptr_bits,_C)(x)
#  endif
  /*typedef stdint_intptr_glue3(uint,stdint_intptr_bits,_t) uintptr_t;
  typedef stdint_intptr_glue3( int,stdint_intptr_bits,_t)  intptr_t;*/
# else
/* TODO -- This following is likely wrong for some platforms, and does
   nothing for the definition of uintptr_t. */
  typedef ptrdiff_t intptr_t;
# endif
# define STDINT_H_UINTPTR_T_DEFINED
#endif

/*
 *  Assumes sig_atomic_t is signed and we have a 2s complement machine.
 */
#ifndef SIG_ATOMIC_MAX
# define SIG_ATOMIC_MAX ((((sig_atomic_t) 1) << (sizeof (sig_atomic_t)*CHAR_BIT-1)) - 1)
#endif

#ifndef INA_OS_WIN32
#define INA_MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define INA_MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _b : _a; })
#else
#define INA_MAX(a,b) max(a,b)
#define INA_MIN(a,b) min(a,b)
#endif

#ifdef INA_OS_WIN32
struct timezone {
     int  tz_minuteswest; /* minutes W of Greenwich */
     int  tz_dsttime;     /* type of dst correction */
};

INA_API(int) gettimeofday(struct timeval *tv, struct timezone *tz);
#endif

#ifdef _WIN32
#define INA_PATH_SEPARATOR   '\\'
#else
#define INA_PATH_SEPARATOR   '/'
#endif

#ifdef __cplusplus
}
#endif 

#endif
