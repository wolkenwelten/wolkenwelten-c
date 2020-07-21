#pragma once

#include <winsock2.h>

long long unsigned int serverSocket;

void serverInit(){
	struct sockaddr_in serv_addr;
	int err;
	unsigned long yesl=1;
	const char yes=1;
	WSADATA wsaData;
	err = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(err != NO_ERROR){
		fprintf(stderr,"WSAStartup failed with error %i\n", WSAGetLastError());
		return;
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(serverSocket == INVALID_SOCKET){
		fprintf(stderr,"Invalid Socket: %i\n",WSAGetLastError());
		exit(1);
	}

	err = ioctlsocket(serverSocket, FIONBIO, &yesl);
	if (err == SOCKET_ERROR){
		fprintf(stderr,"ioctl Socket Error: %i\n",WSAGetLastError());
		exit(1);
	}

	err = setsockopt(serverSocket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
	if (err < 0){ perror("setsockopt: SO_REUSEADDR"); exit(1); }

	memset((char *) &serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port        = htons(optionPort);

	err = bind(serverSocket,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
	if(err == SOCKET_ERROR){
		fprintf(stderr,"Bind Socket Error: %i\n",WSAGetLastError());
		exit(1);
	}
	err = listen(serverSocket,128);
	if(err == SOCKET_ERROR){
		fprintf(stderr,"Listen Socket Error: %i\n",WSAGetLastError());
		exit(1);
	}
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
	long long unsigned int clientSock;
	int err,yes=1;

	clientSock = accept(serverSocket,NULL, NULL);
	if (clientSock == INVALID_SOCKET){
		err = WSAGetLastError();
		if(err == WSAEWOULDBLOCK){return;}
		fprintf(stderr,"Accept Error: %i\n",err);
		return;
	}
	err = ioctlsocket(clientSock, FIONBIO, (unsigned long *) &yes);
	if (err == SOCKET_ERROR){
		fprintf(stderr,"accept ioctl Socket Error: %i\n",WSAGetLastError());
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
		int len = 1;
		while(len > 0){
			if(clients[i].flags&1){
				len = recv(clients[i].socket,(void *)(clients[i].recvWSBuf + clients[i].recvWSBufLen),sizeof(clients[i].recvWSBuf) - clients[i].recvWSBufLen,0);
			}else{
				len = recv(clients[i].socket,(void *)(clients[i].recvBuf + clients[i].recvBufLen),sizeof(clients[i].recvBuf) - clients[i].recvBufLen,0);
			}
			if(len == SOCKET_ERROR){
				const int err = WSAGetLastError();
				if(err == WSAEWOULDBLOCK){break;}
				if(err == WSAEINPROGRESS){break;}
				fprintf(stderr,"ERROR receiving: %i\n",err);
				serverKill(i);
				break;
			}else{
				if(clients[i].flags&1){
					clients[i].recvWSBufLen += len;
				}else{
					clients[i].recvBufLen += len;
				}
			}
		}
	}
}

int serverSendClient(int c){
	while(clients[c].sendBufSent < clients[c].sendBufLen){
		const int ret = send(clients[c].socket,(void *)(clients[c].sendBuf+clients[c].sendBufSent),clients[c].sendBufLen-clients[c].sendBufSent,0);
		if(ret == SOCKET_ERROR){
			const int err = WSAGetLastError();
			if(err == WSAEWOULDBLOCK){return 0;}
			if(err == WSAEINPROGRESS){return 0;}
			fprintf(stderr,"ERROR sending: %i\n",err);
			serverKill(c);
			return 2;
		}else{
			clients[c].sendBufSent += ret;
		}
	}
	clients[c].sendBufSent = 0;
	clients[c].sendBufLen  = 0;
	return 1;
}
