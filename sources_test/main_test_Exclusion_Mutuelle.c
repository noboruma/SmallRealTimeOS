/* NOYAUTEST.C */
/*--------------------------------------------------------------------------*
 *			      Programme de tests			    *
 *--------------------------------------------------------------------------*/

#include "serialio.h"
#include "noyau.h"

/*
 ** Test du noyau preemptif. Lier noyautes.c avec noyau.c et noyaufil.c
 */

TACHE	tacheInit(void);
TACHE	Conso(void);
TACHE	Prod(void);

int field = 0;//Simule notre FIFO (Nous l'avons mise en place dans le cadre des multi consommateurs)

//Permet aux tâches de se connaître
int tacheC,tacheP;

TACHE	Conso(void)
{
  puts("------> Start Conso");
  while(1){
	  puts("------> Conso : je dors");
	  dort();
	  puts("------> Conso : je mange");
	  field=0;
	  puts("------> Conso : je reveille");
	  reveille(tacheP);
  }
  fin_tache();
}

TACHE	Prod(void)
{
  puts("------> Start Prod");
  while(1){
	  puts("------> Prod : je prod");
	  field = 1;
	  puts("------> Prod : je reveille");
	  reveille(tacheC);
	  puts("------> Prod : je dors");
	  dort();
  }
  fin_tache();
}

TACHE	tacheInit(void)
{
  puts("------> EXEC tache A");
  tacheP = cree(Prod);
  tacheC = cree(Conso);
  active(tacheC);
  active(tacheP);

  puts("------> FIN tache A");
  fin_tache();
}
/*
TACHE	tacheB(void)
{
  int i=0;
  long j;
  puts("------> DEBUT tache B");
  while (1) {
    for (j=0; j<30000L; j++);
    printf("======> Dans tache B %d\n",i);
    i++;
  }
}

TACHE	tacheC(void)
{
  int i=0;
  long j;
  puts("------> DEBUT tache C");
  while (1) {
    for (j=0; j<60000L; j++);
    printf("======> Dans tache C %d\n",i);
    i++;
  }
}

TACHE	tacheD(void)
{
  int i=0;
  long j;
  puts("------> DEBUT tache D");
  while (1) {
    for (j=0; j<120000L; j++);
    printf("======> Dans tache D %d\n",i++);
    if (i==50) noyau_exit();
  }
}
*/

int main()
{
  serial_init(115200);
  puts("Test noyau");
  puts("Noyau preemptif");
  start(tacheInit);
  while(1);
  return(0);
}

