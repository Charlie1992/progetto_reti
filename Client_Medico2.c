#include "Client_lib.h"

int main(int argc, char *argv[]){
    
    struct sockaddr_in servaddr;
    int socket,i,j,count;
    char lista_date[100][11], numprenotazioni[2],numTotDate[4];
    PRENOTAZIONE prenotazione[100];
    
    //*************comunicazione con il server reparto ****************************
    
    // AF_INET : famiglia di ip che stiamo considerando (IP4v)
    // SOCK_STREAM : canale bidirezionale, sequenziale affidabile che opera su
    // connessione. I dati vengono ricevuti e trasmessi come un
    // flusso continuo
    // 0 : tipo di protocollo
    socket = SOCKET(AF_INET, SOCK_STREAM, 0);
    
    //salva la famiglia di ip che stiamo considerando
    servaddr.sin_family = AF_INET;
    // salva la potra in formato network order
    servaddr.sin_port = htons(medico2_reparto2_port);
    
    // controlla se in input non c'è il ip mi conetto con il locahost
    if (argc == 1){
        INET_PTON(AF_INET, localhost, &servaddr);
        // altrimenti mi connetto al indirizzo ip passato da riga di comando
    }else{
        INET_PTON(AF_INET, argv[1], &servaddr);
    }
    CONNECTION(socket, servaddr, sizeof(servaddr));
    
    //lettura numero prenotazioni totali
    FullRead(socket, numprenotazioni, sizeof(numprenotazioni));
    //lettura delle prenotazioni
    FullRead(socket, prenotazione, sizeof(prenotazione));
    //lettura num tot date
    FullRead(socket, numTotDate, sizeof(numTotDate));
    //lettura lista date
    FullRead(socket, lista_date, sizeof(lista_date));
 
    printf("-%d-\n",atoi(numTotDate));
    for(i=1;i<atoi(numTotDate);i++){
        
        count=0;
        
        printf("\nin data %s ci sono le seguenti prenotazioni \n",lista_date[i]);
        for (j=1; j<=atoi(numprenotazioni); j++) {
            if(strcmp(lista_date[i],prenotazione[j].data_visita)==0 && strcmp(prenotazione[j].data_visita, "---")!=0){
                printf("\ncodice prenotazione: R1%d",j);
                printf("\nnome :%s",prenotazione[j].nome);
                printf("\ncognome :%s",prenotazione[j].cognome);
                printf("\nvisita :%s \n",prenotazione[i].nome_visita_scelta);
                count++;
            }
        }
        if(count==0){
            printf("non ci sono prenotazioni\n");
        }
        
    }
    exit(0);
}
