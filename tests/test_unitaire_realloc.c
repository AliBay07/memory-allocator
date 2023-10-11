#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../headers/mem_space.h"
#include "../headers/mem.h"
#include "../headers/mem_os.h"
#include "../src/mem.c"

/**
 * Teste mem_realloc avec un pointeur NULL.
 * 
 * Description : Cette fonction teste mem_realloc en passant un pointeur NULL
 * en tant qu'argument. Elle vérifie que mem_realloc alloue un nouveau bloc de
 * mémoire de la taille spécifiée et renvoie un pointeur non NULL.
 */
void test_pointer_NULL()
{
    int *ptr1 = (int *)mem_realloc(NULL, 5 * sizeof(int));
    assert(ptr1 != NULL);
    mem_free(ptr1);
}

/**
 * Teste mem_realloc pour agrandir une zone mémoire allouée précédemment.
 * 
 * Description : Cette fonction alloue d'abord une zone mémoire, y écrit des
 * données, puis utilise mem_realloc pour agrandir la zone. Elle vérifie que le
 * pointeur renvoyé par mem_realloc est non NULL et que les données précédentes
 * sont toujours présentes dans la zone agrandie.
 */
void test_realloc_larger()
{
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
}

/**
 * Teste mem_realloc pour réduire la taille d'une zone mémoire allouée précédemment.
 * 
 * Description : Cette fonction alloue d'abord une zone mémoire, y écrit des données,
 * puis utilise mem_realloc pour réduire la taille de la zone. Elle vérifie que le pointeur
 * renvoyé par mem_realloc est non NULL et que les données précédentes sont toujours présentes
 * dans la zone réduite.
 */
void test_realloc_smaller()
{
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
}

int main()
{

    mem_fit_function_t *fit_functions[] = {mem_first_fit, mem_best_fit, mem_worst_fit};

    for (int i = 0; i < 3; i++)
    {
        mem_init();
        mem_set_fit_handler(fit_functions[i]);
        test_pointer_NULL();
        test_realloc_larger();
        test_realloc_smaller();
    }
    
    printf("Tests successed\n");

    return 0;
}
