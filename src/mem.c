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
    mem_fit_function_t *get_correct_block = info->function_type;

    size_t total_size = size + sizeof(struct bb);

    struct fb *correct_block = (struct fb *)(*get_correct_block)((mem_free_block_t *)info->first, total_size);
    struct fb *new_block;

    if (correct_block == NULL)
    {
        return NULL;
    }

    size_t remaining_size = correct_block->size - total_size;

    if (remaining_size >= sizeof(struct fb))
    {
        new_block = (struct fb *)((char *)correct_block + total_size);
        new_block->next = correct_block->next;
        new_block->prev = correct_block->prev;
        new_block->size = correct_block->size;
    }
    else
    {
        if (new_block->next != NULL && (char *)new_block->next == (char *)new_block + new_block->size)
        {
            new_block->next = new_block->next->next;
            new_block->size += remaining_size;
        }
        else
        {
            size += remaining_size;
        }
    }

    if (new_block->prev == NULL)
    {
        info->first = new_block;
    }

    new_block->size -= total_size;
    struct bb *new_bb = (struct bb *)((char *)new_block - total_size);
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

    struct bb *freed_block = (struct bb *)((char *)zone - sizeof(struct bb));
    size_t freed_block_size = freed_block->size;
    struct fb *new_fb = (struct fb *)freed_block;

    new_fb->size = freed_block_size + sizeof(struct bb);

    struct fb *current_block = info->first;
    struct fb *prev_block = NULL;

    // Find the position to insert new_fb into the linked list
    while (current_block != NULL && current_block < new_fb)
    {
        prev_block = current_block;
        current_block = current_block->next;
    }

    // Update the pointers of new_fb
    new_fb->prev = prev_block;
    new_fb->next = current_block;

    // Insert new_fb into the linked list
    if (prev_block != NULL)
    {
        prev_block->next = new_fb;
    }
    else
    {
        info->first = new_fb;
    }

    if (current_block != NULL)
    {
        current_block->prev = new_fb;
    }

    // // Merge with the previous block if it's also free
    // if (new_fb->prev != NULL && (char *)new_fb->prev + new_fb->prev->size == (char *)new_fb)
    // {
    //     struct fb *prev_fb = new_fb->prev;
    //     prev_fb->size += new_fb->size;

    //     // Remove new_fb from the linked list
    //     prev_fb->next = new_fb->next;
    //     if (new_fb->next != NULL)
    //     {
    //         new_fb->next->prev = prev_fb;
    //     }

    //     new_fb = prev_fb; // Update the pointer to the merged block
    // }

    // // Merge with the next block if it's also free
    // if (new_fb->next != NULL && (char *)new_fb + new_fb->size == (char *)new_fb->next)
    // {
    //     struct fb *next_fb = new_fb->next;
    //     new_fb->size += next_fb->size;

    //     // Remove next_fb from the linked list
    //     new_fb->next = next_fb->next;
    //     if (next_fb->next != NULL)
    //     {
    //         next_fb->next->prev = new_fb;
    //     }
    // }


    

    // if (zone == NULL)
    // {
    //     return;
    // }

    // struct block *block_to_free = (struct block *)zone - 1;

    // block_to_free->type = 1;

    // while (block_to_free->prev != NULL && block_to_free->prev->type == 1)
    // {
    //     struct block *prev_block = block_to_free->prev;
    //     prev_block->next = block_to_free->next;

    //     if (block_to_free->next != NULL)
    //     {
    //         block_to_free->next->prev = prev_block;
    //     }

    //     block_to_free = prev_block;
    // }

    // if (block_to_free->prev != NULL)
    // {
    //     struct block *prev_block = block_to_free->prev;
    //     struct block *new_block = (struct block *)((char *)(prev_block + 1) + prev_block->size);
    //     new_block->next = block_to_free->next;
    //     new_block->prev = block_to_free->prev;
    //     new_block->type = 1;

    //     if (block_to_free->next != NULL)
    //     {
    //         block_to_free->next->prev = new_block;
    //     }

    //     if (block_to_free->prev != NULL)
    //     {
    //         block_to_free->prev->next = new_block;
    //     }

    //     block_to_free = new_block;
    // }

    // while (block_to_free->next != NULL && block_to_free->next->type == 1)
    // {
    //     struct block *next_block = block_to_free->next;
    //     block_to_free->next = next_block->next;

    //     if (next_block->next != NULL)
    //     {
    //         next_block->next->prev = block_to_free;
    //     }
    // }

    // if (block_to_free->next != NULL)
    // {
    //     block_to_free->size = (char *)block_to_free->next - (char *)block_to_free;
    // }
    // else
    // {
    //     char *start_addr = (char *)mem_space_get_addr();
    //     size_t size = mem_space_get_size();
    //     char *end_addr = start_addr + size;
    //     block_to_free->size = end_addr - (char *)block_to_free;
    // }
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
            print((void *)busy_block, busy_block->size, 0);
            address += sizeof(struct bb) + busy_block->size;
        }
        else
        {
            print((void *)current_block, current_block->size, current_block == info->first ? 2 : 1);
            address = (char *)current_block + sizeof(struct fb) + current_block->size;
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

    while (current_block != NULL)
    {
        if (current_block->size >= wanted_size)
        {
            return (mem_free_block_t *)current_block;
        }

        current_block = current_block->next;
    }

    return NULL;
}

//-------------------------------------------------------------
mem_free_block_t *mem_best_fit(mem_free_block_t *first_free_block, size_t wanted_size)
{
    struct fb *current_block = (struct fb *)first_free_block;
    struct fb *best_fit_block = NULL;

    while (current_block != NULL)
    {
        if (current_block->size >= wanted_size &&
            (best_fit_block == NULL || current_block->size < best_fit_block->size))
        {
            best_fit_block = current_block;
        }

        current_block = current_block->next;
    }

    return (mem_free_block_t *)best_fit_block;
}

//-------------------------------------------------------------
mem_free_block_t *mem_worst_fit(mem_free_block_t *first_free_block, size_t wanted_size)
{
    struct fb *current_block = (struct fb *)first_free_block;
    struct fb *worst_fit_block = NULL;

    while (current_block != NULL)
    {
        if (current_block->size >= wanted_size &&
            (worst_fit_block == NULL || current_block->size > worst_fit_block->size))
        {
            worst_fit_block = current_block;
        }

        current_block = current_block->next;
    }

    return (mem_free_block_t *)worst_fit_block;
}
