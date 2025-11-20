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

graph_t* graph_create(int num_users, int num_transactions_hint) {

    if (num_users <= 0) num_users = 1;
    if (num_transactions_hint <= 0) num_transactions_hint = 1;

    graph_t *g = malloc(sizeof(graph_t));
    g->num_users = num_users;
    g->num_transactions = 0;
    g->transaction_capacity = num_transactions_hint;

    g->users = malloc(num_users * sizeof(user_t));
    g->transactions = malloc(g->transaction_capacity * sizeof(transaction_t));
    g->adj_list = calloc(num_users * num_users, sizeof(int));

    // Assign simple human-readable labels
    for (int i = 0; i < num_users; i++) {
        g->users[i].user_id = i;

        snprintf(g->users[i].label, sizeof(g->users[i].label),
                 "Client-%03d", i);

        snprintf(g->users[i].account_number,
                 sizeof(g->users[i].account_number),
                 "ACC_%06d", i);

        g->users[i].balance = 1000 + (rand() % 9000);
    }

    return g;
}

void graph_add_transaction(graph_t *g, int from_user, int to_user,
                           double amount, long timestamp) {

    if (from_user == to_user) return;
    if (from_user < 0 || from_user >= g->num_users) return;
    if (to_user < 0 || to_user >= g->num_users) return;

    if (g->num_transactions >= g->transaction_capacity) {
        g->transaction_capacity *= 2;
        g->transactions = realloc(g->transactions,
                                  g->transaction_capacity * sizeof(transaction_t));
    }

    transaction_t *t = &g->transactions[g->num_transactions++];
    t->from_user = from_user;
    t->to_user = to_user;
    t->amount = amount;
    t->timestamp = timestamp;

    g->adj_list[from_user * g->num_users + to_user]++;
}

void graph_print_stats(graph_t *g) {
    printf("\n=== Graph Statistics ===\n");
    printf("Total Users: %d\n", g->num_users);
    printf("Total Transactions: %d\n", g->num_transactions);

    int edges = 0;
    for (int i = 0; i < g->num_users; i++)
        for (int j = 0; j < g->num_users; j++)
            if (g->adj_list[i * g->num_users + j] > 0)
                edges++;

    double density = (double)edges / (g->num_users * g->num_users);
    printf("Total Edges: %d\n", edges);
    printf("Edge Density: %.6f\n", density);
}

user_t* graph_get_users(graph_t *g) { return g->users; }
int graph_get_num_users(graph_t *g) { return g->num_users; }
int* graph_get_adj_list(graph_t *g) { return g->adj_list; }

void graph_free(graph_t *g) {
    free(g->users);
    free(g->transactions);
    free(g->adj_list);
    free(g);
}



// =====================================================
// Cycle result management (deduplicated)
// =====================================================
cycle_result_t* cycle_result_create(void) {
    cycle_result_t *r = malloc(sizeof(cycle_result_t));
    r->capacity = 512;
    r->count = 0;
    r->cycles = malloc(r->capacity * sizeof(fraud_cycle_t));
    return r;
}

void cycle_result_add(cycle_result_t *result, int *cycle, int length, double amount) {

    // Remove duplicates
    for (int i = 0; i < result->count; i++) {
        if (result->cycles[i].length != length) continue;
        if (!memcmp(result->cycles[i].cycle, cycle, length * sizeof(int)))
            return;
    }

    if (result->count >= result->capacity) {
        result->capacity *= 2;
        result->cycles = realloc(result->cycles,
                                 result->capacity * sizeof(fraud_cycle_t));
    }

    fraud_cycle_t *fc = &result->cycles[result->count++];
    fc->cycle = malloc(length * sizeof(int));
    memcpy(fc->cycle, cycle, length * sizeof(int));
    fc->length = length;
    fc->total_amount = amount;
}

void cycle_result_free(cycle_result_t *result) {
    for (int i = 0; i < result->count; i++)
        free(result->cycles[i].cycle);
    free(result->cycles);
    free(result);
}
