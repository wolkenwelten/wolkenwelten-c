#pragma once

#include <unistd.h>
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

#ifdef __APPLE__
	#include <mach-o/dyld.h>
	#include <libgen.h>
	const char *getServerExecutablePath(){
		static char path[512];
		static char ret[512];
		u32 size = sizeof(path);
		if (_NSGetExecutablePath(path, &size) == 0){
			snprintf(ret,sizeof(ret)-1,"%s/wolkenwelten-server",dirname(path));
			return ret;
		} else {
			fprintf(stderr,"buffer too small; need size %u\n", size);
		}
		return NULL;
	}
#else
	const char *getServerExecutablePath(){
		static char *serverPath = NULL;
		char *serverPaths[4] = {
			"wolkenwelten-server",
			"/bin/wolkenwelten-server",
			"/usr/bin/wolkenwelten-server",
			"/usr/local/bin/wolkenwelten-server",
		};
		if(serverPath == NULL){
			for(int i=0;i<4;++i){
				if(fileExists(serverPaths[i])){
					serverPath = serverPaths[i];
					break;
				}
			}
		}
		return serverPath;
	}
#endif

void startSingleplayerServer(){
	char seed[64];
	char save[64];
	const char *wolkenweltenServer = getServerExecutablePath();
	if(optionWorldSeed == 0){
		optionWorldSeed = (int)(time(NULL)&0xFFFF);
	}
	snprintf(save,sizeof(seed)-1,"-worldSeed=%i",optionWorldSeed);
	snprintf(seed,sizeof(seed)-1,"-savegame=%s",optionSavegame);
	singlePlayerPID = fork();
	if (singlePlayerPID == 0){
		execl(wolkenweltenServer,"wolkenwelten-server",seed,"-singleplayer",save, (char *)NULL);
		return;
	}
	strncpy(serverName,"localhost",sizeof(serverName)-1);
	serverName[sizeof(serverName)-1]=0;
	usleep(1000);
}

void closeSingleplayerServer(){
	int tries = 10;
	while(singlePlayerPID > 0){
		if(--tries <= 0){
			kill(singlePlayerPID,SIGKILL);
			return;
		}
		kill(singlePlayerPID,SIGTERM);
		usleep(100000);
	}
}

void zombieKiller(int sig){
	int status;
	if(sig > 0){
		waitpid(singlePlayerPID, &status, 0);
		singlePlayerPID = 0;
	}
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
	fprintf(stderr,"clientInit\n");
	if(!signalHandlerBound){
		signal(SIGCHLD,zombieKiller);
		signalHandlerBound = true;
	}
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

#ifndef __EMSCRIPTEN__
	int rcvbuf = 1<<20;
	setsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
	err = setsockopt(serverSocket,IPPROTO_TCP,TCP_NODELAY,&yes,sizeof(yes));
	if (err < 0){
		menuSetError("ERROR, setsockopt");
		return;
	}
#endif

	while(connect(serverSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		if(errno == EINVAL){
			menuSetError("Error connecting to host");
			return;
		}else if(errno == EINPROGRESS){
			if(++connectionTries > 5){
				menuSetError("Error connecting to host");
				return;
			}
			break;
		}else if(errno == ECONNREFUSED){
			if(++connectionTries > 5){
				menuSetError("Error connecting to host");
				return;
			}
			clientFreeRetry();
			return;
		}
		menuSetError("Error connecting to host");
		return;
	}
	fprintf(stderr,"connected\n");
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
