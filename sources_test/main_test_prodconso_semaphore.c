/* NOYAUTEST.C */
/*--------------------------------------------------------------------------*
 *			      Programme de tests			    *
 *--------------------------------------------------------------------------*/

#include "serialio.h"
#include "noyau.h"

/*
 ** Test du noyau preemptif. Lier ce fichier avec noyau.c et noyaufil.c
 */

TACHE	tacheInit(void);
TACHE	Conso(void);
TACHE	Prod(void);

int field = 0;//Simule notre FIFO (Nous l'avons mise en place dans le cadre des multi consommateurs)
int tacheC;
int tacheP;
int mySem,mySem2;

TACHE	Conso(void)
{
  puts("------> Start Conso");
  while(1){
	  puts("------> Conso : puis-je ?");
	  s_wait(mySem2);
	  field=0; puts("------> Conso : je mange");
	  s_signal(mySem);puts("------> Conso : go");
  }
  fin_tache();
}

TACHE	Prod(void)
{
  puts("------> Start Prod");
  while(1){
	  puts("------> Prod : puis-je?");
	  s_wait(mySem);
	  puts("------> Prod : je prod");
	  field = 1;
	  puts("------> Prod : go");
	  s_signal(mySem2);
  }
  fin_tache();
}

TACHE	tacheInit(void)
{
  puts("------> EXEC tache tacheInit");
  tacheP = cree(Prod);
  tacheC = cree(Conso);
  active(tacheC);
  active(tacheP);

  puts("------> FIN tache tacheInit");
  fin_tache();
}

int main()
{
  serial_init(115200);
  puts("Test noyau");
  puts("Noyau preemptif");

  s_init();
  mySem = s_cree(1);
  mySem2 = s_cree(0);
  start(tacheInit);

  while(1);

  return(0);
}

