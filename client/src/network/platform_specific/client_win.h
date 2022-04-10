#pragma once
/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <process.h>
#include <winsock2.h>
#include <windows.h>
#include <lmcons.h>
#include <unistd.h>

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
	char cmd[1024];
	STARTUPINFO si;

	if(spSpawned){return;}
	if(optionWorldSeed == 0){
		optionWorldSeed = (int)(time(NULL)&0xFFFF);
	}
	char *exeFile = clientGetServerExecutable();
	printf("Starting '%s'\n",exeFile);
	if(exeFile == NULL || ((exeFile[0] == 20) && (exeFile[1] == 0))){
		fprintf(stderr,"Couldn't find server executable");
		closeSingleplayerServer();
		return;
	}
	snprintf(cmd,sizeof(cmd)-1,"%s -singleplayer -worldSeed=%i -savegame=%s",exeFile,optionWorldSeed,optionSavegame);
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
	lastPing = getTicks();
	clientInit();
}

void closeSingleplayerServer(){
	if(!singleplayer){return;}
	TerminateProcess(pi.hProcess,0);
	spSpawned       = false;
	singlePlayerPID = 0;
	serverSocket    = 0;
}

void clientInit(){
	struct sockaddr_in serv_addr;
	struct hostent *serveraddr;
	int err,yes=1;
	int cTries=10;
	tryWinsockInit();
	if(serverSocket != 0){return;}
	if(singleplayer && (singlePlayerPID == 0)){
		startSingleplayerServer();
		return;
	}
	goodbyeSent = false;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket <= 0){
		serverSocket = 0;
		menuSetError("Error opening socket");
		return;
	}

	serveraddr = gethostbyname(serverName);
	if (serveraddr == NULL) {
		menuSetError("Error, no such host");
		return;
	}

	memset((char *) &serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)serveraddr->h_addr_list[0], serveraddr->h_length);
	serv_addr.sin_port   = htons(serverPort);
	err = setsockopt(serverSocket,IPPROTO_TCP,TCP_NODELAY,(const char *)&yes,sizeof(yes));

	while(connect(serverSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0){
		err = WSAGetLastError();
		if(err == WSAEINPROGRESS){break;}
		if(--cTries > 0){
			usleep(1000);
			break;
		}
		menuSetError("Error connecting to host");
		return;
	}
	err = ioctlsocket(serverSocket, FIONBIO, (unsigned long *) &yes);
	sendBufLen              = 0;
	sendBufSent             = 0;
	sentBytesCurrentSession = 0;
	recvBytesCurrentSession = 0;
	recvUncompressedBytesCurrentSession = 0;
	clientGreetServer();
}

void clientRead(){
	if(serverSocket <= 0){return;}
	for(int i=4;i>0;i--){
		const int len = recv(serverSocket,(void *)(recvBuf + recvBufLen),sizeof(recvBuf) - recvBufLen, 0);
		if(len == SOCKET_ERROR){
			const int err = WSAGetLastError();
			if(err == WSAEWOULDBLOCK){break;}
			if(err == WSAEINPROGRESS){break;}
			clientFree();
			return;
		}
		if(len == 0){break;}
		recvBufLen += len;
		recvBytesCurrentSession += len;
	}
}

void clientWrite(){
	if(serverSocket <= 0){return;}
	for(int i=4;i>0;i--){
		const int ret = send(serverSocket,(void *)(sendBuf+sendBufSent),sendBufLen-sendBufSent, 0);
		if(ret == SOCKET_ERROR){
			const int err = WSAGetLastError();
			if(err == WSAEWOULDBLOCK){break;}
			if(err == WSAEINPROGRESS){break;}
			clientFree();
			return;
		}else if(ret == 0){break;}
		sendBufSent += ret;
		sentBytesCurrentSession += ret;
	}
	if(sendBufSent >= sendBufLen){
		sendBufSent = 0;
		sendBufLen  = 0;
	}
}

void clientFreeSpecific(){
	if(serverSocket > 0){
		closesocket(serverSocket);
		serverSocket = 0;
	}
	spSpawned = false;
	singlePlayerPID = 0;
}
