#pragma once

#include "graph.h"

typedef struct {
    double execution_time;
    int cycles_found;
} algo_result_t;

cycle_result_t* find_cycles_dfs_bruteforce(graph_t *g, algo_result_t *result);
cycle_result_t* find_cycles_tarjan(graph_t *g, algo_result_t *result);
