//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#include "../headers/mem.h"
#include "../src/mem_space.c"
#include "../headers/mem_os.h"
#include <assert.h>

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

// Initializing the structure info
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

    if (info->first->next == NULL || size == 0)
    {
        return NULL;
    }

    mem_fit_function_t *get_correct_block = info->function_type;

    size_t total_size = size + sizeof(struct bb);

    if (total_size < sizeof(struct fb))
    {
        total_size = sizeof(struct fb);
    }

    struct fb *previous_block = (struct fb *)(*get_correct_block)((mem_free_block_t *)info->first, total_size);

    if (previous_block->next == NULL || previous_block->next->free_size == 0)
    {
        return NULL;
    }

    struct fb *correct_block = (struct fb *)previous_block->next;

    size_t remaining_size = correct_block->free_size - total_size;

    if (remaining_size >= sizeof(struct fb))
    {
        struct fb *new_block = (struct fb *)((char *)correct_block + total_size);
        new_block->next = correct_block->next;
        previous_block->next = new_block;
        new_block->free_size = remaining_size;
    }
    else
    {
        size += remaining_size;
        previous_block->next = correct_block->next;
    }

    struct bb *new_bb = (struct bb *)correct_block;
    if (size < 8)
    {
        size = 8;
    }
    new_bb->busy_size = size;

    return (void *)(new_bb + 1);
}

//-------------------------------------------------------------
// mem_get_size
//-------------------------------------------------------------
size_t mem_get_size(void *zone)
{
<<<<<<< HEAD
    // Vérifiez que la zone n'est pas NULL
=======
    size_t taille = 0;
    // Vérifiez que la zone n'est pas NULL

>>>>>>> 309456a8b21bde0009c806b94ac80b344544ee9b
    if (zone != NULL)
    {
        // pointeur vers la structure bb précédant la zone
        struct bb *bb_ptr = (struct bb *)zone - 1;
<<<<<<< HEAD
        return bb_ptr->busy_size;
    }
    else
    {
        return 0; // Retournez 0 si la zone est NULL
=======
        struct bb *tmp_bb = bb_ptr;
        struct fb *tmp_fb = info->first;

        // vérification si ce n'est pas dans un fb
        while (tmp_bb != NULL && tmp_fb != NULL)
        {
            if ((char*)tmp_bb >= (char*)tmp_fb && (char*)tmp_bb <= (char*)tmp_fb)
            {
                return taille;
            }
            tmp_bb = (struct bb *)(tmp_bb + tmp_bb->busy_size) + sizeof(struct bb);
            tmp_fb = tmp_fb->next;
        }
        if (bb_ptr->busy_size < sizeof(struct fb))
        {
            taille = sizeof(struct fb);
        }
        else
        {
            taille = bb_ptr->busy_size;
        }
        return taille;
    }
    else
    {
        return taille; // Retournez 0 si la zone est NULL
>>>>>>> 309456a8b21bde0009c806b94ac80b344544ee9b
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

    struct fb *prev_block = info->first;
    struct fb *current_block = prev_block->next;

<<<<<<< HEAD
    // Loop until we either reach the end of our memory or we reach a point where 
    // the adress of prev_block is before the adress of the busy block
    // This way, we get the free blocks that are right before and right
    // after the zone we're trying to free
=======
>>>>>>> 309456a8b21bde0009c806b94ac80b344544ee9b
    while (current_block != NULL && (char *)current_block < (char *)bb_ptr)
    {
        prev_block = current_block;
        current_block = current_block->next;
    }

    struct bb *busy_block;
    if (prev_block->free_size <= sizeof(struct fb))
    {
        busy_block = (struct bb *)((char *)prev_block + sizeof(struct fb));
    }
    else
    {
        busy_block = (struct bb *)((char *)prev_block + prev_block->free_size);
    }

    int adresses_match = 0;
    while ((char *)busy_block < (char *)zone)
    {

        if ((char *)busy_block + sizeof(struct bb) > (char *)zone)
        {
            return;
        }
        else if ((char *)busy_block + sizeof(struct bb) == zone)
        {
            adresses_match = 1;
            break;
        }

        busy_block = (struct bb *)((char *)busy_block + sizeof(struct bb) + busy_block->busy_size);
    }

    if (adresses_match == 0)
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
// mem_realloc
//-------------------------------------------------------------
void *mem_realloc(void *pointer, size_t size)
{
    if (pointer == NULL)
    {
        return mem_alloc(size);
    }
    else if (size == 0)
    {
        mem_free(pointer);
        return NULL;
    }
    else
    {
        size_t old_size = mem_get_size(pointer);

        struct bb *bb_ptr = (struct bb *)pointer - 1;
        size_t busy_size = bb_ptr->busy_size;

        struct fb *prev_block = info->first;
        struct fb *current_block = prev_block->next;

        while (current_block != NULL && (char *)current_block < (char *)bb_ptr)
        {
            prev_block = current_block;
            current_block = current_block->next;
        }

        struct bb *busy_block;
        if (prev_block->free_size <= sizeof(struct fb))
        {
            busy_block = (struct bb *)((char *)prev_block + sizeof(struct fb));
        }
        else
        {
            busy_block = (struct bb *)((char *)prev_block + prev_block->free_size);
        }

        int adresses_match = 0;
        while ((char *)busy_block < (char *)pointer)
        {

            if ((char *)busy_block + sizeof(struct bb) > (char *)pointer)
            {
                return NULL;
            }
            else if ((char *)busy_block + sizeof(struct bb) == pointer)
            {
                adresses_match = 1;
                break;
            }

            busy_block = (struct bb *)((char *)busy_block + sizeof(struct bb) + busy_block->busy_size);
        }

        if (adresses_match == 0)
        {
            return NULL;
        }

        if (size > old_size)
        {
            if (current_block != NULL && (char *)busy_block + busy_block->busy_size == (char *)current_block &&
                current_block->free_size - (size - old_size) >= sizeof(struct fb))
            {
                current_block = (struct fb*)(char *)current_block + (size - old_size);
                busy_block->busy_size = size;
                return (void *)busy_block + 1;
            }

            void *new_pointer = mem_alloc(size);

            char *src = (char *)pointer;
            char *dest = (char *)new_pointer;

            for (size_t i = 0; i < size; i++)
            {
                dest[i] = src[i];
            }

            mem_free(pointer);

            return new_pointer;
        }
        else if (size < old_size)
        {

            if (current_block != NULL && (char *)busy_block + busy_block->busy_size == (char *)current_block)
            {
                current_block = (struct fb*)(char *)current_block - (old_size - size);
                current_block->free_size += (old_size - size);
                busy_block->busy_size = size;
            }
            else if ((old_size - size) >= sizeof(struct fb))
            {
                busy_block->busy_size = size;
                struct fb *new_fb = (struct fb *)((char *)busy_block + busy_block->busy_size);
                new_fb->free_size = (old_size - size);
                prev_block->next = new_fb;
                new_fb->next = current_block;
            }

            return pointer;
        } else {
            return pointer;
        }
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
        if ((busy_block->busy_size + sizeof(struct bb)) > sizeof(struct fb))
        {
            address += sizeof(struct bb) + busy_block->busy_size;
        }
        else
        {
            address += sizeof(struct fb);
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

    struct fb *previous_block = (struct fb *)first_free_block;
    struct fb *current_block = previous_block->next;

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
    struct fb *previous_block = (struct fb *)first_free_block;
    struct fb *current_block = previous_block->next;
    struct fb *best_fit_block = NULL;
    struct fb *best_fit_previous_block = previous_block;

    while (current_block != NULL)
    {
        if (current_block->free_size >= wanted_size &&
            (best_fit_block == NULL || current_block->free_size < best_fit_block->free_size))
        {
            best_fit_previous_block = previous_block;
            best_fit_block = current_block;
        }

        previous_block = current_block;
        current_block = current_block->next;
    }

    return (mem_free_block_t *)best_fit_previous_block;
}

//-------------------------------------------------------------
mem_free_block_t *mem_worst_fit(mem_free_block_t *first_free_block, size_t wanted_size)
{
    struct fb *previous_block = (struct fb *)first_free_block;
    struct fb *current_block = previous_block->next;
    struct fb *worst_fit_block = NULL;
    struct fb *worst_fit_previous_block = previous_block;

    while (current_block != NULL)
    {
        if (current_block->free_size >= wanted_size &&
            (worst_fit_block == NULL || current_block->free_size > worst_fit_block->free_size))
        {
            worst_fit_previous_block = previous_block;
            worst_fit_block = current_block;
        }

        previous_block = current_block;
        current_block = current_block->next;
    }

    return (mem_free_block_t *)worst_fit_previous_block;
}