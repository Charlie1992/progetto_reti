#include "Client_lib.h"

int main(int argc, char *argv[])
{
    char recvline[1024], sendline[1024];
    int socket;
    struct sockaddr_in servaddr;


    int i = 0;
    char scelta[2], reparto[2], kbuffer[2], data_scelta[11], conferma[4], nome[15], cognome[15], cod_ricetta[10], cod_prenotazione[5];
    LISTA_TIPOLOGIA_VISITE lista_tipologia_visite[100];
    char data_diponibili[100][20];
    char charflag[2];



    //*************comunicazione con il server centrale ****************************

    // AF_INET : famiglia di ip che stiamo considerando (IP4v)
    // SOCK_STREAM : canale bidirezionale, sequenziale affidabile che opera su
    // connessione. I dati vengono ricevuti e trasmessi come un
    // flusso continuo
    // 0 : tipo di protocollo
    socket = SOCKET(AF_INET, SOCK_STREAM, 0);

    //salva la famiglia di ip che stiamo considerando
    servaddr.sin_family = AF_INET;
    // salva la potra in formato network order
    servaddr.sin_port = htons(client_cup_port);

    // controlla se in input non c'è il ip mi conetto con il locahost
    if (argc == 1)
    {
        INET_PTON(AF_INET, localhost, &servaddr);
        // altrimenti mi connetto al indirizzo ip passato da riga di comando
    }
    else
    {
        INET_PTON(AF_INET, argv[1], &servaddr);
    }

    CONNECTION(socket, servaddr, sizeof(servaddr));

    bzero(scelta, 2);
    bzero(reparto, 2);
    bzero(kbuffer, 2);
    bzero(data_scelta, 11);
    bzero(nome, 15);
    bzero(cognome, 15);
    bzero(cod_ricetta, 10);
    
  
    printf("*********Menu***********\n");
    printf("1.Prenota visita\n");
    printf("2.Informazione visita prenotata\n");
    printf("3.Cancella visita\n");
    printf("4.Esci\n");
    scanf("%s", scelta);

    //inzio prpcedure di prenotazione
    if (strcmp(scelta, "1") == 0)
    {
        printf("inzio prpcedure di prenotazione\n");
        printf("Scelta %s\n", scelta);
        //invio scelta la server CUP
        FullWrite(socket, scelta, sizeof(scelta));

        //letture delle possibili prenotazioni
        FullRead(socket, lista_tipologia_visite, sizeof(lista_tipologia_visite));
        for (i = 1; i < conta_ltv(lista_tipologia_visite); i++)
        {
            printf("Id:%d Nome: %s \n", lista_tipologia_visite[i].id_tv, lista_tipologia_visite[i].nome_tv);
        }

        printf("Segli reparto ");
        scanf("%s", reparto);
        //se il reparto e compreso tra 1 e il numero tot delle possibili visite
        if (atoi(reparto) >= 1 && atoi(reparto) <= conta_ltv(lista_tipologia_visite))
        {
            //invio scelta al server cup
            printf("Reparto %s\n", reparto);
            FullWrite(socket, reparto, sizeof(reparto));

            //leggo date e dim di date disponibili
            FullRead(socket, kbuffer, sizeof(kbuffer));
            FullRead(socket, data_diponibili, sizeof(data_diponibili));

            printf("Data disponibili:\n");
            for (i = 1; i <= atoi(kbuffer); i++)
            {
                printf("%s \n", data_diponibili[i]);
            }
            do
            {
                printf("Inserire data:");
                scanf("%s", data_scelta);

                //invio data scelta al server centrale
                FullWrite(socket, data_scelta, sizeof(data_scelta));

                //leggo la conferma dal server cup
                FullRead(socket, conferma, sizeof(conferma));
                printf("Conferma %s\n", conferma);
            } while (strcmp(conferma, "si") != 0);

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

            bzero(cod_prenotazione, 5);
            FullRead(socket, cod_prenotazione, sizeof(cod_prenotazione));
            printf("Il codice delle prenotazione e' %s\n", cod_prenotazione);
            exit(1);

            // se non e compreso
        }
        else
        {
            printf("Errore reparto inesistente.\n");
            exit(-1);
        }
    }
    else if (strcmp(scelta, "2") == 0)
    {
        printf("\nAvvio procedura di recupero informazioni della visita prenotata\n");

        //invio scelta la server CUP
        FullWrite(socket, scelta, sizeof(scelta));
        do
        {
            printf("Inserisci il codice della prenotazione :");
            scanf("%s", cod_prenotazione);
            FullWrite(socket, cod_prenotazione, sizeof(cod_prenotazione));
            FullRead(socket, charflag, sizeof(charflag));
        } while (strcmp(charflag, "0") == 0);
    }
    else
    {
        exit(0);
    }

    exit(0);
}
