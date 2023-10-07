#include "../headers/mem_space.h"
#include "../headers/mem.h"
#include "../headers/mem_os.h"
#include "../src/mem.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void test_free_null()
{
    mem_free(NULL);   
}

static void test_free_valid_zone()
{
     void *ptr = mem_alloc(16);
    mem_free(ptr);

    
    void *new_ptr = mem_alloc(16);
    assert(new_ptr != NULL);
    assert(new_ptr == ptr);
}

static void test_free_fusion()
{
    void *ptr1 = mem_alloc(64);

    mem_free(ptr1);
    mem_free((char *)ptr1 + 32);

    void *new_ptr = mem_alloc(64);
    
    assert(new_ptr != NULL);
    assert(new_ptr == ptr1);
}


static void test_free_out_of_start_memory()
{
    void *mem_start = mem_space_get_addr();
    void *ptr = mem_alloc(16);
    mem_free(ptr);
    mem_free((char *)mem_start - 1);
}

static void test_free_out_of_end_memory()
{
    void *mem_end = mem_space_get_addr();
    void *ptr = mem_alloc(16);
    mem_free(ptr);
    mem_free((char *)mem_end + 1);
}

int main(int argc, char *argv[])
{
    mem_init();
    test_free_null();
    test_free_valid_zone();
    test_free_fusion();
    test_free_out_of_start_memory();
    test_free_out_of_end_memory();
    printf("Tests succeeded\n");
    return 0;
}
