/* NOYAUTEST.C */
/*--------------------------------------------------------------------------*
 *			      Programme de tests			    *
 *--------------------------------------------------------------------------*/

#include "serialio.h"
#include "noyau.h"

/*
 ** Test du noyau preemptif. Lier ce fichier avec noyau.c et noyaufil.c
 */
typedef struct{
	int file[MAX_TACHES];
	int deb,fin;
}FIFO;

FIFO field;

int mySem,mySem2;

TACHE	Conso(void)
{
  puts("------> Start Conso 1 ");
  while(1){
	  puts("------> Conso 1 : puis-je ?");
	  s_wait(mySem2);
	  printf("Conso 1 : j'ai mangé : %d\n",field.file[field.deb]);
	  field.deb = (field.deb+1) % MAX_TACHES;
	  s_signal(mySem);puts("------> Conso 1 : go");
  }
  fin_tache();
}

TACHE	Conso2(void)
{
  puts("------> Start Conso 2");
  while(1){
	  puts("------> Conso 2 : puis-je ?");
	  s_wait(mySem2);
	  printf("Conso 2 : j'ai mangé : %d\n",field.file[field.deb]);
	  field.deb = (field.deb+1) % MAX_TACHES;
	  s_signal(mySem);puts("------> Conso 2 : go");
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
	  field.file[field.fin] = field.fin;
	  field.fin = (field.fin+1) % MAX_TACHES;
	  puts("------> Prod : go");
	  s_signal(mySem2);
	  s_signal(mySem2);
	  s_signal(mySem2);
  }
  fin_tache();
}

TACHE	tacheInit(void)
{
  puts("------> EXEC tache Init");
  active(cree(Conso));
  active(cree(Conso2));
  active(cree(Prod));

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
  field.deb=0;
  field.fin=0;
  int i=0;
  for (i=0;i<MAX_TACHES;i++)
	  field.file[i]=0;

  mySem = s_cree(1);
  mySem2 = s_cree(0);
  start(tacheInit);

  while(1);

  return(0);
}

