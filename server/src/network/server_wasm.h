#pragma once

#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

int serverSocket;

void serverInit(){
}

void serverFree(){
}

void serverAccept(){
}

void serverKill(uint c){
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

void wasmSetOption(){

}

void wasmInit(){
	mainInit();
}

void wasmUpdate(){
	mainTick();
}

void wasmRead(){

}

void wasmWrite(){

}
