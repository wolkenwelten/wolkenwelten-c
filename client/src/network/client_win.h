#pragma once

#include <process.h>
#include <winsock2.h>

bool signalHandlerBound = false;
int  serverSocket       = 0;
bool winsockInit        = false;
bool spSpawned          = false;

void clientGetName(){
	gethostname(playerName,28);
}

void startSingleplayerServer(){
	char seed[64];
	if(spSpawned){return;}
	if(optionWorldSeed == 0){
		optionWorldSeed = (int)(time(NULL)&0xFFFF);
	}
	snprintf(seed,sizeof(seed)-1,"-worldSeed=%i",optionWorldSeed);
	_spawnl( _P_DETACH , "wolkenwelten-server.exe", seed, "-singleplayer=1",(const char *)NULL);
	spSpawned = true;
	singlePlayerPID = 1;
	strncpy(serverName,"localhost",sizeof(serverName)-1);
}

void closeSingleplayerServer(){
	spSpawned = false;
	singlePlayerPID = 0;
}

void clientInit(){
	struct sockaddr_in serv_addr;
	struct hostent *serveraddr;
	int err,yes=1;
	if(!winsockInit){
		WSADATA wsaData;
		err = WSAStartup(MAKEWORD(2,2), &wsaData);
		if(err != 0){
			fprintf(stderr,"WSAStartup failed with error %i\n", WSAGetLastError());
			return;
		}
		winsockInit = true;
	}

	if(serverSocket != 0){return;}
	if(singleplayer && (singlePlayerPID == 0)){
		startSingleplayerServer();
		return;
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket <= 0){
		menuError = "Error opening socket";
		gameRunning = false;
		serverSocket = 0;
		return;
	}

	serveraddr = gethostbyname(serverName);
	if (serveraddr == NULL) {
		fprintf(stderr,"ERROR, no such host '%s'\n",serverName);
		clientFree();
		menuError = "Error, no such host";
		gameRunning = false;
		return;
	}

	memset((char *) &serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)serveraddr->h_addr_list[0], serveraddr->h_length);
	serv_addr.sin_port        = htons(serverPort);


	while(connect(serverSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0){
		err = WSAGetLastError();
		if(err == WSAEINPROGRESS){break;}
		if(!singleplayer){
			clientFree();
			menuError = "Error connecting to host";
			gameRunning = false;
			return;
		}
	}
	err = ioctlsocket(serverSocket, FIONBIO, (unsigned long *) &yes);
	sendBufLen  = 0;
	sendBufSent = 0;
	clientGreetServer();
}

void clientFree(){
	if(serverSocket > 0){
		fprintf(stderr,"Client Free!\n");
		closesocket(serverSocket);
		serverSocket = 0;
		menuError = "Connection closed";
		if(singleplayer){
			*serverName = 0;
		}
	}
}

void clientRead(){
	int len = 1;
	if(serverSocket <= 0){clientInit();}
	if(serverSocket <= 0){return;}
	while(len > 0){
		len = recv(serverSocket,(void *)(recvBuf + recvBufLen),sizeof(recvBuf) - recvBufLen, 0);
		if(len == SOCKET_ERROR){
			const int err = WSAGetLastError();
			if(err == WSAEWOULDBLOCK){return;}
			if(err == WSAEINPROGRESS){return;}
			fprintf(stderr,"ERROR receiving: %i\n",err);
			clientFree();
		}else{
			recvBufLen += len;
			recvBytesCurrentSession += len;
		}
	}
}

void clientSendAllToServer(){
	if(serverSocket <= 0){clientInit();}
	if(serverSocket <= 0){return;}

	while(sendBufSent < sendBufLen){
		const int ret = send(serverSocket,(void *)(sendBuf+sendBufSent),sendBufLen-sendBufSent, 0);
		if(ret == SOCKET_ERROR){
			const int err = WSAGetLastError();
			if(err == WSAEWOULDBLOCK){return;}
			if(err == WSAEINPROGRESS){return;}
			fprintf(stderr,"ERROR sending: %i\n",err);
			clientFree();
			return;
		}else{
			sendBufSent += ret;
			sentBytesCurrentSession += ret;
		}
	}
	sendBufSent = 0;
	sendBufLen  = 0;
}
