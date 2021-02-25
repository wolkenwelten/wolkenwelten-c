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
		fprintf(stderr,"[SRV] WSAStartup failed with error %i\n", WSAGetLastError());
		return;
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(serverSocket == INVALID_SOCKET){
		fprintf(stderr,"[SRV] Invalid Socket: %i\n",WSAGetLastError());
		exit(1);
	}

	err = ioctlsocket(serverSocket, FIONBIO, &yesl);
	if (err == SOCKET_ERROR){
		fprintf(stderr,"[SRV] ioctl Socket Error: %i\n",WSAGetLastError());
		exit(1);
	}

	err = setsockopt(serverSocket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
	if (err < 0){ perror("[SRV] setsockopt: SO_REUSEADDR"); exit(1); }

	memset((char *) &serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port        = htons(optionPort);

	err = bind(serverSocket,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
	if(err == SOCKET_ERROR){
		fprintf(stderr,"[SRV] Bind Socket Error: %i\n",WSAGetLastError());
		exit(1);
	}
	err = listen(serverSocket,128);
	if(err == SOCKET_ERROR){
		fprintf(stderr,"[SRV] Listen Socket Error: %i\n",WSAGetLastError());
		exit(1);
	}
	printf("[SRV] Listening on Port %i\n",optionPort);
}

void serverFree(){
	for(uint i=0;i<clientCount;i++){
		msgGoodbye(i);
		serverSendClient(i);
		close(clients[i].socket);
		serverCloseClient(i);
	}
	clientCount = 0;
	if(serverSocket > 0){close(serverSocket);}
}

void serverAccept(){
	long long unsigned int clientSock;
	int err,yes=1;

	clientSock = accept(serverSocket,NULL, NULL);
	if (clientSock == INVALID_SOCKET){
		err = WSAGetLastError();
		if(err == WSAEWOULDBLOCK){return;}
		fprintf(stderr,"[SRV] Accept Error: %i\n",err);
		return;
	}
	err = ioctlsocket(clientSock, FIONBIO, (unsigned long *) &yes);
	if (err == SOCKET_ERROR){
		fprintf(stderr,"[SRv] accept ioctl Socket Error: %i\n",WSAGetLastError());
		close(clientSock);
		return;
	}
	clients[clientCount].socket = clientSock;
	serverInitClient(clientCount++);
}

void serverKill(uint c){
	close(clients[c].socket);
	serverCloseClient(c);
}

void serverRead(){
	for(uint i=0;i<clientCount;i++){
		if(clients[i].state == STATE_CLOSED){ continue; }
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
				fprintf(stderr,"[SRV] ERROR receiving: %i\n",err);
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

uint serverSendRaw(uint c, void *p, uint len){
	if(p == NULL){return 0;}
	uint tries = 3;
	uint sent  = 0;
	while(sent < len){
		const int ret = send(clients[c].socket,p+sent,len-sent,0);
		if(ret == SOCKET_ERROR){
			const int err = WSAGetLastError();
			if((err == WSAEWOULDBLOCK) || (err == WSAEINPROGRESS)){
				if(sent > 0)     {return sent;}
				if(--tries == 0) {return 0;}
				usleep(1);
				continue;
			}
			return 0;
		}
		sent += ret;
	}
	return sent;
}
