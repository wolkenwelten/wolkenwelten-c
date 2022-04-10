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

#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

bool signalHandlerBound = false;
int serverSocket        = 0;

void clientGetName(){
	getlogin_r(playerName,sizeof(playerName)-1);
	playerName[sizeof(playerName)-1]=0;
}

bool fileExists(const char *fn){
	struct stat buffer;
	return (stat(fn, &buffer) == 0);
}

void startSingleplayerServer(){
	char seed[64];
	char save[64];
	char *wolkenweltenServer = clientGetServerExecutable();
	if(!mayTryToStartServer){
		menuSetError("Couldn't start Server!");
		return;
	}
	mayTryToStartServer = false;
	if (wolkenweltenServer == NULL){
		printf("[CLI] Server exectuable not found\n");
		return;
	}
	if(optionWorldSeed == 0){
		optionWorldSeed = (int)(time(NULL)&0xFFFF);
	}
	snprintf(save,sizeof(seed),"-worldSeed=%i",optionWorldSeed);
	snprintf(seed,sizeof(seed),"-savegame=%s",optionSavegame);
	singlePlayerPID = fork();
	if (singlePlayerPID == 0){
		execl(wolkenweltenServer,wolkenweltenServer,seed,"-singleplayer",save, (char *)NULL);
		menuSetError("Error exec'ing Server!");
		exit(1); // So we immediatly exit if we can't exec, otherwise we might fork again
	}
	strncpy(serverName,"localhost",sizeof(serverName));
	usleep(1000);
	lastPing = getTicks();
	clientInit();
}

void closeSingleplayerServer(){
	int tries = 10;
	while(singlePlayerPID != 0){
		if(--tries < 0){
			kill(singlePlayerPID,SIGKILL);
			return;
		}
		kill(singlePlayerPID,SIGTERM);
		printf("[CLI] Kill");
		usleep(10000);
	}
}

void zombieKiller(int sig){
	(void)sig;
	int status;
	int pid = waitpid(singlePlayerPID, &status, WNOHANG);
	if(pid == singlePlayerPID){singlePlayerPID = 0;}
}

void clientFreeRetry(){
	if(serverSocket > 0){
		close(serverSocket);
		serverSocket = 0;
	}
	usleep(4000);
}

void clientInit(){
	struct sockaddr_in serv_addr;
	struct hostent *serveraddr;
	int err,yes=1;
	if(!signalHandlerBound){
		signal(SIGCHLD,zombieKiller);
		signalHandlerBound = true;
	}
	if(serverSocket > 0){return;}
	if(singleplayer && (singlePlayerPID == 0)){
		printf("[CLI] Trying to start SP Server: %u\n",singlePlayerPID);
		startSingleplayerServer();
		return;
	}
	goodbyeSent = false;

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

	int rcvbuf = 1<<20;
	setsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
	err = setsockopt(serverSocket,IPPROTO_TCP,TCP_NODELAY,&yes,sizeof(yes));
	if (err < 0){
		menuSetError("ERROR, setsockopt");
		return;
	}

	while(connect(serverSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		if(errno == EINVAL){
			menuSetError("Error connecting to host EINVAL");
			return;
		}else if(errno == EINPROGRESS){
			usleep(4000);
			break;
		}else if(errno == ECONNREFUSED){
			clientFreeRetry();
			return;
		}
		menuSetError("Error connecting to host ELSE");
		return;
	}
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
	for(int i=4;i>0;i--){
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
	for(int i=4;i>0;i--){
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

void clientFreeSpecific(){
	if(serverSocket > 0){
		close(serverSocket);
		serverSocket = 0;
	}
}
