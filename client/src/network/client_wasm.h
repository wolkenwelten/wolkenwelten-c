#pragma once
#include <emscripten.h>

int serverSocket = 0;

EM_JS(int, wsSendData, (unsigned char* data, int len), {
	let str = ' ';
	for(let i=0;i<len;i++){
		str += (HEAP8[data+i]+' ');
	}
	console.log('Send['+len+']:' + str);
	return len;
});

EM_JS(int, wsRecvData, (unsigned char* buf, int size), {
	return 0;
});

EM_JS(void, wsInitClient, (const char* server,const char* clientName), {
	let srv = UTF8ToString(server);
	let client = UTF8ToString(clientName);
	console.log("Connecting to "+srv+' as '+client);
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

}

void clientFreeRetry(){

}

void clientInit(){
	wsInitClient(serverName,playerName);
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
		//const int len = recv(serverSocket,recvBuf + recvBufLen,sizeof(recvBuf) - recvBufLen, 0);
		len = 0;
		recvBufLen += len;
		recvBytesCurrentSession += len;
	}
}

void clientSendAllToServer(){
	if(serverSocket == 0){clientInit();}
	while(sendBufSent < sendBufLen){
		sendBuf[sendBufLen]=0;
		//fprintf(stderr,"%s\n",sendBuf);
		const int ret = wsSendData(sendBuf,sendBufLen);
		//const int ret = write(serverSocket,sendBuf+sendBufSent,sendBufLen-sendBufSent);
		sendBufSent += ret;
		sentBytesCurrentSession += ret;
	}
	sendBufSent = 0;
	sendBufLen  = 0;
}
