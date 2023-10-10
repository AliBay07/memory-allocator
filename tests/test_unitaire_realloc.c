#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../headers/mem_space.h"
#include "../headers/mem.h"
#include "../headers/mem_os.h"
#include "../src/mem.c"

int main()
{

    mem_init();

    // Test case 1: Reallocating a NULL pointer should behave like malloc
    int *ptr1 = (int *)mem_realloc(NULL, 5 * sizeof(int));
    assert(ptr1 != NULL);
    mem_free(ptr1);

    // Test case 2: Reallocating a non-NULL pointer with a larger size
    int *ptr2 = (int *)mem_alloc(3 * sizeof(int));
    assert(ptr2 != NULL);
    ptr2[0] = 1;
    ptr2[1] = 2;
    ptr2[2] = 3;

    int *newPtr2 = (int *)mem_realloc(ptr2, 5 * sizeof(int));
    assert(newPtr2 != NULL);
    assert(newPtr2[0] == 1);
    assert(newPtr2[1] == 2);
    assert(newPtr2[2] == 3);
    mem_free(newPtr2);

    // Test case 3: Reallocating a non-NULL pointer with a smaller size
    int *ptr3 = (int *)mem_alloc(5 * sizeof(int));
    assert(ptr3 != NULL);
    ptr3[0] = 1;
    ptr3[1] = 2;
    ptr3[2] = 3;
    ptr3[3] = 4;
    ptr3[4] = 5;

    int *newPtr3 = (int *)mem_realloc(ptr3, 3 * sizeof(int));
    assert(newPtr3 != NULL);
    assert(newPtr3[0] == 1);
    assert(newPtr3[1] == 2);
    assert(newPtr3[2] == 3);
    mem_free(newPtr3);

    printf("Tests successed\n");

    return 0;
}
