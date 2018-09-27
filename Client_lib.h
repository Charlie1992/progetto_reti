#include <stdio.h>  
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define localhost "127.0.0.1"
#define client_cup_port 1024

// struttura dati memorizzare le possibili visite 
typedef struct lista_tipologia_visite{
    int id_tv;
    char nome_tv[50];
}LISTA_TIPOLOGIA_VISITE;

// struttura per memorizzare i dati delle prenotazioni
typedef struct prenotazione{
    char    cod_ricetta[10];
    char    nome[15];
    char    cognome[15];
    char    data_visita[10];
}PRENOTAZIONE;

//funzione per contare il numero totale delle tipologie delle visite disponibili
int conta_ltv(LISTA_TIPOLOGIA_VISITE lista_tipologia_visite[]){
    int count = 1;
    while(lista_tipologia_visite[count].id_tv!=0){
        count++;
    }
    return count++;
}


//------------ FUNZIONI WRAPPER-------------------

// crea il socket per la comunicazione
int SOCKET(int family,int type,int protocol){
    if((socket(family,type,protocol))<0){
        perror("socket");
        exit(-1);
    }
    return socket(family,type,protocol<0);
}


// converte la stringa passata come secondo argomento 
// in un indirizzo di rete scritto in network order e
// lo memorizza nella locazione di memoria puntata dal 
// terzo argomento
void INET_PTON(int family,char *ip,struct sockaddr_in *servaddr){
    if(inet_pton(family,ip,&servaddr->sin_addr)<0){
        perror("inet_pton");
        exit(-1);
    }
}

// Connette il socket sockfd all'indirizzo serv_addr
void CONNECTION(int socket,struct sockaddr_in servaddr,int size){
    if(connect(socket,(struct sockaddr *) &servaddr,size)<0){
        perror("connection");
        exit(-1);
    }
}

ssize_t FullWrite(int fd, const void *buf, size_t count)
{
    size_t nleft;
    ssize_t nwritten;
    nleft = count;
    while (nleft > 0) {             /* repeat until no left */
        if((nwritten=write(fd,buf,nleft))<0){
            if(errno==EINTR){
                /*se interrotto da un SYSTEM CALL ripeti il ciclo*/
                continue;
            }else{
                exit(nwritten);    /*altrimenti esci con un errore*/
            }
        }
        nleft-=nwritten;/*deincrementa la variabile */
        buf+=nwritten;/*incrementa il buffer*/
    }
    return (nleft);
}

ssize_t FullRead(int fd, void *buf, size_t count)
{
    size_t nleft;
    ssize_t nread;
    nleft = count;
    while (nleft > 0) {             /* repeat until no left */
        if((nread=read(fd,buf,nleft))<0){
            if(errno==EINTR){
                /*se interrotto da un SYSTEM CALL ripeti il ciclo*/
                continue;
            }else{
                exit(nread); /*altrimenti esci con un errore*/
            }
        }else if(nread==0){
            break;    /*interrompi il ciclo poichè il socket è vuoto ed è                         stato chiuso */
        }
        nleft-=nread; /*deincrementa la variabile */
        buf +=nread; /*incrementa il buffer*/
    }
    buf = 0;
    return (nleft);
}
