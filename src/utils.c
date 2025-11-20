#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cjson/cJSON.h>

// ============== TIMER STRUCT ==============

typedef struct Timer {
    clock_t start;
    clock_t end;
} timer_t;

// ============== TIMING UTILITIES ==============

timer_t* timer_create(void) {
    timer_t *t = (timer_t *)malloc(sizeof(timer_t));
    t->start = 0;
    t->end = 0;
    return t;
}

void timer_start(timer_t *t) {
    t->start = clock();
}

void timer_stop(timer_t *t) {
    t->end = clock();
}

double timer_elapsed_sec(timer_t *t) {
    return (double)(t->end - t->start) / CLOCKS_PER_SEC;
}

double timer_elapsed_ms(timer_t *t) {
    return (double)(t->end - t->start) / CLOCKS_PER_SEC * 1000.0;
}

void timer_free(timer_t *t) {
    free(t);
}

// ============== JSON PARSING ==============

graph_t* load_graph_from_json(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *buffer = (char *)malloc(size + 1);
    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    fclose(file);
    
    cJSON *json = cJSON_Parse(buffer);
    if (!json) {
        fprintf(stderr, "Failed to parse JSON\n");
        free(buffer);
        return NULL;
    }
    
    cJSON *users_item = cJSON_GetObjectItem(json, "users");
    cJSON *transactions_item = cJSON_GetObjectItem(json, "transactions");
    
    int num_users = cJSON_GetArraySize(users_item);
    int num_transactions = cJSON_GetArraySize(transactions_item);
    
    graph_t *g = graph_create(num_users, num_transactions);
    
    // Load users
    int idx = 0;
    cJSON *user = NULL;
    cJSON_ArrayForEach(user, users_item) {
        cJSON *id = cJSON_GetObjectItem(user, "user_id");
        cJSON *balance = cJSON_GetObjectItem(user, "balance");
        user_t *users = graph_get_users(g);
        users[idx].user_id = id->valueint;
        users[idx].balance = balance->valuedouble;
        snprintf(users[idx].account_number, 32, "ACC_%06d", id->valueint);
        idx++;
    }
    
    // Load transactions
    cJSON *txn = NULL;
    cJSON_ArrayForEach(txn, transactions_item) {
        cJSON *from = cJSON_GetObjectItem(txn, "from_user");
        cJSON *to = cJSON_GetObjectItem(txn, "to_user");
        cJSON *amount = cJSON_GetObjectItem(txn, "amount");
        cJSON *timestamp = cJSON_GetObjectItem(txn, "timestamp");
        
        graph_add_transaction(g, from->valueint, to->valueint, 
                            amount->valuedouble, timestamp->valueint);
    }
    
    cJSON_Delete(json);
    free(buffer);
    
    return g;
}

// ============== DATA GENERATION ==============

void generate_banking_data(const char *output_file, int num_users, int num_transactions) {
    srand((unsigned int)time(NULL));
    // Create JSON structure
    cJSON *root = cJSON_CreateObject();
    cJSON *metadata = cJSON_CreateObject();
    cJSON *users_array = cJSON_CreateArray();
    cJSON *transactions_array = cJSON_CreateArray();
    
    cJSON_AddNumberToObject(metadata, "num_users", num_users);
    cJSON_AddNumberToObject(metadata, "num_transactions", num_transactions);
    cJSON_AddNumberToObject(metadata, "fraud_rate", 0.15);
    cJSON_AddItemToObject(root, "metadata", metadata);
    cJSON_AddItemToObject(root, "users", users_array);
    cJSON_AddItemToObject(root, "transactions", transactions_array);
    
    // Generate users
    for (int i = 0; i < num_users; i++) {
        cJSON *user = cJSON_CreateObject();
        cJSON_AddNumberToObject(user, "user_id", i);
        cJSON_AddNumberToObject(user, "balance", 5000.0 + (rand() % 90000));
        cJSON_AddItemToArray(users_array, user);
    }
    
    // Generate transactions with fraud cycles
    int base_time = (int)time(NULL);
    
    for (int i = 0; i < num_transactions; i++) {
        cJSON *txn = cJSON_CreateObject();
        
        // 15% chance of fraud cycle
        if (rand() % 100 < 15) {
            int cycle_len = 3 + (rand() % 4);  // 3-6 node cycles
            int start = rand() % num_users;
            int current = start;
            
            for (int j = 0; j < cycle_len && i < num_transactions; j++) {
                int next = rand() % num_users;
                while (next == current) next = rand() % num_users;
                
                cJSON *cycle_txn = cJSON_CreateObject();
                cJSON_AddNumberToObject(cycle_txn, "from_user", current);
                cJSON_AddNumberToObject(cycle_txn, "to_user", next);
                cJSON_AddNumberToObject(cycle_txn, "amount", 100.0 + (rand() % 5000));
                cJSON_AddNumberToObject(cycle_txn, "timestamp", base_time + i);
                cJSON_AddItemToArray(transactions_array, cycle_txn);
                
                current = next;
                i++;
            }
            i--;  // Adjust loop counter
        } else {
            // Normal transaction
            int from = rand() % num_users;
            int to = rand() % num_users;
            if (from != to) {
                cJSON_AddNumberToObject(txn, "from_user", from);
                cJSON_AddNumberToObject(txn, "to_user", to);
                cJSON_AddNumberToObject(txn, "amount", 10.0 + (rand() % 10000));
                cJSON_AddNumberToObject(txn, "timestamp", base_time + i);
                cJSON_AddItemToArray(transactions_array, txn);
            } else {
                cJSON_Delete(txn);
            }
        }
    }
    
    // Write to file
    char *json_str = cJSON_Print(root);
    FILE *file = fopen(output_file, "w");
    if (file) {
        fprintf(file, "%s\n", json_str);
        fclose(file);
        printf("✓ Generated %s\n", output_file);
    }
    
    cJSON_Delete(root);
    free(json_str);
}
