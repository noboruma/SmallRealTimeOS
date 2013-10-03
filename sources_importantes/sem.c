/*
 * sem.c
 *
 *  Created on: 12 juin 2013
 *      Author: mi03
 */
#include "noyau.h"

#define MAX_SEM 10
#define UNUSED 0xFFFF
#define FAILED 0xFFFE

typedef struct{
	short file[MAX_TACHES]; /* File circulaire des taches en attente */
	short deb,fin;
	short valeur;		/* cpt de semaphore e(s) */
}SEMAPHORE;

SEMAPHORE _sem[MAX_SEM];

/*
 * Initialisation du syseme de sémaphore
 */
void s_init(void){
	short i,j;
	_lock_();
        //Pour chaque sémaphores possibles
	for ( i=0; i<MAX_SEM;i++){
		_sem[i].valeur=UNUSED;
		for(j=0;j<MAX_TACHES;j++)
			_sem[i].file[j]=UNUSED;//Ce n'est pas nécessaire car notre file circulaire est géré via ses indices.
		_sem[i].deb = 0;
		_sem[i].fin = 0;
	}
	_unlock_();

}

/*
 * Création d'une semaphore (allocation du compteur)
 */
ushort s_cree(short v){

	short i=0,res;

	_lock_();
	while (i<MAX_SEM && _sem[i].valeur != UNUSED)
		i++;

	if(i >= MAX_SEM)
		res = FAILED;
	else{
		_sem[i].valeur = v;
		res = i;
	}

	_unlock_();
	return res;

}

/*
 * Libération de sémaphore
 */
void s_close(ushort n){
	_lock_();
	if(n>=0 && n < MAX_SEM)
		_sem[n].valeur = UNUSED;
	_unlock_();
}

/*
 * P(n) : Puis-je prendre le sémaphore N ?
 */
void s_wait(ushort n){

	_lock_();
	if(n<0 && n >= MAX_TACHES)
		return;

        // S'il n'est pas possible d'aller plus loin
	if(_sem[n].valeur <= 0){
                //Mise ne place dans la file
		_sem[n].file[_sem[n].fin]=_tache_c;
		_sem[n].fin = (_sem[n].fin+1) % MAX_TACHES;
                //Attente passive
		dort();
	}
        //La tâche continue et entre dans la section critique
	_sem[n].valeur--;
	_unlock_();
}

/*
 * V(n) : Vas-y j'en ai fini avec la sémaphore N !
 */
void s_signal(ushort n){
	_lock_();
	if(n<0 && n >= MAX_TACHES)
		return;

	_sem[n].valeur++;

        //S'il reste des tâches qui attendent, il faut les prévenir !
	if(_sem[n].deb != _sem[n].fin){
		int tache = _sem[n].file[(_sem[n].deb)];
		 _sem[n].file[_sem[n].deb] = UNUSED;// Ce n'est pas nécessaire car notre file circulaire ne relira jamais un ancienne valeur.
		 _sem[n].deb = (_sem[n].deb + 1)%MAX_TACHES;
		reveille(tache);
	}
	_unlock_();

}
