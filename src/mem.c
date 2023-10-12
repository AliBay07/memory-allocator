//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#include "../headers/mem.h"
#include "../src/mem_space.c"
#include "../headers/mem_os.h"
#include <assert.h>

// Structure représentant les zones de mémoire libre
struct fb
{
    size_t free_size;
    struct fb *next;
};

// Structure représentant les zones de mémoire occupées
struct bb
{
    size_t busy_size;
};

// Structure tête qui contient le premier maillon et la fonction de placement
struct info_alloc
{
    struct fb *first;
    mem_fit_function_t *function_type;
};

struct info_alloc *info;

/**
 * Fonction mem_init
 *
 * Paramètres : Aucun
 *
 * Renvoie : Aucune valeur de retour (void)
 *
 * Description : Initialise la gestion de la mémoire. Elle alloue un bloc initial
 * de mémoire et configure la structure d'information pour le suivi de la mémoire
 * allouée et libre. Cette fonction configure également le gestionnaire de placement
 * de la mémoire pour utiliser le "premier ajustement" (mem_first_fit) comme stratégie.
 */
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

/**
 * Fonction mem_alloc
 *
 * Paramètres : size (taille de l'allocation)
 *
 * Renvoie : Un pointeur void* vers la mémoire allouée, ou NULL en cas d'échec.
 *
 * Description : Alloue un bloc de mémoire de la taille spécifiée et retourne un pointeur
 * vers le début de ce bloc. Cette fonction utilise une stratégie de placement de mémoire
 * définie précédemment. Si l'allocation échoue, elle renvoie NULL.
 */
void *mem_alloc(size_t size)
{

    if (info->first->next == NULL || size == 0)
    {
        return NULL;
    }

    if (size < 8)
    {
        size = 8;
    }

    mem_fit_function_t *get_correct_block = info->function_type;

    size_t total_size = size + sizeof(struct bb);

    struct fb *previous_block = (struct fb *)(*get_correct_block)((mem_free_block_t *)info->first, total_size);

    if (previous_block == NULL || previous_block->next == NULL || previous_block->next->free_size < total_size)
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
    new_bb->busy_size = size;

    return (void *)(new_bb + 1);
}

/**
 * Fonction mem_get_size
 *
 * Paramètres : zone (pointeur vers une zone allouée)
 *
 * Renvoie : La taille de la zone allouée en octets, ou 0 si la zone est NULL.
 *
 * Description : Cette fonction renvoie la taille en octets de la zone mémoire allouée
 * à partir du pointeur passé en paramètre. Si le pointeur est NULL, elle renvoie 0. La
 * fonction recherche la zone de mémoire allouée en parcourant la liste des blocs alloués
 * et calcule sa taille en fonction des informations de gestion de mémoire associées.
 */
size_t mem_get_size(void *zone)
{
    size_t taille = 0;

    if (zone != NULL)
    {
        struct bb *bb_ptr = (struct bb *)zone - 1;
        struct bb *tmp_bb = bb_ptr;
        struct fb *tmp_fb = info->first;

        while (tmp_bb != NULL && tmp_fb != NULL)
        {
            if ((char *)tmp_bb >= (char *)tmp_fb && (char *)tmp_bb <= (char *)tmp_fb)
            {
                return taille;
            }
            tmp_bb = (struct bb *)(tmp_bb + tmp_bb->busy_size) + sizeof(struct bb);
            tmp_fb = tmp_fb->next;
        }

        taille = bb_ptr->busy_size;
        return taille;
    }
    else
    {
        return taille;
    }
}

/**
 * Fonction mem_free
 *
 * Paramètres : zone (pointeur vers la zone mémoire à libérer)
 *
 * Renvoie : Aucune valeur de retour (void)
 *
 * Description : Libère la zone mémoire spécifiée en paramètre. La fonction vérifie si
 * la zone est valide, puis la retourne dans la liste de zones libres si elle est valide.
 * Si la zone précédente ou suivante est également libre, elle fusionne ces zones en une
 * seule zone plus grande.
 */
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

/**
 * Fonction mem_realloc
 *
 * Paramètres :
 *   - pointer (pointeur vers la zone mémoire à réallouer)
 *   - size (nouvelle taille de la zone mémoire)
 *
 * Renvoie : Un pointeur vers la zone mémoire réallouée, ou NULL en cas d'échec.
 *
 * Description : Réalloue la zone mémoire spécifiée par le pointeur avec la nouvelle taille
 * spécifiée. La fonction gère les cas où le pointeur est NULL, la nouvelle taille est nulle,
 * ou lorsque la zone doit être agrandie ou rétrécie. Elle fusionne également les zones adjacentes
 * si possible. Si l'allocation échoue, elle renvoie NULL.
 */
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
                current_block = (struct fb *)(char *)current_block + (size - old_size);
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
                current_block = (struct fb *)(char *)current_block - (old_size - size);
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
        }
        else
        {
            return pointer;
        }
    }
}

/**
 * Fonction mem_show
 *
 * Paramètres :
 *   - print (pointeur vers une fonction d'impression)
 *
 * Renvoie : Aucune valeur de retour (void)
 *
 * Description : Affiche des informations sur la mémoire allouée et libre à l'aide d'une
 * fonction d'impression spécifiée. La fonction parcourt la mémoire en suivant la liste
 * des blocs libres et alloués, et utilise la fonction d'impression pour afficher des détails
 * sur chaque zone mémoire.
 */
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

/**
 * Fonction mem_set_fit_handler
 *
 * Paramètres :
 *   - mff (pointeur vers une fonction de placement de mémoire)
 *
 * Renvoie : Aucune valeur de retour (void)
 *
 * Description : Définit le gestionnaire de placement de mémoire pour être utilisé
 * lors des opérations d'allocation. La fonction associe la fonction de placement
 * spécifiée avec le gestionnaire de mémoire.
 */
void mem_set_fit_handler(mem_fit_function_t *mff)
{
    info->function_type = mff;
}

/**
 * Fonction mem_first_fit
 *
 * Paramètres :
 *   - first_free_block (pointeur vers le premier bloc libre)
 *   - wanted_size (taille désirée de l'allocation)
 *
 * Renvoie : Un pointeur vers le bloc libre précédent correspondant au "premier ajustement,"
 *           ou NULL si aucun bloc libre n'est disponible.
 *
 * Description : Cherche le premier bloc libre de taille suffisante pour satisfaire la demande
 * de mémoire spécifiée par "wanted_size." Elle renvoie un pointeur vers le bloc libre précédent
 * au premier bloc satisfaisant. Si aucun bloc n'est trouvé, elle renvoie NULL.
 */
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

/**
 * Fonction mem_best_fit
 *
 * Paramètres :
 *   - first_free_block (pointeur vers le premier bloc libre)
 *   - wanted_size (taille désirée de l'allocation)
 *
 * Renvoie : Un pointeur vers le bloc libre précédent correspondant au "meilleur ajustement,"
 *           ou NULL si aucun bloc libre n'est disponible.
 *
 * Description : Recherche le bloc libre qui offre le "meilleur ajustement" (best fit) pour
 * satisfaire la demande de mémoire spécifiée par "wanted_size." La fonction renvoie un pointeur
 * vers le bloc libre précédent du meilleur bloc satisfaisant. Si aucun bloc n'est trouvé, elle
 * renvoie NULL.
 */
mem_free_block_t *mem_best_fit(mem_free_block_t *first_free_block, size_t wanted_size)
{
    struct fb *previous_block = (struct fb *)first_free_block;
    struct fb *current_block = previous_block->next;
    struct fb *best_fit_block = NULL;
    struct fb *best_fit_previous_block = NULL;

    while (current_block != NULL)
    {
        if (current_block->free_size >= wanted_size &&
            (best_fit_block == NULL || current_block->free_size < best_fit_block->free_size))
        {
            best_fit_previous_block = previous_block;
            best_fit_block = previous_block->next;
        }

        previous_block = current_block;
        current_block = current_block->next;
    }

    return (mem_free_block_t *)best_fit_previous_block;
}

/**
 * Fonction mem_worst_fit
 *
 * Paramètres :
 *   - first_free_block (pointeur vers le premier bloc libre)
 *   - wanted_size (taille désirée de l'allocation)
 *
 * Renvoie : Un pointeur vers le bloc libre précédent correspondant au "pire ajustement,"
 *           ou NULL si aucun bloc libre n'est disponible.
 *
 * Description : Recherche le bloc libre qui offre le "pire ajustement" (worst fit) pour
 * satisfaire la demande de mémoire spécifiée par "wanted_size." La fonction renvoie un pointeur
 * vers le bloc libre précédent du pire bloc satisfaisant. Si aucun bloc n'est trouvé, elle
 * renvoie NULL.
 */
mem_free_block_t *mem_worst_fit(mem_free_block_t *first_free_block, size_t wanted_size)
{
    struct fb *previous_block = (struct fb *)first_free_block;
    struct fb *current_block = previous_block->next;
    struct fb *worst_fit_block = NULL;
    struct fb *worst_fit_previous_block = NULL;

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
