#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>

// Filho
typedef struct s_filho {
    char *nome;
    int pid;
    struct s_filho *next;
} filho;

// Freguesia
typedef struct s_freg {
    char *nome;
    int f_n_casos;
    struct s_freg *next;
} freg;

// Concelho
typedef struct s_conc {
    char *nome;
    int c_n_casos;
    struct s_freg *freguesias;
    struct s_conc *next;
} conc;

// Distrito
typedef struct s_dist {
    char *nome;
    int d_n_casos;
    struct s_conc *concelhos;
} dist;

// Estutura para Guardar os Filhos
filho *filhos = NULL;

int conta=0;

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

// SIGPIPE
void hand_pipe(int s){
    if (s == SIGPIPE){
        /*
        filho *aux = filhos;
        aux = aux->next;
        while(aux){
            if(strcmp(aux->nome,"")!=0){
                printf("\nKILL :: Filho %s com o PID %d\n",aux->nome,aux->pid);
                kill(aux->pid,9);
            }
            aux = aux->next;
        }
        */
        printf("ERRROOOOOOO SIGPIPE\n");
    }
    //exit(0);
}

// Inicializa Filho
void initFilho(filho *d){
    d->nome = "";
    d->pid = 0;
    d->next = NULL;
}

// Inicializa Freg
void initFreg(freg *d){
    d->nome = "";
    d->f_n_casos = 0;
    d->next = NULL;
}

// Inicializa Concelho
void initConc(conc *d){
    freg *f = malloc(sizeof(freg));
    d->nome = "";
    d->c_n_casos = 0;
    initFreg(f);
    d->freguesias = f;
    d->next = NULL;
}

// Inicializa Distrito
void initDist(dist *d,char *d_nome){
    conc *c = malloc(sizeof(conc));
    d->nome = d_nome;
    d->d_n_casos = 0;
    initConc(c);
    d->concelhos = c;
}

// Tira ' ' do inicio da String
char* tira(char *str){
    memmove(str, str+1, strlen(str));
    return str;
}

// Função auxiliar que retira o \n do final da string se existir
char* normaliza(char *linha){
    if ((int)linha[strlen(linha)-1] == 10) linha[strlen(linha)-1] = '\0';
    return linha;
}

// Tira Espaços
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

// Adiciona Filho
void addFilho (filho *lista_ptr, char *nome, const int pid){
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

// Adiciona Concelho
void addConc (dist *lista_ptr, char *n_concelho, const int n_casos){

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

// Adiciona Freg
void addFreg (dist *lista_ptr, char *n_concelho, char *n_freguesia, const int n_casos){
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

// Altera PID
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
                char linha_lista[BUFSIZ]; // Linha TXT
                char *caminho[10];
                char *saveptr; //strtok_r
                char path[BUFSIZ]; // TXT
                char *nome = strdup(aux->nome);
                int valor,f_cts,f_fdin;
                char f_myfifo[BUFSIZ];
                char f_buf[BUFSIZ]; // String Recebida
                char str[BUFSIZ]; // String Escrita File

                sprintf(f_myfifo, "/tmp/%s",nome); // PATH Pipe

                mkfifo(f_myfifo, 0666);
                dist *d = NULL;
                d = malloc(sizeof(dist));
                initDist(d,nome);
                sprintf(path, "/tmp/%s.txt",nome);
                FILE *file_lista= fopen(path, "r");
                while (fgets(linha_lista, BUFSIZ, file_lista)){
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
                        
                        if(strcmp(caminho[0],"agregar")==0){
                            dist *aux = d;
                            int n_path;
                            int nivel;
                            char *path2;

                            if (caminho[3]){
                                nivel = atoi(caminho[2]);
                                path2 = strdup(caminho[3]);
                            }
                            if (caminho[4]) {
                                nivel = atoi(caminho[3]);
                                path2 = strdup(caminho[4]);
                            }

                            if (caminho[5]){
                                nivel = atoi(caminho[4]);
                                path2 = strdup(caminho[5]);
                            }

                            path2=ajudasemPrint(path2);

                            n_path = open(path2,O_CREAT | O_TRUNC | O_WRONLY ,0666);

                            conc *aux2 = aux->concelhos;
                            if (strcmp(aux->nome,"") != 0) {
                                if (nivel == 0){
                                    char str[BUFSIZ];
                                    sprintf(str,"%s:%d\n",aux->nome,aux->d_n_casos);
                                  //  printf("MSG|%s|\n", str);
                                    write(n_path,str,strlen(str));
                                }
                            }

                            while (aux2){
                                if (strcmp(aux2->nome,"") != 0) {
                                    if (nivel == 1){
                                        char str[BUFSIZ];
                                        sprintf(str,"%s:%s:%d\n",aux->nome,aux2->nome,aux2->c_n_casos);
                                        write(n_path,str,strlen(str));
                                    }
                                }
                                freg *aux3 = aux2->freguesias;
                                while (aux3) {
                                    if (strcmp(aux3->nome,"") != 0) {
                                        if ((nivel == 2) && ((strcmp(aux2->nome,caminho[2])==0) || (atoi(caminho[2]) !=0))) {
                                            char str[BUFSIZ];
                                            sprintf(str,"%s:%s:%s:%d\n",aux->nome,aux2->nome,aux3->nome,aux3->f_n_casos);
                                            write(n_path,str,strlen(str));
                                        }
                                    }
                                    aux3 = aux3->next;
                                }
                                aux2 = aux2->next;
                            }
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
                            sprintf(str,"%s:%s:%s:%s\n",caminho[0],caminho[1],caminho[2],caminho[3]);
                            write(f_fdin,str,strlen(str));
                            memset(str, 0, sizeof(str));
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

// Incrementa
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
        char fifo[BUFSIZ];
        sprintf(fifo, "/tmp/%s",aux->nome);
        char v[BUFSIZ];
        int stf;
        sprintf(v,"%s:%s:%s:%d",nome[0],nome[1],nome[2],valor);
        //printf("|%s|\n", v);
        stf = open(fifo, O_WRONLY);
        write(stf, v, sizeof(v));
        memset(v, 0, sizeof(v));
        close(stf);

    } else {
        // Não existe Filho com o Distrito
        int pid;
        // Cria Filho
        if ((pid=fork())==0) {
            // FILHO
            int f_cts;
            int f_fdin;
            char f_buf[BUFSIZ]; // String Recebida
            
            char f_myfifo[BUFSIZ];
            char f_file[BUFSIZ];
            char str[BUFSIZ];
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
            sprintf(str,"%s:%s:%s:%d\n",nome[0],nome[1],nome[2],valor);
            write(f_fdin,str,strlen(str));
            memset(str, 0, sizeof(str));
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

                        n_path = open(path,O_CREAT | O_TRUNC | O_WRONLY ,0666);
                        
                        conc *aux2 = aux->concelhos;
                        if (strcmp(aux->nome,"") != 0) {
                           if (nivel == 0){
                                char str[BUFSIZ];
                                sprintf(str,"%s:%d\n",aux->nome,aux->d_n_casos);
                                  //  printf("MSG|%s|\n", str);
                                write(n_path,str,strlen(str));
                            }
                        }
                        while (aux2){
                            if (strcmp(aux2->nome,"") != 0) {
                                if (nivel == 1){
                                    char str[BUFSIZ];
                                    sprintf(str,"%s:%s:%d\n",aux->nome,aux2->nome,aux2->c_n_casos);
                                  //  printf("MSG|%s|\n", str);
                                    write(n_path,str,strlen(str));
                                }
                            }
                            freg *aux3 = aux2->freguesias;
                            while (aux3) {
                                if (strcmp(aux3->nome,"") != 0) {
                                   if ((nivel == 2) && ((strcmp(aux2->nome,caminho[2])==0) || (atoi(caminho[2]) !=0))) {
                                        char str[BUFSIZ];
                                        sprintf(str,"%s:%s:%s:%d\n",aux->nome,aux2->nome,aux3->nome,aux3->f_n_casos);
                                        write(n_path,str,strlen(str));
                                    }
                                }
                                aux3 = aux3->next;
                            }
                            aux2 = aux2->next;
                        }
                        //printf("\n");
                        printf("%d FIM AGREGAR\n",conta);
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
                        sprintf(str,"%s:%s:%s:%s\n",caminho[0],caminho[1],caminho[2],caminho[3]);
                        write(f_fdin,str,strlen(str));
                        memset(str, 0, sizeof(str));
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

// Agrega
int agregar(char *prefixo[], unsigned nivel, char *path){
    int n_mtf;
    char mtf[BUFSIZ];
    char msg[BUFSIZ];
    sprintf(mtf, "/tmp/%s",prefixo[1]); // PATH Pipe
    n_mtf = open(mtf, O_WRONLY);
    sprintf(msg,"%s:%s:%d:%s",prefixo[0],prefixo[1],(int)nivel,path);
    if (prefixo[2]){
        if (prefixo[3]) {
            sprintf(msg,"%s:%s:%s:%s:%d:%s",prefixo[0],prefixo[1],prefixo[2],prefixo[3],(int)nivel,path);
        } else {
            sprintf(msg,"%s:%s:%s:%d:%s",prefixo[0],prefixo[1],prefixo[2],(int)nivel,path);
        }
    }
    write(n_mtf, msg, BUFSIZ);
    memset(msg, 0, sizeof(msg));
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

// Envia Mensagem ao Filho
void mensagem (char *filho, char *msg){
    int n_mtf;
    char mtf[BUFSIZ];
    sprintf(mtf, "/tmp/%s",filho); // PATH Pipe
    n_mtf = open(mtf, O_WRONLY);
    write(n_mtf, msg, sizeof(msg));
    close(n_mtf);
}

int main(int argc, char const *argv[]){
    int cts;
    char *myfifo = "/tmp/cts";
    char buf[BUFSIZ];
    char *t,*r;
    
    filhos = malloc(sizeof(filho));
    
    initFilho(filhos); //Inicializar
    
    signal(SIGINT,hand_int); //Control-C
    signal(SIGPIPE,hand_pipe); //Control-C
    mkfifo(myfifo, 0666); // Pipe Cliente to Server
    
    cts = open(myfifo, O_RDONLY);
    while (1)
    {
        read(cts, buf, BUFSIZ);
        if (strcmp(buf,"")>0){
            normaliza(buf);
            //printf("SERVER: |%s|\n", buf);
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
                    conta++;
                    //printf("Adicionado com Sucesso!! %d\n",conta);
                } else {
                    printf("Erro ao Adicionar!!\n");
                }
                
            } else if (strcmp(buf,"filhos") == 0) {
                filho *aux = filhos;
                while (aux){
                    if (strcmp(aux->nome,"")!=0){
                        printf("DISTRITO: %s\n",aux->nome);
                        printf("PID: %d",aux->pid);
                        printf("\n");
                    }
                    aux = aux->next;
                }
            } else if ((buf[0] == 'l') && (buf[1] == 'i')) {
                t = strtok_r(buf," ",&r);
                if (r && t && strcmp(t,"lista") == 0) {
                    mensagem(r,"lista");
                } else {                
                filho *aux = filhos;
                while (aux){
                    if (strcmp(aux->nome,"")!=0){
                        mensagem(aux->nome,"lista");
                    }
                    aux = aux->next;
                }
                }
            } else if ((buf[0] == 'a') && (buf[1] == 'g')) {
            t = strtok_r(buf," ",&r);
            if (r && t && strcmp(t,"agregar") == 0) {
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
                nivel = atoi(valor);
                agregar(agrega,nivel,path);
            }
        } else {
            printf("COMANDO ERRADO!!\n");
        }

        }
        memset(buf, 0, sizeof(buf));
    }
    
    close(cts);
    
    return 0;
}