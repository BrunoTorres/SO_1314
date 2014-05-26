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


void initFreg(freg *d){
    d->nome = "";
    d->f_n_casos = 0;
    d->next = NULL;
}

void initConc(conc *d){
    freg *f = malloc(sizeof(freg));
    d->nome = "";
    d->c_n_casos = 0;
    initFreg(f);
    d->freguesias = f;
    d->next = NULL;
}

void initDist(dist *d,char *d_nome){
    conc *c = malloc(sizeof(conc));
    d->nome = d_nome;
    d->d_n_casos = 0;
    initConc(c);
    d->concelhos = c;
}

char* tira(char *str){
    memmove(str, str+1, strlen(str));
    return str;
}

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

void addConc (dist *lista_ptr, char *n_concelho, const int n_casos)
{

    conc *antes_ptr; 
    conc *depois_ptr;
    conc *novo_ptr;
    //dist *aux = lista_ptr;
    freg *nova_freg;
    int flag=0;
    
    
    antes_ptr = lista_ptr->concelhos;
    depois_ptr =  antes_ptr->next;
    
    while (1) {
        if (depois_ptr == NULL)
            break;
        
        if (strcmp(depois_ptr->nome,n_concelho)==0){
            depois_ptr->c_n_casos += n_casos;
            flag=1;
            break;
        }
        depois_ptr = depois_ptr->next;
        antes_ptr = antes_ptr->next;
    }
    
    if (flag!=1) {
        novo_ptr = malloc(sizeof(conc));
        nova_freg = malloc(sizeof(freg));
        initFreg(nova_freg);
        //char *str;
        //sprintf(str,"%s",n_concelho);
        novo_ptr->nome = n_concelho;
        novo_ptr->c_n_casos = n_casos;
        novo_ptr->freguesias = nova_freg;
        antes_ptr->next = novo_ptr;
        novo_ptr->next = depois_ptr;
    }
}

/*
void addConc (dist *lista_ptr, char *n_concelho, const int n_casos){
    dist *aux, *ant, *actual;
    *ant = **lista_ptr;
    *actual = **lista_ptr;

    while (*actual && (strcmp(n_concelho,(*actual)->nome)!= 0)){
        *ant = *actual;
        *actual = (*actual)->next;
    }

    if (*actual && (strcmp(n_concelho,(*actual)->nome)== 0)) {
        (*actual)->c_n_casos+= n_casos;
    } else {
        *aux = malloc(sizeof(conc));
        (*aux)->nome = strdup(n_concelho);
        (*aux)->c_n_casos = n_casos;
        (*aux)->next = (*actual);
        (*aux)->freguesias = NULL;

        if ((*ant) == (*actual)){
            (**lista_ptr) = (*aux);
        } else {
            (*ant)->next = (*aux);
        }
    }
}
*/

void addFreg (dist *lista_ptr, char *n_concelho, char *n_freguesia, const int n_casos)
{
    freg *antes_ptr;
    freg *depois_ptr;
    freg *novo_ptr;
    dist *aux = lista_ptr;
    conc *aux2 = aux->concelhos;
    int flag =0;
    
    aux2 = aux->concelhos;
    while (1) {
        if (strcmp(aux2->nome, n_concelho)==0) {
            break;
        }

        aux2 = aux2->next;
    }

    
    antes_ptr = aux2->freguesias;
    depois_ptr =  antes_ptr->next;
    
    while (1) {
        if (depois_ptr == NULL)
            break;
        
        if (strcmp(depois_ptr->nome,n_freguesia)==0) {
            depois_ptr->f_n_casos += n_casos;
            flag =1;
        }
        /* Advance the pointers */
        depois_ptr = depois_ptr->next;
        antes_ptr = antes_ptr->next;
    }
    
    if (flag != 1){
        novo_ptr = malloc(sizeof(freg));
        novo_ptr->nome = n_freguesia;
        novo_ptr->f_n_casos = n_casos;
        
        antes_ptr->next = novo_ptr;
        novo_ptr->next = depois_ptr;
    }
}

int incrementar(char *nome[], unsigned valor){
    //printf("%s -> %d\n", nome[0], valor);
    filho *aux = filhos;
    int client_to_server;
    
    //TRIM ' '
    if (nome[1] && (nome[1][0] == ' ')) nome[1] = tira(nome[1]);
    if (nome[2] && (nome[2][0] == ' ')) nome[2] = tira(nome[2]);
    /*
    int i =0;
    while (((nome[0][i] >= 65) && (nome[0][i] <= 90)) || ((nome[0][i] >= 97) && (nome[0][i] <= 122)) ){
        i++;
    }
    nome[0][i] = '\0';

    i =0;
    while (((nome[1][i] >= 65) && (nome[1][i] <= 90)) || ((nome[1][i] >= 97) && (nome[1][i] <= 122)) ){
        i++;
    }
    nome[1][i] = '\0';

    i =0;
    while (((nome[2][i] >= 65) && (nome[2][i] <= 90)) || ((nome[2][i] >= 97) && (nome[2][i] <= 122)) ){
        i++;
    }
    nome[2][i] = '\0';
    */
    printf("1 - %s\n2 - %s\n3 - %s\nValor - %d\n\n",nome[0],nome[1],nome[2],valor);
    
    // Verifica se existe algum filho com o Distrito
    while (aux){
        if (strcmp(nome[0],aux->nome)==0) break;
        aux = aux->next;
    }
    
    // Existe um Filho com o Distrito
    if (aux) {
        char fifo[20];
        sprintf(fifo, "/tmp/%s",aux->nome);
        char v[100];
        int stf;
        sprintf(v,"%s:%s:%s:%d",nome[0],nome[1],nome[2],valor);
        stf = open(fifo, O_WRONLY);
        write(stf, v, sizeof(v));
        memset(v, 0, sizeof(v));
    } else {
        // Não existe Filho com o Distrito
        int pid;
        // Cria Filho
        if ((pid=fork())==0) {
            // FILHO

            int f_cts;
            int f_fdin;
            char f_buf[BUFSIZ]; // String Recebida
            
            char f_myfifo[20];
            char f_file[20];
            char *caminho[4];
            char *saveptr;

            sprintf(f_file,"/tmp/%s.txt",nome[0]); // PATH TXT
            sprintf(f_myfifo, "/tmp/%s",nome[0]); // PATH Pipe
            
            mkfifo(f_myfifo, 0666);
            
            f_fdin = open(f_file,O_CREAT |  O_TRUNC | O_WRONLY,0666); // Escreve TXT
            

            dist *d = NULL;
            d = malloc(sizeof(dist));
            initDist(d,nome[0]);
            
            char v[15];
            sprintf(v, "%d", valor);
            addConc(d, nome[1], valor);
            addFreg(d, nome[1], nome[2], valor);
            write(f_fdin, nome[0], strlen(nome[0]));
            write(f_fdin, nome[1], strlen(nome[1]));
            write(f_fdin, nome[2], strlen(nome[2]));
            write(f_fdin, v, strlen(v));
            write(f_fdin,"\n",1);
            
            f_cts = open(f_myfifo, O_RDONLY); // Lê do Pipe
            while (1)
            {
                read(f_cts, f_buf, BUFSIZ);
                if (strcmp(f_buf,"")>0){
                    caminho[0] = strtok_r(f_buf,":", &saveptr);
                    caminho[1] = strtok_r(NULL,":", &saveptr);
                    caminho[2] = strtok_r(NULL,":", &saveptr);
                    caminho[3] = strtok_r(NULL,":", &saveptr);
                    //addConc(d, caminho[1], atoi(caminho[3]));
                    //addFreg(d, caminho[1], caminho[2], atoi(caminho[3]));
                    write(f_fdin,caminho[0],strlen(caminho[0]));
                    write(f_fdin,caminho[1],strlen(caminho[1]));
                    write(f_fdin,caminho[2],strlen(caminho[2]));
                    write(f_fdin,caminho[3],strlen(caminho[3]));
                    write(f_fdin,"\n",1);
                    memset(caminho, 0, sizeof(caminho));

                    // TESTE
                    dist *aux = d;
                    int fd;
                    fd = open("/tmp/fim.txt",O_CREAT |  O_TRUNC | O_WRONLY,0666); // Escreve TXT
                    //if(aux){
                        write(fd,"D ",2);
                        write(fd,aux->nome,strlen(aux->nome));
                        conc *aux2 = aux->concelhos;
                        while(aux2){
                            write(fd,"C ",2);
                            write(fd,aux2->nome,strlen(aux2->nome));
                            freg *aux3 = aux2->freguesias;
                            while(aux3){
                                write(fd,"F ",2);
                                write(fd,aux3->nome,strlen(aux3->nome));
                                write(fd,"\n",1);
                                aux3 = aux3->next;
                            }    
                            aux2 = aux2->next;
                        }
                    //}
                    
                }
                
                memset(f_buf, 0, sizeof(f_buf));
                
            }
            close(f_cts);
            close(f_fdin);
            _exit(1);
            /*
            //addConc(d, nome[1], atoi(nome[3]));
            //addFreg(d, nome[1], nome[2], atoi(nome[3]));
            
            // Escrever estrutura para o ficheiro .txt
            
            dist *esc = d;
            char v[15];
            conc *esc2 = esc->concelhos;
            printf("DIST: %s\n", esc->nome);
            
            while(esc2){
                freg *esc3 = esc2->freguesias;
                while(esc3){
                    write(f_fdin, esc->nome, strlen(esc->nome));
                    printf("W DISTRITO: %s\n", esc->nome);
                    write(f_fdin, esc2->nome, strlen(esc2->nome));
                    printf("W CONCELHO: %s\n", esc2->nome);
                    write(f_fdin, esc3->nome, strlen(esc3->nome));
                    printf("W FREGUESIA: %s\n", esc3->nome);
                    sprintf(v, "%d", esc3->f_n_casos);
                    write(f_fdin, v, strlen(v));
                    write(f_fdin,"\n",1);
                    esc3 = esc3->next;
                }
                esc2 = esc2->next;
            }
            //write(f_cts, v, sizeof(v));
            //memset(v, 0, sizeof(v));
            //write(f_fdin,"\n",1);
            while (1)
            {
                read(f_cts, f_buf, BUFSIZ);
                if (strcmp(f_buf,"")>0){
                    //printf("MSG > %s\n", buf);
                    //write(f_fdin,f_buf,strlen(f_buf));
                    caminho[0] = strtok_r(f_buf,":", &saveptr);
                    caminho[1] = strtok_r(NULL,":", &saveptr);
                    caminho[2] = strtok_r(NULL,":", &saveptr);
                    caminho[3] = strtok_r(NULL,":", &saveptr);
                    //addConc(d, caminho[1], atoi(caminho[3]));
                    //addFreg(d, caminho[1], caminho[2], atoi(caminho[3]));
                    write(f_fdin,caminho[0],strlen(caminho[0]));
                    write(f_fdin,caminho[1],strlen(caminho[1]));
                    write(f_fdin,caminho[2],strlen(caminho[2]));
                    write(f_fdin,caminho[3],strlen(caminho[3]));
                    write(f_fdin,"\n",1);
                    memset(caminho, 0, sizeof(caminho));
                }
                memset(f_buf, 0, sizeof(f_buf));
                
            }
            close(f_cts);
            close(f_fdin);
            _exit(1);
            */
        } else {
            addFilho(filhos, nome[0], pid);
            //signal(SIGCHLD,hand2); //Control-C
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
// Segue Filho
void hand2(int s){
    if (s == SIGCHLD){
        
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