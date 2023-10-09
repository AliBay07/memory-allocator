#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../headers/mem_space.h"
#include "../headers/mem.h"
#include "../headers/mem_os.h"
#include "../src/mem.c"

#define NB_MAX_STORES 1000

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

void small_size_allocations() {
    size_t total_allocated = 0;
    size_t mem_size = mem_space_get_size();
    int min = 5;
    int max = 10;

    AllocationRecord allocation_table[NB_MAX_STORES];
    size_t table_size = 0;

    clock_t start_time = clock();

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

void big_size_allocations() {
    size_t total_allocated = 0;
    size_t mem_size = mem_space_get_size();
    int min = 20;
    int max = 30;

    AllocationRecord allocation_table[NB_MAX_STORES];
    size_t table_size = 0;

    clock_t start_time = clock();

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


int main(int argc, char *argv[])
{
    mem_init();

    small_size_allocations();
    big_size_allocations();

    printf("Tests succeeded.\n");
    return 0;
}