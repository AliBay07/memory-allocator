#include "../headers/mem_space.h"
#include "../headers/mem.h"
#include "../headers/mem_os.h"
#include "../src/mem.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void  test_malloc_size0()
{
    void *ptr = mem_alloc(0);
    assert(ptr == NULL);
}

static void test_malloc(size_t size)
{
    void *ptr = mem_alloc(size);
    assert(ptr != NULL);
}

static void test_malloc_invalid_zone(size_t size)
{
    void *ptr1 = mem_alloc(size);
    void *ptr2 = mem_alloc(size);

    assert(ptr2 == NULL);
}

static void  test_malloc_out_of_end_memory()
{
    size_t mem_size = mem_space_get_size();

    size_t too_large_size = mem_size + 1;
    void *ptr = mem_alloc(too_large_size);
    assert(ptr == NULL);
}

static void test_malloc_out_of_start_memory()
{
    void *mem_start = mem_space_get_addr();
    void *too_small_size = (void *)((char *)mem_start - 1);
    void *ptr = mem_alloc((size_t)too_small_size);
    assert(ptr == NULL);
}



int main(int argc, char *argv[]) {
    mem_init();
    test_malloc_size0();
    test_malloc(16);
    test_malloc_out_of_end_memory();
    test_malloc_out_of_start_memory();
    printf("Tests successed\n");
    return 0;
}




