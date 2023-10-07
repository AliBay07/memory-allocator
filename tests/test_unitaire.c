#include "../headers/mem_space.h"
#include "../headers/mem.h"
#include "../headers/mem_os.h"
#include "../src/mem.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>


#define NB_TESTS 5

//test de mem_alloc cas general, 1er block, dernier block


static void * test_malloc(size_t size)
{
    if(size == 0)
    {
        return NULL;
    }
    
}




//test de mem_free cas general, cas fusion, 1er block, dernier block


