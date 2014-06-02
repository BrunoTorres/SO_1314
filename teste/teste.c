#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "contagem.h"


void hand(int s){
	if (s == SIGALRM){
		kill(getpid(),SIGCONT);		
	}
}

int main(int argc, char const *argv[])
{
	char *nomes[] = {"Viana","Cerveira","Campos",NULL};
	char *nomes_v[] = {"Viana","Cerveira","Lovelhe",NULL};
	char *nomes_v2[] = {"Viana","Cerveira","Covas",NULL};
	char *nomes_aux[] = {"Braga","Barcelos","Guimaraes",NULL};
	char *nomes2[] = {"Viana",NULL};
	char *nomes3[] = {"Braga",NULL};
	int valor = 1,num=0;

	while (num< 400){
		incrementar(nomes,valor);
		incrementar(nomes_v,valor);
		incrementar(nomes_v2,valor);
		incrementar(nomes_aux,valor);
		//agregar(nomes2,0,"viana_estupido.txt");
		//alarm(5);
		//pause();
		num++;
	}
	agregar(nomes2,2,"viana.txt");
	agregar(nomes3,2,"braga.txt");
	return 0;
}
