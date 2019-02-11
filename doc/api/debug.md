

---

```C
#ifndef _LIBINAC_DEBUG_H_
#define _LIBINAC_DEBUG_H_

```

Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.


---

```C
#ifdef INA_DEBUG
#define INA_TRACE(fmt, ...)     \
    fprintf(stderr,             \
        "%s:%d:%s(): " fmt "\n",\
        __FILE__,               \
        __LINE__,               \
        __FUNCTION__,           \
        ##__VA_ARGS__           \
        )
#if INA_TRACE_LEVEL>0
#define INA_TRACE1(fmt, ...)  INA_TRACE(fmt, ##__VA_ARGS__)
#else
#define INA_TRACE1(fmt, ...)
#endif
#if INA_TRACE_LEVEL>1
#define INA_TRACE2(fmt, ...)  INA_TRACE(fmt, ##__VA_ARGS__)
#else 
#define INA_TRACE2(fmt, ...)
#endif
#if INA_TRACE_LEVEL>2
#define INA_TRACE3(fmt, ...)  INA_TRACE(fmt, ##__VA_ARGS__)
#else
#define INA_TRACE3(fmt, ...)
#endif
#else
#define INA_TRACE(fmt, ...)
#define INA_TRACE1(fmt, ...)
#define INA_TRACE2(fmt, ...)
#define INA_TRACE3(fmt, ...)
#endif 

```

Trace macros
