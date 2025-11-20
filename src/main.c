#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../include/graph.h"
#include "../include/algorithms.h"
#include "../include/utils.h"

typedef struct {
    int root;
    fraud_cycle_t **items;
    int count;
    int capacity;
} subnetwork_t;

static subnetwork_t *subnets = NULL;
static int subnet_count = 0;

static void subnetwork_add(fraud_cycle_t *c) {
    int root = c->cycle[0];

    for (int i = 0; i < subnet_count; i++) {
        if (subnets[i].root == root) {
            subnetwork_t *s = &subnets[i];
            if (s->count >= s->capacity) {
                s->capacity *= 2;
                s->items = realloc(s->items, s->capacity * sizeof(fraud_cycle_t*));
            }
            s->items[s->count++] = c;
            return;
        }
    }

    subnet_count++;
    subnets = realloc(subnets, subnet_count * sizeof(subnetwork_t));

    subnetwork_t *s = &subnets[subnet_count - 1];
    s->root = root;
    s->count = 1;
    s->capacity = 8;
    s->items = malloc(8 * sizeof(fraud_cycle_t*));
    s->items[0] = c;
}

static void print_cycle_id(const fraud_cycle_t *fc) {
    for (int j = 0; j < fc->length; j++) {
        printf("%d", fc->cycle[j]);
        if (j < fc->length - 1) printf(" -> ");
        else printf(" -> %d", fc->cycle[0]);
    }
}

static int compare_cycles(const void *a, const void *b) {
    const fraud_cycle_t *ca = (const fraud_cycle_t*)a;
    const fraud_cycle_t *cb = (const fraud_cycle_t*)b;

    int len = (ca->length < cb->length ? ca->length : cb->length);

    for (int i = 0; i < len; i++)
        if (ca->cycle[i] != cb->cycle[i])
            return ca->cycle[i] - cb->cycle[i];

    return ca->length - cb->length;
}

int main(void) {
    srand(time(NULL));

    int num_users = 50;
    int num_txns  = 3000;

    printf("\n==============================\n");
    printf(" Fraud Ring Detection System\n");
    printf("==============================\n");

    timer_t *t0 = timer_create();
    timer_start(t0);

    graph_t *g = graph_create(num_users, num_txns);

    int added = 0;
    int cycles_to_inject = num_txns * 0.10;

    while (added < num_txns && cycles_to_inject--) {
        int len = 3 + rand() % 4;
        int nodes[len];

        for (int i = 0; i < len; i++)
            nodes[i] = rand() % num_users;

        for (int i = 0; i < len; i++) {
            int from = nodes[i];
            int to   = nodes[(i + 1) % len];

            graph_add_transaction(
                g, from, to,
                100 + rand() % 5000,
                time(NULL)
            );

            added++;
            if (added >= num_txns) break;
        }
    }

    while (added < num_txns) {
        int from = rand() % num_users;
        int to   = rand() % num_users;
        if (from == to) continue;

        graph_add_transaction(
            g, from, to,
            10 + rand() % 3000,
            time(NULL)
        );

        added++;
    }

    timer_stop(t0);
    printf("Graph created in %.3f ms.\n\n", timer_elapsed_ms(t0));
    graph_print_stats(g);

    algo_result_t bf_r;
    cycle_result_t *bf = find_cycles_dfs_bruteforce(g, &bf_r);

    qsort(bf->cycles, bf->count, sizeof(fraud_cycle_t), compare_cycles);

    printf("\n-----------------------------------------\n");
    printf(" Brute Force Cycle Detection\n");
    printf("-----------------------------------------\n");
    printf("Time: %.3f ms\n", bf_r.execution_time);
    printf("Cycles Found: %d\n\n", bf->count);

    int limit = (bf->count < 20 ? bf->count : 20);
    printf("Showing first %d cycles:\n\n", limit);

    for (int i = 0; i < limit; i++) {
        print_cycle_id(&bf->cycles[i]);
        printf("\n");
    }

    if (bf->count > limit)
        printf("... (%d more not shown)\n", bf->count - limit);

    algo_result_t tj_r;
    cycle_result_t *tj = find_cycles_tarjan(g, &tj_r);

    printf("\n-----------------------------------------\n");
    printf(" Tarjan SCC Cycle Detection\n");
    printf("-----------------------------------------\n");
    printf("Time: %.3f ms\n", tj_r.execution_time);
    printf("Cycles Found: %d\n", tj->count);

    user_t *users = graph_get_users(g);

    for (int i = 0; i < tj->count; i++)
        subnetwork_add(&tj->cycles[i]);

    printf("\nIdentified Fraud Subnetworks: %d\n", subnet_count);
    printf("-----------------------------------------\n");

    for (int i = 0; i < subnet_count; i++) {
        subnetwork_t *s = &subnets[i];

        qsort(s->items, s->count, sizeof(fraud_cycle_t*), compare_cycles);

        printf("\nSubnetwork #%d\n", i + 1);
        printf("Root Node: %d (%s)\n", s->root, users[s->root].label);
        printf("Cycles in SCC:\n");

        for (int j = 0; j < s->count; j++) {
            printf("  ");
            print_cycle_id(s->items[j]);
            printf("\n");
        }
    }

    printf("\n=========================================\n");
    printf(" Summary\n");
    printf("=========================================\n");

    printf("Brute Force Cycles: %d\n", bf->count);
    printf("Tarjan Cycles:      %d\n", tj->count);
    printf("Subnetworks:        %d\n", subnet_count);

    printf("Speedup (BF/TJ): %.2fx\n",
           (tj_r.execution_time > 0)
               ? bf_r.execution_time / tj_r.execution_time
               : 0);

    for (int i = 0; i < subnet_count; i++)
        free(subnets[i].items);
    free(subnets);

    cycle_result_free(bf);
    cycle_result_free(tj);
    graph_free(g);
    timer_free(t0);

    return 0;
}
