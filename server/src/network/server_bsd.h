#pragma once

#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

int serverSocket;

#define PERR(msg); fprintf(stderr,"%s:%x %s\n",__FILE__,__LINE__,(msg));

void serverInit(){
	struct sockaddr_in serv_addr;
	int err,yes=1;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket <= 0){ perror("socket"); exit(1); }

	fcntl(serverSocket, F_SETFL, O_NONBLOCK);

	err = setsockopt(serverSocket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
	if (err < 0){ perror("setsockopt"); exit(1); }

	memset((char *) &serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port        = htons(optionPort);

	err = bind(serverSocket,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
	if(err < 0){ perror("bind"); exit(1); }
	err = listen(serverSocket,128);
	if(err < 0){ perror("listen"); exit(1); }
	printf("Listening on Port %i\n",optionPort);
}

void serverFree(){
	for(int i=0;i<clientCount;i++){
		close(clients[i].socket);
	}
	clientCount = 0;
	if(serverSocket > 0){
		close(serverSocket);
	}
}

void serverAccept(){
	struct sockaddr_in cAddr;
	socklen_t cLen = sizeof(cAddr);
	int clientSock;
	int err,yes=1;

	clientSock = accept(serverSocket,(struct sockaddr *) &cAddr, &cLen);
	if (clientSock < 0){
		return;
	}
	fcntl(clientSock, F_SETFL, O_NONBLOCK);
	err = setsockopt(clientSock,IPPROTO_TCP,TCP_NODELAY,&yes,sizeof(yes));
	if (err < 0){
		close(clientSock);
		return;
	}

	clients[clientCount].c                   = characterNew();
	clients[clientCount].socket              = clientSock;
	clients[clientCount].recvBufLen          = 0;
	clients[clientCount].sendBufSent         = 0;
	clients[clientCount].sendBufLen          = 0;
	clients[clientCount].chngReqQueueLen     = 0;
	clients[clientCount].chnkReqQueueLen     = 0;
	clients[clientCount].state               = 0;
	clients[clientCount].flags               = 0;
	clientCount++;
}

void serverKill(int c){
	close(clients[c].socket);
	serverCloseClient(c);
}

void serverRead(){
	for(int i=0;i<clientCount;i++){
		if(clients[i].state == 2){ continue; }
		int len = 1;
		while(len > 0){
			if(clients[i].flags&1){
				len = recv(clients[i].socket,clients[i].recvWSBuf + clients[i].recvWSBufLen,sizeof(clients[i].recvWSBuf) - clients[i].recvWSBufLen,0);
			}else{
				len = recv(clients[i].socket,clients[i].recvBuf + clients[i].recvBufLen,sizeof(clients[i].recvBuf) - clients[i].recvBufLen,0);
			}
			if(len > 0){
				if(clients[i].flags&1){
					clients[i].recvWSBufLen += len;
				}else{
					clients[i].recvBufLen += len;
				}
			}
		}
		if(len < 0){
			if(errno == EAGAIN){continue;}
			serverKill(i);
			continue;
		}
	}
}

int serverSendClient(int c){
	while(clients[c].sendBufSent < clients[c].sendBufLen){
		const int ret = send(clients[c].socket,clients[c].sendBuf+clients[c].sendBufSent,clients[c].sendBufLen-clients[c].sendBufSent,0);
		if(ret < 0){
			if(errno == EAGAIN){
				continue;
			}else{
				serverKill(c);
				return 2;
			}
		}else{
			clients[c].sendBufSent += ret;
		}
	}
	clients[c].sendBufSent = 0;
	clients[c].sendBufLen  = 0;
	return 1;
}
