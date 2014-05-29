#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include "server.h"

char * ajudasemPrint(char* nome){
    int i=0;
    while(nome[i]!= '.')
        i++;
    i++;
    while(((nome[i]>= 65)&&(nome[i]<=90)) || ((nome[i]>= 97)&&(nome[i]<=122)) )
        i++;
    nome[i]='\0';
    return  nome;
}

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

// Função auxiliar que retira o \n do final da string se existir

char* normaliza(char *linha) 
{
    if ((int)linha[strlen(linha)-1] == 10) linha[strlen(linha)-1] = '\0';
    return linha;
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
    
    lista_ptr->d_n_casos += n_casos;
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
        
        novo_ptr->nome = strdup(n_concelho);
        novo_ptr->c_n_casos = n_casos;
        novo_ptr->freguesias = nova_freg;
        antes_ptr->next = novo_ptr;
        novo_ptr->next = depois_ptr;
    }
}

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
            break;
        }
        /* Advance the pointers */
        depois_ptr = depois_ptr->next;
        antes_ptr = antes_ptr->next;
    }
    
    if (flag != 1){
        novo_ptr = malloc(sizeof(freg));
        novo_ptr->nome = strdup(n_freguesia);
        novo_ptr->f_n_casos = n_casos;
        
        antes_ptr->next = novo_ptr;
        novo_ptr->next = depois_ptr;
    }
}

void alterapid(char *dist, int pid){
    filho *aux = filhos;
    aux = aux->next;
    while (aux){
        if (strcmp(aux->nome,dist)==0){
            aux->pid = pid;
            break;
        }
        aux = aux->next;
    }
}

// Segue Filho
void hand_chld(int s){
    if (s == SIGCHLD){
        pid_t pid;
        pid = wait(NULL);
        filho *aux = filhos;
        int flag =0;

        aux = aux->next;
        while (aux){
            if (aux->pid == pid){
                printf("O Filho %s com o pid %d Crashou!\n", aux->nome,aux->pid);
                flag =1;
                break;
            }
            aux = aux->next;
        }

        if (flag!=0){
            // CRIAR FILHO DE NOVO
            if ((pid=fork())==0){
                printf("O Filho %s com o pid %d Reiniciou!\n", aux->nome,getpid());
                char linha_lista[100]; // Linha TXT
                char *caminho[10];
                char *saveptr;
                char *path; // TXT
                char *nome = strdup(aux->nome);
                int valor,f_cts,f_fdin;
                char f_myfifo[20];
                char f_buf[BUFSIZ]; // String Recebida

                sprintf(f_myfifo, "/tmp/%s",nome); // PATH Pipe

                mkfifo(f_myfifo, 0666);
                dist *d = NULL;
                d = malloc(sizeof(dist));
                initDist(d,nome);
                
                sprintf(path, "/tmp/%s.txt",nome);
                FILE *file_lista= fopen(path, "r");
                while (fgets(linha_lista, 100, file_lista)){
                    normaliza(linha_lista);
                    caminho[0] = strtok_r(linha_lista,":", &saveptr);
                    caminho[1] = strtok_r(NULL,":", &saveptr);
                    caminho[2] = strtok_r(NULL,":", &saveptr);
                    caminho[3] = strtok_r(NULL,":", &saveptr);
                    valor = atoi(caminho[3]);
                    addConc(d, caminho[1], valor);
                    addFreg(d, caminho[1], caminho[2], valor);
                }
                fclose(file_lista);

                f_fdin = open(path,O_RDWR | O_APPEND,0666); // Escreve TXT
                f_cts = open(f_myfifo, O_RDONLY); // Lê do Pipe
                
                while (1) {
                    read(f_cts, f_buf, BUFSIZ);
                    if (strcmp(f_buf,"")>0){
                        normaliza(f_buf);
                        caminho[0] = strtok_r(f_buf,":", &saveptr);
                        caminho[1] = strtok_r(NULL,":", &saveptr);
                        caminho[2] = strtok_r(NULL,":", &saveptr);
                        caminho[3] = strtok_r(NULL,":", &saveptr);
                        caminho[4] = strtok_r(NULL,":", &saveptr);
                        caminho[5] = strtok_r(NULL,":", &saveptr);
                    //printf("0-|%s| 1-|%s| 2-|%s| 3-|%s| 4-|%s| 5-|%s| \n", caminho[0], caminho[1], caminho[2],caminho[3],caminho[4],caminho[5]);
                        if(strcmp(caminho[0],"agregar")==0){
                            dist *aux = d;
                            int n_path;
                            int nivel;
                            char *path;
                        //printf("ANTES DE\n");

                            if (caminho[3]){
                                nivel = atoi(caminho[2]);
                                path = strdup(caminho[3]);
                            }
                            if (caminho[4]) {
                                nivel = atoi(caminho[3]);
                                path = strdup(caminho[4]);
                            }

                            if (caminho[5]){
                                nivel = atoi(caminho[4]);
                                path = strdup(caminho[5]);
                            }

                            path=ajudasemPrint(path);
                        //printf("CHEGUEI!!! PATH: %s\n",path);

                            n_path = open(path,O_CREAT | O_TRUNC | O_WRONLY ,0666);
                        //printf("OPEEEENNNNN\n");
                            conc *aux2 = aux->concelhos;
                            if (strcmp(aux->nome,"") != 0) {
                           // printf("NOME DISTRITO %s\n",aux->nome);
                            //printf("Nº Casos Distrito %d\n",aux->d_n_casos);
                                if (nivel == 0){
                                    char str[80];
                                    sprintf(str,"%s:%d\n",aux->nome,aux->d_n_casos);
                                  //  printf("MSG|%s|\n", str);
                                    write(n_path,str,strlen(str));
                                }
                            }
                            while (aux2){
                                if (strcmp(aux2->nome,"") != 0) {
                                //printf("\tNOME CONCELHO %s\n",aux2->nome);
                                //printf("\tNº Casos Concelho %d\n",aux2->c_n_casos);
                                //printf("NIVEL: %d\n", nivel);
                                    if (nivel == 1){
                                        char str[80];
                                        sprintf(str,"%s:%s:%d\n",aux->nome,aux2->nome,aux2->c_n_casos);
                                  //  printf("MSG|%s|\n", str);
                                        write(n_path,str,strlen(str));
                                    }
                                }
                                freg *aux3 = aux2->freguesias;
                                while (aux3) {
                                    if (strcmp(aux3->nome,"") != 0) {
                                   // printf("\t\tNOME FREGUESIA %s\n",aux3->nome);
                                   // printf("\t\tNº Casos Freguesia %d\n",aux3->f_n_casos);
                                        if ((nivel == 2) && ((strcmp(aux2->nome,caminho[2])==0) || (atoi(caminho[2]) !=0))) {
                                            char str[80];
                                            sprintf(str,"%s:%s:%s:%d\n",aux->nome,aux2->nome,aux3->nome,aux3->f_n_casos);
                                            write(n_path,str,strlen(str));
                                        }
                                    }
                                    aux3 = aux3->next;
                                }
                                aux2 = aux2->next;
                            }
                        //printf("\n");
                            close(n_path);

                        } else if (strcmp(caminho[0],"lista")==0){
                            dist *aux = d;

                            conc *aux2 = aux->concelhos;
                            if (strcmp(aux->nome,"") != 0) {
                                printf("NOME DISTRITO %s\n",aux->nome);
                                printf("Nº Casos Distrito %d\n",aux->d_n_casos);
                            }
                            while (aux2){
                                if (strcmp(aux2->nome,"") != 0) {
                                    printf("\tNOME CONCELHO %s\n",aux2->nome);
                                    printf("\tNº Casos Concelho %d\n",aux2->c_n_casos);
                                }
                                freg *aux3 = aux2->freguesias;
                                while (aux3) {
                                    if (strcmp(aux3->nome,"") != 0) {
                                        printf("\t\tNOME FREGUESIA %s\n",aux3->nome);
                                        printf("\t\tNº Casos Freguesia %d\n",aux3->f_n_casos);
                                    }
                                    aux3 = aux3->next;
                                }
                                aux2 = aux2->next;
                            }
                            printf("\n");
                        } else {
                            addConc(d, caminho[1], atoi(caminho[3]));
                            addFreg(d, caminho[1], caminho[2], atoi(caminho[3]));
                            write(f_fdin,caminho[0],strlen(caminho[0]));
                            write(f_fdin,":",1);
                            write(f_fdin,caminho[1],strlen(caminho[1]));
                            write(f_fdin,":",1);
                            write(f_fdin,caminho[2],strlen(caminho[2]));
                            write(f_fdin,":",1);
                            write(f_fdin,caminho[3],strlen(caminho[3]));
                            write(f_fdin,"\n",1);
                            memset(caminho, 0, sizeof(caminho));
                        } 
                    }

                    memset(f_buf, 0, sizeof(f_buf));

                }
                close(f_cts);
                close(f_fdin);
                _exit(1);

            } else {
                alterapid(aux->nome, pid);
            }
        }
    }
}

int incrementar(char *nome[], unsigned valor){
    filho *aux = filhos;
    int client_to_server;
    
    //TRIM ' '
    if (nome[1] && (nome[1][0] == ' ')) nome[1] = tira(nome[1]);
    if (nome[2] && (nome[2][0] == ' ')) nome[2] = tira(nome[2]);
    
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
           // printf("Init Child: %s\n",nome[0]);
            int f_cts;
            int f_fdin;
            char f_buf[BUFSIZ]; // String Recebida
            
            char f_myfifo[20];
            char f_file[20];
            char *caminho[10];
            char *saveptr;

            sprintf(f_file,"/tmp/%s.txt",nome[0]); // PATH TXT
            sprintf(f_myfifo, "/tmp/%s",nome[0]); // PATH Pipe
            
            mkfifo(f_myfifo, 0666);
            
            f_fdin = open(f_file,O_CREAT |  O_TRUNC | O_WRONLY | O_APPEND,0666); // Escreve TXT

            dist *d = NULL;
            d = malloc(sizeof(dist));
            initDist(d,nome[0]);
            
            char v[15];
            sprintf(v, "%d", valor);
            addConc(d, nome[1], valor);
            addFreg(d, nome[1], nome[2], valor);
            write(f_fdin, nome[0], strlen(nome[0]));
            write(f_fdin,":",1);
            write(f_fdin, nome[1], strlen(nome[1]));
            write(f_fdin,":",1);
            write(f_fdin, nome[2], strlen(nome[2]));
            write(f_fdin,":",1);
            write(f_fdin, v, strlen(v));
            write(f_fdin,"\n",1);
            
            f_cts = open(f_myfifo, O_RDONLY); // Lê do Pipe
            while (1)
            {
                read(f_cts, f_buf, BUFSIZ);
                if (strcmp(f_buf,"")>0){
                    normaliza(f_buf);
                    caminho[0] = strtok_r(f_buf,":", &saveptr);
                    caminho[1] = strtok_r(NULL,":", &saveptr);
                    caminho[2] = strtok_r(NULL,":", &saveptr);
                    caminho[3] = strtok_r(NULL,":", &saveptr);
                    caminho[4] = strtok_r(NULL,":", &saveptr);
                    caminho[5] = strtok_r(NULL,":", &saveptr);
                    //printf("0-|%s| 1-|%s| 2-|%s| 3-|%s| 4-|%s| 5-|%s| \n", caminho[0], caminho[1], caminho[2],caminho[3],caminho[4],caminho[5]);
                    if(strcmp(caminho[0],"agregar")==0){
                        dist *aux = d;
                        int n_path;
                        int nivel;
                        char *path;
                        //printf("ANTES DE\n");
                        
                        if (caminho[3]){
                            nivel = atoi(caminho[2]);
                            path = strdup(caminho[3]);
                        }
                        if (caminho[4]) {
                            nivel = atoi(caminho[3]);
                            path = strdup(caminho[4]);
                        }

                        if (caminho[5]){
                            nivel = atoi(caminho[4]);
                            path = strdup(caminho[5]);
                        }
                        
                        path=ajudasemPrint(path);
                        //printf("CHEGUEI!!! PATH: %s\n",path);

                        n_path = open(path,O_CREAT | O_TRUNC | O_WRONLY ,0666);
                        //printf("OPEEEENNNNN\n");
                        conc *aux2 = aux->concelhos;
                        if (strcmp(aux->nome,"") != 0) {
                           // printf("NOME DISTRITO %s\n",aux->nome);
                            //printf("Nº Casos Distrito %d\n",aux->d_n_casos);
                            if (nivel == 0){
                                char str[80];
                                sprintf(str,"%s:%d\n",aux->nome,aux->d_n_casos);
                                  //  printf("MSG|%s|\n", str);
                                write(n_path,str,strlen(str));
                            }
                        }
                        while (aux2){
                            if (strcmp(aux2->nome,"") != 0) {
                                //printf("\tNOME CONCELHO %s\n",aux2->nome);
                                //printf("\tNº Casos Concelho %d\n",aux2->c_n_casos);
                                //printf("NIVEL: %d\n", nivel);
                                if (nivel == 1){
                                    char str[80];
                                    sprintf(str,"%s:%s:%d\n",aux->nome,aux2->nome,aux2->c_n_casos);
                                  //  printf("MSG|%s|\n", str);
                                    write(n_path,str,strlen(str));
                                }
                            }
                            freg *aux3 = aux2->freguesias;
                            while (aux3) {
                                if (strcmp(aux3->nome,"") != 0) {
                                   // printf("\t\tNOME FREGUESIA %s\n",aux3->nome);
                                   // printf("\t\tNº Casos Freguesia %d\n",aux3->f_n_casos);
                                    if ((nivel == 2) && ((strcmp(aux2->nome,caminho[2])==0) || (atoi(caminho[2]) !=0))) {
                                        char str[80];
                                        sprintf(str,"%s:%s:%s:%d\n",aux->nome,aux2->nome,aux3->nome,aux3->f_n_casos);
                                        write(n_path,str,strlen(str));
                                    }
                                }
                                aux3 = aux3->next;
                            }
                            aux2 = aux2->next;
                        }
                        //printf("\n");
                        close(n_path);

                    } else if (strcmp(caminho[0],"lista")==0){
                        dist *aux = d;

                        conc *aux2 = aux->concelhos;
                        if (strcmp(aux->nome,"") != 0) {
                            printf("NOME DISTRITO %s\n",aux->nome);
                            printf("Nº Casos Distrito %d\n",aux->d_n_casos);
                        }
                        while (aux2){
                            if (strcmp(aux2->nome,"") != 0) {
                                printf("\tNOME CONCELHO %s\n",aux2->nome);
                                printf("\tNº Casos Concelho %d\n",aux2->c_n_casos);
                            }
                            freg *aux3 = aux2->freguesias;
                            while (aux3) {
                                if (strcmp(aux3->nome,"") != 0) {
                                    printf("\t\tNOME FREGUESIA %s\n",aux3->nome);
                                    printf("\t\tNº Casos Freguesia %d\n",aux3->f_n_casos);
                                }
                                aux3 = aux3->next;
                            }
                            aux2 = aux2->next;
                        }
                        printf("\n");
                    } else {
                        addConc(d, caminho[1], atoi(caminho[3]));
                        addFreg(d, caminho[1], caminho[2], atoi(caminho[3]));
                        write(f_fdin,caminho[0],strlen(caminho[0]));
                        write(f_fdin,":",1);
                        write(f_fdin,caminho[1],strlen(caminho[1]));
                        write(f_fdin,":",1);
                        write(f_fdin,caminho[2],strlen(caminho[2]));
                        write(f_fdin,":",1);
                        write(f_fdin,caminho[3],strlen(caminho[3]));
                        write(f_fdin,"\n",1);
                        memset(caminho, 0, sizeof(caminho));
                    } 
                }
                
                memset(f_buf, 0, sizeof(f_buf));
                
            }
            close(f_cts);
            close(f_fdin);
            _exit(1);

        } else {
            addFilho(filhos, nome[0], pid);
            signal(SIGCHLD,hand_chld); //Child Follow
        }
    }
    return 0;
}

int agregar(char *prefixo[], unsigned nivel, char *path){
    int n_mtf;
    char mtf[20];
    char msg[60];
    sprintf(mtf, "/tmp/%s",prefixo[1]); // PATH Pipe
    n_mtf = open(mtf, O_WRONLY);
    printf("TOPPPPP\n");
    sprintf(msg,"%s:%s:%d:%s",prefixo[0],prefixo[1],(int)nivel,path);
    printf("AGREGAR ANTES IF\n");
    if (prefixo[2]){
        if (prefixo[3]) {
            sprintf(msg,"%s:%s:%s:%s:%d:%s",prefixo[0],prefixo[1],prefixo[2],prefixo[3],(int)nivel,path);
        } else {
            sprintf(msg,"%s:%s:%s:%d:%s",prefixo[0],prefixo[1],prefixo[2],(int)nivel,path);
        }
    }
    printf("AGREGAR FIM IF\n");
    write(n_mtf, msg, strlen(msg));
    close(n_mtf);
    return 0;
}

// Control-C
void hand_int(int s){
    if (s == SIGINT){
        filho *aux = filhos;
        aux = aux->next;
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

void mensagem (char *filho, char *msg){
    int n_mtf;
    char mtf[20];
    sprintf(mtf, "/tmp/%s",filho); // PATH Pipe
    n_mtf = open(mtf, O_WRONLY);
    write(n_mtf, msg, sizeof(msg));
    close(n_mtf);
}

int main(int argc, char const *argv[])
{
    int cts;
    char *myfifo = "/tmp/cts";
    char buf[BUFSIZ];
    int fdin;
    char *t,*r;
    
    filhos = malloc(sizeof(filho));
    
    initFilho(filhos); //Inicializar
    
    signal(SIGINT,hand_int); //Control-C
    mkfifo(myfifo, 0666); // Pipe Cliente to Server
    
    fdin = open("log.txt",O_APPEND | O_TRUNC | O_WRONLY,0666);
    
    cts = open(myfifo, O_RDONLY);
    while (1)
    {
        read(cts, buf, BUFSIZ);
        if (strcmp(buf,"")>0){
            normaliza(buf);
            if(buf[0]=='['){
                tira(buf);
                char *saveptr;
                char *c = strdup(buf);
                char *caminho_s = strtok_r(c, "]", &saveptr);
                char *valor = strtok_r(NULL, ",", &saveptr);
                
                char *caminho[3];
                caminho[0] = strtok_r(caminho_s,",", &caminho[1]);
                caminho[1] = strtok_r(caminho[1],",", &caminho[2]);
                
                if (incrementar(caminho,atoi(valor))==0){
                    printf("Adicionado com Sucesso!!\n");
                } else {
                    printf("Erro ao Adicionar!!\n");
                }
                
            } 
            if (strcmp(buf,"filhos") == 0) {
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
            if (strcmp(buf,"lista") == 0) {
                filho *aux = filhos;
                while (aux){
                    if (strcmp(aux->nome,"")!=0){
                        mensagem(aux->nome,"lista");
                    }
                    aux = aux->next;
                }
            }
            t = strtok_r(buf," ",&r);
            if (strcmp(t,"agregar") == 0) {
                char *agrega[20];
                char *valor,*aux,*path;
                char *resto;
                int nivel;
                agrega[0] = buf;
                aux = strtok_r(NULL,"]",&r);
                resto = strdup(r);
                tira(aux);
                agrega[1] = strtok_r(aux,",",&r);
                agrega[2] = strtok_r(NULL,",",&r);
                agrega[3] = strtok_r(NULL,",",&r);
                valor = strtok_r(resto," ",&r);
                path = strdup(r);
                //TRIM ' '
                if (agrega[2] && (agrega[2][0] == ' ')) agrega[2] = tira(agrega[2]);
                if (agrega[3] && (agrega[3][0] == ' ')) agrega[3] = tira(agrega[3]);
                //tira(valor);
                nivel = atoi(valor);
                //printf("AGREGARRRRRRR 0-|%s| 1-|%s| 2-|%s| 3-|%s| 4-|%s| Valor=%d\n",agrega[0],agrega[1],agrega[2],agrega[3],agrega[4],nivel);
                agregar(agrega,nivel,path);
            }

        }
        memset(buf, 0, sizeof(buf));
    }
    
    close(cts);
    close(fdin);
    
    return 0;
}