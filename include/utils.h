#pragma once

#include "graph.h"
#include <time.h>

typedef struct Timer timer_t;

timer_t* timer_create(void);
void timer_start(timer_t *t);
void timer_stop(timer_t *t);
double timer_elapsed_ms(timer_t *t);
double timer_elapsed_sec(timer_t *t);
void timer_free(timer_t *t);

// JSON parsing
graph_t* load_graph_from_json(const char *filename);

// Data generation
void generate_banking_data(const char *output_file, int num_users, int num_transactions);
