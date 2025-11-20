#include "../include/algorithms.h"
#include "../include/utils.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

typedef struct {
    int *path;
    int *visited;
    int path_len;
    int start_node;
    int max_depth;
    cycle_result_t *result;
    graph_t *g;
} dfs_context_t;

static void dfs_bruteforce_recursive(dfs_context_t *ctx, int node, int depth) {
    if (depth > ctx->max_depth) return;
    
    int *adj_list = graph_get_adj_list(ctx->g);
    int num_users = graph_get_num_users(ctx->g);
    int *row = adj_list + node * num_users;
    
    for (int next = 0; next < num_users; next++) {
        if (row[next] == 0) continue;
        
        if (next == ctx->start_node && ctx->path_len >= 3) {
            double total = 0.0;
            for (int i = 0; i < ctx->path_len; i++) {
                int from = ctx->path[i];
                int to = ctx->path[(i + 1) % ctx->path_len];
                int *from_row = adj_list + from * num_users;
                total += from_row[to];
            }
            cycle_result_add(ctx->result, ctx->path, ctx->path_len, total);
        } else if (!ctx->visited[next] && next != ctx->start_node) {
            ctx->visited[next] = 1;
            ctx->path[ctx->path_len++] = next;
            dfs_bruteforce_recursive(ctx, next, depth + 1);
            ctx->path_len--;
            ctx->visited[next] = 0;
        }
    }
}

cycle_result_t* find_cycles_dfs_bruteforce(graph_t *g, algo_result_t *result) {
    timer_t *timer = timer_create();
    timer_start(timer);
    
    cycle_result_t *cycles = cycle_result_create();
    int num_users = graph_get_num_users(g);
    
    dfs_context_t ctx;
    ctx.path = (int *)malloc(num_users * sizeof(int));
    ctx.visited = (int *)calloc(num_users, sizeof(int));
    ctx.result = cycles;
    ctx.g = g;
    ctx.max_depth = fmin(num_users, 8);
    
    for (int start = 0; start < num_users; start++) {
        if (start % 10 == 0) {
            printf("    [DFS Progress] Processing node %d/%d (cycles found: %d)\n", start, num_users, cycles->count);
            fflush(stdout);
        }
        
        ctx.start_node = start;
        ctx.path_len = 1;
        ctx.path[0] = start;
        memset(ctx.visited, 0, num_users * sizeof(int));
        ctx.visited[start] = 1;
        
        dfs_bruteforce_recursive(&ctx, start, 0);
    }
    
    free(ctx.path);
    free(ctx.visited);
    
    timer_stop(timer);
    result->execution_time = timer_elapsed_sec(timer);
    result->cycles_found = cycles->count;
    timer_free(timer);
    
    return cycles;
}

typedef struct {
    int *ids;
    int *low;
    int *on_stack;
    int *stack;
    int stack_ptr;
    int id_counter;
    cycle_result_t *result;
    graph_t *g;
} tarjan_context_t;

static void tarjan_dfs(tarjan_context_t *ctx, int node) {
    int num_users = graph_get_num_users(ctx->g);
    int *adj_list = graph_get_adj_list(ctx->g);
    
    ctx->ids[node] = ctx->id_counter;
    ctx->low[node] = ctx->id_counter;
    ctx->id_counter++;
    ctx->stack[ctx->stack_ptr++] = node;
    ctx->on_stack[node] = 1;
    
    int *row = adj_list + node * num_users;
    
    for (int next = 0; next < num_users; next++) {
        if (row[next] == 0) continue;
        
        if (ctx->ids[next] == -1) {
            tarjan_dfs(ctx, next);
        }
        
        if (ctx->on_stack[next]) {
            ctx->low[node] = fmin(ctx->low[node], ctx->low[next]);
        }
    }
    
    if (ctx->ids[node] == ctx->low[node]) {
        int scc_size = 0;
        int *scc = (int *)malloc(num_users * sizeof(int));
        
        while (1) {
            int popped = ctx->stack[--ctx->stack_ptr];
            ctx->on_stack[popped] = 0;
            scc[scc_size++] = popped;
            if (popped == node) break;
        }
        
        if (scc_size > 1) {
            double total = 0.0;
            for (int i = 0; i < scc_size; i++) {
                int from = scc[i];
                int to = scc[(i + 1) % scc_size];
                int *from_row = adj_list + from * num_users;
                total += from_row[to];
            }
            cycle_result_add(ctx->result, scc, scc_size, total);
        }
        
        free(scc);
    }
}

cycle_result_t* find_cycles_tarjan(graph_t *g, algo_result_t *result) {
    timer_t *timer = timer_create();
    timer_start(timer);
    
    cycle_result_t *cycles = cycle_result_create();
    int num_users = graph_get_num_users(g);
    
    tarjan_context_t ctx;
    ctx.ids = (int *)malloc(num_users * sizeof(int));
    ctx.low = (int *)malloc(num_users * sizeof(int));
    ctx.on_stack = (int *)calloc(num_users, sizeof(int));
    ctx.stack = (int *)malloc(num_users * sizeof(int));
    ctx.stack_ptr = 0;
    ctx.id_counter = 0;
    ctx.result = cycles;
    ctx.g = g;
    
    memset(ctx.ids, -1, num_users * sizeof(int));
    memset(ctx.low, -1, num_users * sizeof(int));
    
    for (int i = 0; i < num_users; i++) {
        if (ctx.ids[i] == -1) {
            tarjan_dfs(&ctx, i);
        }
    }
    
    free(ctx.ids);
    free(ctx.low);
    free(ctx.on_stack);
    free(ctx.stack);
    
    timer_stop(timer);
    result->execution_time = timer_elapsed_sec(timer);
    result->cycles_found = cycles->count;
    timer_free(timer);
    
    return cycles;
}
