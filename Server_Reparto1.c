#include "Server_lib.h"

int main(int argc, char *argv[])
{

    int list_fd, conn_fd, logging = 1;
    struct sockaddr_in servaddr, client;
    char buffer[1024];
    char buffrd[1024], buffwr[1024];
    int len;
    pid_t pid;

    FILE *file;
    char scelta[2], numprenotazioni[2], kbuffer[2], conferma[4], cod_prenotazione[4];
    int i = 0, j = 0, count_datap = 0, k = 1;

    char data_diponibili[100][20];
    char lista_date[100][20], data_scelta[11];

    PRENOTAZIONE prenotazione[100];
    PRENOTAZIONE recuperoDati[2];
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
    // servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    // funione per memeorizzare la porta sulla quale il serve
    // ci risponde. Questo valore sara salvato nel campo sin_port
    servaddr.sin_port = htons(cup_server_reparto1_port);

    // controlla se in input non c'è il ip mi conetto con il locahost
    if (argc == 1)
    {
        INET_PTON(AF_INET, localhost, &servaddr);
        // altrimenti mi connetto al indirizzo ip passato da riga di camndo
    }
    else
    {
        INET_PTON(AF_INET, argv[1], &servaddr);
    }

    CONNECTION(list_fd, servaddr, sizeof(servaddr));
    while (1)
    {
        bzero(scelta, 2);
        bzero(numprenotazioni, 2);
        bzero(data_scelta, 11);

        //ricevo scelta del server cup
        FullRead(list_fd, scelta, sizeof(scelta));
        //se 1 avvivo procedura di prenotazione visita
        if (strcmp(scelta, "1") == 0)
        {

            // fork per gestire le connesione
            if ((pid = fork()) < 0)
            {
                perror("fork error");
                exit(-1);
            }

            // se sono il filgio
            if (pid == 0)
            {
                k = 1;
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
                    if (count_datap == 0)
                    {
                        strcpy(data_diponibili[k], lista_date[i]);
                        k++;
                    }
                }

                printf("Data disponibili:\n");
                for (i = 1; i <= k; i++)
                {
                    printf("%s \n", data_diponibili[i]);
                }

                //invio date dipsponibili e dim  al server cup
                sprintf(kbuffer, "%d", k);
                FullWrite(list_fd, kbuffer, sizeof(kbuffer));
                FullWrite(list_fd, data_diponibili, sizeof(data_diponibili));

               do{
                    FullRead(list_fd, data_scelta, sizeof(data_scelta));
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
                    FullWrite(list_fd, conferma, sizeof(conferma));
                } while (strcmp(conferma, "si") != 0);

                FullRead(list_fd, cod_prenotazione, sizeof(cod_prenotazione));
                printf("Il codice prenotazione e' R1%s\n", cod_prenotazione);

                FullRead(list_fd, prenotazione, sizeof(prenotazione));
                printf("Dati prenotazione: \n ");
                printf("Nome: %s \n ", prenotazione[atoi(cod_prenotazione)].nome);
                printf("Cognome: %s \n ", prenotazione[atoi(cod_prenotazione)].cognome);
                printf("Data: %s \n ", prenotazione[atoi(cod_prenotazione)].data_visita);
                printf("Codice ricetta: %s \n ", prenotazione[atoi(cod_prenotazione)].cod_ricetta);
            }
            else
            {
                wait(NULL);
            }
            //avvio procedura di recupero informazione visita prenotata
        }
        else if (strcmp(scelta, "2") == 0)
        {
            //fork per gestire la connessione
            if ((pid = fork()) < 0)
            {
                perror("fork error");
                exit(-1);
            }
            if (pid == 0)
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
                    bzero(cod_prenotazione, 4);
                    FullRead(list_fd, cod_prenotazione, sizeof(cod_prenotazione));
                    printf("\n il codice prenotazione e' R1%s\n", cod_prenotazione);

                    printf("Codice %d \n", atoi(prenotazione[atoi(cod_prenotazione)].cod_ricetta));
                     strcpy(conferma, "no");
                    if (atoi(prenotazione[atoi(cod_prenotazione)].cod_ricetta) != 0)                   
                    {
                        bzero(conferma, 4);
                        strcpy(conferma, "si");
                    }
                   
                    printf("\nconferma :%s\n", conferma);

                    FullWrite(list_fd, conferma, sizeof(conferma));
                    strcpy(recuperoDati[1].nome,prenotazione[atoi(cod_prenotazione)].nome);
                    strcpy(recuperoDati[1].cognome,prenotazione[atoi(cod_prenotazione)].cognome);
                    strcpy(recuperoDati[1].data_visita,prenotazione[atoi(cod_prenotazione)].data_visita);
                    strcpy(recuperoDati[1].cod_ricetta,prenotazione[atoi(cod_prenotazione)].cod_ricetta);
                    printf("%s\n",recuperoDati[1].cod_ricetta);
                    printf("%s\n",recuperoDati[1].nome);
                    printf("%s\n",recuperoDati[1].cognome);
                    printf("%s\n",recuperoDati[1].data_visita);
                    printf("Invio datirecuparti al server CUP\n");
                    //invio dati 
                    FullWrite(list_fd,recuperoDati,sizeof(recuperoDati));
                    exit(0);
            }
            else
            {
                wait(NULL);
            }
        }
    }

    exit(0);
}
