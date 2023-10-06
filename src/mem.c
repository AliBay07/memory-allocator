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
    size_t total_size_of_bb = size + sizeof(struct bb);
    size_t min_size = sizeof(struct fb*);
    struct fb *previous_block = (struct fb *)(*get_correct_block)((mem_free_block_t *)info->first, total_size_of_bb);
    struct fb* current_block;
    struct bb* new_bb;
    
    if(min_size>total_size_of_bb)
    {
        total_size_of_bb == min_size;
    }
    //1st case any bloc returned (first address returned)

    //2nd case first block returned
    if(previous_block == NULL)
    {
        printf("free size %zu\n",info->first->free_size);
        if(info->first->free_size < size)
        {
            return NULL;
        }
        current_block = info->first;
        //1st subcase only 1 free block in the list
        if(current_block->next == NULL)
        {
            new_bb = (struct bb*)info->first;
            new_bb->busy_size = size;
            current_block =(struct fb*) ((char*)new_bb + sizeof(struct bb*)+ new_bb->busy_size); //si la zone est dans un fb
            current_block->free_size -= size;
            info->first = current_block;
            printf("passe ici\n");
        }
        //2nd subcase more than 1 free block in the list, calculate remaining size to add a free block after busy block
        if(current_block->next != NULL )
        {
            size_t remaining_size = current_block->free_size - total_size_of_bb;
            if(remaining_size >= sizeof(struct fb*))
            {
                new_bb = (struct bb*)info->first;
                new_bb->busy_size = size;
                current_block =(struct fb*) ((char*)new_bb + sizeof(struct bb*)+ new_bb->busy_size); //si la zone est dans un fb
                current_block->free_size = remaining_size;
            }
            else
            {
                new_bb = (struct bb*)info->first;
                new_bb->busy_size = size+remaining_size;
                info->first = current_block->next;   
            }
            
        }
        return (void *) (new_bb+1);
    }
    //3rd case a block is returned
    if(previous_block != NULL)
    {
        printf("taille prev_block %zu\n",previous_block->free_size);
        //4th case the last block is returned
        if(previous_block->next == NULL)
        {
            printf("passe ici");
            new_bb =(struct bb*) ((char*)previous_block+previous_block->free_size);
            new_bb->busy_size = size;
            previous_block->next = NULL;
            return (void *) (new_bb+1);
        }
        current_block = previous_block->next;
        size_t remaining_size = current_block->free_size - total_size_of_bb;
        //1st calculate remaining size to add a free block after busy block
        if(remaining_size >= sizeof(struct fb*))
        {
            new_bb = (struct bb*)current_block;
            new_bb->busy_size = size;
            current_block =(struct fb*) ((char*)new_bb + sizeof(struct bb*)+ new_bb->busy_size); //si la zone est dans un fb
            current_block->free_size = remaining_size;
        }
        else
        {
            new_bb = (struct bb*)current_block;
            new_bb->busy_size = size+remaining_size;
            current_block = current_block->next;
        }
        return (void *) (new_bb+1);
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
    struct fb* current_block_fb = info->first;
    struct fb* previous_block_fb = NULL;
    printf("adress of current_block allocated %p \n",info->first);
    printf("adress of zone to free %p \n",zone);
    //empecher free avant adresse info->first ou zone NULL
    if (zone == NULL) {
        printf("passe là\n");
        return;
    }
    //1er cas la zone est deja libre (comprends le cas où le block suivant est un bb car on ne va pas écraser le bb suivant)
    while(current_block_fb !=NULL && (char *)current_block_fb < (char*)zone ) //parcourt jusqu a trouver le bloc précédent
    {
        printf("taille previous_block %zu\n",previous_block_fb->free_size);
        previous_block_fb = current_block_fb;
        current_block_fb = current_block_fb->next;
    }
    //si la zone est en plein milieu d'un block
    /*if((char*)zone>= (char*)previous_block_fb && (char*)zone<=previous_block_fb->free_size) //si la zone est dans un fb
    {
            return;
    }*/

    //2e cas la zone est occupee
    struct bb* current_block_allocated = (struct bb*)(current_block_fb+current_block_fb->free_size)+sizeof(struct bb*);
    printf("size of current_block allocated %zu \n",current_block_allocated->busy_size);

    //parcours les bb jusqu'à trouver l'adresse de debut du bb
    while((char*)current_block_allocated< (char*)zone)
    {
        current_block_allocated += sizeof(struct bb*) + current_block_allocated->busy_size;
    }

    //si l'adresse donnee n'est pas le début d'un bloc occupé 
    if((char*)current_block_allocated != (char*)zone)
    {
        return;
    }

    struct fb* new_free_block = (struct fb*)current_block_allocated-sizeof(struct bb*);
    
    //si on libere le 1er bloc occupé
    if(previous_block_fb == info->first->next) //on pointe le 1er bloc libre
    {
        info->first->next = new_free_block;
    }

    

    /*new_free_block->next = (struct fb*)current_block_allocated;
    new_free_block->free_size = sizeof(struct bb*) +current_block_allocated->busy_size;



    
     






    if (zone == NULL) {
        return;
    }

    struct bb *bb_ptr = (struct bb *)zone - 1;
    size_t busy_size = bb_ptr->busy_size;

    struct fb *new_fb = (struct fb *)bb_ptr;
    new_fb->free_size = busy_size + sizeof(struct bb);

    struct fb *current_block = info->first;
    struct fb *prev_block = NULL;

    while (current_block != NULL && (char*)current_block < (char*)new_fb)
    {
        prev_block = current_block;
        current_block = current_block->next;
        printf(" adresse de previous block free %p \n", prev_block);
        printf(" adresse de current block free %p \n", current_block);
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
    }*/
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
    int i =0;
    while (current_block != NULL)
    {
        printf("i = %d\n",i);
        printf("taille de current_block %zu \n",current_block->free_size);
        if (current_block->free_size >= wanted_size)
        {
            return (mem_free_block_t *)previous_block;
        }
        previous_block = current_block;
        current_block = current_block->next;
        i++;
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
