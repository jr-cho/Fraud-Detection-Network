#include "../include/graph.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Graph {
    user_t *users;
    transaction_t *transactions;
    int num_users;
    int num_transactions;
    int transaction_capacity;
    int *adj_list;
};

graph_t* graph_create(int num_users, int num_transactions) {
    graph_t *g = (graph_t *)malloc(sizeof(graph_t));
    g->num_users = num_users;
    g->num_transactions = 0;
    g->transaction_capacity = num_transactions > 0 ? num_transactions : 1;

    g->users = (user_t *)malloc(num_users * sizeof(user_t));
    g->transactions = (transaction_t *)malloc(g->transaction_capacity * sizeof(transaction_t));

    g->adj_list = (int *)malloc(num_users * num_users * sizeof(int));

    for (int i = 0; i < num_users; i++) {
        g->users[i].user_id = i;
        snprintf(g->users[i].account_number, 32, "ACC_%06d", i);
        g->users[i].balance = 10000.0 + (rand() % 90000);
    }
    
    memset(g->adj_list, 0, num_users * num_users * sizeof(int));
    
    return g;
}

void graph_add_transaction(graph_t *g, int from_user, int to_user, double amount, long timestamp) {
    if (from_user >= g->num_users || to_user >= g->num_users) return;

    if (g->num_transactions >= g->transaction_capacity) {
        int new_capacity = g->transaction_capacity * 2;
        transaction_t *resized = (transaction_t *)realloc(
            g->transactions, new_capacity * sizeof(transaction_t));
        if (resized == NULL) {
            fprintf(stderr, "[ERROR] Failed to grow transaction buffer to %d entries\n", new_capacity);
            return;
        }
        g->transactions = resized;
        g->transaction_capacity = new_capacity;
    }

    g->transactions[g->num_transactions].from_user = from_user;
    g->transactions[g->num_transactions].to_user = to_user;
    g->transactions[g->num_transactions].amount = amount;
    g->transactions[g->num_transactions].timestamp = timestamp;
    g->num_transactions++;

    int *row = g->adj_list + from_user * g->num_users;
    row[to_user]++;
}

void graph_print_stats(graph_t *g) {
    printf("\n=== Graph Statistics ===\n");
    printf("Total Users: %d\n", g->num_users);
    printf("Total Transactions: %d\n", g->num_transactions);
    
    int edges = 0;
    for (int i = 0; i < g->num_users; i++) {
        int *row = g->adj_list + i * g->num_users;
        for (int j = 0; j < g->num_users; j++) {
            if (row[j] > 0) edges++;
        }
    }
    printf("Total Edges: %d\n", edges);
    printf("Edge Density: %.4f\n", (double)edges / (g->num_users * g->num_users));
}

user_t* graph_get_users(graph_t *g) {
    return g->users;
}

int graph_get_num_users(graph_t *g) {
    return g->num_users;
}

int* graph_get_adj_list(graph_t *g) {
    return g->adj_list;
}

void graph_free(graph_t *g) {
    free(g->users);
    free(g->transactions);
    free(g->adj_list);
    free(g);
}

cycle_result_t* cycle_result_create(void) {
    cycle_result_t *result = (cycle_result_t *)malloc(sizeof(cycle_result_t));
    result->cycles = (fraud_cycle_t *)malloc(1000 * sizeof(fraud_cycle_t));
    result->count = 0;
    result->capacity = 1000;
    return result;
}

void cycle_result_add(cycle_result_t *result, int *cycle, int length, double amount) {
    if (result->count >= result->capacity) {
        result->capacity *= 2;
        result->cycles = (fraud_cycle_t *)realloc(result->cycles, result->capacity * sizeof(fraud_cycle_t));
    }
    
    result->cycles[result->count].cycle = (int *)malloc(length * sizeof(int));
    memcpy(result->cycles[result->count].cycle, cycle, length * sizeof(int));
    result->cycles[result->count].length = length;
    result->cycles[result->count].total_amount = amount;
    result->count++;
}

void cycle_result_print(cycle_result_t *result) {
    printf("\n=== Fraud Cycles Detected: %d ===\n", result->count);
    for (int i = 0; i < result->count && i < 20; i++) {
        printf("Cycle %d: ", i + 1);
        for (int j = 0; j < result->cycles[i].length; j++) {
            printf("%d -> ", result->cycles[i].cycle[j]);
        }
        printf("%d (Amount: $%.2f)\n", result->cycles[i].cycle[0], result->cycles[i].total_amount);
    }
    if (result->count > 20) printf("... and %d more cycles\n", result->count - 20);
}

void cycle_result_free(cycle_result_t *result) {
    for (int i = 0; i < result->count; i++) {
        free(result->cycles[i].cycle);
    }
    free(result->cycles);
    free(result);
}
