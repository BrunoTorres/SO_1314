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
	char *nomes_aux[] = {"Braga","Barcelos","Guimaraes",NULL};
	char *nomes2[] = {"Viana",NULL};
	int valor = 1,num=0;

	while (num< 200){
		incrementar(nomes,valor);
		incrementar(nomes_aux,valor);
		//agregar(nomes2,0,"viana.txt");
		//alarm(5);
		//pause();
		num++;
	}
	//agregar(nomes2,0,"viana.txt");
	return 0;
}
