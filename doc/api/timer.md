

---

```C
typedef struct ina_timer_s ina_timer_t;
```
Timer

---

```C
INA_API(ina_rc_t) ina_timer_new(ina_timer_t **timer);
```

Creates a new timer.


**Parameters**
 - `timer`: Where to store the timer



**Return**

INA_SUCCESS



---

```C
INA_API(void) ina_timer_free(ina_timer_t **timer);
```

Destroy a timer.


**Parameters**
 - `timer`: Timer to free



---

```C
INA_API(ina_rc_t ) ina_timer_event_new(ina_timer_t *timer,
                                        time_t msec,
                                        ina_timer_event_t **event);
```

Creates a new time event for a timer


**Parameters**
 - `timer`: Timer
 - `msec`: Event interval in milliseconds



**Return**

Pointer to timer event or NULL if an error occurred.



---

```C
INA_API(ina_rc_t) ina_timer_event_new_with_time(ina_timer_t *timer,
                                            time_t n_msec,
                                            time_t e_msec,
                                            ina_timer_event_t **event);
```

Create a new time event for a timer while providing current time.


**Parameters**
 - `timer`: Timer
 - `n_msec`: Current time in millisecond since epoch
 - `e_msec`: Event interval in milliseconds



**Return**

Pointer to timer event or NULL if an error occurred.



---

```C
INA_API(ina_rc_t) ina_timer_event_free(ina_timer_t *timer,
                                       ina_timer_event_t *e);
```

Delete a time event from a timer.

Parameter
timer  Timer
e      Timer event to delete from timer.


**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_timer_event_get_id(const ina_timer_event_t *event, uint64_t *id);
```

Get ID of a time event

Parameter
e      Timer event
id     Pointer where to store event ID


**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_timer_next_event(const ina_timer_t *timer,
                                        ina_timer_event_t **event);
```

Get the next elapsed time event.


**Parameters**
 - `timer`: Timer to query
 - `event`: Pointer where to store next elapsed time event
or NULL if no events elapsed.



**Return**

INA_EEGAIN or INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_timer_next_event_with_time(const ina_timer_t *timer,
                                                 time_t now_millis,
                                                 ina_timer_event_t **event);
```

Get the next elapsed time event by providing the milliseconds since epoch.


**Parameters**
 - `timer`: Timer to query
now_millis Time since epoch
 - `event`: Pointer where to store next elapsed time event
or NULL if no events elapsed.




**Return**

INA_EEGAIN or INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_timer_time_to_next_event(const ina_timer_t *timer,
                                               time_t *how_long_msec);
```

Calculate time in milliseconds until the next time event will elapse.


**Parameters**
 - `timer`: Timer to query
 - `how_long_msec`: Where to store milliseconds when the next event will elapse.



**Return**

INA_SUCCESS

