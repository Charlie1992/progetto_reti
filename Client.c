#include "Client_lib.h"

int main(int argc, char *argv[]){
    
    int     socket, // comunicazione con server  
            i=0;    // dichiarazione  di un contatore 
     //strutture relative al server
    struct  sockaddr_in servaddr;
    
    //dichiarazione di buffer per inviare e ricevere dati
    char    scelta[2], visita[2], kbuffer[4], data_scelta[11], 
            conferma[4], nome[15], cognome[15], cod_ricetta[15], 
            cod_prenotazione[5], data_diponibili[200][11], charflag[2];

    //dichiarazione di una struttara di tipo LISTA_TIPOLOGIA_VISITE
    LISTA_TIPOLOGIA_VISITE lista_tipologia_visite[100];
    //dichiarazione di una struttara di tipo PRENOTAZIONE
    PRENOTAZIONE recuperoDati[100];
    
    //*************comunicazione con il server reparto ****************************
    // AF_INET : famiglia di ip che stiamo considerando (IP4v)
    // SOCK_STREAM : canale bidirezionale, sequenziale affidabile 
    // che opera sulla connessione
    // I dati vengono ricevuti e trasmessi come un flusso continuo
    // 0 : tipo di protocollo 
    socket = SOCKET(AF_INET, SOCK_STREAM, 0);
    //salva la famiglia di ip che stiamo considerando
    servaddr.sin_family = AF_INET;
    // salva la potra in formato network order
    servaddr.sin_port = htons(client_cup_port);
    // se in input non c'è il ip, allora mi connetto con il locahost
    if (argc == 1){
        INET_PTON(AF_INET, localhost, &servaddr);
    }else{  // altrimenti mi connetto all'indirizzo ip passato da riga di comando
        INET_PTON(AF_INET, argv[1], &servaddr);
    }
    
    //  connette il socket all'indirizzo 
    CONNECTION(socket, servaddr, sizeof(servaddr));

    // pulizia dei buffer
    bzero(scelta, 2);
    bzero(visita, 2);
    bzero(kbuffer, 4);
    bzero(data_scelta, 11);
    bzero(nome, 15);
    bzero(cognome, 15);
    bzero(cod_ricetta, 15);

    // stampa del menu
    printf("*********Menu**********\n");
    printf("1.Prenota visita\n");
    printf("2.Informazione visita prenotata\n");
    printf("3.Cancella visita\n");
    printf("4.Esci\n");

    do{
        printf("Inserisci scelta :");
        scanf("%s", scelta);
    }while(atoi(scelta)>4 || atoi(scelta)<1); //resto nel while fintanto che scelta non è compresa tra 1 e 4 
    // inzio propcedura di prenotazione
    if (strcmp(scelta, "1") == 0){
        printf("inzio propcedura di prenotazione\n");
        printf("Scelta : %s\n", scelta);
        // invio scelta la server CUP
        FullWrite(socket, scelta, sizeof(scelta));
        // letture delle possibili visite 
        FullRead(socket, lista_tipologia_visite, sizeof(lista_tipologia_visite));
        for (i = 1; i < conta_ltv(lista_tipologia_visite); i++){
            printf("Id:%d Nome: %s \n", lista_tipologia_visite[i].id_tv, lista_tipologia_visite[i].nome_tv);
        }
        printf("Segli visita :");
        scanf("%s", visita);
        // se il reparto è compreso tra 1 e il numero tot delle possibili visite
        if (atoi(visita) >= 1 && atoi(visita) <= conta_ltv(lista_tipologia_visite)){
            // invio scelta al server cup
            FullWrite(socket, visita, sizeof(visita));
            // leggo date e dimensione delle date disponibili dal server cup
            FullRead(socket, kbuffer, sizeof(kbuffer));
            FullRead(socket, data_diponibili, sizeof(data_diponibili));
            printf("Date disponibili:\n");
            for (i = 0; i < atoi(kbuffer); i++){
                printf("%s \n", data_diponibili[i]);
            }
            
            do{
                printf("Inserire data:");
                scanf("%s", data_scelta);
                // invio data scelta al server cup
                FullWrite(socket, data_scelta, sizeof(data_scelta));
                // leggo la conferma dal server cup
                FullRead(socket, conferma, sizeof(conferma));
                printf("Conferma %s\n", conferma);
            } while (strcmp(conferma, "si") != 0);// resto nel while fintanto che la conferma è negativa
            
            // acquisizione dei dati dell'utente
            printf("Inserire il nome: ");
            scanf("%s", nome);
            printf("Inserire il cognome: ");
            scanf("%s", cognome);
            printf("Inserire il condice ricetta: ");
            scanf("%s", cod_ricetta);
            
            // invio dati al server cup
            FullWrite(socket, nome, sizeof(nome));
            FullWrite(socket, cognome, sizeof(cognome));
            FullWrite(socket, cod_ricetta, sizeof(cod_ricetta));
            // pulizia del buffer cod_prenotazione
            bzero(cod_prenotazione, 5);
            // lettura del codice prenotazione dal serve cup
            FullRead(socket, cod_prenotazione, sizeof(cod_prenotazione));
            printf("Il codice delle prenotazione e' %s\n", cod_prenotazione);
            exit(1);
        }else{// se non e compreso
            printf("Errore visita inesistente.\n");
            exit(-1);
        }
    }else if (strcmp(scelta, "2") == 0){// inizio recupero infarmazioni visita
        printf("\nAvvio procedura di recupero informazioni della visita prenotata\n");
        // invio scelta al server CUP
        FullWrite(socket, scelta, sizeof(scelta));
        do{
            printf("Inserisci il codice della prenotazione :");
            scanf("%s", cod_prenotazione);
            // invio codice prenotazione al server cup
            FullWrite(socket, cod_prenotazione, sizeof(cod_prenotazione));
            // lettura del flag dal server cup
            FullRead(socket, charflag, sizeof(charflag));
        } while (strcmp(charflag, "0") == 0);// resto nel while fintanto che il flag è diverso da 1
        // lettura della conferma dal server cup
        FullRead(socket,conferma,sizeof(conferma));
        // se la conferma è si        
        if(strcmp(conferma, "si") == 0){
            // lettura dei dati dal server cup
            FullRead(socket,recuperoDati,sizeof(recuperoDati));
            printf("\ni dati sono");
            printf("\nricetta :%s",recuperoDati[1].cod_ricetta);
            printf("\nnome :%s",recuperoDati[1].nome);
            printf("\ncognome :%s",recuperoDati[1].cognome);
            printf("\ndata :%s",recuperoDati[1].data_visita);
            printf("\nvisita :%s\n",recuperoDati[1].nome_visita_scelta);
        }else{ // se la conferma è no
            printf("Non ci sta la prenotazone \n");
            exit(-1);
        }
    }else if (strcmp(scelta, "3") == 0){ // inizio cancellazione visita
        printf("\navvio procedura di cancellazione visita\n");
        // invio scelta ala server CUP
        FullWrite(socket, scelta, sizeof(scelta));
        do{
            printf("inserisci il codice della prenotazione da eliminare:");
            scanf("%s", cod_prenotazione);
            // invio codice prenotazione al server cup
            FullWrite(socket, cod_prenotazione, sizeof(cod_prenotazione));
            // lettura del flag dal server cup
            FullRead(socket, charflag, sizeof(charflag));
        } while (strcmp(charflag, "0") == 0); // resto nel while fintanto che il flag è diverso da 1
        FullRead(socket,conferma,sizeof(conferma));
        // lettura della conferma dal server cup
        printf("\nconferma :%s\n",conferma);
        // se la conferma è si
        if(strcmp(conferma,"si")==0){
            printf("Cancellazione avvenuta con sucesso.\n");
            exit(1);
        }else{ // se la conferma è no
            printf("Prenotazione inesistente.\n");
            exit(-1);
        }
    }else{ // se la scelta è 4 esco
        printf("\nArrivederci!\n");
        exit(0);
    }
    
    exit(0);
}
