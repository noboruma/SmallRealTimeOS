#include "noyau.h"
#include "imx_serial.h"

/*
 ** Test du noyau preemptif. Lier ce fichier avec noyau.c et noyaufil.c
 */
int main(void)
{
	serial_init(115200);
	printf("Kernel started !");
	file_init();

	ajoute(3);
        affic_file();
 	ajoute(5);
	ajoute(1);
	ajoute(0);
	ajoute(2);
	affic_file();

	suivant();
	affic_file();

	retire(0);
	affic_file();

	ajoute(6);

	affic_file();

	while(1);

	return 0;
}
