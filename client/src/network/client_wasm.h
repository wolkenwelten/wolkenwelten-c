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

#include <arpa/inet.h>
#include <emscripten.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unistd.h>

int serverSocket = 0;
worker_handle spServer;
bool wasmInFlight = false;

void clientGetName(){
	snprintf(playerName,sizeof(playerName),"WASM_Player");
}

bool fileExists(const char *fn){
	struct stat buffer;
	return (stat(fn, &buffer) == 0);
}


void startSingleplayerServer(){
	spServer = emscripten_create_worker("server.js");
	serverSocket = 1;
	singlePlayerPID = 123;
	return;
}

void closeSingleplayerServer(){
	if(singlePlayerPID != 0){
		emscripten_destroy_worker(spServer);
	}
	serverSocket = 0;
	singlePlayerPID = 0;
	return;
}

void clientFreeSpecific(){
	if(singlePlayerPID != 0){
		closeSingleplayerServer();
		serverSocket = 0;
		spServer = 0;
	}else if(serverSocket > 0){
		close(serverSocket);
		serverSocket = 0;
	}
}

void clientFreeRetry(){
	clientFreeSpecific();
	usleep(1000);
}

void clientWSInit(){
	struct sockaddr_in serv_addr;
	struct hostent *serveraddr;
	int err,yes=1;
	if(singleplayer){return;}
	++connectionTries;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket <= 0){
		menuSetError("Error opening socket");
		return;
	}

	serveraddr = gethostbyname(serverName);
	if (serveraddr == NULL) {
		menuSetError("Error, no such host");
		return;
	}

	memset((char *) &serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)serveraddr->h_addr_list[0], serveraddr->h_length);
	serv_addr.sin_port        = htons(serverPort);

	while(connect(serverSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		if(errno == EINVAL){
			menuSetError("Error connecting to host EINVAL");
			return;
		}else if(errno == EINPROGRESS){
			if(++connectionTries > 10){
				menuSetError("Error connecting to host EINPROGRESS");
				return;
			}
			break;
		}else if(errno == ECONNREFUSED){
			if(++connectionTries > 10){
				menuSetError("Error connecting to host ECONNREFUSED");
				return;
			}
			usleep(500);
			clientFreeRetry();
			return;
		}
		menuSetError("Error connecting to host ELSE");
		return;
	}
	connectionState = 0;
	connectionTries = 0;
	fcntl(serverSocket, F_SETFL, O_NONBLOCK);
	err = setsockopt(serverSocket,IPPROTO_TCP,TCP_NODELAY,&yes,sizeof(yes));
	sendBufLen              = 0;
	sendBufSent             = 0;
	sentBytesCurrentSession = 0;
	recvBytesCurrentSession = 0;
	recvUncompressedBytesCurrentSession = 0;
	clientGreetServer();
}

void clientSPInitCB(char *data,int size,void *arg){
	(void)data;
	(void)size;
	(void)arg;

	wasmInFlight=false;
}

void clientSPInit(){
	emscripten_call_worker(spServer,"wasmInit",NULL,0,(em_worker_callback_func)clientSPInitCB,NULL);
	wasmInFlight=true;
	connectionState = 1;
	sendBufLen              = 0;
	sendBufSent             = 0;
	sentBytesCurrentSession = 0;
	recvBytesCurrentSession = 0;
	recvUncompressedBytesCurrentSession = 0;
	clientGreetServer();
}

void clientInit(){
	if(serverSocket > 0){return;}
	if(singleplayer && (singlePlayerPID == 0)){
		startSingleplayerServer();
	}
	if(singlePlayerPID == 0){
		clientWSInit();
	}else{
		clientSPInit();
	}
}

void clientWSRead(){
	if(singleplayer){return;}
	for(int i=64;i>0;i--){
		const int len = recv(serverSocket,recvBuf + recvBufLen,sizeof(recvBuf) - recvBufLen, 0);
		if(len < 0){
			if(errno == EAGAIN){break;}
			clientFree();
			return;
		}else if(len == 0){break;}
		recvBufLen += len;
		recvBytesCurrentSession += len;
	}
}

void clientRead(){
	if(serverSocket <= 0){clientInit();}
	if(serverSocket <= 0){return;}
	if(singlePlayerPID == 0){
		clientWSRead();
	}
}

void clientWSWrite(){
	if(singleplayer){return;}
	for(int i=64;i>0;i--){
		const int ret = write(serverSocket,sendBuf+sendBufSent,sendBufLen-sendBufSent);
		if(ret < 0){
			if(errno == EAGAIN){break;}
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

void clientSPTranceiveCB(char *data,int size,void *arg){
	(void)arg;
	if((recvBufLen + size) > sizeof(recvBuf)){
		fprintf(stderr,"clientSPTranceive buffer full!!!\n");
		clientParse();
	}
	memcpy(&recvBuf[recvBufLen],data,size);
	recvBufLen += size;
	recvBytesCurrentSession += size;
	wasmInFlight = false;
}

void clientSPWrite(){
	if(wasmInFlight){return;}
	wasmInFlight = true;

	sentBytesCurrentSession += sendBufLen;
	emscripten_call_worker(spServer,"wasmTranceive",(char *)sendBuf,sendBufLen,(em_worker_callback_func)clientSPTranceiveCB,NULL);
	sendBufLen = 0;
}

void clientWrite(){
	if(serverSocket <= 0){clientInit();}
	if(serverSocket <= 0){return;}
	if(singlePlayerPID == 0){
		clientWSWrite();
	}else {
		clientSPWrite();
	}
}
