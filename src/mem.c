//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#include "../headers/mem.h"
#include "../src/mem_space.c"
#include "../headers/mem_os.h"
#include <assert.h>

/* fb pour free block */
struct fb { 
	size_t size ;
	struct fb *next ;
};

// structure utilisée pour gérer info_alloc, qui pointera vers le premier maillon libre 
struct info_alloc {
	struct fb* first;
};

struct info_alloc* info;

//-------------------------------------------------------------
// mem_init
//-------------------------------------------------------------
/**
 * Initialize the memory allocator.
 * If already init it will re-init.
**/
void mem_init() {

    // Récupérer l'adresse de la mémoire la convertir en un pointeur vers struct info_alloc
    info = (struct info_alloc*)mem_space_get_addr();

    // Calculer l'adresse de init_fb en fonction de la taille de struct info_alloc
    struct fb* init_fb = (struct fb*)((char*)info + sizeof(struct info_alloc));

    // Initialiser le champ next de init_fb à NULL
    init_fb->next = NULL;

    // Calculer la taille de init_fb en fonction de la taille totale de mémoire disponible
    // moins la taille de struct info_alloc et struct fb
    init_fb->size = mem_space_get_size() - (sizeof(struct info_alloc) + sizeof(struct fb));

    // Faire pointer info->first vers init_fb
    info->first = init_fb;

    // assert(!"NON IMPLEMENTÉ !");
}


//-------------------------------------------------------------
// mem_alloc
//-------------------------------------------------------------
/**
 * Allocate a bloc of the given size.
**/
void* mem_alloc(size_t size) {
    // Récupérer le premier maillon de la liste chaînée
    struct fb* head = info->first;
    
    // Initialiser le pointeur courant au premier maillon
    struct fb* current_node = head;
    
    // Initialiser le pointeur précédent avec le premier maillon
    struct fb* previous_node = current_node;

    // Parcourir la liste chaînée
    while (current_node != NULL) {
        // Vérifier si la taille du maillon actuel est inférieure ou égale à (size + sizeof(struct fb))
        if (current_node->size <= (size + sizeof(struct fb))) {

            // Calculer la nouvelle adresse en ajoutant l'offset à l'adresse du maillon actuel
            char* new_address = (char*)current_node + (size + sizeof(struct fb));
            
            // Mettre à jour le pointeur current_node avec la nouvelle adresse
            current_node = (struct fb*)new_address;

            // Mettre à jour la taille du nouveau maillon current_node
            current_node->size -= (size + sizeof(struct fb));

            // Retourner un pointeur vers le maillon précédent
            return (void*)previous_node;
        }

        // Mettre à jour le pointeur précédent avec le pointeur courant
        previous_node = current_node;
        
        // Passer au maillon suivant dans la liste chaînée
        current_node = current_node->next;
    }

    // Si aucune condition n'est satisfaite, retourner NULL
    return NULL;
}


//-------------------------------------------------------------
// mem_get_size
//-------------------------------------------------------------
size_t mem_get_size(void * zone)
{
    //TODO: implement
	assert(! "NOT IMPLEMENTED !");
    return 0;
}

//-------------------------------------------------------------
// mem_free
//-------------------------------------------------------------
/**
 * Free an allocaetd bloc.
**/
void mem_free(void *zone) {
    //TODO: implement
	assert(! "NOT IMPLEMENTED !");
}

//-------------------------------------------------------------
// Itérateur(parcours) sur le contenu de l'allocateur
// mem_show
//-------------------------------------------------------------
void mem_show(void (*print)(void *, size_t, int free)) {
    //TODO: implement
	assert(! "NOT IMPLEMENTED !");
}

//-------------------------------------------------------------
// mem_fit
//-------------------------------------------------------------
void mem_set_fit_handler(mem_fit_function_t *mff) {
	//TODO: implement
	assert(! "NOT IMPLEMENTED !");
}

//-------------------------------------------------------------
// Stratégies d'allocation
//-------------------------------------------------------------
mem_free_block_t *mem_first_fit(mem_free_block_t *first_free_block, size_t wanted_size) {
    //TODO: implement
	assert(! "NOT IMPLEMENTED !");
	return NULL;
}
//-------------------------------------------------------------
mem_free_block_t *mem_best_fit(mem_free_block_t *first_free_block, size_t wanted_size) {
    //TODO: implement
	assert(! "NOT IMPLEMENTED !");
	return NULL;
}

//-------------------------------------------------------------
mem_free_block_t *mem_worst_fit(mem_free_block_t *first_free_block, size_t wanted_size) {
    //TODO: implement
	assert(! "NOT IMPLEMENTED !");
	return NULL;
}
