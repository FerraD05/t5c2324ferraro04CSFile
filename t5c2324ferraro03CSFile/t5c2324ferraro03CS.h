/*
* file:			t5c2324ferraro02CS.h
* soluzione:	t5c2324ferraro02CS.sln
* autore:		Marco Trebino
* data:			03/11/2023
* scopo:		primi passi con libreria winsock
* note tecniche:
* analisi / stategie risolutive / protocollo:
* pseudocodice:
*/

#define DEFAULT_PORT "40000"
#define DEFAULT_BUFLEN 1024

#define BYTE_INVITATI_FILE 540

#define STR_PRESENTAZIONE_CLIENT "sono il client xyz3.5 del server trebino"

#define STR_PRESENTAZIONE_SERVER "bravo il mio client xyz3.5%%%%%%%%%"

#define SHIFT 1

#define FILE_ESISTENTE 
#define FILE_NON_ESISTENTE "no:file inesistente"

void cifraCesare(char* testo, int spostamento) 
{
    int lunghezza = (int)strlen(testo);

    for (int i = 0; i < lunghezza; i++) 
    {
        char carattere = testo[i];
        char alfabeto = (islower(carattere)) ? 'a' : (isupper(carattere)) ? 'A' : '\0';

        if (alfabeto) 
        {
            testo[i] = (carattere - alfabeto + spostamento + 26) % 26 + alfabeto;
        }
        else 
        {
            // Carattere non alfabetico, cifralo in qualche modo (esempio: spostamento di 1)
            testo[i] = carattere + spostamento;
        }
    }
}

