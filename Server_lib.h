#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/wait.h>

#define localhost "127.0.0.1"
#define client_cup_port 1024
#define cup_server_reparto1_port 1025
#define cup_server_reparto2_port 1027
#define medico1_reparto1_port 1030
#define medico2_reparto2_port 1031

char    lista_date[200][11];

// struttura per memorizzare i dati delle prenotazioni
typedef struct prenotazione{
    char    cod_ricetta[15];
    char    nome[15];
    char    cognome[15];
    char    data_visita[11];
    char    nome_visita_scelta[30];
}PRENOTAZIONE;

// struttura dati memorizzare le possibili visite 
typedef struct lista_tipologia_visite{
    int id_tv;
    char nome_tv[50];
}LISTA_TIPOLOGIA_VISITE;


void ListaNomiVisite(LISTA_TIPOLOGIA_VISITE nome_visite[]){
        nome_visite[1].id_tv = 1 ; 
        strcpy(nome_visite[1].nome_tv,"Elettrocardiogramma");
        nome_visite[2].id_tv  = 2;
        strcpy(nome_visite[2].nome_tv ,"Ecocardiogramma");
        nome_visite[3].id_tv = 3;
        strcpy(nome_visite[3].nome_tv,"Holter_ECG");
        nome_visite[4].id_tv = 4;
        strcpy(nome_visite[4].nome_tv,"Holter_Cardiaco");
        nome_visite[5].id_tv = 5;
        strcpy(nome_visite[5].nome_tv,"Esame_Del_Sangue");
        nome_visite[6].id_tv = 6;
        strcpy(nome_visite[6].nome_tv,"Vaccinazioni");
        nome_visite[7].id_tv = 7;
        strcpy(nome_visite[7].nome_tv,"Emocroma");
        nome_visite[8].id_tv = 8;
        strcpy(nome_visite[8].nome_tv,"Consulenza_Alimentare");
        nome_visite[9].id_tv = -1;
        strcpy(nome_visite[9].nome_tv,"-1");
}

void riempi_lista_data(char lista_date[][11]){
    strcpy(lista_date[1],"01/10/2018");
    strcpy(lista_date[2],"02/10/2018");
    strcpy(lista_date[3],"03/10/2018");
    strcpy(lista_date[4],"04/10/2018");
    strcpy(lista_date[5],"05/10/2018");
    strcpy(lista_date[6],"06/10/2018");
    strcpy(lista_date[7],"07/10/2018");
    strcpy(lista_date[8],"08/10/2018");
    strcpy(lista_date[9],"09/10/2018");
    strcpy(lista_date[10],"10/10/2018");
    strcpy(lista_date[11],"11/10/2018");
    strcpy(lista_date[12],"12/10/2018");
    strcpy(lista_date[13],"13/10/2018");
    strcpy(lista_date[14],"14/10/2018");
    strcpy(lista_date[15],"15/10/2018");
    strcpy(lista_date[16],"16/10/2018");
    strcpy(lista_date[17],"17/10/2018");
    strcpy(lista_date[18],"18/10/2018");
    strcpy(lista_date[19],"19/10/2018");
    strcpy(lista_date[20],"20/10/2018");
    strcpy(lista_date[21],"21/10/2018");
    strcpy(lista_date[22],"22/10/2018");
    strcpy(lista_date[23],"23/10/2018");
    strcpy(lista_date[24],"24/10/2018");
    strcpy(lista_date[25],"25/10/2018");
    strcpy(lista_date[26],"26/10/2018");
    strcpy(lista_date[27],"27/10/2018");
    strcpy(lista_date[28],"28/10/2018");
    strcpy(lista_date[29],"29/10/2018");
    strcpy(lista_date[30],"30/10/2018");
    strcpy(lista_date[31],"-1");
}


//conta date 
int conta_date(char lista_data[][11]){
    int count = 1;
    while(strcmp(lista_data[count],"-1")!=0){
        count++;
    }
    return count;
}

//controllo codice prenotazione
int controllo(char codice[]){
	int x = 0;
	int i = 0;
	char buff;
	
	for(i =1;i<4;i++){
		buff='0'+i;
		if(codice[0]=='R' && codice[1]==buff){
			x=1;
			return x;			
		}
	}
	return x;
}

// crea il socket per la comunicazione 
int SOCKET(int family,int type,int protocol){
    if((socket(family,type,protocol))<0){
        perror("socket");
        exit(-1);
    }
    return socket(family,type,protocol<0);
}

// la funzione,asseggna l'indirizzo alla socket,
// ha come parametri:
// listenfd = socket creata
// &servaddr = indirizzo d'assegnare alla socket
// sizeof(servaddr) = dimensione in byte della struttura
void BIND(int socket,struct sockaddr_in servaddr){
    if((bind(socket,(struct sockaddr *) &servaddr,sizeof(servaddr)))<0){
        perror("bind");
        exit(-1);
    }
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

// Mette il socket in modalita' di ascolto in
// attesa di nuove connessioni
// Il secondo argmoneto specifica quante connessioni 
// possono essere in attesa di essere accettate
void LISTEN(int socket,int n){
    if((listen(socket,n))<0){
        perror("listen");
        exit(-1);
    }
}

// La funzione, accetta una nuova connessione, ha come parametri:
// listenfd = socket creata
// 2° o il 3° parametro servono ad identificare il client
int ACCEPT(int listenfd, struct sockaddr *client, socklen_t *serveraddr_length){
    int conn_fd;
    if ( ( conn_fd = accept(listenfd, client, serveraddr_length) ) < 0 ) {
        perror("accept");
        exit(1);
    }
    return conn_fd;
}


int ClientLink(struct sockaddr_in servaddr,int port,char *ip_address,int val){
        int socket;

        socket = SOCKET(AF_INET, SOCK_STREAM, 0);
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        if(val==1){
            INET_PTON(AF_INET,localhost,&servaddr);   
        }else{
            INET_PTON(AF_INET,ip_address,&servaddr);
        }
        CONNECTION(socket,servaddr,sizeof(servaddr));
        
        return socket;

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
				exit(nwritten);	/*altrimenti esci con un errore*/
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
			break;	/*interrompi il ciclo poichè il socket è vuoto ed è 						stato chiuso */
		}
		nleft-=nread; /*deincrementa la variabile */
		buf +=nread; /*incrementa il buffer*/
	}	
	buf = 0; 
    return (nleft); 
}
