#include "Server_lib.h"

int main(int argc, char *argv[])
{
    int list_fd, conn_fd, // comunicazione con server cup
        list_fd_medico,conn_fd_medico, // comunicazione con medico 
        i = 0, j = 0, count_datap = 0, k = 1, enable = 1;

    struct sockaddr_in servaddr,servaddr_medico;  // strutture relative al server cup, client medico
    pid_t pid,pid1;

    FILE *file, *file1;
    // dichiarazione di buffer per inviare e ricevere dati
    char scelta[2], numprenotazioni[2], kbuffer[4], conferma[4],
         cod_prenotazione[5], data_diponibili[200][11], 
         lista_date[200][11], data_scelta[11],numTotDate[4];

    // dichiarazione di due struttare di tipo PRENOTAZIONE
    PRENOTAZIONE prenotazione[100], recuperoDati[2];

    // riempio la lista tipoligia visite
    riempi_lista_data(lista_date);

    //*************comunicazione con il server centrale******************
    // Creazione di una socket passando come parametro:
    // AF_INET =   famiglia di ip che stiamo considerando (IP4v)
    // SOCK_STREAM = canale bidirezionale, dove i dati vengono ricevuti e trasmessi come un flusso continuo
    // 0 : tipo di protocollo
    list_fd = SOCKET(AF_INET, SOCK_STREAM, 0);
    // salva la famiglia di ip che stiamo considerando
    servaddr.sin_family = AF_INET;
    // viene utilizzato come indirizzo del server
    // l'app accetterà connessioni da qualsiasi indirizzo
    // associato al server
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    // funione per memeorizzare la porta sulla quale il serve ci risponde. 
    // questo valore sara salvato nel campo sin_port
    servaddr.sin_port = htons(cup_server_reparto2_port);

    //IMPOSTA SOCKETS IN MODO DA POTER RIUTILIZZARE L'INDIRIZZO IP
    setsockopt(list_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    // assegna l'indirizzo alla socket
    BIND(list_fd,servaddr);
    // mette il socket in modalita' di ascolto in
    // attesa di nuove connessioni
    LISTEN(list_fd,1024);

     //*************comunicazione con il server centrale******************
    // Creazione di una socket passando come parametro:
    // AF_INET =   famiglia di ip che stiamo considerando (IP4v)
    // SOCK_STREAM = canale bidirezionale, dove i dati vengono ricevuti e trasmessi come un flusso continuo
    // 0 : tipo di protocollo
    list_fd_medico = SOCKET(AF_INET, SOCK_STREAM, 0);
    // salva la famiglia di ip che stiamo considerando 
    servaddr_medico.sin_family = AF_INET;
    // viene utilizzato come indirizzo del server
    // l'app accetterà connessioni da qualsiasi indirizzo associato al server
    servaddr_medico.sin_addr.s_addr = htonl(INADDR_ANY);
    // funione per memeorizzare la porta sulla quale il serve ci risponde. 
    // questo valore sara salvato nel campo sin_port
    servaddr_medico.sin_port = htons(medico2_reparto2_port);

    // IMPOSTA SOCKETS IN MODO DA POTER RIUTILIZZARE L'INDIRIZZO IP
    setsockopt(list_fd_medico, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    // assegna l'indirizzo alla socket
    BIND(list_fd_medico, servaddr_medico);
    // mette il socket in modalita' di ascolto in
    // attesa di nuove connessioni
    LISTEN(list_fd_medico, 1024);


    pid = fork();  // genero un filgio per gestire la comunicazione con il server CUP
    pid1 = fork(); // genero un filgio per gestire la comunicazione con client medico

    if (pid == 0)// se sono il figlio
    {
        while (1)
        {
            // pulizia dei buffer
            bzero(scelta, 2);
            bzero(numprenotazioni, 2);
            bzero(data_scelta, 11);
            // permette di accettare le nuove connessioni
            // che saranno poi gestite dal processo figlio mediante la fork().
            conn_fd = ACCEPT(list_fd,NULL,NULL);
            
            // fork per gestire le connessione
            if ((pid = fork()) < 0)
            {
                perror("fork error");
                exit(-1);
            }
            // se sono il filgio
            if (pid == 0)
            {
                // chiude list_fd, e interagisce con il client
                // tramite la connessione con conn_sd
                close(list_fd);
                //ricevo scelta del server cup
                FullRead(conn_fd, scelta, sizeof(scelta));
                
                if (strcmp(scelta, "1") == 0){ // inizio procedura di prenotazione visita
                    k = 0;
                    printf("avvio procedura di prenotazione visita\n");

                    // apertura del file  reparto  
                    file = fopen("reparto2.txt", "r+");
                    // letture numero prenotazioni esistenti
                    fscanf(file, "%s", numprenotazioni);

                    // lettura dal file delle prenotazione esistenti
                    for (i = 1; i <= atoi(numprenotazioni); i++)
                    {
                        fscanf(file, "%s %s %s %s %s\n", 
                        prenotazione[i].nome, 
                        prenotazione[i].cognome, 
                        prenotazione[i].data_visita, 
                        prenotazione[i].cod_ricetta, 
                        prenotazione[i].nome_visita_scelta);
                    }
                    // chiusura del file
                    fclose(file);
                    // controllo delle date disponibili
                    for (i = 1; i < conta_date(lista_date); i++)
                    {
                        count_datap = 0;
                        for (j = 1; j <= atoi(numprenotazioni); j++)
                        {
                            // se la data della lista è presente nella date delle prenotazioni
                            if (strcmp(lista_date[i], prenotazione[j].data_visita) == 0)
                            {
                                count_datap++;
                            }
                        }
                        // se la data non si ripete per più di tre volte
                        if (count_datap < 15)
                        {
                            // salvo la date in date disponibili
                            strcpy(data_diponibili[k], lista_date[i]);
                            // incremento la posizione
                            k++;
                        }
                    }
                    printf("Data disponibili:\n");
                    for (i = 0; i < k; i++)
                    {
                        printf("%s \n", data_diponibili[i]);
                    }
                    // invio date dipsponibili e dimenzione  al server cup
                    sprintf(kbuffer, "%d", k);
                    FullWrite(conn_fd, kbuffer, sizeof(kbuffer));
                    FullWrite(conn_fd, data_diponibili, sizeof(data_diponibili));
                    do
                    {
                        // ricevo data scelta del server cup
                        FullRead(conn_fd, data_scelta, sizeof(data_scelta));
                        printf("Data scelta %s\n", data_scelta);

                        // ricerca della data
                        strcpy(conferma, "no");
                        for (i = 0; i <= k; i++)
                        {
                            // se la data scelta è presente nella lista di date disponibili
                            if (strcmp(data_scelta, data_diponibili[i]) == 0)
                            {
                                // copio si in conferma
                                strcpy(conferma, "si");
                            }
                        }
                        printf("Conferma %s\n", conferma);
                        // invio conferma al server centrale
                        FullWrite(conn_fd, conferma, sizeof(conferma));
                    } while (strcmp(conferma, "si") != 0);// resto nel while fintanto che la conferma è negativa
                    // leggo codice prenotazione dal serve cup
                    FullRead(conn_fd, cod_prenotazione, sizeof(cod_prenotazione));
                    printf("Il codice prenotazione e' R1%s\n", cod_prenotazione);
                    // leggo dati delle prenotazione dal serve cup
                    FullRead(conn_fd, prenotazione, sizeof(prenotazione));
                    printf("Dati prenotazione: \n ");
                    printf("Nome: %s \n ", prenotazione[atoi(cod_prenotazione)].nome);
                    printf("Cognome: %s \n ", prenotazione[atoi(cod_prenotazione)].cognome);
                    printf("Codice Ricetta: %s \n ", prenotazione[atoi(cod_prenotazione)].cod_ricetta);
                    printf("Data: %s \n ", prenotazione[atoi(cod_prenotazione)].data_visita);
                    printf("Nome Visita: %s \n ", prenotazione[atoi(cod_prenotazione)].nome_visita_scelta);

                }else if (strcmp(scelta, "2") == 0){ // inizio recupero infarmazioni visita
                    // pulizia buffer
                    bzero(conferma, 4);

                    printf("\navvio procedura di recupero informazione visita prenotata\n");

                    // apertura del file del reparto 
                    file = fopen("reparto2.txt", "r+");
                    // lettura numero prenotazioni esistenti
                    fscanf(file, "%s", numprenotazioni);

                    // lettura da file delle prenotazioni esistenti
                    for (i = 1; i <= atoi(numprenotazioni); i++)
                    {
                        fscanf(file, "%s %s %s %s %s\n", prenotazione[i].nome,
                        prenotazione[i].cognome, 
                        prenotazione[i].data_visita, 
                        prenotazione[i].cod_ricetta, 
                        prenotazione[i].nome_visita_scelta);
                    }
                    // chiusura del file
                    fclose(file);

                    // pulizia buffer
                    bzero(cod_prenotazione, 5);
                    // lettura codice prenotazione
                    FullRead(conn_fd, cod_prenotazione, sizeof(cod_prenotazione));
                    printf("\n il codice prenotazione e' R1%s\n", cod_prenotazione);
                    // copio in conferma "no"
                    strcpy(conferma, "no");
                    // se il codice ricetta è diverso da 0
                    if (atoi(prenotazione[atoi(cod_prenotazione)].cod_ricetta) != 0)
                    {
                        // pulizia buffer
                        bzero(conferma, 4);
                        // copio in conferma "si"
                        strcpy(conferma, "si");
                    }
                    printf("\nconferma :%s\n", conferma);
                    // invio conferma al server cup
                    FullWrite(conn_fd, conferma, sizeof(conferma));
                    // copio i dati della prenotazione scelta nella struttura recupero Dati
                    strcpy(recuperoDati[1].nome, prenotazione[atoi(cod_prenotazione)].nome);
                    strcpy(recuperoDati[1].cognome, prenotazione[atoi(cod_prenotazione)].cognome);
                    strcpy(recuperoDati[1].data_visita, prenotazione[atoi(cod_prenotazione)].data_visita);
                    strcpy(recuperoDati[1].cod_ricetta, prenotazione[atoi(cod_prenotazione)].cod_ricetta);
                    strcpy(recuperoDati[1].nome_visita_scelta, prenotazione[atoi(cod_prenotazione)].nome_visita_scelta);
                    printf("%s\n", recuperoDati[1].cod_ricetta);
                    printf("%s\n", recuperoDati[1].nome);
                    printf("%s\n", recuperoDati[1].cognome);
                    printf("%s\n", recuperoDati[1].data_visita);
                    printf("%s\n", recuperoDati[1].nome_visita_scelta);
                    printf("Invio datirecuparti al server CUP\n");
                    // invio dati al server cup 
                    FullWrite(conn_fd, recuperoDati, sizeof(recuperoDati));
                }else if (strcmp(scelta, "3") == 0){ // inizio cancellazione visita
                    // pulizia buffer conferma
                    bzero(conferma, 4);
                    printf("\navvio procedura di  cancellazione visita  prenotata\n");

                    // apertura del file del reparto 
                    file = fopen("reparto2.txt", "r+");
                    // lettura numero prenotazioni esistenti
                    fscanf(file, "%s", numprenotazioni);

                    // lettura da file delle prenotazione esistenti
                    for (i = 1; i <= atoi(numprenotazioni); i++)
                    {
                        fscanf(file, "%s %s %s %s %s\n", 
                        prenotazione[i].nome, 
                        prenotazione[i].cognome, 
                        prenotazione[i].data_visita, 
                        prenotazione[i].cod_ricetta, 
                        prenotazione[i].nome_visita_scelta);
                    }
                    // chiusura dal file
                    fclose(file);
                    

                    // pulizia buffer
                    bzero(cod_prenotazione, 5);
                    // lettura codice prenotazione dal server cup
                    FullRead(conn_fd, cod_prenotazione, sizeof(cod_prenotazione));
                    printf("\n il codice prenotazione e' R1%s\n", cod_prenotazione);
                    // copio no in conferma
                    strcpy(conferma, "no");
                    // se il codice ricetta è diverso da 0
                    if (atoi(prenotazione[atoi(cod_prenotazione)].cod_ricetta) != 0)
                    {
                        // cancellazione del file
                        unlink("reparto2.txt");
                        
                        // pulizia del buffer
                        bzero(conferma, 4);
                        // copio si in conferma
                        strcpy(conferma, "si");

                        // cancellazione della prenotazione
                        for (i = 1; i <= atoi(numprenotazioni); i++)
                        {
                            if (i == atoi(cod_prenotazione))
                            { //salva su file
                                strcpy(prenotazione[i].nome, "---");
                                strcpy(prenotazione[i].cognome, "---");
                                strcpy(prenotazione[i].data_visita, "---");
                                strcpy(prenotazione[i].cod_ricetta, "---");
                                strcpy(prenotazione[i].nome_visita_scelta, "---");
                            }
                        }
                        // creazione del nuovo file 
                        file1 = fopen("reparto2.txt", "w+");

                        // mi posiziono all'inizio del file
                        fseek(file1, 0, SEEK_SET);
                        // salvo il numero di prenotazioni esistenti
                        fprintf(file1, "%s\n\n", numprenotazioni);

                        // salvataggio delle prenotazioni
                        for (i = 1; i <= atoi(numprenotazioni); i++)
                        {
                            //salva su file
                            fprintf(file1, "%s\n", prenotazione[i].nome);
                            fprintf(file1, "%s\n", prenotazione[i].cognome);
                            fprintf(file1, "%s\n", prenotazione[i].data_visita);
                            fprintf(file1, "%s\n", prenotazione[i].cod_ricetta);
                            fprintf(file1, "%s\n", prenotazione[i].nome_visita_scelta);
                            fprintf(file1,"\n");
                        }
                        // chiusura del file
                        fclose(file1);
                    }
                    printf("\nconferma :%s\n", conferma);
                    // invio la conferma al server cup 
                    FullWrite(conn_fd, conferma, sizeof(conferma));
                }
                // chiusura delle comunicazione
                close(conn_fd);
                exit(0);
           
            }else{ // se sono il padre 
                  // chiudo la comunicazione con il client
                 close(conn_fd);
            }
        }
    }
    else // altrimenti
    {
        wait(NULL);
    }

    if (pid1 == 0)// se sono il figlio
    {
        while (1)
        {
            // permette di accettare le nuove connessioni
            // che saranno poi gestite dal processo figlio mediante la fork().
            conn_fd_medico = ACCEPT(list_fd_medico, NULL, NULL);

            // fork per gestire le connessione
            if ((pid1 = fork()) < 0)
            {
                perror("fork error");
            }
                exit(-1);
            // se sono il figlio
            if (pid1 == 0)
            {
                // chiude list_fd_medico, e interagisce con il client
                // tramite la connessione con conn_sd_medico
                close(list_fd_medico);
                // apertura del file del reparto 
                file = fopen("reparto2.txt", "r+");
                // lettura numero prenotazioni esistenti
                fscanf(file, "%s", numprenotazioni);

                // lettura da file delle prenotazioni esistenti
                for (i = 1; i <= atoi(numprenotazioni); i++)
                {
                        fscanf(file, "%s %s %s %s %s\n", 
                        prenotazione[i].nome, 
                        prenotazione[i].cognome, 
                        prenotazione[i].data_visita, 
                        prenotazione[i].cod_ricetta, 
                        prenotazione[i].nome_visita_scelta);
                }
                // chiusura del file
                fclose(file);
                // invio del numero delle prenotazioni al medico
                FullWrite(conn_fd_medico, numprenotazioni, sizeof(numprenotazioni));
                // invio delle prenotazioni al medico
                FullWrite(conn_fd_medico, prenotazione, sizeof(prenotazione));
                // converto in char il numero totale di date 
                sprintf(numTotDate, "%d", conta_date(lista_date));
                // invio numero totali di date al medico
                FullWrite(conn_fd_medico, numTotDate, sizeof(numTotDate));
                // invio della lista date totali al medico
                FullWrite(conn_fd_medico, lista_date, sizeof(lista_date));
                
                // chiusura delle comunicazione
                close(conn_fd_medico);
                exit(1);
            }
            else// se sono il padre 
            {    // chiudo la comunicazione con il client
                close(conn_fd_medico);
            }
        }
    }
    else // altrimenti
    {
        wait(NULL);
    }

    exit(0);
}
