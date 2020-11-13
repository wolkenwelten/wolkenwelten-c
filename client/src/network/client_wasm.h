#pragma once

#include <arpa/inet.h>
#include <emscripten.H>
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

void clientGetName(){
	snprintf(playerName,sizeof(playerName),"WebPlayer");
	playerName[sizeof(playerName)-1]=0;
}

bool fileExists(const char *fn){
	struct stat buffer;
	return (stat(fn, &buffer) == 0);
}


void startSingleplayerServer(){
	spServer = emscripten_create_worker("server.js");
	serverSocket = -1;
	return;
}

void closeSingleplayerServer(){
	if(spServer != 0){
		emscripten_destroy_worker(spServer);
	}
	serverSocket = 0;
	return;
}

void clientFreeSpecific(){
	if(serverSocket > 0){
		close(serverSocket);
		serverSocket = 0;
	}
}

void clientFreeRetry(){
	if(serverSocket > 0){
		close(serverSocket);
		serverSocket = 0;
	}
	usleep(1000);
}

void clientInit(){
	struct sockaddr_in serv_addr;
	struct hostent *serveraddr;
	int err,yes=1;
	if(serverSocket > 0){return;}
	if(singleplayer && (singlePlayerPID == 0)){
		startSingleplayerServer();
		return;
	}
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

void clientRead(){
	if(serverSocket <= 0){clientInit();}
	if(serverSocket <= 0){return;}
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

void clientWrite(){
	if(serverSocket <= 0){clientInit();}
	if(serverSocket <= 0){return;}
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
