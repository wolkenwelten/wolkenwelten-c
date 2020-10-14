#pragma once
#include <emscripten.h>

int serverSocket = 0;

EM_JS(int, wsSend, (unsigned char* data, int len), {
	return wsSendData(data,len);
});
EM_JS(int, wsRecv, (unsigned char* buf, int size), {
	return wsRecvData(buf,size);
});
EM_JS(void, wsInit, (const char* server,const char* clientName), {
	return wsInitClient(UTF8ToString(server),UTF8ToString(clientName));
});
EM_JS(void, wsFree, (), {
	return wsFreeClient();
});

void clientGetName(){
	snprintf(playerName,sizeof(playerName)-1,"webPlayer");
	playerName[sizeof(playerName)-1]=0;
}

void startSingleplayerServer(){

}

void closeSingleplayerServer(){

}

void clientFreeSpecific(){
	wsFree();
}

void clientFreeRetry(){
	wsFree();
}

void clientInit(){
	wsInit(serverName,playerName);
	serverSocket            = 1;
	sendBufLen              = 0;
	sendBufSent             = 0;
	sentBytesCurrentSession = 0;
	recvBytesCurrentSession = 0;
	recvUncompressedBytesCurrentSession = 0;
	clientGreetServer();
}

void clientRead(){
	int len = 1;
	if(serverSocket == 0){clientInit();}
	while(len > 0){
		const int len = wsRecv(recvBuf + recvBufLen,sizeof(recvBuf) - recvBufLen);
		recvBufLen += len;
		recvBytesCurrentSession += len;
	}
}

void clientWrite(){
	if(serverSocket == 0){clientInit();}
	if(sendBufSent < sendBufLen){
		const int ret = wsSend(sendBuf+sendBufSent,sendBufLen-sendBufSent);
		sendBufSent += ret;
		sentBytesCurrentSession += ret;
	}
	if(sendBufSent >= sendBufLen){
		sendBufSent = 0;
		sendBufLen  = 0;
	}
}
