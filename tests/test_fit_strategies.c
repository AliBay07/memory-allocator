#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../headers/mem_space.h"
#include "../headers/mem.h"
#include "../headers/mem_os.h"
#include "../src/mem.c"

#define NB_MAX_STORES 100000

typedef struct {
    void* address;
    size_t size;
} AllocationRecord;

size_t generate_random_int(int min, int max) {
    size_t random_number = rand() % (max - min + 1) + min;
    return random_number;
}

void* allocate_and_track(size_t size, AllocationRecord* table, size_t* table_size, size_t* total_allocated) {
    void* allocated_mem = mem_alloc(size);
    if (allocated_mem != NULL) {
        if (*table_size < NB_MAX_STORES) {
            table[*table_size].address = allocated_mem;
            table[*table_size].size = size;
            (*table_size)++;
            (*total_allocated) += size;
        }
    }
    return allocated_mem;
}

void initialize_memory(size_t size) {
    void* allocated_blocks[NB_MAX_STORES];
    int block_index = 0;

    while (1) {
        void* allocated_block = mem_alloc(size);
        if (allocated_block == NULL) {
            break;
        }
        allocated_blocks[block_index++] = allocated_block;
    }

    for (int i = 1; i < block_index; i += 2) {
        mem_free(allocated_blocks[i]);
    }
}

/**
 * Exécute un test d'allocation de mémoire avec une stratégie de placement donnée.
 *
 * Paramètres :
 *   - fit_strategy (pointeur vers la stratégie de placement de mémoire)
 *   - min (taille minimale de l'allocation)
 *   - max (taille maximale de l'allocation)
 */
void run_allocation_test(mem_fit_function_t* fit_strategy, int min, int max) {
    size_t total_allocated = 0;

    AllocationRecord allocation_table[NB_MAX_STORES];
    size_t table_size = 0;

    mem_set_fit_handler(fit_strategy);

    size_t alloc_size = generate_random_int(min, max);
    initialize_memory(alloc_size);

    clock_t start_time = clock();

    while (1) {
        alloc_size = generate_random_int(min, max);

        void* allocated_mem = allocate_and_track(alloc_size, allocation_table, &table_size, &total_allocated);
        if (allocated_mem == NULL) {
            break;
        }
    }

    for (size_t i = 0; i < table_size; i++) {
        mem_free(allocation_table[i].address);
    }

    clock_t end_time = clock();
    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("Time taken: %.8f seconds\n\n", execution_time);
}

int main() {

    mem_fit_function_t* fit_strategies[] = {mem_first_fit, mem_best_fit, mem_worst_fit};
    const char* strategy_names[] = {"First Fit Strategy", "Best Fit Strategy", "Worst Fit Strategy"};
    int min_size = 20;
    int max_size = 30;

    for (int i = 0; i < 3; i++) {
        mem_init();
        printf("%s\n", strategy_names[i]);
        run_allocation_test(fit_strategies[i], min_size, max_size);
    }

    printf("Tests successed\n");
    return 0;
}
