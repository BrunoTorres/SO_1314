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
	char *nomes[] = {"Porto","Maia","Campos",NULL};
	char *nomes_v[] = {"Porto","Maia","Lovelhe",NULL};
	char *nomes_v2[] = {"Porto","Maia","Covas",NULL};
	char *nomes_aux[] = {"Faro","Barcelos","Guimaraes",NULL};
	char *nomes2[] = {"Porto",NULL};
	char *nomes3[] = {"Faro",NULL};
	int valor = 1,num=0;
	signal(SIGALRM,hand);

	while (num< 300000){
		incrementar(nomes,valor);
		incrementar(nomes_v,valor);
		incrementar(nomes_v2,valor);
		incrementar(nomes_aux,valor);
		agregar(nomes2,0,"Porto_meio.txt");
		//alarm(1);
		//pause();
		num++;
	}
	agregar(nomes2,2,"Porto.txt");
	agregar(nomes3,2,"Faro.txt");
		return 0;
}
