/*
* file:			t5c2324trebino03CSserver.cpp
* progetto:		t5c2324trebino03CSserver.vcxproj
* soluzione:	t5c2324trebino03CS.sln
* autore:		Samuele Davide Ferraro
* data:			03/11/2023
* scopo:		primi passi con libreria winsock (lato server )
* note tecniche:
* analisi / stategie risolutive / protocollo:
* pseudocodice:
		Initialize Winsock.
		Create a socket.
		Bind the socket.
		Listen on the socket for a client.
		Accept a connection from a client.
		Receive and send data.
		Disconnect.
*/

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#include "../t5c2324ferraro03CS.h"

size_t strlen_until_newline(const char* str);

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

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET ListenSocket = INVALID_SOCKET;		//socket di riferimento per il server

	// Create a SOCKET for the server to listen for client connections

	ListenSocket = socket(result->ai_family,
		result->ai_socktype,
		result->ai_protocol);

	//Check for errors to ensure that the socket is a valid socket.
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 2;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket,
		result->ai_addr,
		(int)result->ai_addrlen);

	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 3;
	}

	//listen 
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 4;
	}

	printf("il Server rimane in attesa di una connessione TCP\n");

	SOCKET clientSocket;	//socket per la comunicazione con il client che contatterà questo server


	SOCKADDR_IN addr;
	int addrsize = sizeof(addr);

	clientSocket = INVALID_SOCKET;


	// Accept a client socket ossia effettuazione dell'handshacking con il client
	//ClientSocket = accept(ListenSocket, NULL, NULL);


	clientSocket = accept(ListenSocket, (SOCKADDR*)&addr, &addrsize);

	char sIndIPClient[16];		//per memorizzare IP del client

	inet_ntop(AF_INET, &addr.sin_addr, sIndIPClient, sizeof(sIndIPClient));

	printf("ip client = %s\n", sIndIPClient);

	//verifica se l'handshacking ha avuto successo
	if (clientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	printf("il server e' stato contattato dal client TCP con IP = %s\n", sIndIPClient);

	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	char sendbuf[DEFAULT_BUFLEN];

	//inizio del protocollo applicativo 
	//il server inizia a ricevere 


	iResult = recv(clientSocket, recvbuf, DEFAULT_BUFLEN - 1, 0);
	if (iResult <= 0)
	{
		printf("errore di ricezione:%d", iResult);
		shutdown(clientSocket, SD_BOTH);
		closesocket(ListenSocket);
		WSACleanup();
		return 2;
	}

	//decodifica stringa
	cifraCesare(recvbuf, -SHIFT);

	if (strcmp(recvbuf, STR_PRESENTAZIONE_CLIENT) != 0)
	{
		printf("ERRORE presentazione client non riconosciuta!\n");
		shutdown(clientSocket, SD_BOTH);
		closesocket(ListenSocket);
		WSACleanup();
		return 3;
	}
	printf("client riconosciuto!\n");

	strcpy_s(sendbuf, sizeof(sendbuf), STR_PRESENTAZIONE_SERVER);
	cifraCesare(sendbuf, SHIFT);

	iResult = send(clientSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		return 4;
	}

	//ricezione nome file da inviare
	iResult = recv(clientSocket, recvbuf, DEFAULT_BUFLEN - 1, 0);
	if (iResult <= 0)
	{
		printf("errore di ricezione:%d\n", iResult);
		shutdown(clientSocket, SD_BOTH);
		closesocket(ListenSocket);
		WSACleanup();
		return 5;
	}
	//decodifica stringa
	cifraCesare(recvbuf, -SHIFT);;

	printf("%s: nome file\n", recvbuf);	//debug
	//apertura file richiesto
	FILE* fp;
	errno_t err;
	if (err = fopen_s(&fp, recvbuf, "rb") != 0)
	{
		//file richiesto inesistente
		strcpy_s(sendbuf, DEFAULT_BUFLEN, FILE_NON_ESISTENTE);
		iResult = send(clientSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);
		if (iResult == SOCKET_ERROR) 
		{
			printf("send failed: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 6;
		}
		shutdown(clientSocket, SD_BOTH);
		closesocket(ListenSocket);
		WSACleanup();
		return 7;
	}

	//mandare la grandezza del file
	fseek(fp, 0, SEEK_END);
	int nByte = ftell(fp);
	int nByteInviati = 0;
	fseek(fp, 0, SEEK_SET);
	sprintf_s(sendbuf, "%d", nByte);

	//manda grandezza file
	iResult = send(clientSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		return 8;
	}

	//manda file
	char appoggio[DEFAULT_BUFLEN];
	int letto;
	while (nByteInviati != nByte)
	{
		letto = (int)fread(appoggio,sizeof(char), BYTE_INVITATI_FILE, fp);
		appoggio[letto] = '\0';
		sprintf_s(sendbuf, "%s", appoggio); 
		iResult = send(clientSocket, sendbuf, (int)strlen(sendbuf) -1 , 0);
		if (iResult == SOCKET_ERROR)
		{
			printf("send failed: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 8;
		}
		nByteInviati = nByteInviati + (ftell(fp) - nByteInviati);
	}

	printf("il server termina!\n");
	shutdown(clientSocket, SD_BOTH);
	closesocket(ListenSocket);
	WSACleanup();
	return 0;
}

size_t strlen_until_newline(const char* str) 
{
	size_t len = 0;
	while (str[len] != '\0' && str[len] != '\n') 
	{
		len++;
	}
	return len;
}