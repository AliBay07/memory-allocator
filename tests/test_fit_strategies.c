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

/**
 * Exécute un test d'allocation de mémoire avec une stratégie de placement donnée.
 * 
 * Paramètres : 
 *   - fit_strategy (pointeur vers la stratégie de placement de mémoire)
 *   - min (taille minimale de l'allocation)
 *   - max (taille maximale de l'allocation)
 * 
 * Description : Cette fonction exécute un test d'allocation de mémoire en utilisant la
 * stratégie de placement spécifiée. Elle alloue de manière aléatoire des zones mémoire
 * de tailles comprises entre "min" et "max," en suivant la stratégie de placement donnée.
 * Les allocations et les libérations sont suivies et enregistrées dans la table "allocation_table."
 * La fonction mesure le temps d'exécution du test et affiche le rapport mémoire utilisée sur
 * la mémoire totale disponible, ainsi que le temps écoulé.
 */
void run_allocation_test(mem_fit_function_t* fit_strategy, int min, int max) {
    size_t total_allocated = 0;
    size_t mem_size = mem_space_get_size();

    AllocationRecord allocation_table[NB_MAX_STORES];
    size_t table_size = 0;

    clock_t start_time = clock();

    mem_set_fit_handler(fit_strategy);

    while (1) {
        size_t alloc_size = generate_random_int(min, max);

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

    printf("Memory ratio when allocating sizes between %d and %d bytes: %f\n", min, max, (double)total_allocated / mem_size);
    printf("Time taken: %.8f seconds\n\n", execution_time);
}

int main() {
    mem_init();

    mem_fit_function_t* fit_strategies[] = {mem_first_fit, mem_best_fit, mem_worst_fit};
    const char* strategy_names[] = {"First Fit Strategy", "Best Fit Strategy", "Worst Fit Strategy"};
    int min_size = 20;
    int max_size = 30;

    for (int i = 0; i < 3; i++) {
        printf("%s\n", strategy_names[i]);
        run_allocation_test(fit_strategies[i], min_size, max_size);
    }

    printf("Tests succeeded.\n");
    return 0;
}

