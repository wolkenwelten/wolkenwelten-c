#pragma once

#include <process.h>
#include <winsock2.h>
#include <windows.h>
#include <lmcons.h>

bool signalHandlerBound = false;
int  serverSocket       = 0;
bool winsockInit        = false;
bool spSpawned          = false;
PROCESS_INFORMATION pi;

void tryWinsockInit(){
	int err;
	if(!winsockInit){
		WSADATA wsaData;
		err = WSAStartup(MAKEWORD(2,2), &wsaData);
		if(err != 0){
			fprintf(stderr,"WSAStartup failed with error %i\n", WSAGetLastError());
			return;
		}
		winsockInit = true;
	}
}

void clientGetName(){
	TCHAR infoBuf[28];
	DWORD bufCharCount = sizeof(infoBuf);
	if(!GetUserName(infoBuf,&bufCharCount)){
		tryWinsockInit();
		gethostname(playerName,28);
	}else{
		snprintf(playerName,sizeof(playerName)-1,"%s",infoBuf);
		playerName[sizeof(playerName)-1]=0;
	}
}


void startSingleplayerServer(){
	static char cmd[128];
	STARTUPINFO si;

	if(spSpawned){return;}
	if(optionWorldSeed == 0){
		optionWorldSeed = (int)(time(NULL)&0xFFFF);
	}
	snprintf(cmd,sizeof(cmd)-1,"wolkenwelten-server.exe -singleplayer -worldSeed=%i -savegame=%s",optionWorldSeed,optionSavegame);
	ZeroMemory( &si, sizeof(si) );
	ZeroMemory( &pi, sizeof(pi) );
	si.cb = sizeof(si);
	// tell the application that we are setting the window display
	// information within this structure
	si.dwFlags = STARTF_USESHOWWINDOW;
	// set the window display to HIDE
	si.wShowWindow = SW_HIDE;

	if(CreateProcess( NULL,   // No module name (use command line)
		cmd,            // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory
		&si,            // Pointer to STARTUPINFO structure
		&pi )){         // Pointer to PROCESS_INFORMATION structure
		spSpawned = true;
		singlePlayerPID = pi.dwProcessId;
	}
	strncpy(serverName,"localhost",sizeof(serverName)-1);
}

void closeSingleplayerServer(){
	if(!singleplayer){return;}
	TerminateProcess(pi.hProcess,0);
	spSpawned = false;
	singlePlayerPID = 0;
	serverSocket = 0;
}

void clientInit(){
	struct sockaddr_in serv_addr;
	struct hostent *serveraddr;
	int err,yes=1;
	tryWinsockInit();

	if(serverSocket != 0){return;}
	if(singleplayer && (singlePlayerPID == 0)){
		startSingleplayerServer();
		return;
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket <= 0){
		menuSetError("Error opening socket");
		gameRunning = false;
		serverSocket = 0;
		return;
	}

	serveraddr = gethostbyname(serverName);
	if (serveraddr == NULL) {
		fprintf(stderr,"ERROR, no such host '%s'\n",serverName);
		clientFree();
		menuSetError("Error, no such host");
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
			menuSetError("Error connecting to host");
			gameRunning = false;
			return;
		}
	}
	err = ioctlsocket(serverSocket, FIONBIO, (unsigned long *) &yes);
	sendBufLen              = 0;
	sendBufSent             = 0;
	sentBytesCurrentSession = 0;
	recvBytesCurrentSession = 0;
	recvUncompressedBytesCurrentSession = 0;
	clientGreetServer();
}

void clientFreeSpecific(){
	if(serverSocket > 0){
		closesocket(serverSocket);
		serverSocket = 0;
	}
	closeSingleplayerServer();
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
			if(err == WSAEWOULDBLOCK){break;}
			if(err == WSAEINPROGRESS){break;}
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
