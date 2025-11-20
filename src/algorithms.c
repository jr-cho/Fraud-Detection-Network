#include "../include/algorithms.h"
#include "../include/utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

static void normalize_cycle(int *cycle, int length) {
    if (length <= 0) return;

    int min_index = 0;
    for (int i = 1; i < length; i++)
        if (cycle[i] < cycle[min_index])
            min_index = i;

    int *tmp = malloc(length * sizeof(int));
    int *rev = malloc(length * sizeof(int));
    if (!tmp || !rev) {
        free(tmp);
        free(rev);
        return;
    }

    for (int i = 0; i < length; i++)
        tmp[i] = cycle[(min_index + i) % length];

    for (int i = 0; i < length; i++)
        rev[i] = tmp[length - 1 - i];

    int use_rev = 0;
    for (int i = 0; i < length; i++) {
        if (rev[i] < tmp[i]) { use_rev = 1; break; }
        if (rev[i] > tmp[i]) break;
    }

    memcpy(cycle, use_rev ? rev : tmp, length * sizeof(int));
    free(tmp);
    free(rev);
}

typedef struct {
    graph_t *g;
    cycle_result_t *result;
    int *visited;
    int *stack;
    int stack_len;
    int start;
} bf_ctx_t;

static void dfs_bf(bf_ctx_t *ctx, int node) {

    ctx->visited[node] = 1;
    ctx->stack[ctx->stack_len++] = node;

    int *adj = graph_get_adj_list(ctx->g);
    int n = graph_get_num_users(ctx->g);
    int *row = adj + node * n;

    for (int v = 0; v < n; v++) {

        if (!row[v])
            continue;

        if (v == ctx->start && ctx->stack_len >= 3) {
            int length = ctx->stack_len;
            int *cycle = malloc(length * sizeof(int));
            memcpy(cycle, ctx->stack, length * sizeof(int));
            normalize_cycle(cycle, length);
            cycle_result_add(ctx->result, cycle, length, 0.0);
            free(cycle);
        }

        if (!ctx->visited[v])
            dfs_bf(ctx, v);
    }

    ctx->visited[node] = 0;
    ctx->stack_len--;
}

cycle_result_t* find_cycles_dfs_bruteforce(graph_t *g, algo_result_t *result) {

    timer_t *t = timer_create();
    timer_start(t);

    int n = graph_get_num_users(g);

    bf_ctx_t ctx;
    ctx.g = g;
    ctx.result = cycle_result_create();
    ctx.visited = calloc(n, sizeof(int));
    ctx.stack = malloc(n * sizeof(int));
    ctx.stack_len = 0;

    for (int start = 0; start < n; start++) {
        ctx.start = start;
        memset(ctx.visited, 0, n * sizeof(int));
        ctx.stack_len = 0;
        dfs_bf(&ctx, start);
    }

    timer_stop(t);

    result->execution_time = timer_elapsed_ms(t);
    result->cycles_found   = ctx.result->count;

    free(ctx.visited);
    free(ctx.stack);
    timer_free(t);

    return ctx.result;
}

typedef struct {
    graph_t *g;
    cycle_result_t *result;
    int *stack;
    int *on_stack;
    int *ids;
    int *low;
    int stack_len;
    int id;
    int n;
} tarjan_t;

static int int_compare(const void *a, const void *b) {
    return (*(const int*)a - *(const int*)b);
}

static void dfs_tarjan(tarjan_t *t, int at) {

    t->ids[at] = t->low[at] = t->id++;
    t->stack[t->stack_len++] = at;
    t->on_stack[at] = 1;

    int *adj = graph_get_adj_list(t->g);
    int *row = adj + at * t->n;

    for (int v = 0; v < t->n; v++) {

        if (!row[v])
            continue;

        if (t->ids[v] == -1) {
            dfs_tarjan(t, v);
            t->low[at] = (t->low[v] < t->low[at]) ? t->low[v] : t->low[at];
        }
        else if (t->on_stack[v]) {
            t->low[at] = (t->ids[v] < t->low[at]) ? t->ids[v] : t->low[at];
        }
    }

    if (t->ids[at] == t->low[at]) {

        int *nodes = malloc(t->n * sizeof(int));
        int count = 0;

        while (1) {
            int node = t->stack[--t->stack_len];
            t->on_stack[node] = 0;
            nodes[count++] = node;
            if (node == at) break;
        }

        if (count > 1) {

            qsort(nodes, count, sizeof(int), int_compare);

            for (int i = 0; i < count; i++) {

                int *cycle = malloc(count * sizeof(int));

                for (int j = 0; j < count; j++)
                    cycle[j] = nodes[(i + j) % count];

                normalize_cycle(cycle, count);
                cycle_result_add(t->result, cycle, count, 0.0);

                free(cycle);
            }
        }

        free(nodes);
    }
}

cycle_result_t* find_cycles_tarjan(graph_t *g, algo_result_t *result) {

    int n = graph_get_num_users(g);

    tarjan_t t;
    t.g = g;
    t.result = cycle_result_create();
    t.n = n;

    t.stack = malloc(n * sizeof(int));
    t.on_stack = calloc(n, sizeof(int));
    t.ids = malloc(n * sizeof(int));
    t.low = malloc(n * sizeof(int));
    t.stack_len = 0;
    t.id = 0;

    for (int i = 0; i < n; i++) {
        t.ids[i] = -1;
        t.low[i] = 0;
        t.on_stack[i] = 0;
    }

    timer_t *tm = timer_create();
    timer_start(tm);

    for (int i = 0; i < n; i++)
        if (t.ids[i] == -1)
            dfs_tarjan(&t, i);

    timer_stop(tm);

    result->execution_time = timer_elapsed_ms(tm);
    result->cycles_found   = t.result->count;

    free(t.stack);
    free(t.on_stack);
    free(t.ids);
    free(t.low);
    timer_free(tm);

    return t.result;
}
