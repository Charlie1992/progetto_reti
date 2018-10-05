#include "Server_lib.h"

int main(int argc, char *argv[])
{
    int list_fd, conn_fd, i = 0, j = 0, count_datap = 0, k = 1, enable = 1;
    struct sockaddr_in servaddr;
    pid_t pid;

    FILE *file, *file1;
    char scelta[2], numprenotazioni[2], kbuffer[2], conferma[4], cod_prenotazione[5], data_diponibili[100][20], lista_date[100][20], data_scelta[15];
    PRENOTAZIONE prenotazione[100], recuperoDati[2];
    //DATI recuperoDati[2];
    riempi_lista_data(lista_date);

    //*************comunicazione con il server centrale******************
    // Creazione di una socket passando come parametro:
    // AF_INET =  IPv4 internet protocols
    // SOCK_STREAM = canale bidirezionale, dove i dati vengono ricevuti e trasmessi come un flusso continuo
    list_fd = SOCKET(AF_INET, SOCK_STREAM, 0);
    // memorizziamo nel campo sin_family IPv4 internet protocol
    servaddr.sin_family = AF_INET;
    // viene utilizzato come indirizzo del server
    // l'app accetterà connessioni da qualsiasi indirizzo
    // associato al server
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    // funione per memeorizzare la porta sulla quale il serve
    // ci risponde. Questo valore sara salvato nel campo sin_port
    servaddr.sin_port = htons(cup_server_reparto1_port);

    //IMPOSTA SOCKETS IN MODO DA POTER RIUTILIZZARE L'INDIRIZZO IP
    setsockopt(list_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (bind(list_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    if (listen(list_fd, 1024) < 0)
    {
        perror("listen");
        exit(1);
    }

    pid = fork();

    if (pid == 0)
    {
        while (1)
        {
            bzero(scelta, 2);
            bzero(numprenotazioni, 2);
            bzero(data_scelta, 15);

            if ((conn_fd = accept(list_fd, NULL, NULL)) < 0)
            {
                perror("accept");
                exit(1);
            }

            // fork per gestire le connesione
            if ((pid = fork()) < 0)
            {
                perror("fork error");
                exit(-1);
            }
            // se sono il filgio
            if (pid == 0)
            {
                close(list_fd);
                //ricevo scelta del server cup
                FullRead(conn_fd, scelta, sizeof(scelta));
                //se 1 avvivo procedura di prenotazione visita
                if (strcmp(scelta, "1") == 0)
                {
                    k = 0;
                    printf("avvio procedura di prenotazione visita\n");

                    //apertura del file del reparto
                    file = fopen("reparto1.txt", "r+");
                    //letturera numero prenotazioni esistenti
                    fscanf(file, "%s", numprenotazioni);

                    //lettura dai prenotazione esistenti
                    for (i = 1; i <= atoi(numprenotazioni); i++)
                    {
                        fscanf(file, "%s %s %s %s\n", prenotazione[i].nome, prenotazione[i].cognome, prenotazione[i].data_visita, prenotazione[i].cod_ricetta);
                    }
                    fclose(file);

                    for (i = 1; i < conta_date(lista_date); i++)
                    {
                        count_datap = 0;
                        for (j = 1; j <= atoi(numprenotazioni); j++)
                        {
                            if (strcmp(lista_date[i], prenotazione[j].data_visita) == 0)
                            {
                                count_datap++;
                            }
                        }
                        if (count_datap < 3)
                        {
                            strcpy(data_diponibili[k], lista_date[i]);
                            k++;
                        }
                    }

                    printf("Data disponibili:\n");
                    for (i = 0; i < k; i++)
                    {
                        printf("%s \n", data_diponibili[i]);
                    }

                    //invio date dipsponibili e dim  al server cup
                    sprintf(kbuffer, "%d", k);
                    FullWrite(conn_fd, kbuffer, sizeof(kbuffer));
                    FullWrite(conn_fd, data_diponibili, sizeof(data_diponibili));
                    do
                    {
                        FullRead(conn_fd, data_scelta, sizeof(data_scelta));
                        printf("Data scelta %s\n", data_scelta);

                        //riceca della data
                        strcpy(conferma, "no");
                        for (i = 1; i <= k; i++)
                        {
                            if (strcmp(data_scelta, data_diponibili[i]) == 0)
                            {
                                strcpy(conferma, "si");
                            }
                        }
                        printf("Conferma %s\n", conferma);
                        //invio conferma al server centrale
                        FullWrite(conn_fd, conferma, sizeof(conferma));
                    } while (strcmp(conferma, "si") != 0);

                    FullRead(conn_fd, cod_prenotazione, sizeof(cod_prenotazione));
                    printf("Il codice prenotazione e' R1%s\n", cod_prenotazione);

                    FullRead(conn_fd, prenotazione, sizeof(prenotazione));
                    printf("Dati prenotazione: \n ");
                    printf("Nome: %s \n ", prenotazione[atoi(cod_prenotazione)].nome);
                    printf("Cognome: %s \n ", prenotazione[atoi(cod_prenotazione)].cognome);
                    printf("Codice Ricetta: %s \n ", prenotazione[atoi(cod_prenotazione)].cod_ricetta);
                    printf("Data: %s \n ", prenotazione[atoi(cod_prenotazione)].data_visita);
                }
                else if (strcmp(scelta, "2") == 0)
                {

                    bzero(conferma, 4);

                    printf("\navvio procedura di recupero informazione visita prenotata\n");

                    //apertura del file del reparto
                    file = fopen("reparto1.txt", "r+");
                    //letturera numero prenotazioni esistenti
                    fscanf(file, "%s", numprenotazioni);

                    //lettura dai prenotazione esistenti
                    for (i = 1; i <= atoi(numprenotazioni); i++)
                    {
                        fscanf(file, "%s %s %s %s\n", prenotazione[i].nome, prenotazione[i].cognome, prenotazione[i].data_visita, prenotazione[i].cod_ricetta);
                    }
                    fclose(file);

                    //lettura codice prenotazione
                    bzero(cod_prenotazione, 5);
                    FullRead(conn_fd, cod_prenotazione, sizeof(cod_prenotazione));
                    printf("\n il codice prenotazione e' R1%s\n", cod_prenotazione);

                    printf("Codice %d \n", atoi(prenotazione[atoi(cod_prenotazione)].cod_ricetta));
                    strcpy(conferma, "no");
                    if (atoi(prenotazione[atoi(cod_prenotazione)].cod_ricetta) != 0)
                    {
                        bzero(conferma, 4);
                        strcpy(conferma, "si");
                    }

                    printf("\nconferma :%s\n", conferma);

                    FullWrite(conn_fd, conferma, sizeof(conferma));
                    strcpy(recuperoDati[1].nome, prenotazione[atoi(cod_prenotazione)].nome);
                    strcpy(recuperoDati[1].cognome, prenotazione[atoi(cod_prenotazione)].cognome);
                    strcpy(recuperoDati[1].data_visita, prenotazione[atoi(cod_prenotazione)].data_visita);
                    strcpy(recuperoDati[1].cod_ricetta, prenotazione[atoi(cod_prenotazione)].cod_ricetta);
                    printf("%s\n", recuperoDati[1].cod_ricetta);
                    printf("%s\n", recuperoDati[1].nome);
                    printf("%s\n", recuperoDati[1].cognome);
                    printf("%s\n", recuperoDati[1].data_visita);
                    printf("Invio datirecuparti al server CUP\n");
                    //invio dati
                    FullWrite(conn_fd, recuperoDati, sizeof(recuperoDati));
                }
                else if (strcmp(scelta, "3") == 0)
                {

                    bzero(conferma, 4);

                    printf("\navvio procedura di recupero informazione visita prenotata\n");

                    //apertura del file del reparto
                    file = fopen("reparto1.txt", "r+");
                    //letturera numero prenotazioni esistenti
                    fscanf(file, "%s", numprenotazioni);

                    //lettura dai prenotazione esistenti
                    for (i = 1; i <= atoi(numprenotazioni); i++)
                    {
                        fscanf(file, "%s %s %s %s\n", prenotazione[i].nome, prenotazione[i].cognome, prenotazione[i].data_visita, prenotazione[i].cod_ricetta);
                    }

                    fclose(file);
                    unlink("reparto1.txt");

                    //lettura codice prenotazione
                    bzero(cod_prenotazione, 5);
                    FullRead(conn_fd, cod_prenotazione, sizeof(cod_prenotazione));
                    printf("\n il codice prenotazione e' R1%s\n", cod_prenotazione);

                    strcpy(conferma, "no");
                    if (atoi(prenotazione[atoi(cod_prenotazione)].cod_ricetta) != 0)
                    {
                        bzero(conferma, 4);
                        strcpy(conferma, "si");

                        //cancellazione della prenotazione
                        for (i = 1; i <= atoi(numprenotazioni); i++)
                        {
                            if (i == atoi(cod_prenotazione))
                            { //salva su file
                                strcpy(prenotazione[i].nome, "---");
                                strcpy(prenotazione[i].cognome, "---");
                                strcpy(prenotazione[i].data_visita, "---");
                                strcpy(prenotazione[i].cod_ricetta, "---");
                            }
                        }

                        file1 = fopen("reparto1.txt", "w+");

                        //mi posiziono all'inizio del file
                        fseek(file1, 0, SEEK_SET);
                        fprintf(file1, "%s\n\n", numprenotazioni);

                        //salvataggio delle prenotazioni
                        for (i = 1; i <= atoi(numprenotazioni); i++)
                        {
                            printf("codice prenotazione  %d\n\n", i);
                            //salva su file
                            fprintf(file1, "%s\n", prenotazione[i].nome);
                            fprintf(file1, "%s\n", prenotazione[i].cognome);
                            fprintf(file1, "%s\n", prenotazione[i].data_visita);
                            fprintf(file1, "%s\n\n", prenotazione[i].cod_ricetta);
                        }
                        fclose(file1);
                    }
                    printf("\nconferma :%s\n", conferma);

                    FullWrite(conn_fd, conferma, sizeof(conferma));

                    //exit(1);
                }
                close(conn_fd);
                exit(0);
            }else{
                 close(conn_fd);
            }
        }
    }
    else
    {
        wait(NULL);
    }

    exit(0);
}
