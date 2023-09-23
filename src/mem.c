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

//structure utilisée pour gérer les blocs occupés
struct bb {
	size_t size;
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

	//Initialiser le pointeur vers le bloc à allouer
	struct bb* allocated_block = NULL;

    // Parcourir la liste chaînée
    while (current_node != NULL) {
        // Vérifier si la taille du maillon actuel est inférieure ou égale à (size + sizeof(struct fb))
        if (current_node->size >= (size + sizeof(struct fb))) {

            // Calculer la nouvelle adresse en ajoutant l'offset à l'adresse du maillon actuel
            char* new_address = (char*)current_node + (size + sizeof(struct fb));

            //Calculer la nouvelle adresse en ajoutant l'offset à l'adresse du pointeur vers la zone allouée
			char* bb_address = (char*)current_node + (size + sizeof(struct bb));

			// Mettre à jour le pointeur allocated_block avec la nouvelle adresse
			allocated_block = (struct bb*)new_address;

			// Mettre à jour la taille du pointeur de la zone allouée
			allocated_block->size = (size + sizeof(struct bb));
            // Mettre à jour le pointeur current_node avec la nouvelle adresse
            current_node = (struct fb*)new_address;

            // Mettre à jour la taille du nouveau maillon current_node
            current_node->size -= (size + sizeof(struct fb));

            // Retourner un pointeur vers le maillon précédent
            return (void*)previous_node;
        }
		//Initialiser le pointeur de la zone occupée

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
size_t mem_get_size(void *zone) {
    //on vérifie que la zone n'est pas NULL
    if (zone != NULL) {
        return sizeof(*zone);
    }
	else{
		return 0;
	}
}

//-------------------------------------------------------------
// mem_free
//-------------------------------------------------------------
/**
 * Free an allocaetd bloc.
**/
void mem_free(void *zone) {
	//on récupère la 1er maillon de la liste chaînée
    struct fb* head = info->first;
	struct fb* tmp = head;
	int free =0;
	printf("knslvncd");
	//parcours de la liste pour vérifier que la zone est bien occupée
	while(tmp->next !=NULL)
	{
		if(zone != tmp)
		{
			tmp = tmp->next;
		}
		else
		{
			printf("cette adresse est déjà libre");
			free = 1;
			break;
		}
	}

	//parcours de la liste pour ajouter un maillon à la liste (soit trier en même temps soit trier à la toute fin)
	//penser à fusionner les zones
	tmp = head;
	struct fb* new_node;
	struct fb* tmp2 = tmp->next;
	if(free = 0)
	{
		while(tmp->next!=NULL)
		{
			//ajout d'un maillon dans la liste de zones libres
			if((char*)zone < (char*)tmp)
			{
				tmp->next = new_node;
				new_node->next = tmp2;
				new_node->size = mem_get_size(zone)+sizeof(struct bb);
			}

		}
		if((char*)zone > (char*)tmp)
		{
			tmp->next = new_node;
			new_node->next = NULL;
			new_node->size = mem_get_size(zone)+sizeof(struct bb);
		}
	}

	//zone de fusion
	assert(! "NOT IMPLEMENTED !");
}

//-------------------------------------------------------------
// Itérateur(parcours) sur le contenu de l'allocateur
// mem_show
//-------------------------------------------------------------
void mem_show(void (*print)(void *, size_t, int free)) {
    //printf("1ere adresse libre :=  %p\n",info->first);

	//calcul de la 1ère adresse
	struct fb* first_adress = (struct fb*)((char*)info + sizeof(struct info_alloc));
	//printf("1ere adresse après info_alloc %p\n",first_adress);

	//déclaration d'un tmp pour parcourir la liste
	struct fb* tmp = (struct fb*)((char*)info + sizeof(struct info_alloc));
	tmp = info->first;
	
	//taille définie dans mem_space.c, divisée par 2 car nombre maximum de zone libre dans la mémoire
	unsigned long *tailles_zones_libres[(MEMORY_SIZE/2)+1];
	unsigned long *adresses_zones_libres[(MEMORY_SIZE/2)+1];
	adresses_zones_libres[0] = (unsigned long*)first_adress;
	int i = 0;
	
	//parcours de la liste chaînée des zones libres et calcul des adresses
	if(tmp->next !=NULL)
	{
		printf("lknvdscdsl");
		while(tmp->next)
		{	
			tailles_zones_libres[i] = (unsigned long*)info->first->next->size;
			unsigned long uinttemp = (unsigned long)adresses_zones_libres[i]+(unsigned long)tailles_zones_libres[i];
			adresses_zones_libres[i+1] = (unsigned long*)uinttemp;
			tmp = tmp->next;
			i++;
		}
		i = 0;
		while(adresses_zones_libres[i+1]!=NULL)
		{
			print(adresses_zones_libres[i],*tailles_zones_libres[i],1);
		}
	}
	

	//afficher zones libres
	

	//pour les zones occupées prendre first_adress puis comparer avec tableau zones libre pour faire un tableau des zones occupées 



	//assert(! "NOT IMPLEMENTED !");
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
