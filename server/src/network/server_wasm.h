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
