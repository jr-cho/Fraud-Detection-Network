#include "../include/utils.h"
#include <stdlib.h>
#include <time.h>

struct Timer {
    clock_t start;
    clock_t end;
};

timer_t* timer_create(void) {
    timer_t *t = (timer_t *)malloc(sizeof(timer_t));
    t->start = 0;
    t->end = 0;
    return t;
}

void timer_start(timer_t *t) {
    t->start = clock();
}

void timer_stop(timer_t *t) {
    t->end = clock();
}

double timer_elapsed_sec(timer_t *t) {
    return (double)(t->end - t->start) / CLOCKS_PER_SEC;
}

double timer_elapsed_ms(timer_t *t) {
    return (double)(t->end - t->start) / CLOCKS_PER_SEC * 1000.0;
}

void timer_free(timer_t *t) {
    free(t);
}
