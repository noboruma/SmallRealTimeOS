/* NOYAUFILE.C */
/*--------------------------------------------------------------------------*
 *  gestion de la file d'attente des taches pretes et actives               *
 *  la file est rangee dans un tableau. ce fichier decrit toutes            *
 *  les primitives de base                                                  *
 *--------------------------------------------------------------------------*/
#include <stdint.h>
#include "serialio.h"
#include "noyau.h"

/* variables communes a toutes les procedures *
 *--------------------------------------------*/

static uint16_t  _file[MAX_TACHES];   /* indice=numero de tache */
				   /* valeur=tache suivante  */
static uint16_t  _queue;              /* valeur de la derniere tache */
                        		   /* pointe la prochaine tache a activer */

/*     initialisation de la file      *
 *------------------------------------*
entre  : sans
sortie : sans
description : la queue est initialisee vide, queue prend la valeur de tache
	      impossible
*/

void	file_init( void )
{
    uint16_t i;
    for (i = 0; i < MAX_TACHES; ++i)
        _file[i] = F_VIDE;
    _queue = F_VIDE;
}

/*        ajouter une tache dans la pile      *
 *--------------------------------------------*
entree : n numero de la tache a entrer
sortie : sans
description : ajoute la tache n en fin de pile
*/

void	ajoute ( uint16_t n )
{
    uint16_t last;

    // hors borne
    if(n >= MAX_TACHES) return;
    
    // si la pile est vide
    if(_queue == F_VIDE) {
        _file[n] = n;
        _queue = n;
    } else {
       last = _file[_queue];
        _file[_queue] = n;
        _file[n] = last;
        _queue = n;
    }
}

/*           retire une tache de la file        *
 *----------------------------------------------*
entree : t numero de la tache a sortir
sortie : sans
description: sort la tache t de la file. L'ordre de la file n'est pas
	     modifie
*/

void	retire( uint16_t t )
{
    uint16_t i;
    // hors borne
    if(t >= MAX_TACHES || _file[_queue] == F_VIDE) 
	return;
    
    // Recuperation tâche precedente
    i = 0;
    while (i < MAX_TACHES && _file[i] != t)
	++i;
    
    if(i == MAX_TACHES)
	return;// Pas trouvé
    
    _file[i] = _file[t];
    _file[t] = F_VIDE;

    // Si on a touché à la queue.
    if(t == _queue) {      
	_queue = i;
        //S'il ne restait plus que queue
	if(t == i)
	    _queue = F_VIDE;
    }
}


/*        recherche du suivant a executer       *
 *----------------------------------------------*
entree : sans
sortie : t numero de la tache a activer
description : la tache a activer est sortie de la file. queue pointe la
	      suivante
*/
uint16_t	suivant( void )
{
    uint16_t next = _queue;
    _queue = _file[_queue];
    return next;
}

/*     affichage du dernier element     *
 *--------------------------------------*
entree : sans
sortie : sans
description : affiche la valeur de queue
*/

void affic_queue( void )
{
    printf("Queue : %u\n", _queue);
}

/*     affichage de la file     *
 *------------------------------*
entree : sans
sortie : sans
description : affiche les valeurs de la file
*/

void affic_file( void )
{
    uint16_t i;
    printf("Values : ");
    for (i = 0; i < MAX_TACHES; ++i)
        if(_file[i] == F_VIDE)
            printf(" - |");
        else
            printf(" %u |", _file[i]);

    printf("\nIndex  : ");
    for (i = 0; i < MAX_TACHES; ++i)
        printf(" %u |", i);
    printf("\n");
}
