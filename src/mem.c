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
struct fb
{
	size_t size;
	struct fb *next;
};

// structure utilisée pour gérer les blocs occupés
struct bb
{
	size_t size;
};

// structure utilisée pour gérer info_alloc, qui pointera vers le premier maillon libre
struct info_alloc
{
	struct fb *first;
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
	// Récupérer l'adresse de la mémoire la convertir en un pointeur vers struct info_alloc
	info = (struct info_alloc *)mem_space_get_addr();

	// Calculer l'adresse de init_fb en fonction de la taille de struct info_alloc
	struct fb *init_fb = (struct fb *)((char *)info + sizeof(struct info_alloc));

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
void *mem_alloc(size_t size)
{
	printf("Adresse du pointeur tmp : %p\n", (char *)info->first);
	// Initialiser le pointeur courant au premier maillon
	struct fb *current_node = info->first;
	printf("Adresse du pointeur info->first avant alloc: %p\n", (char *)info->first);
	size_t total_size = size + sizeof(struct bb);
	
	if (total_size >= sizeof(struct fb))
	{

		// Parcourir la liste chaînée
		while (current_node != NULL)
		{

			// Vérifier si la taille du maillon actuel est inférieure ou égale à total_size)
			if (current_node->size >= total_size)
			{
				
				size_t remaining_size = current_node->size - total_size;

				// Calculer la nouvelle adresse en ajoutant l'offset à l'adresse du maillon actuel
				char *new_address_fb = (char *)current_node + total_size;

				char *adresse_allouer = (char *)current_node + (sizeof(struct bb *));

				char *new_address_bb = (char *)current_node;

				// Mettre à jour le pointeur current_node avec la nouvelle adresse
				current_node = (struct fb *)new_address_fb;

				// Mettre à jour la taille du nouveau maillon current_node
				current_node->size = remaining_size;

				struct bb *new_bb = (struct bb *)new_address_bb;

				new_bb->size = size;

				// to update later when free is implemented
				info->first = (struct fb *)new_address_fb;
				printf("Adresse du pointeur info->first après alloc: %p\n", (char *)info->first);
				// Retourner un pointeur vers le maillon précédent
				return (void *)adresse_allouer;
			}

			// Passer au maillon suivant dans la liste chaînée
			current_node = current_node->next;
		}
		
	}



	// Si aucune condition n'est satisfaite, retourner NULL
	return NULL;
}

//-------------------------------------------------------------
// mem_get_size
//-------------------------------------------------------------
size_t mem_get_size(void *zone)
{
	// on vérifie que la zone n'est pas NULL
	if (zone != NULL)
	{
		return sizeof((char*)zone);
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
	// on récupère la 1er maillon de la liste chaînée
	struct fb *tmp = info->first;
	int free = 0;
	printf("1er affichage adresse de zone %p \n",zone); //adresse
	
	//printf("%p \n",zone); //adresse
	// parcours de la liste pour vérifier que la zone est bien occupée
	if(tmp->next == NULL)
	{
		printf("2e affichage adresse de tmp %p \n",tmp);
		if ((char*) zone == (char *)tmp)
		{
			printf("cette adresse est déjà libre");
			free = 1;
		}
	}

	while (tmp->next != NULL)
	{
		if ((char*) zone != (char *)tmp)
		{
			tmp = tmp->next;
		}
		else
		{
			printf("Cette adresse est déjà libre");
			free = 1;
			break;
		}
	}
	if(free == 0)
	{
		struct fb* current = (struct fb *)zone;
		printf("3e affichage adresse de current initialisation %p \n",current);
		current->size = mem_get_size(zone)+sizeof(struct bb*);
		struct fb* tmphead = info->first;

		//si 1er bloc adresse après init probablement inutile
		struct fb *first_adress = (struct fb *)((char *)info + sizeof(struct info_alloc));
		if((char *)first_adress == (char*)current)
		{
			info->first = current;
			current->next = tmphead;
			printf("4e affichage adresse de current si c'est le 1er bloc %p \n",current);
		}

		else
		{
			//si ajout en plein milieu de la chaîne avec 1 seul free block
 			if(tmphead->next == NULL)
			{
				info->first = current;
				current->next = tmphead;
				printf("5e affichage adresse de current si 1 seul free block %p \n",current);
			}
			//si ajout en plein milieu de la chaîne
			else if(tmphead->next!=NULL)
			{
				while(tmphead->next!=NULL)
				{
					if((char*)current < (char*)tmphead->next)
					{
						current->next = tmphead->next;
						tmphead->next = current;
						printf("6e affichage adresse de current si en plein milieu %p \n",current);
						break;
					}
					else
					{
						tmphead = tmphead->next;
					}
				}
			}
			
			//si bloc après le dernier maillon
			else
			{
				if((char*)current > (char*)tmphead)
					{
						current->next = NULL;
						tmphead->next = current;
						printf("7e affichage adresse de current si block après dernier fb %p \n",current);
					}
			}
		}
	}
	// zone de fusion
	//parcours de la liste des zones libres
	tmp = info->first;
	while(tmp !=NULL && tmp->next!=NULL)
	{
		char *next_address = (char *)tmp + tmp->size;
		if((char *)next_address == (char *)tmp->next)
		{
			tmp->size += tmp->next->size;
 			tmp->next = tmp->next->next;
		}
		tmp = tmp->next;
	}
	
	

	

	//assert(!"NOT IMPLEMENTED !");
}

//-------------------------------------------------------------
// Itérateur(parcours) sur le contenu de l'allocateur
// mem_show
//-------------------------------------------------------------
void mem_show(void (*print)(void *, size_t, int free))
{
	// printf("1ere adresse libre :=  %p\n",info->first);

	// calcul de la 1ère adresse
	struct fb *first_adress = (struct fb *)((char *)info + sizeof(struct info_alloc));
	// printf("1ere adresse après info_alloc %p\n",first_adress);

	// déclaration d'un tmp pour parcourir la liste
	struct fb *tmp = (struct fb *)((char *)info + sizeof(struct info_alloc));
	tmp = info->first;

	// taille définie dans mem_space.c, divisée par 2 car nombre maximum de zone libre dans la mémoire
	unsigned long *tailles_zones_libres[(MEMORY_SIZE / 2) + 1];
	unsigned long *adresses_zones_libres[(MEMORY_SIZE / 2) + 1];
	adresses_zones_libres[0] = (unsigned long *)first_adress;
	int i = 0;

	// parcours de la liste chaînée des zones libres et calcul des adresses
	if (tmp->next != NULL)
	{
		while (tmp->next)
		{
			tailles_zones_libres[i] = (unsigned long *)info->first->next->size;
			unsigned long uinttemp = (unsigned long)adresses_zones_libres[i] + (unsigned long)tailles_zones_libres[i];
			adresses_zones_libres[i + 1] = (unsigned long *)uinttemp;
			tmp = tmp->next;
			i++;
		}
		i = 0;
		while (adresses_zones_libres[i + 1] != NULL)
		{
			print(adresses_zones_libres[i], *tailles_zones_libres[i], 1);
		}
	}

	// afficher zones libres

	// pour les zones occupées prendre first_adress puis comparer avec tableau zones libre pour faire un tableau des zones occupées

	// assert(! "NOT IMPLEMENTED !");
}

//-------------------------------------------------------------
// mem_fit
//-------------------------------------------------------------
void mem_set_fit_handler(mem_fit_function_t *mff)
{
	// TODO: implement
	assert(!"NOT IMPLEMENTED !");
}

//-------------------------------------------------------------
// Stratégies d'allocation
//-------------------------------------------------------------
mem_free_block_t *mem_first_fit(mem_free_block_t *first_free_block, size_t wanted_size)
{
	// TODO: implement
	assert(!"NOT IMPLEMENTED !");
	return NULL;
}
//-------------------------------------------------------------
mem_free_block_t *mem_best_fit(mem_free_block_t *first_free_block, size_t wanted_size)
{
	// TODO: implement
	assert(!"NOT IMPLEMENTED !");
	return NULL;
}

//-------------------------------------------------------------
mem_free_block_t *mem_worst_fit(mem_free_block_t *first_free_block, size_t wanted_size)
{
	// TODO: implement
	assert(!"NOT IMPLEMENTED !");
	return NULL;
}
