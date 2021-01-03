#pragma once

#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <emscripten.h>

int serverSocket;

void serverInit(){
	return;
}

void serverFree(){
	return;
}

void serverAccept(){
	return;
}

void serverKill(uint c){
	(void)c;
	return;
}

void serverRead(){
	return;
}

uint serverSendRaw(uint c, void *p, uint len){
	(void)c;
	(void)p;
	(void)len;

	return 0;
}

void wasmInit(){
	optionWorldSeed    = 69;
	optionSingleplayer = true;
	mainInit();
	clients[clientCount].socket = 1;
	serverInitClient(clientCount++);

	emscripten_worker_respond(NULL, 0);
}

void wasmTranceive(char *data, int size, void *arg){
	memcpy(&clients[0].recvBuf[clients[0].recvBufOff], data, size);
	clients[0].recvBufLen += size;

	mainTick();

	const uint len = clients[0].sendBufLen;
	clients[0].sendBufLen = 0;

	emscripten_worker_respond((char *)clients[0].sendBuf, len);
}
