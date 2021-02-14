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
	clients[clientCount].socket = clientSock;
	serverInitClient(clientCount++);
}

void serverKill(uint c){
	#ifndef __EMSCRIPTEN__
		close(clients[c].socket);
	#endif
	serverCloseClient(c);
}

void serverRead(){
	for(uint i=0;i<clientCount;i++){
		if(clients[i].state == STATE_CLOSED){ continue; }
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

uint serverSendRaw(uint c, void *p, uint len){
	if(p == NULL){return 0;}
	uint tries = 3;
	uint sent  = 0;
	while(sent < len){
		const int ret = send(clients[c].socket,p+sent,len-sent,0);
		if(ret < 0){
			if(errno == EAGAIN){
				if(sent > 0)     {return sent;}
				if(--tries == 0) {return 0;}
				usleep(1);
			}
			return 0;
		}
		sent += ret;
	}
	return sent;
}
