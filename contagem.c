#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include "contagem.h"


int incrementar(char *nome[], unsigned valor){
	int cts;
	char *myfifo = "/tmp/cts";
	char str[BUFSIZ];
	cts = open(myfifo, O_WRONLY);
	sprintf(str,"[%s,%s,%s] %d",nome[0],nome[1],nome[2],(int)valor);
	write(cts, str, sizeof(str));
	memset(str, 0, sizeof(str));
	close(cts);
	return 0;
}

int agregar(char *prefixo[], unsigned nivel, char *path){
	int cts;
	char *myfifo = "/tmp/cts";
	char str[BUFSIZ];
	cts = open(myfifo, O_WRONLY);
	int num_prefixo=0;

	while(prefixo[num_prefixo]) num_prefixo++;

	switch (num_prefixo){
		case 1: sprintf(str,"agregar [%s] %d %s",prefixo[0],(int)nivel,path); 
			break;
		case 2: sprintf(str,"agregar [%s,%s] %d %s",prefixo[0],prefixo[1],(int)nivel,path); 
			break;
		case 3: sprintf(str,"agregar [%s,%s,%s] %d %s",prefixo[0],prefixo[1],prefixo[2],(int)nivel,path);
			break;
		default: sprintf(str,"ERRO");
			break;
	}
	if (strcmp(str,"ERRO")!=0){
		write(cts, str, sizeof(str));
		memset(str, 0, sizeof(str));
	}
	close(cts);
	return 0;
}
