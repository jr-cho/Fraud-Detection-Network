#pragma once

#include "graph.h"

typedef struct {
    double execution_time;
    int cycles_found;
} algo_result_t;

// Brute force DFS approach
cycle_result_t* find_cycles_dfs_bruteforce(graph_t *g, algo_result_t *result);

// Tarjan's algorithm for SCCs (Strongly Connected Components)
cycle_result_t* find_cycles_tarjan(graph_t *g, algo_result_t *result);
