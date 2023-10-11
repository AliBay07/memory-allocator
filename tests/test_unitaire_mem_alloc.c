#include "../headers/mem_space.h"
#include "../headers/mem.h"
#include "../headers/mem_os.h"
#include "../src/mem.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Teste la fonction mem_alloc avec une taille de 0.
 * 
 * Description : Cette fonction teste mem_alloc en essayant d'allouer
 * une zone mémoire de taille 0. Elle vérifie que la fonction renvoie
 * NULL, ce qui est attendu dans ce cas.
 */
void test_malloc_size0()
{
    void *ptr = mem_alloc(0);
    assert(ptr == NULL);
}

/**
 * Teste la fonction mem_alloc avec une taille spécifiée.
 * 
 * Paramètres : size (taille de l'allocation)
 * 
 * Description : Cette fonction teste mem_alloc en essayant d'allouer
 * une zone mémoire de la taille spécifiée. Elle vérifie que la fonction
 * renvoie un pointeur non NULL, indiquant une allocation réussie.
 */
void test_malloc(size_t size)
{
    void *ptr = mem_alloc(size);
    assert(ptr != NULL);
}

/**
 * Teste la fonction mem_alloc en demandant une taille supérieure à la mémoire disponible.
 * 
 * Description : Cette fonction teste mem_alloc en essayant d'allouer une zone mémoire
 * dont la taille est supérieure à la taille totale de la mémoire disponible. Elle vérifie
 * que la fonction renvoie NULL, car l'allocation ne peut pas être satisfaite.
 */
void test_malloc_out_of_end_memory()
{
    size_t mem_size = mem_space_get_size();
    size_t too_large_size = mem_size + 1;
    void *ptr = mem_alloc(too_large_size);
    assert(ptr == NULL);
}

/**
 * Teste la fonction mem_alloc en demandant une taille négative.
 * 
 * Description : Cette fonction teste mem_alloc en essayant d'allouer une zone mémoire
 * avec une taille négative. Elle vérifie que la fonction renvoie NULL, car une taille
 * négative n'est pas valide pour une allocation.
 */
void test_malloc_out_of_start_memory()
{
    void *mem_start = mem_space_get_addr();
    void *too_small_size = (void *)((char *)mem_start - 1);
    void *ptr = mem_alloc((size_t)too_small_size);
    assert(ptr == NULL);
}

int main() {

    mem_fit_function_t *fit_functions[] = {mem_first_fit, mem_best_fit, mem_worst_fit};

    for (int i = 0; i < 3; i++) {
        mem_init();
        mem_set_fit_handler(fit_functions[i]);
        test_malloc_size0();
        test_malloc(16);
        test_malloc_out_of_end_memory();
        test_malloc_out_of_start_memory();
    }

    printf("Tests successed\n");
    return 0;
}