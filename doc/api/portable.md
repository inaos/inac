

---

```C
#if defined __ECC || defined __ICC || defined __INTEL_COMPILER
#  define INA_COMPILER_STRING "Intel C/C++"
#  define INA_COMPILER_INTEL 1
#endif

```

Determine compilation environment


---

```C
#  define INA_COMPILER_APPLECC 1
#endif

```
we don't define the compiler string here, let it be GNU

---

```C
#if defined linux || defined __linux__
#  define INA_OS_LINUX 1 
#  define INA_OS_STRING "Linux"
#endif

```

Determine target operating system


---

```C
#if defined __WATCOMC__  && defined __386__ && defined __DOS__
#  define INA_OS_DOS32 1
#  define INA_OS_STRING "DOS/32-bit"
#endif

```
NOTE: make sure you use /bt=DOS if compiling for 32-bit DOS,
otherwise Watcom assumes host=target

---

```C
#if defined GEKKO
#  define INA_CPU_PPC750 1
#  define INA_CPU_STRING "IBM PowerPC 750 (NGC)"
#endif

```

Determine target CPU


---

```C
#if !defined INA_OS_STRING
#  define INA_OS_EMBEDDED 1 
#  if defined _R5900
#     define INA_OS_STRING "Sony PS2(embedded)"
#  else
#     define INA_OS_STRING "Embedded/Unknown"
#  endif
#endif

```

Attempt to autodetect building for embedded on Sony PS2


---

```C
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

```

Handle cdecl, stdcall, fastcall, etc.


---

```C
#ifndef __dead
#  define __dead
#endif

```

Handle some useful macros


---

```C
#ifdef __cplusplus
#  ifdef INA_OS_WIN32
#    define INA_INLINE __inline
#  else
#   define INA_INLINE static inline
#  endif
#else
#  ifdef INA_OS_WIN32
#    define INA_INLINE __forceinline
#  else
#    define INA_INLINE static inline
#  endif
#endif
#ifndef INA_INLINE
#define INA_INLINE
#endif

```
If your compiler supports the inline keyword in C, INA_INLINE is
defined to `inline', otherwise empty. In C++, the inline is always
supported.

---

```C
#if defined INA_EXPORT
#  undef INA_EXPORT
#endif

```

Define INA_EXPORT signature based on INA_DLL and INA_LIB (only Windows)


---

```C
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

```
for all other compilers, we're just making a blanket assumption

---

```C
#if !defined INA_EXPORT
#  define INA_EXPORT
#endif

```
On pretty much everything else, we can thankfully just ignore this

---

```C
#ifdef INA_API
#  undef INA_API
#endif

```

(Re)define INA_API export signature


---

```C
#if defined INA_CPU_X86 || defined INA_CPU_AXP || defined INA_CPU_STRONGARM || defined INA_OS_WIN32 || defined INA_OS_WINCE || defined __MIPSEL__
#  define INA_ENDIAN_STRING "little"
#  define INA_LITTLE_ENDIAN 1
#else
#  define INA_ENDIAN_STRING "big"
#  define INA_BIG_ENDIAN 1
#endif

```

Try to infer endianess.  Basically we just go through the CPUs we know are
little endian, and assume anything that isn't one of those is big endian.
As a sanity check, we also do this with operating systems we know are
little endian, such as Windows.  Some processors are bi-endian, such as
the MIPS series, so we have to be careful about those.


---

```C
#define INA_CASSERT(name, x) typedef int _INA_dummy_## name[(x) ? 1 : -1 ]

```

----------------------------------------------------------------------------
Cross-platform compile time assertion macro
----------------------------------------------------------------------------


---

```C
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

```

----------------------------------------------------------------------------
Cross-platform numeric types
----------------------------------------------------------------------------


---

```C
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

```

Something really weird is going on with Open Watcom.  Just pull some of
these duplicated definitions from Open Watcom's stdint.h file for now.


---

```C
#ifndef UINT8_MAX
# define UINT8_MAX 0xff
#endif
#ifndef uint8_t
# if (UCHAR_MAX == UINT8_MAX) || defined (S_SPLINT_S)
#ifndef UINT8_C
    typedef unsigned char uint8_t;
```

Deduce the type assignments from limits.h under the assumption that
integer sizes in bits are powers of 2, and follow the ANSI
definitions.


---

```C
#ifndef INT32_C
# define INT32_C(v) v ## L
#endif
# ifndef PRINTF_INT32_MODIFIER
#  define PRINTF_INT32_MODIFIER "l"
# endif
#elif (INT_MAX == INT32_MAX)
#ifndef INT32_C
  typedef signed int int32_t;
```
typedef signed long int32_t;

---

```C
#  ifndef UINT64_C
#    define UINT64_C(v) v ## ULL
#  endif
#  ifndef INT64_C
#    define  INT64_C(v) v ## LL
#  endif
#  ifdef PRINTF_INT64_MODIFIER
#  undef PRINTF_INT64_MODIFIER
#  endif
#    define PRINTF_INT64_MODIFIER "l"
# elif defined(__MWERKS__) || defined (__SUNPRO_C) || defined (__SUNPRO_CC) || defined (__APPLE_CC__) || defined (_LONG_LONG) || defined (_CRAYC) || defined (S_SPLINT_S)
#  define stdint_int64_defined
   typedef long long int64_t;
```
__extension__ typedef long long int64_t;
__extension__ typedef unsigned long long uint64_t;

---

```C
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

```

Width of hexadecimal for number field.


---

```C
/*
#if !defined(stdint_least_defined) && !defined(_GCC_WRAP_STDINT_H) 
  typedef   int8_t   int_least8_t;
```

Because this file currently only supports platforms which have
precise powers of 2 as bit sizes for the default integers, the
least definitions are all trivial.  Its possible that a future
version of this file could have different definitions.


---

```C
# ifdef _STDINT_H_INCLUDED
typedef   int_least8_t   int_fast8_t;
```

The ANSI C committee pretending to know or specify anything about
performance is the epitome of misguided arrogance.  The mandate of
this file is to ONLY ever support that absolute minimum
definition of the fast integer types, for compatibility purposes.
No extensions, and no attempt to suggest what may or may not be a
faster integer type will ever be made in this file.  Developers are
warned to stay away from these types when using this or any other
stdint.h.


---

```C
#if defined(__WATCOMC__) || defined(_MSC_VER) || defined (__GNUC__)
# include <wchar.h>
# ifndef WCHAR_MIN
#  define WCHAR_MIN 0
# endif
# ifndef WCHAR_MAX
#  define WCHAR_MAX ((wchar_t)-1)
# endif
#endif

```

Whatever piecemeal, per compiler thing we can do about the wchar_t
type limits.


---

```C
#if defined (_MSC_VER) && defined (_UINTPTR_T_DEFINED)
# define STDINT_H_UINTPTR_T_DEFINED
#endif

```

Whatever piecemeal, per compiler/platform thing we can do about the
(u)intptr_t types and limits.


---

```C
# endif

```
TODO -- what did Intel do about x86-64?

---

```C
# else
/* TODO -- This following is likely wrong for some platforms, and does
   nothing for the definition of uintptr_t. */
  typedef ptrdiff_t intptr_t;
```
typedef stdint_intptr_glue3(uint,stdint_intptr_bits,_t) uintptr_t;
typedef stdint_intptr_glue3( int,stdint_intptr_bits,_t)  intptr_t;

---

```C
#ifndef SIG_ATOMIC_MAX
# define SIG_ATOMIC_MAX ((((sig_atomic_t) 1) << (sizeof (sig_atomic_t)*CHAR_BIT-1)) - 1)
#endif

```

Assumes sig_atomic_t is signed and we have a 2s complement machine.


---

```C
#ifdef INA_OS_WIN32
#  if defined(INA_COMPILER_MSVC) || defined(INA_COMPILER_INTEL)
#    define INA_ALIGNED(x) __declspec(align(x))
#    define INA_VSALIGNED128 INA_ALIGNED(128)
#    define INA_VSALIGNED64 INA_ALIGNED(64)
#    define INA_VSALIGNED32 INA_ALIGNED(32)
#    define INA_VSALIGNED16 INA_ALIGNED(16)
#    define INA_VSALIGNED8 INA_ALIGNED(8)
#    define INA_VSALIGNED4 INA_ALIGNED(4)
#    define INA_VSALIGNED2 INA_ALIGNED(2)
#    define INA_ALIGNED128
#    define INA_ALIGNED64
#    define INA_ALIGNED32
#    define INA_ALIGNED16
#    define INA_ALIGNED8
#    define INA_ALIGNED4
#    define INA_ALIGNED2
#    define INA_PACKED
#    define INA_VS_BEGIN_PACK __pragma(pack(1))
#    define INA_VS_END_PACK __pragma(pack())
#  else
#    error UNSUPPORTED COMPILER
#  endif
#else
#  if defined(INA_COMPILER_GCC) || defined(INA_COMPILER_INTEL)
#    define INA_ALIGNED(x) __attribute__((aligned(x)))
#    define INA_ALIGNED128 INA_ALIGNED(128)
#    define INA_ALIGNED64 INA_ALIGNED(64)
#    define INA_ALIGNED32 INA_ALIGNED(32)
#    define INA_ALIGNED16 INA_ALIGNED(16)
#    define INA_ALIGNED8 INA_ALIGNED(8)
#    define INA_ALIGNED4 INA_ALIGNED(4)
#    define INA_ALIGNED2 INA_ALIGNED(2)
#    define INA_VSALIGNED128
#    define INA_VSALIGNED64
#    define INA_VSALIGNED32
#    define INA_VSALIGNED16
#    define INA_VSALIGNED8
#    define INA_VSALIGNED4
#    define INA_VSALIGNED2
#    ifndef INA_PACKED
#      define INA_PACKED __attribute__ ((__packed__))
#    endif
#    define INA_VS_BEGIN_PACK
#    define INA_VS_END_PACK
#  else
#    error UNSUPPORTED COMPILER
#  endif
#endif

```
Pack

---

```C
#ifdef INA_OS_WIN32
#define INA_ATOMIC_INC(vv_ptr) InterlockedIncrement64(vv_ptr)
#define INA_ATOMIC_DEC(vv_ptr) InterlockedDecrement64(vv_ptr)
#define INA_ATOMIC_SWAP(vv_ptr,old,new) InterlockedCompareExchange64(vv_ptr,new,old)
#elif defined(__GNUC__) && ( __GNUC__ * 100 + __GNUC_MINOR__ >= 401 )
#define INA_ATOMIC_INC(vv_ptr) __sync_fetch_and_add(vv_ptr, 1)
#define INA_ATOMIC_DEC(vv_ptr) __sync_fetch_and_sub(vv_ptr, 1)
#define INA_ATOMIC_SWAP(vv_ptr,old,new) __sync_val_compare_and_swap(vv_ptr,old,new)
#else
#error Compiler not supported yet for INAC!
#endif

```
Atomic operations

---

```C
#ifdef INA_OS_WIN32
#define INA_LIKELY(x)    (x)
#define INA_UNLIKELY(x)  (x)
#elif defined(__GNUC__) && ( __GNUC__ * 100 + __GNUC_MINOR__ >= 401 )
#define INA_LIKELY(x)    __builtin_expect(!!(x), 1)
#define INA_UNLIKELY(x)  __builtin_expect(!!(x), 0)
#else
#error Compiler not supported yet for INAC!
#endif

```
Branch prediction hints

---

```C
#ifdef INA_OS_WIN32
#define INA_RESTRICT    __restrict
#elif defined(__GNUC__) && ( __GNUC__ * 100 + __GNUC_MINOR__ >= 401 )
#define INA_RESTRICT    __restrict__
#else
#error Compiler not supported yet for INAC!
#endif

```
C99 restrict a.k.a. pointer aliasing hint

---

```C
#ifdef INA_OS_WIN32
#define INA_BSWAP_16 _byteswap_ushort
#define INA_BSWAP_32 _byteswap_ulong
#define INA_BSWAP_64 _byteswap_uint64
#elif defined(__GNUC__) && ( __GNUC__ * 100 + __GNUC_MINOR__ >= 401 )
#define INA_BSWAP_16 __builtin_bswap16
#define INA_BSWAP_32 __builtin_bswap32
#define INA_BSWAP_64 __builtin_bswap64
#else
#error Compiler not supported yet for INAC!
#endif

```
byte swapping

---

```C
#ifdef INA_OS_WIN32
typedef SOCKET ina_fd_t;
```
FD for net.h

---

```C
#define S_ISUID      0x08000000   /* does nothing  */
#define S_ISGID      0x04000000   /* does nothing  */
#define S_ISVTX      0x02000000   /* does nothing  */
#define S_IRUSR      _S_IREAD     /* read by user  */
#define S_IWUSR      _S_IWRITE    /* write by user */
#define S_IXUSR      0x00400000   /* does nothing  */
#   ifndef STRICT_UGO_PERMISSIONS
#define S_IRGRP      _S_IREAD     /* read by *USER*  */
#define S_IWGRP      _S_IWRITE    /* write by *USER* */
#define S_IXGRP      0x00080000   /* does nothing    */
#define S_IROTH      _S_IREAD     /* read by *USER*  */
#define S_IWOTH      _S_IWRITE    /* write by *USER* */
#define S_IXOTH      0x00010000   /* does nothing    */
#   else
#define S_IRGRP      0x00200000   /* does nothing */
#define S_IWGRP      0x00100000   /* does nothing */
#define S_IXGRP      0x00080000   /* does nothing */
#define S_IROTH      0x00040000   /* does nothing */
#define S_IWOTH      0x00020000   /* does nothing */
#define S_IXOTH      0x00010000   /* does nothing */
#   endif
#define S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)
#define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)
#define S_IRWXO (S_IROTH | S_IWOTH | S_IXOTH)

```
If STRICT_UGO_PERMISSIONS is not defined, then setting Read for any
of User, Group, or Other will set Read for User and setting Write
will set Write for User.  Otherwise, Read and Write for Group and
Other are ignored.

For the POSIX modes that do not have a Windows equivalent, the modes
defined here use the POSIX values left shifted 16 bits.


---

```C
#ifndef INA_TLS
#   ifndef INA_OS_WIN32
#       define INA_TLS(x) __thread x             // MingW, Solaris Studio C/C++, IBM XL C/C++, GNU C, Clang and Intel C++ Compiler (Linux systems)
#   else
#       define INA_TLS(x) __declspec(thread) x   // Visual C++, Intel C/C++ (Windows systems), C++Builder and Digital Mars C++
#   endif
#endif

```

thread-local-safe variable
