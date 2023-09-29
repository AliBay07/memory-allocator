//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#include "../headers/mem.h"
#include "../src/mem_space.c"
#include "../headers/mem_os.h"
#include <assert.h>

#define ALIGN 4;

/* structure pour les block dans la memoire */
struct fb
{
    size_t size;
    struct fb *next;
    struct fb *prev;
};

struct bb
{
    size_t size;
};

// structure utilisée pour gérer info_alloc, qui pointera vers le premier maillon
struct info_alloc
{
    struct fb *first;
    mem_fit_function_t *function_type;
};

struct info_alloc *info;

//-------------------------------------------------------------
// mem_init
//-------------------------------------------------------------
/**
 * Initialize the memory allocator.
 * If already init it will re-init.
 **/
void mem_init()
{
    info = (struct info_alloc *)mem_space_get_addr();

    struct fb *init_block = (struct fb *)((char *)info + sizeof(struct info_alloc));

    init_block->next = NULL;
    init_block->prev = NULL;

    init_block->size = mem_space_get_size() - (sizeof(struct info_alloc));

    info->first = init_block;
    mem_set_fit_handler(mem_first_fit);
}

//-------------------------------------------------------------
// mem_alloc
//-------------------------------------------------------------
/**
 * Allocate a bloc of the given size.
 **/
void *mem_alloc(size_t size)
{

    printf("Before : %zu", size);
    size = ALIGN(size);
    printf("After : %zu", size);

    mem_fit_function_t *get_correct_block = info->function_type;

    size_t total_size = size + sizeof(struct fb);

    struct fb *correct_block = (*get_correct_block)(info->first, total_size);

    if (correct_block == NULL)
    {
        return NULL;
    }

    size_t remaining_size = correct_block->size - total_size;

    if (remaining_size >= sizeof(struct fb))
    {
        struct fb *new_block = (struct fb *)(correct_block + (sizeof(struct bb) + size));
        new_block->size = remaining_size;

        if (correct_block->next != NULL)
        {
            new_block->next = correct_block->next;
        }
        
        new_block->prev = correct_block->prev;
    }

    struct bb* new_bb = (struct bb)correct_block;
    new_bb->size = size;

    return (void *)(new_bb + 1);
}

//-------------------------------------------------------------
// mem_get_size
//-------------------------------------------------------------
size_t mem_get_size(void *zone)
{
    // on vérifie que la zone n'est pas NULL
    if (zone != NULL)
    {
        return sizeof((char *)zone);
    }
    else
    {
        return 0;
    }
}

//-------------------------------------------------------------
// mem_free
//-------------------------------------------------------------
/**
 * Free an allocaetd bloc.
 **/
void mem_free(void *zone)
{
    if (zone == NULL)
    {
        return;
    }

    struct block *block_to_free = (struct block *)zone - 1;

    block_to_free->type = 1;

    while (block_to_free->prev != NULL && block_to_free->prev->type == 1)
    {
        struct block *prev_block = block_to_free->prev;
        prev_block->next = block_to_free->next;

        if (block_to_free->next != NULL)
        {
            block_to_free->next->prev = prev_block;
        }

        block_to_free = prev_block;
    }

    if (block_to_free->prev != NULL)
    {
        struct block *prev_block = block_to_free->prev;
        struct block *new_block = (struct block *)((char *)(prev_block + 1) + prev_block->size);
        new_block->next = block_to_free->next;
        new_block->prev = block_to_free->prev;
        new_block->type = 1;

        if (block_to_free->next != NULL)
        {
            block_to_free->next->prev = new_block;
        }

        if (block_to_free->prev != NULL)
        {
            block_to_free->prev->next = new_block;
        }

        block_to_free = new_block;
    }

    while (block_to_free->next != NULL && block_to_free->next->type == 1)
    {
        struct block *next_block = block_to_free->next;
        block_to_free->next = next_block->next;

        if (next_block->next != NULL)
        {
            next_block->next->prev = block_to_free;
        }
    }

    if (block_to_free->next != NULL)
    {
        block_to_free->size = (char *)block_to_free->next - (char *)block_to_free;
    }
    else
    {
        char *start_addr = (char *)mem_space_get_addr();
        size_t size = mem_space_get_size();
        char *end_addr = start_addr + size;
        block_to_free->size = end_addr - (char *)block_to_free;
    }
}

//-------------------------------------------------------------
// Itérateur(parcours) sur le contenu de l'allocateur
// mem_show
//-------------------------------------------------------------
void mem_show(void (*print)(void *, size_t, int))
{

    struct block *current_block = info->first;

    while (current_block != NULL)
    {
        print((void *)(current_block), current_block->size, current_block->type);
        current_block = current_block->next;
    }
}

//-------------------------------------------------------------
// mem_fit
//-------------------------------------------------------------
void mem_set_fit_handler(mem_fit_function_t *mff)
{
    info->function_type = mff;
}

//-------------------------------------------------------------
// Stratégies d'allocation
//-------------------------------------------------------------
mem_fit_function_t *mem_first_fit(struct fb *first_free_block, size_t wanted_size)
{

    struct bb *current_block = first_free_block;

    while (current_block != NULL)
    {
        if (current_block->size >= wanted_size)
        {
            return current_block;
        }

        current_block = current_block->next;
    }
    return NULL;
}
//-------------------------------------------------------------
mem_fit_function_t *mem_best_fit(struct fb *first_free_block, size_t wanted_size)
{
    // TODO: implement
    assert(!"NOT IMPLEMENTED !");
    return NULL;
}

//-------------------------------------------------------------
mem_fit_function_t *mem_worst_fit(struct fb *first_free_block, size_t wanted_size)
{
    // TODO: implement
    assert(!"NOT IMPLEMENTED !");
    return NULL;
}
