#pragma once

#include <time.h>

typedef struct {
    int from_user;
    int to_user;
    double amount;
    long timestamp;
} transaction_t;

typedef struct {
    int user_id;
    char account_number[32];
    double balance;
} user_t;

typedef struct Graph graph_t;

typedef struct {
    int *cycle;
    int length;
    double total_amount;
} fraud_cycle_t;

typedef struct {
    fraud_cycle_t *cycles;
    int count;
    int capacity;
} cycle_result_t;

// Graph operations
graph_t* graph_create(int num_users, int num_transactions);
void graph_add_transaction(graph_t *g, int from_user, int to_user, double amount, long timestamp);
void graph_print_stats(graph_t *g);
void graph_free(graph_t *g);

// User access
user_t* graph_get_users(graph_t *g);
int graph_get_num_users(graph_t *g);

// Adjacency access
int* graph_get_adj_list(graph_t *g);

// Fraud detection results
cycle_result_t* cycle_result_create(void);
void cycle_result_add(cycle_result_t *result, int *cycle, int length, double amount);
void cycle_result_print(cycle_result_t *result);
void cycle_result_free(cycle_result_t *result);
