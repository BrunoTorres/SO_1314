#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include "server.h"

filho *filhos = NULL;

void initFilho(filho *d){
    d->nome = "";
    d->pid = 0;
    d->next = NULL;
}

char* tira(char *str)
{
    memmove(str, str+1, strlen(str));
    return str;
}

//static filho *filhoritos = NULL;

char *tiraEspacos(char* msg){
    char *aux = malloc(sizeof(msg));
    int n=0,m=0;
    while (msg[n] != '\0'){
        if(!isspace(msg[n])){
            aux[m] = msg[n];
            m++;
        }
        n++;
    }
    aux[m]='\0';
    return aux;
}

void addFilho (filho *lista_ptr, char *nome, const int pid)
{
    filho *antes_ptr;
    filho *depois_ptr;
    filho *novo_ptr;
    
    
    antes_ptr = lista_ptr;
    depois_ptr =  antes_ptr->next;
    
    while (1) {
        if (depois_ptr == NULL)
            break;
        /* Advance the pointers */
        depois_ptr = depois_ptr->next;
        antes_ptr = antes_ptr->next;
    }
    
    novo_ptr = malloc(sizeof(filho));
    novo_ptr->nome = nome;
    novo_ptr->pid = pid;
    
    antes_ptr->next = novo_ptr;
    novo_ptr->next = depois_ptr;
    
}


int incrementar(char *nome[], unsigned valor){
    printf("%s -> %d\n", nome[0], valor);
    filho *aux = filhos;
    int client_to_server;
    
    //TRIM ' '
    if (nome[1] && (nome[1][0] == ' ')) nome[1] = tira(nome[1]);
    if (nome[2] && (nome[2][0] == ' ')) nome[2] = tira(nome[2]);
    
    printf("1 - %s\n2 - %s\n3 - %s\nValor - %d\n\n",nome[0],nome[1],nome[2],valor);

    while (aux){
        // Verifica se existe algum filho com o Distrito
        if (strcmp(nome[0],aux->nome)==0) break;
        aux = aux->next;
    }

    if (aux) {
        // Existe um Filho com o Distrito
        char fifo[20];
        sprintf(fifo, "/tmp/%s",aux->nome);
        char v[100];
        if(nome[2]) sprintf(v, "%s:%s:%d",nome[1],nome[2],valor); else sprintf(v, "%s:%d",nome[1],valor);
        client_to_server = open(fifo, O_WRONLY);
        write(client_to_server, v, sizeof(v));
    } else {
        // Não existe Filho com o Distrito
        int pid;
        if ((pid=fork())==0) {
            // FILHO

            int f_cts; 
            int f_fdin;
            char f_buf[BUFSIZ]; // String Recebida

            char f_myfifo[20];
            char f_file[20];
            sprintf(f_file,"/tmp/%s.txt",nome[0]); // PATH TXT
            sprintf(f_myfifo, "/tmp/%s",nome[0]); // PATH Pipe 
            
            mkfifo(f_myfifo, 0666);
            
            f_fdin = open(f_file,O_CREAT |  O_TRUNC | O_WRONLY,0666); // Escreve TXT
            f_cts = open(f_myfifo, O_RDONLY); // Lê do Pipe
            
            while (1)
            {
                read(f_cts, f_buf, BUFSIZ);

                if (strcmp(f_buf,"")>0){
                    //printf("MSG > %s\n", buf);
                    write(f_fdin,f_buf,strlen(f_buf));
                    write(f_fdin,"\n",1);
                }
                memset(f_buf, 0, sizeof(f_buf));
            }
            close(f_cts);
            close(f_fdin);
            _exit(1);
        } else {
            addFilho(filhos, nome[0], pid);
        }
    }
    return 0;
}

// Control-C
void hand(int s){
    if (s == SIGINT){
        filho *aux = filhos;
        while(aux){
            if(strcmp(aux->nome,"")!=0){
                printf("\nKILL :: Filho %s com o PID %d\n",aux->nome,aux->pid);
                kill(aux->pid,9);
            }
            aux = aux->next;
        }
    }
    exit(0);
}

int main(int argc, char const *argv[])
{
    int cts; 
    char *myfifo = "/tmp/cts";
    char buf[BUFSIZ];
    int fdin;
    
    filhos = malloc(sizeof(filho));
    
    initFilho(filhos); //Inicializar

    signal(SIGINT,hand); //Control-C
    mkfifo(myfifo, 0666); // Pipe Cliente to Server
    
    fdin = open("log.txt",O_APPEND | O_TRUNC | O_WRONLY,0666);
    
    cts = open(myfifo, O_RDONLY);
    while (1)
    {
        read(cts, buf, BUFSIZ);
        
        if (strcmp(buf,"")>0){
            if(buf[0]=='['){
                tira(buf);
                char *saveptr;
                char *c = strdup(buf);
                char *caminho_s = strtok_r(c, "]", &saveptr);
                char *valor = strtok_r(NULL, ",", &saveptr);
                
                char *caminho[3];
                caminho[0] = strtok_r(caminho_s,",", &caminho[1]);
                caminho[1] = strtok_r(caminho[1],",", &caminho[2]);
                
                incrementar(caminho,atoi(valor));
                
            } else {
                filho *aux = filhos;
                while (aux){
                    if (strcmp(aux->nome,"")!=0){
                        printf("DISTRITO: %s\n",aux->nome);
                        printf("PID: %d",aux->pid);
                        printf("\n");
                    }
                    aux = aux->next;
                }
            }
        }
        memset(buf, 0, sizeof(buf));
    }

    close(cts);
    close(fdin);
    
    return 0;
}