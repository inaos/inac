#include <stdlib.h>
#include <libinac/lib.h>

static ina_time_event_t *e1 = NULL;
static ina_time_event_t *e2 = NULL;
static ina_time_event_t *e3 = NULL;
static ina_time_event_t *e4 = NULL;
static ina_time_event_t *e5 = NULL;
static ina_time_event_t *e6 = NULL;
static ina_time_event_t *e7 = NULL;
static ina_time_event_t *e8 = NULL;
static ina_time_event_t *e9 = NULL;
static ina_time_event_t *e10 = NULL;
static ina_time_event_t *e11 = NULL;
static ina_time_event_t *e12 = NULL;
static ina_time_event_t *e13 = NULL;
static ina_time_event_t *e14 = NULL;
static ina_time_event_t *e15 = NULL;
static ina_time_event_t *e16 = NULL;
static ina_time_event_t *e17 = NULL;
static ina_time_event_t *stop_event = NULL;

static ina_rc_t __ina_timer_bench_create_events(ina_timer_t *timer)
{
    e1 = ina_timer_create_event(timer, 50);
    e2 = ina_timer_create_event(timer, 500);
    e3 = ina_timer_create_event(timer, 100);
    e4 = ina_timer_create_event(timer, 200);
    e5 = ina_timer_create_event(timer, 70);
    e6 = ina_timer_create_event(timer, 100);
    e7 = ina_timer_create_event(timer, 1000);
    e8 = ina_timer_create_event(timer, 150);
    e9 = ina_timer_create_event(timer, 555);
    e10 = ina_timer_create_event(timer, 2000);
    e11 = ina_timer_create_event(timer, 1500);
    e12 = ina_timer_create_event(timer, 30);
    e13 = ina_timer_create_event(timer, 8);
    e14 = ina_timer_create_event(timer, 22);
    e15 = ina_timer_create_event(timer, 111);
    e16 = ina_timer_create_event(timer, 222);
    e17 = ina_timer_create_event(timer, 333);
    stop_event = ina_timer_create_event(timer, 20000);
    return INA_SUCCESS;
}
int main(int argc, char **argv)
{
    ina_timer_t *timer;
    int rdtsc = INA_NO;
    int64_t idx = 0;
    ina_stopwatch_t *s1 = NULL; 
    ina_stopwatch_t *s2 = NULL;
    double total = 0;

    INA_OPTS(opt,
        INA_OPT_INT("r", "rdtsc", INA_NO, "Use RDTSC"));

    if (!INA_SUCCEED(ina_app_init(argc, argv, 0, opt))) {
        return EXIT_FAILURE;
    }

    ina_opt_get_int("r", &rdtsc);

    if (!INA_SUCCEED(ina_timer_init(&timer))) {
        return EXIT_FAILURE;
    }

    if (rdtsc) {
        if (!INA_SUCCEED(ina_timer_use_rdtsc(timer, INA_YES))) {
            return EXIT_FAILURE;
        }    
    }

    INA_TIME_STOPWATCH_CREATE(&s1, 1, -1);
    INA_TIME_STOPWATCH_CREATE(&s2, 2, -1);

    INA_TIME_STOPWATCH_START(s1);
    __ina_timer_bench_create_events(timer);
    INA_TIME_STOPWATCH_STOP(s1);

    ina_time_stopwatch_read_stamp(s1, &idx);
    printf("Time to create time-events in micro-sec: %f\n", s1->ts->usec_duration);

    for (;;) {
        ina_time_event_t *e;
        INA_TIME_STOPWATCH_START(s2);
        e = ina_timer_next_event(timer);
        INA_TIME_STOPWATCH_STOP(s2);
        if (e->id == stop_event->id) {
           break;
        }
        ina_time_stopwatch_read_stamp(s2, &idx);
        total += s2->ts->usec_duration;
        ina_time_sleep(1);
    }

    printf("Total time spent in ina_timer_next_event() %f\n", total);

    ina_timer_destroy(&timer);

    INA_TIME_STOPWATCH_DESTROY(&s1);
    INA_TIME_STOPWATCH_DESTROY(&s2);
    
    return EXIT_SUCCESS;
}

