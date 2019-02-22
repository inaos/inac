

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
