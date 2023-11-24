/*
* file:			t5c2324trebino03CSclient.cpp
* progetto:		t5c2324trebino03CSclient.vcxproj
* soluzione:	t5c2324trebino03CS.sln
* autore:		Samuele Davide Ferraro
* data:			03/11/2023
* scopo:		primi passi con libreria winsock (lato client )
* note tecniche:
* analisi / stategie risolutive / protocollo:
* pseudocodice:
	Initialize Winsock.
	Create a socket.
	Connect to the server.
	Send and receive data.
	Disconnect.
*/

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#include "../t5c2324ferraro03CS.h"

int main(int argc, char* argv[])
{
	int iResult;

	struct addrinfo* result = NULL;
	struct addrinfo* ptr = NULL;
	struct addrinfo hints;

	//Create a WSADATA object called wsaData.
	WSADATA wsaData;		//istanziazione dell'oggetto

	//Call WSAStartup and return its value as an integer and check for errors.
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	//azzeramento di tutti i byte della struct hints
	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_INET;		//definito IPV4
	hints.ai_socktype = SOCK_STREAM;	//definisce l'uso come stream
	hints.ai_protocol = IPPROTO_TCP;	//protocollo TCP
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 2;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(
		ptr->ai_family,
		ptr->ai_socktype,
		ptr->ai_protocol
	);

	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 3;
	}

	// Connect to server ossia svolgimento dell'handshacking con in server richiesto
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("errore di connessione (handshacking) con il server %s [%s]\n", argv[1], DEFAULT_PORT);
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		return 4;
	}

	printf("la connessione (handshacking) con il server %s [%s] ha avuto successo!!\n", argv[1], DEFAULT_PORT);

	//invio di una stringa verso il server

	int recvbuflen = DEFAULT_BUFLEN;


	char sendbuf[DEFAULT_BUFLEN];
	char recvbuf[DEFAULT_BUFLEN];

	strcpy_s(sendbuf, DEFAULT_BUFLEN, STR_PRESENTAZIONE_CLIENT);
	//CIFRAZIONE
	cifraCesare(sendbuf, SHIFT);

	// Send il contenuto del buffer , valorizzato con la stringa di presentazione del client cifrato
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 5;
	}
	printf("Inviati al server %d caratteri\n", iResult);

	//attesa ricezione della presentazione del server
	iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN - 1, 0);
	if (iResult <= 0)
	{
		printf("errore di ricezione:%d", iResult);
		shutdown(ConnectSocket, SD_BOTH);
		closesocket(ConnectSocket);
		WSACleanup();
		return 6;
	}
	cifraCesare(recvbuf, -SHIFT);
	if (strcmp(recvbuf, STR_PRESENTAZIONE_SERVER) != 0)
	{
		printf("errore: il server non e' quello giusto!\n");
		shutdown(ConnectSocket, SD_BOTH);
		closesocket(ConnectSocket);
		WSACleanup();
		return 7;
	}
	

	//send del nome del file da ricevere
	strcpy_s(sendbuf, DEFAULT_BUFLEN, argv[2]);
	cifraCesare(sendbuf, SHIFT);
	iResult = send(ConnectSocket, sendbuf, DEFAULT_BUFLEN - 1, 0);
	if (iResult <= 0)
	{
		printf("errore di ricezione:%d", iResult);
		shutdown(ConnectSocket, SD_BOTH);
		closesocket(ConnectSocket);
		WSACleanup();
		return 8;
	}

	//risposta esito nome file + se si grandezza 
	iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN - 1, 0);
	if (iResult <= 0)
	{
		printf("errore di ricezione:%d", iResult);
		shutdown(ConnectSocket, SD_BOTH);
		closesocket(ConnectSocket);
		WSACleanup();
		return 9;
	}

	//controllo esito
	if (strcmp(recvbuf, FILE_NON_ESISTENTE) == 0)
	{
		printf("richiesta file fallita: file inesistente\n");
		shutdown(ConnectSocket, SD_BOTH);
		closesocket(ConnectSocket);
		WSACleanup();
	}
	printf("%s\n", recvbuf);	//debug con esito ricerca file del server

	//ricezione file
	int nByteRicevuti = 0;
	int byteTotali = atoi(recvbuf);
	char buffer[DEFAULT_BUFLEN];
	while (nByteRicevuti != byteTotali)
	{
		iResult = recv(ConnectSocket, recvbuf, BYTE_INVITATI_FILE, 0);
		if (iResult <= 0)
		{
			printf("errore di ricezione:%d\n", iResult);
			system("Pause");
			shutdown(ConnectSocket, SD_BOTH);
			closesocket(ConnectSocket);
			WSACleanup();
			return 10;
		}

		recvbuf[iResult] = '\0';
	
		printf("%s", recvbuf);

		nByteRicevuti = nByteRicevuti + iResult + 1;

 	}
	printf("\n");

	system("Pause");
	printf("il client termina!\n");
	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
}

