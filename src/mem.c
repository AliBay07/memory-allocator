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

// Structure which represents the free blocks in the memory
struct fb
{
    size_t free_size;
    struct fb *next;
};

// Structure which represents the busy blocks in the memory
struct bb
{
    size_t busy_size;
};

// Structure which is palced at the start of the memory
// it will contain the fit function to call, and it will point
// to the first node of the free blocks linked list
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
    // Placing the info_alloc structure at the start of our memory
    info = (struct info_alloc *)mem_space_get_addr();

    // Initializing the intial block, which will be an free block which we won't use
    // this block will make traversing and updating the linked list of free blocks
    // way easier
    struct fb *initial_block = (struct fb *)((char *)info + sizeof(struct info_alloc));

    // Initializing the actual first free block which will represent the first free
    // empty space in our memory
    struct fb *first_block = (struct fb *)((char *)initial_block + sizeof(struct fb));

    // Setting the size fo the inital_block to 0 so we never use it
    initial_block->free_size = 0;
    // Setting the size of the actual first free block to the remaining size in our memory
    // after addinf the info_alloc and initial block
    first_block->free_size = mem_space_get_size() - (sizeof(struct info_alloc)) - (sizeof(struct fb));

    // Setting the pointers of the info_alloc to the initial_block
    info->first = initial_block;
    // Setting the pointers of the initial_block to the first_block
    initial_block->next = first_block;
    // Setting the pointers of the first_block to the NULL
    first_block->next = NULL;

    // Setting the default fit function to be the first fit
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

    // Check if info->first->next is NULL, if so, then there are no more 
    // available empty blocks in our memory so we return NULL
    // Also check if the size requested is 0, then we also return NULL
    if (info->first->next == NULL || size == 0)
    {
        return NULL;
    }

    // Get the correct fit function from info_alloc
    mem_fit_function_t *get_correct_block = info->function_type;

    // Get the total size we want, which will be the size the user requested + 
    // the size of the struct bb which we will insert before the allocated space
    size_t total_size = size + sizeof(struct bb);

    // In the case where the total size is less than the size of a free block structure,
    // when we free that allocated space, we won't be able to set a new free block to 
    // represent the empty sapce in the memory, so in that case, we set the total_size
    // to the size of struct of free block, that way, we are sture that we can alway
    // set a free block after freeing an allocated space
    if (total_size < sizeof(struct fb))
    {
        total_size = sizeof(struct fb);
    }

    // Call the fit function, passing it the first block which is info->first->next, and the
    // total_size, this will return the previous block which is right before the correct block
    // depending on the fit startegy used
    struct fb *previous_block = (struct fb *)(*get_correct_block)((mem_free_block_t *)info->first->next, total_size);

    // If the previous_block->next is NULL, then we are sure that there are no free blocks which can 
    // contain the size the user is trying to allocate, in this case we return a NULL
    if (previous_block->next == NULL)
    {
        return NULL;
    }

    // If the previous_block->next is not NULL, then we are sure that we have a correct_block
    struct fb *correct_block = (struct fb *)previous_block->next;

    // We initialize the remaining size, which is the free_size of the
    // correct block - the total size we are trying to allocate
    size_t remaining_size = correct_block->free_size - total_size;

    // Here we have two cases, either we have enough space in the remaining size to add a new
    // free block, or the remaining size is less than the size of a free block
    if (remaining_size >= sizeof(struct fb))
    {   
        // In the case where we have enough space in the remaining size, we create a new free block
        // and we place it right after the newly allocated block
        // then we fix the pointers of this new free block and the previous block
        struct fb *new_block = (struct fb *)((char *)correct_block + total_size);
        new_block->next = correct_block->next;
        previous_block->next = new_block;
        new_block->free_size = remaining_size;
    }
    else
    {   
        // If we don't have enough free size, we add the remaining size to
        // the block we are allocating to not have external fragmentation, this does
        // result in internal fragmentation, but at least we will be able to recover
        // this internal fragmentation when/if the block is eventually freed
        size += remaining_size;
        previous_block->next = correct_block->next;
    }

    // We create the new busy block and set it in the adress of the previous free block
    // and we add it's size
    struct bb *new_bb = (struct bb *)correct_block;
    new_bb->busy_size = size;

    // We return the pointer to the adress that the user requested
    return (void *)(new_bb + 1);
}

//-------------------------------------------------------------
// mem_get_size
//-------------------------------------------------------------
size_t mem_get_size(void *zone)
{
    // Vérifiez que la zone n'est pas NULL
    if (zone != NULL)
    {
        // pointeur vers la structure bb précédant la zone
        struct bb *bb_ptr = (struct bb *)zone - 1;
        return bb_ptr->busy_size;
    }
    else
    {
        return 0; // Retournez 0 si la zone est NULL
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

    // Check if zone is null, or if the adress of zone - struct bb is before the allowed
    // memory space to be allocated, or if the adress of zone is after the allowed memory
    // space to be allocated, in all these cases, we do nothing
    if (zone == NULL ||
        (zone - sizeof(struct bb)) < (mem_space_get_addr() + sizeof(struct info_alloc) + sizeof(struct fb)) ||
        zone > (mem_space_get_addr() + mem_space_get_size()))
    {
        return;
    }

    // Get the busy block which represents the zone the user is trying to free
    struct bb *bb_ptr = (struct bb *)zone - 1;
    // Get the size of the allocated space the user is trying to free
    size_t busy_size = bb_ptr->busy_size;

    // Traversing through the free block linked list, having the
    // current block being the info->first->next and the previosu block being
    // info->first
    struct fb *prev_block = info->first;
    struct fb *current_block = info->first->next;

    // Loop until we either reach the end of our memory or we reach a point where 
    // the adress of prev_block is before the adress of the busy block
    // This way, we get the free blocks that are right before and right
    // after the zone we're trying to free
    while (current_block != NULL && (char *)current_block < (char *)bb_ptr)
    {
        prev_block = current_block;
        current_block = current_block->next;
    }

    char *address_current_block;
    char *address_previous_block;

    // We check if current block is NULL, if not, then we use the adress of the current_block
    // if the current block is NULL, then we use the end of our memory as the adress of current_block
    (current_block != NULL) ? (address_current_block = (char *)current_block) : (address_current_block = (char *)(mem_space_get_addr() + mem_space_get_size()));

    // For the adress of previous block, if the previous_block is the info->first, then we use
    // this adress plus the size of it's struct as the previous adress
    // else we use the adress of the prev_block + free size of the block
    address_previous_block = ((char *)prev_block == (char *)info->first)
                                 ? (char *)prev_block + sizeof(struct fb)
                                 : (char *)prev_block + prev_block->free_size;

    struct bb *busy_block;

    // In this code, we're making sure that we adress the user is trying to free is an actual
    // adress which represents an memory in the space that was previously allocated

    // We loop while the adress of the previous_block is less than the adress of the
    // current blocl
    while (address_previous_block < (char *)address_current_block)
    {   
        // We get the busy block which is at the previous adress
        busy_block = (struct bb *)address_previous_block;
        // We increment address_previous_block by the size of the
        // busy block structure, so now the address_previous_block represents
        // the adress allocated to a user
        address_previous_block += sizeof(struct bb);

        // We check if the zone is now before that adress, than we are sure that the zone
        // isn't a valid one, we do nothing
        if ((char *)address_previous_block > (char *)zone)
        {
            return;
        }
        // if the zone is equal to the adress_previous_zone, then we are sure that
        // the zone entered by the user is a correct zone which was allocated
        // previously, we break from the while loop
        else if (address_previous_block == zone)
        {
            break;
        }

        // We increment the address_previous_block by the size of the busy block
        address_previous_block += busy_block->busy_size;
    }

    // If the loop ends and the address_previous_block is more or equal to the address_current_block,
    // then the zone entered by the user is not an adress previously allocated, so we do nothing
    if ((char *)address_previous_block >= (char *)address_current_block)
    {
        return;
    }

    // Now that we are sure that the zone the user is trying to free is correct
    // we create a new free block at the adress of the busy block
    struct fb *new_fb = (struct fb *)bb_ptr;

    // We check if the busy size + struct of busy block is less than the size of the 
    // struct of free block, if so we set the size of the free block to that,
    // if not, we set the size of the free block to the size of the struct free block
    // This is to handle the case where is size of busy block + size was less than
    // the size of the free block struct, is that case we allocated size of free block
    // to the user, so this condition will recover that extra space we allocated
    if (busy_size + sizeof(struct bb) > sizeof(struct fb))
    {
        new_fb->free_size = busy_size + sizeof(struct bb);
    }
    else
    {
        new_fb->free_size = sizeof(struct fb);
    }

    // Now we handle the merging of the free blocks, we check if current_block is not NULL and the adress of the end 
    // of new free block is equal to the start of the adress of the current block, then the two free
    // blocks are next to eachother and we can merge them
    if (current_block != NULL && (char *)new_fb + new_fb->free_size == (char *)current_block)
    {   
        // We add the size of the current_block to the free block and we set the pointer
        // of newfb->next to the current_block->next
        new_fb->free_size += current_block->free_size;
        new_fb->next = current_block->next;
    }
    else
    {   
        // If not then we just have to set the next pointer of the new fb to the current block
        new_fb->next = current_block;
    }

    // We do the same thing to check the merging with the previous block, but we also add the condition that the previous block
    // is different than the info->first, as we don't want to merge the free blocks with this inital block
    if (prev_block != NULL && (char *)prev_block + prev_block->free_size == (char *)new_fb && (char *)prev_block != (char *)info->first)
    {
        // We add the size of the newfb to the previous block and we fix the pointer of the next of the pre_block
        // to the next of the new fb
        prev_block->free_size += new_fb->free_size;
        prev_block->next = new_fb->next;

        // We check if the info->first->next is NULL, then we set the next of info->first->next to the prev_block
        if (info->first->next == NULL)
        {
            info->first->next = prev_block;
        }
    }
    else
    {   
        // If we aren't merging, then we check if the info->first->next is NULL
        if (info->first->next == NULL)
        {   
            // if yes than we set the info->first->next to the new fb
            info->first->next = new_fb;
        }
        else
        {
            // else we set the prev_block->next to the new fb
            prev_block->next = new_fb;
        }
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

    if (pointer - sizeof(struct bb) < (mem_space_get_addr() + sizeof(struct info_alloc) + sizeof(struct fb)) ||
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
        if((char*)current_block == (char*)bb_ptr)
        {
            return NULL;
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

    if ((char *)address_previous_block >= (char *)address_current_block ||
        (char *)busy_block + busy_block->busy_size > (char *)(mem_space_get_addr() + mem_space_get_size()))
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
    struct fb *previous_block = info->first;
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
    struct fb *current_block = (struct fb *)first_free_block;
    struct fb *previous_block = info->first;
    struct fb *worst_fit_block = NULL;
    struct fb *worst_fit_previous_block = info->first;

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