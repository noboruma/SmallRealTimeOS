/* NOYAU.C */
/*--------------------------------------------------------------------------*
 *               Code C du noyau preemptif qui tourne sur ARM                *
 *                                 NOYAU.C                                  *
 *--------------------------------------------------------------------------*/

#include <stdint.h>

#include "serialio.h"
#include "imx_timers.h"
#include "imx_aitc.h"
#include "noyau.h"

/*--------------------------------------------------------------------------*
 *            Variables internes du noyau                                   *
 *--------------------------------------------------------------------------*/
static int compteurs[MAX_TACHES]; /* Compteurs d'activations */
CONTEXTE _contexte[MAX_TACHES];   /* tableau des contextes */
volatile uint16_t _tache_c;       /* numéro de tache courante */
uint32_t  _tos;                   /* adresse du sommet de pile */
int  _ack_timer = 1;              /* = 1 si il faut acquitter le timer */

/*--------------------------------------------------------------------------*
 *                        Fin de l'execution                                *
 *----------------------------------------------------------------------- --*/
void	noyau_exit(void)
{
  	/* Désactiver les interruptions */
	_irq_disable_();

  printf("Sortie du noyau\n");
  int i=0;
  for(i=0;i<MAX_TACHES;i++)
	  printf("%u : %u\n",i,compteurs[i]);
	/* afficher par exemple le nombre d'activation de chaque tache */
								
	/* Terminer l'exécution */
	while(1); 		// eviter les exceptions
}

/*--------------------------------------------------------------------------*
 *                        --- Fin d'une tache ---                           *
 * Entree : Neant                                                           *
 * Sortie : Neant                                                           *
 * Descrip: Cette proc. doit etre appelee a la fin des taches               *
 *                                                                          *
 *----------------------------------------------------------------------- --*/
void  fin_tache(void)
{
  /* on interdit les interruptions */
	_irq_disable_();
  /* la tache est enlevee de la file des taches */
	retire(_tache_c);
	_set_arm_mode_(ARMMODE_IRQ);
	schedule();

}

/*--------------------------------------------------------------------------*
 *                        --- Creer une tache ---                           *
 * Entree : Adresse de la tache                                             *
 * Sortie : Numero de la tache creee                                        *
 * Descrip: Cette procedure cree une tache en lui allouant                  *
 *    - une pile (deux piles en fait une pour chaque mode du processeur)    *
 *    - un numero                                                           *
 * Err. fatale: priorite erronnee, depassement du nb. maximal de taches     *
 *	                                                                        *
 *--------------------------------------------------------------------------*/
uint16_t cree( TACHE_ADR adr_tache )
{
  CONTEXTE *p;                    /* pointeur d'une case de _contexte */
  static uint16_t tache =-1;   /* contient numero dernier cree */
	_lock_();					/* debut section critique */

	tache++;					/* numero de tache suivant */

  if (tache >= MAX_TACHES)        /* sortie si depassement */
	{
				/* sortie car depassement       */
		  _unlock_();
		  noyau_exit();
	}	

	p = &_contexte[tache];			/* contexte de la nouvelle tache */

	p->sp_irq= _tos;				/* allocation d'une pile a la tache */
	_tos -= PILE_IRQ;
	p->sp_ini = _tos;
	_tos -= PILE_TACHE;					/* decrementation du pointeur de pile pour la prochaine tache. */

	 _unlock_();					/* fin section critique */

	 p->tache_adr = adr_tache;		/* memorisation adresse debut de tache */
	 p->status = CREE;  			/* mise a l'etat CREE */
	 return(tache);                  /* tache est un uint16_t */
}

/*--------------------------------------------------------------------------*
 *                          --- Elire une tache ---                         *
 * Entree : Numero de la tache                                              *
 * Sortie : Neant                                                           *
 * Descrip: Cette procedure place une tache dans la file d'attente des      *
 *	    taches eligibles.                                                   *
 *	    Les activations ne sont pas memorisee                               *
 * Err. fatale: Tache inconnue	                                            *
 *                                                                          *
 *--------------------------------------------------------------------------*/
void  active( uint16_t tache )
{
  CONTEXTE *p = &_contexte[tache]; /* acces au contexte tache */

  if (p->status == NCREE)
  {
	  noyau_exit();		/* sortie du noyau       */
  }

  _lock_();						/* debut section critique */
  if (p->status == CREE)          	/* n'active que si receptif */
  {
		p->status = PRET;			/* changement d'etat, mise a l'etat PRET */
		ajoute(tache);				/* ajouter la tache dans la liste */
		schedule();					/* activation d'une tache prete */
  }
	_unlock_();						/* fin section critique */
}


/*--------------------------------------------------------------------------*
 *                  ORDONNANCEUR preemptif optimise                         *
 *                                                                          *
 *             !! Cette fonction doit s'exécuter en mode IRQ !!             *
 *  !! Pas d'appel direct ! Utiliser schedule pour provoquer une            *
 *  commutation !!                                                          *
 *--------------------------------------------------------------------------*/
void __attribute__((naked)) scheduler( void )
{
  register CONTEXTE *p;
  register unsigned int sp asm("sp");  /* Pointeur de pile */

  /* Sauvegarder le contexte complet sur la pile IRQ */
  __asm__ __volatile__(
		"stmfd sp, {r0-r14}^\t\n"	/* Sauvegarde registres mode system */
		"nop\t\n"				/* Attendre un cycle */
		"sub sp, sp, #0x3C\t\n"	/* Ajustement pointeur de pile */
		"mrs r0,  spsr\t\n"		/* Sauvegarde de spsr_irq */
		"nop\t\n"
		"stmfd sp!, {r0,lr}\t\n"
		//"STMfd sp!, {lr}\t\n"	/* et de lr_irq */
		  :::);

  if (_ack_timer)                 /* Réinitialiser le timer si nécessaire */
  {
    	/* Acquiter l'événement de comparaison du Timer pour pouvoir */
		/* obtenir le déclencement d'une prochaine interruption */
	  	  struct imx_timer* tim1 = (struct imx_timer *) TIMER1_BASE;
	  	  int tmp = tim1->tstat;
	  	  tim1->tstat = tim1->tstat & ~(0x1);
  }
  else
  {
    _ack_timer = 1;
  }

  	p->sp_irq = sp;				/* memoriser le pointeur de pile */

	_tache_c = suivant();						/* recherche du suivant */
	p = &_contexte[_tache_c];		/* p pointe sur la nouvelle tache courante*/


  	compteurs[_tache_c]++;    /* Incrémenter le compteur d'activations  */

  if (p->status == PRET)          /* tache prete ? */
  {
	sp = p->sp_irq;//sp_ini				/* Charger sp_irq initial */
	_set_arm_mode_(ARMMODE_SYS);	/* Passer en mode système */
	sp = p->sp_ini;//-PILE_IRQ;				/* Charger sp_sys initial */
	p->status = EXEC;				/* status tache -> execution */
	_irq_enable_();					/* autoriser les interuptions   */
	(p->tache_adr)();		/* lancement de la tâche */
  }
  else
  {
		sp = p->sp_irq;				/* tache deja en execution, restaurer sp_irq */
  }

  /* Restaurer le contexte complet depuis la pile IRQ */
  __asm__ __volatile__(
		"ldmfd sp!,{r0,lr}\t\n"		/* Restaurer lr_irq */
		//"ldmfd sp!,{r0}\t\n"		/* et spsr_irq */
		"msr spsr_cxsf,r0 \t\n"
		"nop\t\n"
		"ldmfd sp, {r0-r14}^\t\n"	/* Restaurer registres mode system */
		"nop\t\n"		/* Attendre un cycle */
		"add sp, sp, #0x3C\t\n" /* Ajuster pointeur de pile irq */
		"subs pc, lr,#4\t\n"		/* Retour d'exception */
		  :::);
}


/*--------------------------------------------------------------------------*
 *                  --- Provoquer une commutation ---                       *
 *                                                                          *
 *             !! Cette fonction doit s'exécuter en mode IRQ !!             *
 *  !! Pas d'appel direct ! Utiliser schedule pour provoquer une            *
 *  commutation !!                                                          *
 *--------------------------------------------------------------------------*/
void  schedule( void )
{
	_lock_();								/* Debut section critique */

  /* On simule une exception irq pour forcer un appel correct à scheduler().*/
	_ack_timer = 0;
	_set_arm_mode_(ARMMODE_IRQ);	/* Passer en mode IRQ */

	__asm__ __volatile__(
			"mrs  r0,cpsr\t\n"		/* Sauvegarder cpsr dans spsr */
		  	"msr  spsr_cxsf,r0\t\n"
			"nop\t\n"
		  	"add lr,pc,#0x4\t\n"/* Sauvegarder pc dans lr et l'ajuster car on va dans scheduler et on veut revzenir ici a la fin de scheduler ! */
		  	"b scheduler\t\n"			/* Saut à scheduler */
		  :::);

	_set_arm_mode_(ARMMODE_SYS);	/* Repasser en mode system */

	_unlock_();								/* Fin section critique */
}

/*--------------------------------------------------------------------------*
 *                        --- Lancer le systeme ---                         *
 * Entree : Adresse de la premiere tache a lancer                           *
 * Sortie : Neant                                                           *
 * Descrip: Active la tache et lance le systeme                             *
 *                                                                          *
 *                                                                          *
 * Err. fatale: Neant                                                       *
 *                                                                          *
 *--------------------------------------------------------------------------*/
void	start( TACHE_ADR adr_tache )
{
  short j;
  register unsigned int sp asm("sp");
  struct imx_timer* tim1 = (struct imx_timer *) TIMER1_BASE;
  struct imx_aitc* aitc = (struct imx_aitc *) AITC_BASE;

  for (j=0; j<MAX_TACHES; j++)
  {
	  _contexte[j].status=NCREE;	/* initialisation de l'etat des taches */
	  compteurs[j]=0;
  }
  _tache_c = 0;						/* initialisation de la tache courante */
  file_init(); 						/* initialisation de la file           */

  _tos = sp;				/* Initialisation de la variable Haut de la pile des tâches */
  _set_arm_mode_(ARMMODE_IRQ);		/* Passer en mode IRQ */
  sp = 	_tos;					/* sp_irq initial */
  _set_arm_mode_(ARMMODE_SYS);		/* Repasser en mode SYS */

  _irq_disable_();					/* on interdit les interruptions */

  /* Initialisation du timer à 100 Hz */
	tim1->tctl = 0x00000015;
	tim1->tprer = 0x00000064;
	tim1->tcmp = 0x00000064;//2710;

  /* Initialisation de l'AITC */
	aitc->intenableh = 0x8000000;
	aitc->intennum = TIMER1_INT;//0x3B;

	/* creation et activation premiere tache */
	active(cree(adr_tache));
}


/*-------------------------------------------------------------------------*
 *                    --- Endormir la tâche courante ---                   *
 * Entree : Neant                                                          *
 * Sortie : Neant                                                          *
 * Descrip: Endort la tâche courante et attribue le processeur à la tâche  *
 *          suivante.                                                      *
 *                                                                         *
 * Err. fatale:Neant                                                       *
 *                                                                         *
 *-------------------------------------------------------------------------*/

void  dort(void)
{

}

/*-------------------------------------------------------------------------*
 *                    --- Réveille une tâche ---                           *
 * Entree : numéro de la tâche à réveiller                                 *
 * Sortie : Neant                                                          *
 * Descrip: Réveille une tâche. Les signaux de réveil ne sont pas mémorisés*
 *                                                                         *
 * Err. fatale:tâche non créée                                             *
 *                                                                         *
 *-------------------------------------------------------------------------*/


void reveille(uint16_t t)
{
	
}

