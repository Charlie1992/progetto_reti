#include "Server_lib.h"

int main(int argc, char *argv[]){
   
    
    int list_fd, conn_fd, // comunicazione con client
        list_fd_server1, // comunicazione con server1
        list_fd_server2, // comunnicazione con server2
         len, enable = 1, i = 0, flag = 0;
        struct sockaddr_in servaddr, serv1addr,serv2addr, client; // strutture relative al server, server1, server2, client

    pid_t pid;

    // dichiarazione di buffer per inviare e ricevere dati
    char scelta[2], reparto[2], kbuffer[4],
         data_scelta[11], conferma[4], nome[15],
         cognome[15], cod_ricetta[15],
         numprenotazioni[2], cod_prenotazione[5], cod_prenotazioneReparto[5], 
         charflag[2], cod_reparto[3], cod_pret[5], data_diponibili[200][11];
         LISTA_TIPOLOGIA_VISITE lista_tipologia_visite[100];
    // dichiarazione di due strutture di tipo PRENOTAZIONE
    PRENOTAZIONE prenotazione[100], recuperoDati[2];
    
    FILE *file;

    // riempio la lista tipologia visite
    ListaNomiVisite(lista_tipologia_visite);
   
    
    //****************************comunicazione con il client ***********************
    // Creazione di una socket passando come parametro:
    // AF_INET =   famiglia di ip che stiamo considerando (IP4v)
    // SOCK_STREAM = canale bidirezionale, dove i dati vengono ricevuti e trasmessi come un flusso continuo
    // 0 : tipo di protocollo
    list_fd = SOCKET(AF_INET, SOCK_STREAM, 0);
    // salva la famiglia di ip che stiamo considerando
    servaddr.sin_family = AF_INET;
    // viene utilizzato come indirizzo del server;
    // l'app accetterà connessioni da qualsiasi indirizzo
    // associato al server
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    // funzione per memorizzare la porta sulla quale il server ci risponde 
    // questo valore sarà salvato nel campo sin_port
    servaddr.sin_port = htons(client_cup_port);
    // IMPOSTA SOCKETS IN MODO DA POTER RIUTILIZZARE L'INDIRIZZO IP
    setsockopt(list_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    // assegna l'indirizzo alla socket
    BIND(list_fd, servaddr);
    // Mette il socket in modalità di ascolto in
    // attesa di nuove connessioni
    LISTEN(list_fd, 1024);
    
 while (1){
        // dimensione della struttura client
        len = sizeof(client);
        // permette di accettare le nuove connessioni
        // che saranno poi gestite dal processo figlio mediante la fork().
        conn_fd = ACCEPT(list_fd, (struct sockaddr *)&client,(socklen_t *) &len);
        
        // fork per gestire le connessioni
        if ((pid = fork()) < 0){
            perror("fork error");
            exit(-1);
        }
        // se sono il figlio
        if (pid == 0){
            // chiude list_fd, e interagisce con il client
            // tramite la connessione con conn_sd
            close(list_fd);

            // pulizia dei buffer
            bzero(scelta, 2);
            bzero(reparto, 2);
            bzero(data_scelta, 11);
            bzero(nome, 15);
            bzero(cognome, 15);
            bzero(cod_ricetta, 15);
            bzero(numprenotazioni, 2);
            bzero(cod_prenotazione, 5);
            bzero(cod_reparto, 3);
            bzero(cod_pret, 5);
            
            // lettura scelta dal client
            FullRead(conn_fd, scelta, sizeof(scelta));
            if (strcmp(scelta, "1") == 0){ // avvio procedura di prenotazione
                printf("Avvio procedura di prenotazione.\n");
                // invio al client la lista delle possibili prenotazioni
                printf("invio al client la lista delle possibili prenotazioni\n");
                FullWrite(conn_fd, lista_tipologia_visite, sizeof(lista_tipologia_visite));
                // recezione scelta reparto dal client
                FullRead(conn_fd, reparto, sizeof(reparto));
                printf("Reparto scelto %d \n", atoi(reparto));
                
                if (atoi(reparto) >=  1  && atoi(reparto) <=  4){ // se la scelta è compresa tra 1 e 4 comunico con il server reparto 1
                     
                    //*******************il server cup ora si comporta come client e comunica con il server reparto 1********************
                    list_fd_server1 = ClientLink(serv1addr,cup_server_reparto1_port,argv[1],argc);
                    // invio la scelta al server reparto 1 
                    FullWrite(list_fd_server1, scelta, sizeof(scelta));
                    
                    // ricevo dimensione e date delle date disponibili dal server reparto 1
                    FullRead(list_fd_server1, kbuffer, sizeof(kbuffer));
                    FullRead(list_fd_server1, data_diponibili, sizeof(data_diponibili));
                    printf("Date disponibili:\n");
                    for (i = 0; i < atoi(kbuffer); i++){
                        printf("%s \n", data_diponibili[i]);
                    }
                    
                    // invio dimensione e date disponibili al client
                    FullWrite(conn_fd, kbuffer, sizeof(kbuffer));
                    FullWrite(conn_fd, data_diponibili, sizeof(data_diponibili));
                  do{
                        // ricevo data dal client
                        FullRead(conn_fd, data_scelta, sizeof(data_scelta));
                        printf("Data scelta %s \n", data_scelta);
                        // invio data scelta al server reparto 1
                        FullWrite(list_fd_server1, data_scelta, sizeof(data_scelta));
                        
                        // ricevo la conferma del server reparto 1
                        FullRead(list_fd_server1, conferma, sizeof(conferma));
                        printf("Conferma %s\n", conferma);
                        // invio la conferma al client
                        FullWrite(conn_fd, conferma, sizeof(conferma));
                    } while (strcmp(conferma, "si") != 0);//resto nel while fintanto che la conferma è negativa
                    // copio il codice reparto nel buffer
                    strcpy(cod_prenotazioneReparto, "R1");
                    // lettura dei dati dal client
                    FullRead(conn_fd, nome, sizeof(nome));
                    FullRead(conn_fd, cognome, sizeof(cognome));
                    FullRead(conn_fd, cod_ricetta, sizeof(cod_ricetta));
                    printf("nome %s \n", nome);
                    printf("cognome %s \n", cognome);
                    printf("cod ricetta %s \n", cod_ricetta);
                    printf("Data %s\n", data_scelta);
                    printf("Nome Visita %s\n", lista_tipologia_visite[atoi(reparto)].nome_tv);

                    // apertura del file del reparto 1
                    file = fopen("reparto1.txt", "r+");
                    
                    // lettura numero prenotazioni esistenti
                    fscanf(file, "%s", numprenotazioni);
                    // mi posiziono all'inizio del file
                    fseek(file, 0, SEEK_SET);
                    // converto in char il nuovo numero prenotazioni esistenti
                    sprintf(cod_prenotazione, "%d", atoi(numprenotazioni) + 1);
                    // scrivo sul file il numero di prenotazioni
                    fprintf(file, "%s \n", cod_prenotazione);
                    // copio i dati letti dal client nella struttura prenotazione
                    strcpy(prenotazione[atoi(numprenotazioni) + 1].nome, nome);
                    strcpy(prenotazione[atoi(numprenotazioni) + 1].cognome, cognome);
                    strcpy(prenotazione[atoi(numprenotazioni) + 1].data_visita, data_scelta);
                    strcpy(prenotazione[atoi(numprenotazioni) + 1].cod_ricetta, cod_ricetta);
                    strcpy(prenotazione[atoi(numprenotazioni) + 1].nome_visita_scelta, lista_tipologia_visite[atoi(reparto)].nome_tv );
                    
                    // mi posizione alla fine del file 
                    fseek(file, 0, SEEK_END);
                    // salvo i dati sul file
                    fprintf(file, "\n%s\n", nome);
                    fprintf(file, "%s\n", cognome);
                    fprintf(file, "%s\n", data_scelta);
                    fprintf(file, "%s\n", cod_ricetta);
                    fprintf(file, "%s\n", lista_tipologia_visite[atoi(reparto)].nome_tv);
                    // chiudo il file
                    fclose(file);

                    // concateno a codice reparto il codice della prenotazione 
                    strcat(cod_prenotazioneReparto, cod_prenotazione);
                    printf("\n codice reparto %s", cod_prenotazioneReparto);

                    // invio il codice prenotazione al client
                    FullWrite(conn_fd, cod_prenotazioneReparto, sizeof(cod_prenotazioneReparto));
                    // invio codice prenotazione e i dati al server reparto 1
                    FullWrite(list_fd_server1, cod_prenotazione, sizeof(cod_prenotazione));
                    FullWrite(list_fd_server1, prenotazione, sizeof(prenotazione));
 }else if (atoi(reparto) >  4){ // altrimenti comunico con il server reparto 2     
                    
                    //*******************il server cup ora si comporta come client e comunica con il server reparto 2********************
                    list_fd_server2 = ClientLink(serv2addr,cup_server_reparto2_port,argv[1],argc);
                    // invio la scelta al server reparto 2
                    FullWrite(list_fd_server2, scelta, sizeof(scelta));
                    
                    // ricevo date e dimensione delle date disponibili
                    FullRead(list_fd_server2, kbuffer, sizeof(kbuffer));
                    printf("Valore di k %s \n", kbuffer);
                    FullRead(list_fd_server2, data_diponibili, sizeof(data_diponibili));
                    printf("Date disponibili:\n");
                    for (i = 0; i < atoi(kbuffer); i++){
                        printf("%s \n", data_diponibili[i]);
                    }
                    
                    // invio date e dimensioni disponibili al client
                    FullWrite(conn_fd, kbuffer, sizeof(kbuffer));
                    FullWrite(conn_fd, data_diponibili, sizeof(data_diponibili));
                    do{
                        // ricevo data dal client
                        FullRead(conn_fd, data_scelta, sizeof(data_scelta));
                        printf("Data scelta %s \n", data_scelta);
                        // invio data scelta al server raparto 2
                        FullWrite(list_fd_server2, data_scelta, sizeof(data_scelta));
                        // ricevo la conferma del server reparto 2
                        FullRead(list_fd_server2, conferma, sizeof(conferma));
                        printf("Conferma %s\n", conferma);
                        // invio la conferma al client
                        FullWrite(conn_fd, conferma, sizeof(conferma));
                    } while (strcmp(conferma, "si") != 0);// resto nel while fintanto che la conferma è negativa
                    // copio il codice reparto nel buffer
                    strcpy(cod_prenotazioneReparto, "R2");
                    // lettura dei dati dal client
                    FullRead(conn_fd, nome, sizeof(nome));
                    FullRead(conn_fd, cognome, sizeof(cognome));
                    FullRead(conn_fd, cod_ricetta, sizeof(cod_ricetta));
                    printf("nome %s \n", nome);
                    printf("cognome %s \n", cognome);
                    printf("cod ricetta %s \n", cod_ricetta);
                    printf("Data %s\n", data_scelta);
                    printf("Nome Visita %s\n", lista_tipologia_visite[atoi(reparto)].nome_tv);

                    
                    // apertura del file del reparto 2
                    file = fopen("reparto2.txt", "r+");
                    // lettura numero prenotazioni esistenti
                    fscanf(file, "%s", numprenotazioni);
                    // mi posiziono all'inizio del file
                    fseek(file, 0, SEEK_SET);
                    // converto in char il nuovo numero prenotazioni esistenti
                    sprintf(cod_prenotazione, "%d", atoi(numprenotazioni) + 1);
                    // scrivo sul file il numero di prenotazioni
                    fprintf(file, "%s \n", cod_prenotazione);
                    // copio i dati letti dal client nella struttura prenotazione
                    strcpy(prenotazione[atoi(numprenotazioni) + 1].nome, nome);
                    strcpy(prenotazione[atoi(numprenotazioni) + 1].cognome, cognome);
                    strcpy(prenotazione[atoi(numprenotazioni) + 1].data_visita, data_scelta);
                    strcpy(prenotazione[atoi(numprenotazioni) + 1].cod_ricetta, cod_ricetta);
                    strcpy(prenotazione[atoi(numprenotazioni) + 1].nome_visita_scelta, lista_tipologia_visite[atoi(reparto)].nome_tv );
                    
                    // mi posizione alla fine del file 
                    fseek(file, 0, SEEK_END);
                    // salvo i dati sul file
                    fprintf(file, "\n%s\n", nome);
                    fprintf(file, "%s\n", cognome);
                    fprintf(file, "%s\n", data_scelta);
                    fprintf(file, "%s\n", cod_ricetta);
                    fprintf(file, "%s\n", lista_tipologia_visite[atoi(reparto)].nome_tv);
                    // chiudo il file
                    fclose(file);

                    // concateno a codice reparto il codice della prenotazione 
                    strcat(cod_prenotazioneReparto, cod_prenotazione);
                    printf("\n codice reparto %s", cod_prenotazioneReparto);

                    // invio il codice prenotazione al client
                    FullWrite(conn_fd, cod_prenotazioneReparto, sizeof(cod_prenotazioneReparto));
                    // invio codice prenotazione e i dati al server reparto 2
                    FullWrite(list_fd_server2, cod_prenotazione, sizeof(cod_prenotazione));
                    FullWrite(list_fd_server2, prenotazione, sizeof(prenotazione));
                }
           }else if (strcmp(scelta, "2") == 0){// inizio recupero informazioni visita
                printf("\n Avvio procedura di recupero informazioni della visita prenotata\n");
                // pulizia dei buffer
                bzero(cod_prenotazione, 5);
                bzero(cod_pret, 5);
                bzero(conferma, 4);
                do{
                    // lettura codice prenotazione dal client
                    FullRead(conn_fd, cod_prenotazione, sizeof(cod_prenotazione));
                    printf("\ncodice prenotazione :%s", cod_prenotazione);
                    // controllo se esiste il reparto
                    flag = controllo(cod_prenotazione);
                    // converto in char il flag
                    sprintf(charflag, "%d", flag);
                    // invio il flag a client
                    FullWrite(conn_fd, charflag, sizeof(charflag));
                } while (flag != 1);// resto nel while fintanto che il flag è diverso da 1
                
                // separo il codice reparto dal codice prenotazione
                for (i = 0; i < strlen(cod_prenotazione); i++){
                    if (i < 2){
                        cod_reparto[i] = cod_prenotazione[i];
                    }else{
                        cod_pret[i - 2] = cod_prenotazione[i];
                    }
                }

                printf("\ncodice reparto :%s\n", cod_reparto); 
                printf("\ncodice prenotazione :%s\n", cod_pret); 

                if (strcmp(cod_reparto, "R1") == 0){ // se codice reparto è uguale a R1
                    //*******************il server cup ora si comporta come client e comunica con il server reparto 1********************
                    list_fd_server1 = ClientLink(serv1addr,cup_server_reparto1_port,argv[1],argc);
                    // invio la scelta e il codice prenotazione al server reparto 1
                    FullWrite(list_fd_server1, scelta, sizeof(scelta));
                    FullWrite(list_fd_server1, cod_pret, sizeof(cod_pret));
                    
                    // pulizia del buffer
                    bzero(conferma, 4);
                    // leggo la conferma dal server reparto 1
                    FullRead(list_fd_server1, conferma, sizeof(conferma));
                    printf("\nconferma:%s", conferma);
                    // invio la conferma al client
                    FullWrite(conn_fd, conferma, sizeof(conferma));
                    // lettura dei dati inviati dal server reaprto 1
                    FullRead(list_fd_server1, recuperoDati, sizeof(recuperoDati));
                    
                    if(strcmp(conferma,"si")==0){
                        // stampa dati utente
                        printf("Codice ricetta %s\n", recuperoDati[1].cod_ricetta);
                        printf("Nome %s\n", recuperoDati[1].nome);
                        printf("Cognome %s\n", recuperoDati[1].cognome);
                        printf("data %s\n", recuperoDati[1].data_visita);
                        printf("visita %s\n", recuperoDati[1].nome_visita_scelta);
                        printf("Invio dati recuperati al client\n");
                    }

                    // invio dati recuperati al client
                    FullWrite(conn_fd, recuperoDati, sizeof(recuperoDati));

                }else if(strcmp(cod_reparto, "R2") == 0){ // se codice reparto è uguale a R2
                    //*******************il server cup ora si comporta come client e comunica con il server reparto 2********************
                     list_fd_server2 = ClientLink(serv2addr,cup_server_reparto2_port,argv[1],argc);
                    // invio la scelta e il codice prenotazione al server reparto 2
                    FullWrite(list_fd_server2, scelta, sizeof(scelta));
                    FullWrite(list_fd_server2, cod_pret, sizeof(cod_pret));
                    // pulizia del buffer
                    bzero(conferma, 4);
                    // leggo la conferma dal server reparto 2
                    FullRead(list_fd_server2, conferma, sizeof(conferma));
                    printf("\nconferma:%s", conferma);
                    // invio la conferma al client
                    FullWrite(conn_fd, conferma, sizeof(conferma));
                    // lettura dei dati inviati dal server reparto 2
                    FullRead(list_fd_server2, recuperoDati, sizeof(recuperoDati));
                     
                    if(strcmp(conferma,"si")==0){
                        // stampa dati utente
                        printf("Codice ricetta %s\n", recuperoDati[1].cod_ricetta);
                        printf("Nome %s\n", recuperoDati[1].nome);
                        printf("Cognome %s\n", recuperoDati[1].cognome);
                        printf("data %s\n", recuperoDati[1].data_visita);
                        printf("visita %s\n", recuperoDati[1].nome_visita_scelta);
                        printf("Invio dati recuperati al client\n");
                    }

                    // invio dati recuperati al client
                    FullWrite(conn_fd, recuperoDati, sizeof(recuperoDati));

                }else{ // se il reparto non esiste
                    strcpy(conferma, "no");
                    // invio la conferma al client
                    FullWrite(conn_fd, conferma, sizeof(conferma));
                }
               
                
            }else if (strcmp(scelta, "3") == 0){ // inizio cancellazione visita
                         printf("\n Avvio procedura di cancellazione visita\n");
                // pulizia dei buffer
                bzero(cod_prenotazione, 5);
                bzero(cod_pret, 5);
                bzero(conferma, 4);
                do{
                    // lettura codice prenotazione dal client
                    FullRead(conn_fd, cod_prenotazione, sizeof(cod_prenotazione));
                    printf("\ncodice prenotazione :%s", cod_prenotazione);
                    // controllo se esiste il reparto
                    flag = controllo(cod_prenotazione);
                    // converto in char il flag
                    sprintf(charflag, "%d", flag);
                    // invio il falg a client
                    FullWrite(conn_fd, charflag, sizeof(charflag));
                } while (flag != 1); // resto nel while fintanto che il flag è diverso da 1
                // separo il codice reparto dal codice prenotazione
                for (i = 0; i < strlen(cod_prenotazione); i++){
                    if (i < 2){
                        cod_reparto[i] = cod_prenotazione[i];
                    }else{
                        cod_pret[i - 2] = cod_prenotazione[i];
                    }
                }
                printf("\ncodice reparto :%s\n", cod_reparto); 
                printf("\ncodice prenotazione :%s\n", cod_pret); 
                
                if (strcmp(cod_reparto, "R1") == 0){  // se il codice reparto è uguale a R1
                    //*******************il server cup ora si comporta come client e comunica con il server reparto 1********************
                    list_fd_server1 = ClientLink(serv1addr,cup_server_reparto1_port,argv[1],argc);
                    // invio la scelta e il codice prenotazione al server reparto 1
                    FullWrite(list_fd_server1, scelta, sizeof(scelta));
                    FullWrite(list_fd_server1, cod_pret, sizeof(cod_pret));
                    // lettura conferma prenotazione dal server reparto 1
                    FullRead(list_fd_server1, conferma, sizeof(conferma));
                    printf("\nconferma:%s", conferma);
                    // invio la conferma al client 
                    FullWrite(conn_fd, conferma, sizeof(conferma));
                }else if(strcmp(cod_reparto, "R2") == 0){  // se codice reparto è uguale a R2
                     //*******************il server cup ora si comporta come client e comunica con il server reparto 2********************
                    list_fd_server2 = ClientLink(serv2addr,cup_server_reparto2_port,argv[1],argc);
                    // invio la scelta e il codice prenotazione al server reparto 2
                    FullWrite(list_fd_server2, scelta, sizeof(scelta));
                    FullWrite(list_fd_server2, cod_pret, sizeof(cod_pret));
                    // lettura conferma prenotazione dal server reparto 2
                    FullRead(list_fd_server2, conferma, sizeof(conferma));
                    printf("\nconferma:%s", conferma);
                    // invio la conferma al client 
                    FullWrite(conn_fd, conferma, sizeof(conferma));
                }else{ // se il reparto non esiste
                    strcpy(conferma, "no");
                    // invio la conferma al client
                    FullWrite(conn_fd, conferma, sizeof(conferma));
                }               
            }else{
                printf("\nErrore \n");
            }
            // chiusura delle comunicazioni
            close(list_fd_server1);
            close(list_fd_server2);
            close(conn_fd);
            exit(0);
        }else{// sono il padre
            // chiudo la comunicazione con il client
            close(conn_fd);
        }
    }
    
    exit(0);
}
