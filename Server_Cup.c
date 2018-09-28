#include "Server_lib.h"

int main(int argc, char *argv[])
{
    int list_fd, conn_fd,
        list_fd_server1, conn_fd_server1,
        logging = 1, len, enable = 1, i = 0;

    struct sockaddr_in servaddr, serv1addr, client;
    char buffer[1024];
    pid_t pid;
    char buffrd[1024], buffwr[1024];

    char scelta[2], reparto[2], kbuffer[2],
        data_scelta[11], conferma[4], nome[15],
        cognome[15], cod_ricetta[10],
        numprenotazioni[2], cod_prenotazione[4];

    char cod_prenotazioneReparto[5];
    int flag = 0;
    char charflag[2];
    char cod_reparto[3];
    char cod_pret[8];

    LISTA_TIPOLOGIA_VISITE lista_tipologia_visite[100];
    char data_diponibili[100][20];
    PRENOTAZIONE prenotazione[100];
    PRENOTAZIONE recuperoDati[2];

    FILE *file;

    ListaNomiVisite(lista_tipologia_visite);

    //****************************comunicazione con il server 1********************
    list_fd_server1 = SOCKET(AF_INET, SOCK_STREAM, 0);
    serv1addr.sin_family = AF_INET;
    serv1addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv1addr.sin_port = htons(cup_server_reparto1_port);
    setsockopt(list_fd_server1, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    BIND(list_fd_server1, serv1addr);
    LISTEN(list_fd_server1, 1024);
    conn_fd_server1 = ACCEPT(list_fd_server1, (struct sockaddr *)NULL, NULL);

    //****************************comunicazione con il cliente ***********************
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
    servaddr.sin_port = htons(client_cup_port);
    //IMPOSTA SOCKETS IN MODO DA POTER RIUTILIZZARE L'INDIRIZZO IP
    setsockopt(list_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    BIND(list_fd, servaddr);
    LISTEN(list_fd, 1024);

    while (1)
    {
        len = sizeof(client);
        conn_fd = ACCEPT(list_fd, (struct sockaddr *)&client, &len);

        // fork per gestire le connesione
        if ((pid = fork()) < 0)
        {
            perror("fork error");
            exit(-1);
        }

        // se sono il filgio
        if (pid == 0)
        {
            // chiude list_fd interagisce con il client
            // tramite la connessione con conn_sd*
            close(list_fd);

            //clear dei buffer
            bzero(scelta, 2);
            bzero(reparto, 2);
            bzero(data_scelta, 11);
            bzero(nome, 15);
            bzero(cognome, 15);
            bzero(cod_ricetta, 10);
            bzero(numprenotazioni, 2);
            bzero(cod_prenotazione, 4);
            bzero(cod_reparto, 3);
            bzero(cod_pret, 8);

            //lettura scelta dal client
            FullRead(conn_fd, scelta, sizeof(scelta));

            //Avvio procedura di prenotazione
            if (strcmp(scelta, "1") == 0)
            {
                printf("Avvio procedura di prenotazione.\n");

                //invio al cliente la lista delle possibili prenotazioni
                printf("invio al cliente la lista delle possibili prenotazioni\n");
                FullWrite(conn_fd, lista_tipologia_visite, sizeof(lista_tipologia_visite));

                //recezione scelta reparto
                FullRead(conn_fd, reparto, sizeof(reparto));
                printf("Reparto scelto %d \n", atoi(reparto));

                //invio  scelta del menu al reparto selezionato
                if (atoi(reparto) == 1)
                {
                    FullWrite(conn_fd_server1, scelta, sizeof(scelta));

                    //ricevo date e dim delle date disponibili
                    FullRead(conn_fd_server1, kbuffer, sizeof(kbuffer));
                    printf("Valore di k %s \n", kbuffer);
                    FullRead(conn_fd_server1, data_diponibili, sizeof(data_diponibili));
                    printf("Data disponibili:\n");
                    for (i = 1; i <= atoi(kbuffer); i++)
                    {
                        printf("%s \n", data_diponibili[i]);
                    }

                    //invio date e dim disponibili al cliente
                    FullWrite(conn_fd, kbuffer, sizeof(kbuffer));
                    FullWrite(conn_fd, data_diponibili, sizeof(data_diponibili));
                    do
                    {
                        //ricevo data dal cliente
                        FullRead(conn_fd, data_scelta, sizeof(data_scelta));
                        printf("Data scelta %s \n", data_scelta);
                        //invio data scelta al server centrale
                        FullWrite(conn_fd_server1, data_scelta, sizeof(data_scelta));

                        //ricevo la conferma del server reparto
                        FullRead(conn_fd_server1, conferma, sizeof(conferma));
                        printf("Conferma %s\n", conferma);
                        FullWrite(conn_fd, conferma, sizeof(conferma));
                    } while (strcmp(conferma, "si") != 0);
                    strcpy(cod_prenotazioneReparto, "R1");
                    //lettura dei dati dal cliente
                    FullRead(conn_fd, nome, sizeof(nome));
                    FullRead(conn_fd, cognome, sizeof(cognome));
                    FullRead(conn_fd, cod_ricetta, sizeof(cod_ricetta));
                    printf("nome %s \n", nome);
                    printf("cognome %s \n", cognome);
                    printf("cod ricetta %s \n", cod_ricetta);
                    printf("Data %s\n", data_scelta);

                    //apertura del file del reparto
                    file = fopen("reparto1.txt", "r+");

                    //letturera numero prenotazioni esistenti
                    fscanf(file, "%s", numprenotazioni);

                    fseek(file, 0, SEEK_SET);
                    sprintf(cod_prenotazione, "%d", atoi(numprenotazioni) + 1);
                    fprintf(file, "%s \n\n", cod_prenotazione);

                    strcpy(prenotazione[atoi(numprenotazioni) + 1].nome, nome);
                    strcpy(prenotazione[atoi(numprenotazioni) + 1].cognome, cognome);
                    strcpy(prenotazione[atoi(numprenotazioni) + 1].data_visita, data_scelta);
                    strcpy(prenotazione[atoi(numprenotazioni) + 1].cod_ricetta, cod_ricetta);

                    fseek(file, 0, SEEK_END);
                    fprintf(file, "\n%s\n", nome);
                    fprintf(file, "%s\n", cognome);
                    fprintf(file, "%s\n", data_scelta);
                    fprintf(file, "%s", cod_ricetta);

                    fclose(file);

                    strcat(cod_prenotazioneReparto, cod_prenotazione);
                    printf("\n codice reparto %s", cod_prenotazioneReparto);
                    FullWrite(conn_fd, cod_prenotazioneReparto, sizeof(cod_prenotazioneReparto));
                    FullWrite(conn_fd_server1, cod_prenotazione, sizeof(cod_prenotazione));
                    FullWrite(conn_fd_server1, prenotazione, sizeof(prenotazione));
                }
            }
            else if (strcmp(scelta, "2") == 0)
            {
                printf("\n Avvio procedura di recupero informazioni della visita prenotata\n");
                bzero(cod_prenotazione, 4);
                bzero(cod_pret, 8);
                bzero(conferma, 4);

                do
                {
                    FullRead(conn_fd, cod_prenotazione, sizeof(cod_prenotazione));
                    printf("\ncodice prenotazione :%s", cod_prenotazione);
                    flag = controllo(cod_prenotazione);
                    sprintf(charflag, "%d", flag);
                    printf("flag :%s\n", charflag);
                    FullWrite(conn_fd, charflag, sizeof(charflag));
                } while (flag != 1);

                for (i = 0; i < strlen(cod_prenotazione); i++)
                {
                    if (i < 2)
                    {
                        cod_reparto[i] = cod_prenotazione[i];
                    }
                    else
                    {
                        cod_pret[i - 2] = cod_prenotazione[i];
                    }
                }
                printf("\ncod_reparto :%s\n", cod_reparto);
                printf("\ncod_pret :%s\n", cod_pret);

                if (strcmp(cod_reparto, "R1") == 0)
                {
                    printf("\nentro nella dase di conferma dopo R1 \n");
                    FullWrite(conn_fd_server1, scelta, sizeof(scelta));
                    FullWrite(conn_fd_server1, cod_pret, sizeof(cod_pret));
                    bzero(conferma, 4);
                    FullRead(conn_fd_server1, conferma, sizeof(conferma));
                    printf("\nconferma:%s", conferma);
                    FullWrite(conn_fd, conferma, sizeof(conferma));
                }
                else
                {
                    printf("\nReparto closato\n");
                    strcpy(conferma, "no");
                    FullWrite(conn_fd, conferma, sizeof(conferma));
                }

                //lettura dei dati inviati dal server Reaprto
                FullRead(conn_fd_server1, recuperoDati, sizeof(recuperoDati));
                printf("OOOOOOOOOOOOOOOOOOOOO\n");
                printf("%s\n", recuperoDati[1].cod_ricetta);
                printf("%s\n", recuperoDati[1].nome);
                printf("%s\n", recuperoDati[1].cognome);
                printf("%s\n", recuperoDati[1].data_visita);
                printf("Invio dati recuperati al client\n");
 
                //invio dati recuparti al cliente
                FullWrite(conn_fd, recuperoDati, sizeof(recuperoDati));
            }
            else if (strcmp(scelta, "3") == 0)
            {

                printf("\n Avvio procedura di recupero informazioni della visita prenotata\n");
                bzero(cod_prenotazione, 4);
                bzero(cod_pret, 8);
                bzero(conferma, 4);

                do
                {
                    FullRead(conn_fd, cod_prenotazione, sizeof(cod_prenotazione));
                    printf("\ncodice prenotazione :%s", cod_prenotazione);
                    flag = controllo(cod_prenotazione);
                    sprintf(charflag, "%d", flag);
                    printf("flag :%s\n", charflag);
                    FullWrite(conn_fd, charflag, sizeof(charflag));
                } while (flag != 1);

                for (i = 0; i < strlen(cod_prenotazione); i++)
                {
                    if (i < 2)
                    {
                        cod_reparto[i] = cod_prenotazione[i];
                    }
                    else
                    {
                        cod_pret[i - 2] = cod_prenotazione[i];
                    }
                }
                printf("\ncod_reparto :%s\n", cod_reparto);
                printf("\ncod_pret :%s\n", cod_pret);

                  if (strcmp(cod_reparto, "R1") == 0)
                {
                    FullWrite(conn_fd_server1, scelta, sizeof(scelta));
                    FullWrite(conn_fd_server1, cod_pret, sizeof(cod_pret));
                    FullRead(conn_fd_server1, conferma, sizeof(conferma));
                    printf("\nconferma:%s", conferma);
                    FullWrite(conn_fd, conferma, sizeof(conferma));
                }
                else
                {
                    printf("\nReparto closato\n");
                    strcpy(conferma, "no");
                    FullWrite(conn_fd, conferma, sizeof(conferma));
                }
                FullWrite(conn_fd, conferma, sizeof(conferma));

            }
            else
            {
                printf("\nErrore \n");
            }

            close(conn_fd);
            exit(0);
        }
        else
        {
            close(conn_fd);
        }

        logging++;
    }

    exit(0);
}
