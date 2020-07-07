#pragma once

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
	getlogin_r(playerName,28);
}

bool fileExists(char *fn){
	struct stat buffer;
	return (stat(fn, &buffer) == 0);
}

#ifdef __APPLE__
	#include <mach-o/dyld.h>
	#include <libgen.h>
	char *getServerExecutablePath(){
		static char path[512];
		static char ret[512];
		uint32_t size = sizeof(path);
		if (_NSGetExecutablePath(path, &size) == 0){
			snprintf(ret,sizeof(ret)-1,"%s/wolkenwelten-server",dirname(path));
			return ret;
		} else {
			fprintf(stderr,"buffer too small; need size %u\n", size);
		}
		return NULL;
	}
#else
	char *getServerExecutablePath(){
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
	char *wolkenweltenServer = getServerExecutablePath();
	if(optionWorldSeed == 0){
		optionWorldSeed = (int)(time(NULL)&0xFFFF);
	}
	snprintf(seed,sizeof(seed)-1,"-worldSeed=%i",optionWorldSeed);
	singlePlayerPID = fork();
	if (singlePlayerPID == 0){
		execl(wolkenweltenServer,"wolkenwelten-server",seed,"-singleplayer=1", (char *)NULL);
		return;
	}
	strncpy(serverName,"localhost",sizeof(serverName)-1);
	usleep(100);
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
		startSingleplayerServer();
		return;
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket <= 0){
		menuError = "Error opening socket";
		gameRunning = false;
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

#ifndef __EMSCRIPTEN__
	int rcvbuf = 1<<20;
	setsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
	err = setsockopt(serverSocket,IPPROTO_TCP,TCP_NODELAY,&yes,sizeof(yes));
	if (err < 0){
		clientFree();
		fprintf(stderr,"ERROR, setsockopt\n");
		gameRunning = false;
		return;
	}
#endif

	while(connect(serverSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		if(errno == EINVAL){
			fprintf(stderr,"ERROR, connect EINVAL\n");
			clientFree();
			return;
		}else if(errno == EINPROGRESS){
			break;
		}else if(errno == ECONNREFUSED){
			if(singleplayer){continue;}
			clientFree();
			return;
		}
		if(!singleplayer){
			perror("Error connecting");
			clientFree();
			menuError = "Error connecting to host";
			gameRunning = false;
			return;
		}
	}
	fcntl(serverSocket, F_SETFL, O_NONBLOCK);
	err = setsockopt(serverSocket,IPPROTO_TCP,TCP_NODELAY,&yes,sizeof(yes));
	sendBufLen  = 0;
	sendBufSent = 0;
	clientGreetServer();
}

void clientFree(){
	if(serverSocket > 0){
		close(serverSocket);
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
		len = recv(serverSocket,recvBuf + recvBufLen,sizeof(recvBuf) - recvBufLen, 0);
		if(len > 0){
			recvBufLen += len;
			recvBytesCurrentSession += len;
		}
	}
	if(len < 0){
		if(errno == EAGAIN){return;}
		clientFree();
	}
}

void clientSendAllToServer(){
	if(serverSocket <= 0){clientInit();}
	if(serverSocket <= 0){return;}

	while(sendBufSent < sendBufLen){
		const int ret = send(serverSocket,sendBuf+sendBufSent,sendBufLen-sendBufSent, 0);
		if(ret < 0){
			if(errno == EAGAIN){
				return;
			}else{
				perror("ERROR sending");
				clientFree();
				return;
			}
		}else{
			sendBufSent += ret;
			sentBytesCurrentSession += ret;
		}
	}
	sendBufSent = 0;
	sendBufLen  = 0;
}
