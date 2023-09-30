//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#include "../headers/mem.h"
#include "../src/mem_space.c"
#include "../headers/mem_os.h"
#include <assert.h>

// #define ALIGN 4;

/* structure pour les block dans la memoire */
struct fb
{
    size_t free_size;
    struct fb *next;
};

struct bb
{
    size_t busy_size;
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

    init_block->free_size = mem_space_get_size() - (sizeof(struct info_alloc));

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
    mem_fit_function_t *get_correct_block = info->function_type;

    size_t total_size = size + sizeof(struct bb);

    struct fb *previous_block = (struct fb *)(*get_correct_block)((mem_free_block_t *)info->first, total_size);
    struct fb *correct_block = previous_block != NULL ? previous_block->next : (struct fb *)info->first;
    struct fb *new_block = NULL;

    if (correct_block == NULL)
    {
        return NULL;
    }

    size_t remaining_size = correct_block->free_size - total_size;

    if (remaining_size >= sizeof(struct fb))
    {
        new_block = (struct fb *)((char *)correct_block + total_size);
        new_block->next = correct_block->next;
        correct_block->next = new_block;
        new_block->free_size = remaining_size;
    }
    else
    {
        if (correct_block->next != NULL && (char *)correct_block->next == (char *)correct_block + correct_block->free_size)
        {
            struct fb *next_block = correct_block->next;
            size_t next_block_size = next_block->free_size;
            struct fb *next_block_next = next_block->next;

            next_block = (struct fb *)((char *)next_block - remaining_size);
            next_block->next = next_block_next;
            next_block->free_size = next_block_size + remaining_size;

            correct_block->next = next_block;
        }
        else
        {
            size += remaining_size;
        }
    }

    if (previous_block == NULL)
    {
        info->first = (new_block != NULL) ? new_block : correct_block->next;
    }

    struct bb *new_bb = (struct bb *)correct_block;
    new_bb->busy_size = size;

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

    if (zone == NULL) {
        return;
    }

    struct bb *bb_ptr = (struct bb *)zone - 1;
    size_t busy_size = bb_ptr->busy_size;

    struct fb *new_fb = (struct fb *)bb_ptr;
    new_fb->free_size = busy_size + sizeof(struct bb);

    struct fb *current_block = info->first;
    struct fb *prev_block = NULL;

    while (current_block != NULL && current_block < new_fb)
    {
        prev_block = current_block;
        current_block = current_block->next;
    }

    if (current_block != NULL && (char *)new_fb + new_fb->free_size == (char *)current_block)
    {
        new_fb->free_size += current_block->free_size;
        new_fb->next = current_block->next;
    }
    else
    {
        new_fb->next = current_block;
    }

    if (prev_block != NULL && (char *)prev_block + prev_block->free_size == (char *)new_fb)
    {
        prev_block->free_size += new_fb->free_size;
        prev_block->next = new_fb->next;
    }
    else
    {
        if (prev_block != NULL)
        {
            prev_block->next = new_fb;
        }
        else
        {
            info->first = new_fb;
        }
    }
}

//-------------------------------------------------------------
// Itérateur(parcours) sur le contenu de l'allocateur
// mem_show
//-------------------------------------------------------------
void mem_show(void (*print)(void *, size_t, int))
{
    struct fb *current_block = info->first;
    char *address = (char *)info + sizeof(struct info_alloc);

    while (current_block != NULL)
    {
        if (address < (char *)current_block)
        {
            struct bb *busy_block = (struct bb *)address;
            print((void *)busy_block, busy_block->busy_size, 0);
            address += sizeof(struct bb) + busy_block->busy_size;
        }
        else
        {
            print((void *)current_block, current_block->free_size, 1);
            address = (char *)current_block + current_block->free_size;
            current_block = current_block->next;
        }
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
mem_free_block_t *mem_first_fit(mem_free_block_t *first_free_block, size_t wanted_size)
{
    struct fb *current_block = (struct fb *)first_free_block;
    struct fb *previous_block = NULL;

    while (current_block != NULL)
    {
        if (current_block->free_size >= wanted_size)
        {
            return (mem_free_block_t *)previous_block;
        }

        previous_block = current_block;
        current_block = current_block->next;
    }

    return NULL;
}

//-------------------------------------------------------------
mem_free_block_t *mem_best_fit(mem_free_block_t *first_free_block, size_t wanted_size)
{
    struct fb *current_block = (struct fb *)first_free_block;
    struct fb *best_fit_block = NULL;
    struct fb *previous_block = NULL;

    while (current_block != NULL)
    {
        if (current_block->free_size >= wanted_size &&
            (best_fit_block == NULL || current_block->free_size < best_fit_block->free_size))
        {
            best_fit_block = current_block;
        }

        previous_block = current_block;
        current_block = current_block->next;
    }

    return (mem_free_block_t *)previous_block;
}

//-------------------------------------------------------------
mem_free_block_t *mem_worst_fit(mem_free_block_t *first_free_block, size_t wanted_size)
{
    struct fb *current_block = (struct fb *)first_free_block;
    struct fb *worst_fit_block = NULL;
    struct fb *previous_block = NULL;

    while (current_block != NULL)
    {
        if (current_block->free_size >= wanted_size &&
            (worst_fit_block == NULL || current_block->free_size > worst_fit_block->free_size))
        {
            worst_fit_block = current_block;
        }

        previous_block = current_block;
        current_block = current_block->next;
    }

    return (mem_free_block_t *)previous_block;
}
