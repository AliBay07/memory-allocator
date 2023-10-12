#include "../headers/mem_space.h"
#include "../headers/mem.h"
#include "../headers/mem_os.h"
#include "../src/mem.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Teste la fonction mem_free avec un pointeur NULL.
 * 
 * Description : Cette fonction teste la fonction mem_free en passant un pointeur NULL
 * en tant qu'argument. Elle vérifie que la fonction mem_free gère correctement le cas
 * où aucun bloc de mémoire n'est alloué et ne provoque pas d'erreur.
 */
void test_free_null()
{
    mem_free(NULL);   
}

/**
 * Teste la fonction mem_free avec une zone de mémoire valide.
 * 
 * Description : Cette fonction teste la fonction mem_free en allouant d'abord un bloc de
 * mémoire, puis en le libérant à l'aide de mem_free. Ensuite, elle alloue à nouveau un bloc
 * de mémoire de la même taille et vérifie que le nouveau pointeur n'est pas NULL et qu'il est
 * égal au pointeur initial.
 */
void test_free_valid_zone()
{
    void *ptr = mem_alloc(16);
    mem_free(ptr);

    void *new_ptr = mem_alloc(16);
    assert(new_ptr != NULL);
    assert(new_ptr == ptr);
}

/**
 * Test de la fusion des blocs mémoire libérés par la fonction mem_free.
 * 
 * Description : Cette fonction teste la fonction mem_free en allouant deux blocs de mémoire
 * de taille 32 octets. Ensuite, elle libère le premier bloc suivi du deuxième bloc.
 * Puis, elle alloue un nouveau bloc de mémoire de taille 64 octets.
 * Elle vérifie que le nouveau pointeur n'est pas NULL et qu'il est égal au pointeur du premier bloc initial,
 * montrant ainsi que les blocs ont été fusionnés.
 */
void test_free_fusion()
{
    void *ptr1 = mem_alloc(32);
    void *ptr2 = mem_alloc(32);

    mem_free(ptr1);
    mem_free(ptr2);

    void *new_ptr = mem_alloc(64);
    
    assert(new_ptr != NULL);
    assert(new_ptr == ptr1);
}

/**
 * Teste la libération d'un pointeur en dehors du début de la mémoire allouée.
 * 
 * Description : Cette fonction teste la fonction mem_free en allouant un bloc de mémoire
 * de 16 octets, puis en essayant de libérer une portion de mémoire avant le début de la zone
 * allouée. Elle vérifie que la fonction mem_free gère correctement cette situation et ne provoque pas
 * d'erreur.
 */
void test_free_out_of_start_memory()
{
    void *mem_start = mem_space_get_addr();
    void *ptr = mem_alloc(16);
    mem_free(ptr);
    mem_free((char *)mem_start - 1);
}

/**
 * Teste la libération d'un pointeur en dehors de la fin de la mémoire allouée.
 * 
 * Description : Cette fonction teste la fonction mem_free en allouant un bloc de mémoire
 * de 16 octets, puis en essayant de libérer une portion de mémoire après la fin de la zone
 * allouée. Elle vérifie que la fonction mem_free gère correctement cette situation et ne provoque pas
 * d'erreur.
 */
void test_free_out_of_end_memory()
{
    void *mem_end = mem_space_get_addr();
    void *ptr = mem_alloc(16);
    mem_free(ptr);
    mem_free((char *)mem_end + 1);
}


int main()
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
