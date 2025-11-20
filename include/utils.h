#pragma once

typedef struct Timer timer_t;

timer_t* timer_create(void);
void timer_start(timer_t *t);
void timer_stop(timer_t *t);
double timer_elapsed_ms(timer_t *t);
void timer_free(timer_t *t);
