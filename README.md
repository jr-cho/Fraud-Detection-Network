# Fraud Ring Detection Using Graph Algorithms

### *COP4531.01 — Algorithm Design & Analysis*

### *Brute Force Cycle Detection vs. Tarjan’s Strongly Connected Components Algorithm*

---

## Overview

This project implements a simplified financial fraud detection system using graph cycle analysis in C.
In this simulation:

* **Nodes** represent bank accounts
* **Directed edges** represent money transfers
* **Cycles** represent suspicious financial activity often associated with collusion or fraud rings

To analyze potential fraud rings, this project implements **two fundamentally different methods of cycle detection**:

1. **Brute Force DFS Cycle Enumeration**
2. **Tarjan’s Algorithm for Strongly Connected Components (SCCs)**

The goal is to highlight the dramatic difference between **exponential algorithms** and **optimal linear time algorithms**, a core learning objective of **COP4531.01 — Algorithm Design & Analysis**.

---

# Table of Contents

* [Background](#background)
* [Graph Model](#graph-model)
* [Brute-Force Cycle Detection](#1-brute-force-cycle-detection)

  * [How It Works](#how-brute-force-works)
  * [Main Code Block Explained](#main-brute-force-code-block-explained)
  * [Time Complexity](#brute-force-time-complexity)
* [Tarjan’s SCC Algorithm](#2-tarjans-strongly-connected-components-scc-algorithm)

  * [How It Works](#how-tarjan-works)
  * [Main Code Block Explained](#main-tarjan-code-block-explained)
  * [Time Complexity](#tarjan-time-complexity)
* [Performance Comparison](#performance-comparison)
* [Fraud Subnetworks](#fraud-subnetworks)
* [Educational Takeaways](#educational-takeaways)
* [Future Work](#future-work)

---

# Background

Detecting fraudulent activity often involves identifying **groups of accounts sending money to each other in loops**.
These loops cycles in a directed graph can indicate:

* Money laundering
* Synthetic account networks
* Coordinated fraud rings
* Transaction layering to obscure audit trails

This project compares two algorithms for detecting these cycles:

| Brute Force             | Tarjan’s Algorithm                      |
| ----------------------- | --------------------------------------- |
| Exhaustive search       | Linear time SCC detection               |
| Slow, exponential       | Fast, guaranteed                        |
| Finds some cycles       | Finds all SCCs + all cycles inside them |
| Used here for education | Used in real financial software         |

---

# Graph Model

The system uses a **directed adjacency matrix**:

```
adj[i][j] = 1  → account i sent money to account j
adj[i][j] = 0  → no transaction
```

Randomized edges + artificially injected cycles create a rich environment for algorithm analysis.

---

# 1. Brute Force Cycle Detection

## How Brute Force Works

Brute force attempts to explore **every possible path** from every starting node.

For each node `s`:

1. Perform DFS
2. Track visited nodes
3. Maintain a stack of the current path
4. If DFS returns to `s`, a cycle is found
5. Normalize the cycle (so duplicates match)
6. Continue exploring paths until:

   * depth limit reached (max length = 12)
   * expansion limit reached (e.g., ~2.5M expansions for ~10–15 seconds runtime)

This method is intentionally **computationally expensive**, demonstrating exponential complexity.

---

## Main Brute Force Code Block Explained

Below is the **core DFS** function that powers brute force cycle detection:

### **`dfs_bf()` — core brute force exploration**

```c
static void dfs_bf(bf_ctx_t *ctx, int node) {
    if (ctx->expansions++ > ctx->max_expansions)
        return;

    if (ctx->stack_len > 12)
        return;

    ctx->visited[node] = 1;
    ctx->stack[ctx->stack_len++] = node;

    int *adj = graph_get_adj_list(ctx->g);
    int n = graph_get_num_users(ctx->g);
    int *row = adj + node * n;

    for (int v = 0; v < n; v++) {
        if (row[v] == 0)
            continue;

        if (!ctx->reachable_to_start[v])
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
```

### **Key ideas demonstrated**

* **Exponential path explosion**
* **Visited state tracking**
* **Path stack construction**
* **Returning to root node identifies cycles**
* **Cycle normalization prevents duplicates**
* **Expansion limit prevents infinite runtime**

---

## Brute Force Time Complexity

Let:

* `V` = number of accounts
* `E` = number of edges
* `b` = average branching factor
* `d` = maximum depth allowed

Worst-case:

```
O(V * b^d)
```

This is **exponential**, meaning runtime becomes impractical for dense graphs.

---

# 2. Tarjan’s Strongly Connected Components (SCC) Algorithm

## How Tarjan Works

Tarjan’s algorithm finds all SCCs in **one DFS pass** using:

* `ids[]`: discovery time of each node
* `low[]`: the lowest discovery ID reachable
* A stack to track nodes in the current DFS path
* A global `id` counter

A root node of an SCC satisfies:

```
ids[node] == low[node]
```

Tarjan then pops all nodes in that SCC off the stack.

### Why this solves fraud detection

A fraud ring is a set of accounts where **every account can reach every other account**.
This is exactly the definition of an SCC.

---

## Main Tarjan Code Block Explained

### **`dfs_tarjan()` — core of Tarjan’s algorithm**

```c
static void dfs_tarjan(tarjan_t *t, int at) {
    t->ids[at] = t->low[at] = t->id++;
    t->stack[t->stack_len++] = at;
    t->on_stack[at] = 1;

    int *adj = graph_get_adj_list(t->g);
    int *row = adj + at * t->n;

    for (int v = 0; v < t->n; v++) {
        if (row[v] == 0)
            continue;

        if (t->ids[v] == -1) {
            dfs_tarjan(t, v);
            if (t->low[v] < t->low[at])
                t->low[at] = t->low[v];
        }
        else if (t->on_stack[v]) {
            if (t->ids[v] < t->low[at])
                t->low[at] = t->ids[v];
        }
    }

    if (t->ids[at] == t->low[at]) {
        int nodes[1024];
        int count = 0;

        while (1) {
            int node = t->stack[--t->stack_len];
            t->on_stack[node] = 0;
            nodes[count++] = node;
            if (node == at) break;
        }

        if (count > 1) {
            for (int i = 0; i < count; i++) {
                int *cycle = malloc(count * sizeof(int));
                for (int j = 0; j < count; j++)
                    cycle[j] = nodes[(i + j) % count];
                normalize_cycle(cycle, count);
                cycle_result_add(t->result, cycle, count, 0.0);
                free(cycle);
            }
        }
    }
}
```

### Key ideas demonstrated

* **Low link values**
* **Stack based DFS tracking**
* **SCC root detection condition**
* **Single pass cycle grouping**
* **Extraction of all cycles in SCC**

Tarjan’s algorithm outputs **complete fraud subnetworks**, not just individual cycles.

---

## Tarjan Time Complexity

Tarjan runs in:

```
O(V + E)
```

This is **optimal**.
No algorithm can asymptotically outperform this for SCC detection.

---

# Performance Comparison

| Feature         | Brute Force DFS         | Tarjan SCC              |
| --------------- | ----------------------- | ----------------------- |
| Time Complexity | Exponential             | Linear                  |
| Runtime         | ~10–15 seconds (capped) | < 5 ms                  |
| Cycles Found    | Subset                  | Complete cycle sets     |
| Scalability     | Poor                    | Excellent               |
| Practical Use   | Academic demo           | Real production systems |

Tarjan is many orders of magnitude faster and more reliable.

---

# Fraud Subnetworks

After Tarjan identifies SCCs, the system groups cycles into subnetworks:

* **Root node** (canonical node of SCC)
* **Cycle set** inside each SCC
* **Labels** associated with accounts
* **Summary identifying suspicious clusters**

This transforms raw cycle output into a clearer analytical picture.
