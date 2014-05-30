#include <stdio.h>
#include "contagem.h"

int main(int argc, char const *argv[])
{
	char *nomes[] = {"Viana","Cerveira","Campos",NULL};
	char *nomes_aux[] = {"Braga","Barcelos","Guimaraes",NULL};
	char *nomes2[] = {"Viana",NULL};
	int valor = 1,num=0;

	while (num< 4050){
		incrementar(nomes,valor);
		incrementar(nomes_aux,valor);
		//agregar(nomes2,0,"viana.txt");
		num++;
	}
	agregar(nomes2,0,"viana.txt");
	return 0;
}