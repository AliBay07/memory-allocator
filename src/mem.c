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

struct fb
{
    size_t free_size;
    struct fb *next;
};

struct bb
{
    size_t busy_size;
};

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

    struct fb *initial_block = (struct fb *)((char *)info + sizeof(struct info_alloc));
    struct fb *first_block = (struct fb *)((char *)initial_block + sizeof(struct fb));

    initial_block->free_size = 0;
    first_block->free_size = mem_space_get_size() - (sizeof(struct info_alloc)) - (sizeof(struct fb));

    info->first = initial_block;
    initial_block->next = first_block;
    first_block->next = NULL;

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

    if(size == 0)
    {
        return NULL;
    }

    if (total_size < sizeof(struct fb))
    {
        total_size = sizeof(struct fb);
    }

    struct fb *previous_block = (struct fb *)(*get_correct_block)((mem_free_block_t *)info->first->next, total_size);

    if (previous_block->next == NULL)
    {
        return NULL;
    }

    struct fb *correct_block = (struct fb *)previous_block->next;

    if ((char *)correct_block + total_size > (char *)(mem_space_get_addr() + mem_space_get_size()))
    {
        return NULL;
    }

    struct fb *new_block = (struct fb *)((char *)correct_block + total_size);

    size_t remaining_size = correct_block->free_size - total_size;

    if (remaining_size >= sizeof(struct fb))
    {
        new_block->next = correct_block->next;
        previous_block->next = new_block;
        new_block->free_size = remaining_size;
    }
    else
    {
        size += remaining_size;
        if (correct_block != NULL)
        {
            previous_block->next = correct_block->next;
        }
    }

    struct bb *new_bb = (struct bb *)correct_block;
    new_bb->busy_size = size;

    return (void *)(new_bb + 1);
}

//-------------------------------------------------------------
// mem_realloc
//-------------------------------------------------------------
void *mem_realloc(void *pointer, size_t size)
{

    if (pointer == NULL ||
        (pointer - sizeof(struct bb)) < (mem_space_get_addr() + sizeof(struct info_alloc) + sizeof(struct fb)) ||
        pointer > (mem_space_get_addr() + mem_space_get_size()))
    {
        return NULL;
    }

    struct bb *bb_ptr = (struct bb *)pointer - 1;
    size_t busy_size = bb_ptr->busy_size;

    struct fb *current_block = info->first;
    struct fb *prev_block = NULL;

    while (current_block != NULL && (char *)current_block < (char *)bb_ptr)
    {
        prev_block = current_block;
        current_block = current_block->next;
    }

    char *address_current_block;
    char *address_previous_block;

    (current_block != NULL) ? (address_current_block = (char *)current_block) : (address_current_block = (char *)(mem_space_get_addr() + mem_space_get_size()));

    if (prev_block != NULL)
    {
        address_previous_block = ((char *)prev_block == (char *)info->first)
                                     ? (char *)prev_block + sizeof(struct fb)
                                     : (char *)prev_block + prev_block->free_size;
    }
    else
    {
        address_previous_block = (char *)mem_space_get_addr() + sizeof(struct info_alloc) + sizeof(struct fb);
    }

    struct bb *busy_block;
    while (address_previous_block < (char *)address_current_block)
    {
        busy_block = (struct bb *)address_previous_block;
        address_previous_block += sizeof(struct bb);
        if ((char *)address_previous_block > (char *)pointer)
        {
            return NULL;
        }
        else if (address_previous_block == pointer)
        {
            break;
        }
        address_previous_block += busy_block->busy_size;
    }

    if ((char *)address_previous_block >= (char *)address_current_block)
    {
        return NULL;
    }

    if (((char *)busy_block + sizeof(struct bb) + busy_block->busy_size) != address_current_block)
    {
        return mem_alloc(size);
    }
    else if (current_block != NULL || current_block->free_size >= size)
    {
        busy_block->busy_size = size;
        prev_block->next = current_block->next;

        return (void *)(busy_block + 1);
    }
    else
    {
        return mem_alloc(size);
    }
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

    if (zone == NULL ||
        (zone - sizeof(struct bb)) < (mem_space_get_addr() + sizeof(struct info_alloc) + sizeof(struct fb)) ||
        zone > (mem_space_get_addr() + mem_space_get_size()))
    {
        return;
    }

    struct bb *bb_ptr = (struct bb *)zone - 1;
    size_t busy_size = bb_ptr->busy_size;

    struct fb *current_block = info->first;
    struct fb *prev_block = NULL;

    while (current_block != NULL && (char *)current_block < (char *)bb_ptr)
    {
        if((char*)current_block == (char*)bb_ptr)
        {
            return;
        }
        prev_block = current_block;
        current_block = current_block->next;
    }

    char *address_current_block;
    char *address_previous_block;

    (current_block != NULL) ? (address_current_block = (char *)current_block) : (address_current_block = (char *)(mem_space_get_addr() + mem_space_get_size()));

    if (prev_block != NULL)
    {
        address_previous_block = ((char *)prev_block == (char *)info->first)
                                     ? (char *)prev_block + sizeof(struct fb)
                                     : (char *)prev_block + prev_block->free_size;
    }
    else
    {
        address_previous_block = (char *)mem_space_get_addr() + sizeof(struct info_alloc) + sizeof(struct fb);
    }

    struct bb *busy_block;
    while (address_previous_block < (char *)address_current_block)
    {
        busy_block = (struct bb *)address_previous_block;
        address_previous_block += sizeof(struct bb);
        if ((char *)address_previous_block > (char *)zone)
        {
            return;
        }
        else if (address_previous_block == zone)
        {
            break;
        }
        address_previous_block += busy_block->busy_size;
    }

    if ((char *)address_previous_block >= (char *)address_current_block)
    {
        return;
    }

    struct fb *new_fb = (struct fb *)bb_ptr;
    if (busy_size + sizeof(struct bb) > sizeof(struct fb))
    {
        new_fb->free_size = busy_size + sizeof(struct bb);
    }
    else
    {
        new_fb->free_size = sizeof(struct fb);
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

    if (prev_block != NULL && (char *)prev_block + prev_block->free_size == (char *)new_fb && (char *)prev_block != (char *)info->first)
    {
        prev_block->free_size += new_fb->free_size;
        prev_block->next = new_fb->next;
    }
    else
    {
        prev_block->next = new_fb;
    }
}

//-------------------------------------------------------------
// Itérateur(parcours) sur le contenu de l'allocateur
// mem_show
//-------------------------------------------------------------
void mem_show(void (*print)(void *, size_t, int))
{
    struct fb *current_block = info->first->next;
    char *address = (char *)info + sizeof(struct info_alloc) + sizeof(struct fb);

    while (current_block != NULL)
    {
        if (address < (char *)current_block)
        {
            struct bb *busy_block = (struct bb *)address;
            print((void *)busy_block, busy_block->busy_size, 0);
            if ((busy_block->busy_size + sizeof(struct bb)) > sizeof(struct fb))
            {
                address += sizeof(struct bb) + busy_block->busy_size;
            }
            else
            {
                address += sizeof(struct fb);
            }
        }
        else
        {
            print((void *)current_block, current_block->free_size, 1);
            address = (char *)current_block + current_block->free_size;
            current_block = current_block->next;
        }
    }

    while (address < (char *)(mem_space_get_addr() + mem_space_get_size()))
    {
        struct bb *busy_block = (struct bb *)address;
        print((void *)busy_block, busy_block->busy_size, 0);
        address += sizeof(struct bb) + busy_block->busy_size;
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
    struct fb *previous_block = info->first;

    while (current_block != NULL)
    {
        if (current_block->free_size >= wanted_size)
        {
            break;
        }

        previous_block = current_block;
        current_block = current_block->next;
    }

    return (mem_free_block_t *)previous_block;
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
