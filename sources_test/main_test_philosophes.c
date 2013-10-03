/* NOYAUTEST.C */
/*--------------------------------------------------------------------------*
 *			      Programme de tests			    *
 *--------------------------------------------------------------------------*/

#include "serialio.h"
#include "noyau.h"

/*
 ** Test du noyau preemptif. Lier ce fichier avec noyau.c et noyaufil.c
 */
#define MANGE 1
#define PENSE 0
#define VEUTMANGE 2

short philo[5];
short curr_id;

int mutex,mySem;

TACHE	Philo(void)
{
  //Système d'identification permettant de ne pas créer 1 fonction par philosophe
  int curr = curr_id;
  printf("------> Start Philo %d\n", curr);
  while(1){
	  int voisGauche, voisGauche2;

	  if(curr == 0){
		  voisGauche = 4;
		  voisGauche2 = 3;
	  } else {
		  voisGauche = curr -1;

		  if(curr == 1)
			  voisGauche2 = 4;
		  else
			  voisGauche2 = curr -2;
	  }

	  if(philo[curr] != MANGE){
		  /* Philosophe veut manger */
		  s_wait(mutex);

		  if(philo[voisGauche] ==PENSE && philo[(curr+1)%5]==PENSE){
			  s_signal(mySem);
			  philo[curr]=MANGE;
		  } else {
			  philo[curr]=VEUTMANGE;
			  //wait
		  }
		  s_signal(mutex);
		  printf("Philosophe %d mange\n", curr);
		  s_wait(mySem);

	  } else {
		  /* Philosophe arrete de manger et pense */
		  s_wait(mutex);
		  if(philo[voisGauche] == VEUTMANGE && philo[voisGauche2] == PENSE){
			  philo[voisGauche] == MANGE;
			  s_signal(mySem);
		  }

		  if(philo[(curr+1)%5] == VEUTMANGE && philo[(curr+2)%5] == PENSE){
			  philo[(curr+1)%5] == MANGE;
			  s_signal(mySem);
		  }

		  philo[curr] = PENSE;
		  printf("Philosophe %d pense\n", curr);
		  s_signal(mutex);
	  }

  }

  fin_tache();
}

TACHE	tacheInit(void)
{
  puts("------> EXEC tache Init");

  //Système d'identifiant pour ne pas avoir à créer 1 fonction par philosophe
  curr_id = 0;
  active(cree(Philo));
  curr_id++;
  active(cree(Philo));
  curr_id++;
  active(cree(Philo));
  curr_id++;
  active(cree(Philo));
  curr_id++;
  active(cree(Philo));

  puts("------> FIN tache Init");
  fin_tache();
}

int main()
{
  serial_init(115200);
  puts("Test noyau");
  puts("Noyau preemptif");

  s_init();

  //Init file FIFO
  int i=0;
  for (i=0;i<5;i++)
	  philo[i]=0;
  mutex = s_cree(1);
  mySem = s_cree(0);
  start(tacheInit);

  while(1);

  return(0);
}

