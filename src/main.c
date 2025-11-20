#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/graph.h"
#include "../include/algorithms.h"
#include "../include/utils.h"

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));
    
    int num_users = 100;
    int num_transactions = 5000;
    
    if (argc > 1) num_users = atoi(argv[1]);
    if (argc > 2) num_transactions = atoi(argv[2]);
    
    printf("Users: %d | Transactions: %d\n", num_users, num_transactions);
    
    timer_t *create_timer = timer_create();
    timer_start(create_timer);
    
    graph_t *g = graph_create(num_users, num_transactions);
    
    // Generate transactions
    int txns_added = 0;
    
    // First, create intentional fraud cycles (15% of transactions)
    int num_cycles = (num_transactions * 15) / 100;
    for (int c = 0; c < num_cycles; c++) {
        int cycle_length = 3 + (rand() % 5);  // 3-7 node cycles
        int *cycle_nodes = (int *)malloc(cycle_length * sizeof(int));
        
        // Select random unique nodes for cycle
        for (int i = 0; i < cycle_length; i++) {
            cycle_nodes[i] = rand() % num_users;
        }
        
        // Create transactions around the cycle
        for (int i = 0; i < cycle_length && txns_added < num_transactions; i++) {
            int from = cycle_nodes[i];
            int to = cycle_nodes[(i + 1) % cycle_length];
            double amount = 100.0 + (rand() / (double)RAND_MAX) * 4900.0;
            graph_add_transaction(g, from, to, amount, (long)time(NULL) + txns_added);
            txns_added++;
        }
        
        free(cycle_nodes);
    }
    
    // Fill remaining transactions with random non-cycle data
    while (txns_added < num_transactions) {
        int from = rand() % num_users;
        int to = rand() % num_users;
        
        if (from != to) {
            double amount = 1.0 + (rand() / (double)RAND_MAX) * 9999.0;
            graph_add_transaction(g, from, to, amount, (long)time(NULL) + txns_added);
            txns_added++;
        }
    }
    
    timer_stop(create_timer);
    printf("Graph creation: %.3f ms\n", timer_elapsed_ms(create_timer));
    
    graph_print_stats(g);
    
    printf("\nDFS Brute Force:\n");
    algo_result_t dfs_result;
    cycle_result_t *dfs_cycles = find_cycles_dfs_bruteforce(g, &dfs_result);
    printf("Time: %.6f sec | Cycles: %d\n", dfs_result.execution_time, dfs_result.cycles_found);
    
    printf("\nTarjan's Algorithm:\n");
    algo_result_t tarjan_result;
    cycle_result_t *tarjan_cycles = find_cycles_tarjan(g, &tarjan_result);
    printf("Time: %.6f sec | Cycles: %d\n", tarjan_result.execution_time, tarjan_result.cycles_found);
    
    printf("\nSpeedup: %.2fx\n", dfs_result.execution_time / tarjan_result.execution_time);
    
    cycle_result_free(dfs_cycles);
    cycle_result_free(tarjan_cycles);
    graph_free(g);
    timer_free(create_timer);
    
    return 0;
}
