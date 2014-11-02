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
static ina_rc_t __ina_timer_bench_exec(int rdtsc, int iteration)
{
    ina_timer_t *timer;
    ina_stopwatch_t *s1 = NULL; 
    ina_stopwatch_t *s2 = NULL;
    ina_stopwatch_t *s3 = NULL;
    double total = 0;
    int64_t idx = 0;

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
    INA_TIME_STOPWATCH_CREATE(&s3, 3, -1);

    INA_TIME_STOPWATCH_START(s1);
    __ina_timer_bench_create_events(timer);
    INA_TIME_STOPWATCH_STOP(s1);

    printf("IT-%d: Time to create time-events in micro-sec: %f\n", iteration, s1->tv->usec_duration);

    INA_TIME_STOPWATCH_START(s3);

    for (;;) {
        ina_time_event_t *e;
        INA_TIME_STOPWATCH_START(s2);
        e = ina_timer_next_event(timer);
        INA_TIME_STOPWATCH_STOP(s2);
        if (e != NULL && e->id == stop_event->id) {
           break;
        }
        total += s2->tv->usec_duration;
        ina_time_sleep(1);
    }

    INA_TIME_STOPWATCH_STOP(s3);

    printf("IT-%d: ina_timer_next_event() in micro-seconds: %f or in sec: %f\n", iteration, total, total/1000/1000);

    printf("IT-%d: Test total time: %f\n", iteration, s3->tv->sec_duration);

    INA_TIME_STOPWATCH_DESTROY(&s1);
    INA_TIME_STOPWATCH_DESTROY(&s2);
    INA_TIME_STOPWATCH_DESTROY(&s3);

    ina_timer_destroy(&timer);

    return INA_SUCCESS;
}
int main(int argc, char **argv)
{
    int rdtsc = INA_NO;
    int iterations = 0;
    int i;

    INA_OPTS(opt,
        INA_OPT_INT("r", "rdtsc", INA_NO, "Use RDTSC"),
        INA_OPT_INT("i", "iterations", 1, "Number of benchmark iterations")
    );

    if (!INA_SUCCEED(ina_app_init(argc, argv, 0, opt))) {
        return EXIT_FAILURE;
    }

    ina_opt_get_int("r", &rdtsc);
    ina_opt_get_int("i", &iterations);

    for (i = 0; i < iterations; i++) {
        __ina_timer_bench_exec(rdtsc, i+1);
    }
    
    return EXIT_SUCCESS;
}

