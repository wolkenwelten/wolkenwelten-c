#define _GNU_SOURCE

#include "server_ws.h"

#include "../misc/sha1.h"
#include "../network/server.h"

#include <stdio.h>
#include <string.h>

const char *base64_encode(const u8 *data,uint input_length,const char *prefix){
	static char encoded_data[128];
	uint i=0,j=0,a,b,c,triple;
	uint output_length = 4 * ((input_length + 2) / 3);
	const uint mod_table[] = {0, 2, 1};
	const char encoding_table[] =
		{'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		 'w', 'x', 'y', 'z', '0', '1', '2', '3',
		 '4', '5', '6', '7', '8', '9', '+', '/'};

	if(prefix){
		while(*prefix != 0){
			encoded_data[j++] = *prefix++;
			input_length++;
			output_length++;
		}
	}

	if ((output_length+j) >= sizeof(encoded_data)){
		fprintf(stderr,"b64 too big ol:%i j:%i >= ed:%u\n",output_length,j,(unsigned int)sizeof(encoded_data));
		return NULL;
	}

	for (i = 0; i < input_length;) {
		a = i < input_length ? (unsigned char)data[i++] : 0;
		b = i < input_length ? (unsigned char)data[i++] : 0;
		c = i < input_length ? (unsigned char)data[i++] : 0;

		triple = (a << 0x10) | (b << 0x08) | c;

		encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
	}

	for (i = 0; i < mod_table[input_length % 3]; i++){
		encoded_data[output_length - 1 - i] = '=';
	}
	encoded_data[output_length]=0;
	return encoded_data;
}

void serverParseWebSocketPacket(uint i){
	for(int max=16;max > 0;--max){
		if(i >= clientCount)            { return; }
		if((clients[i].state != STATE_INTRO) && (clients[i].recvWSBufLen < 16)){return;}
		if((clients[i].state == STATE_INTRO) && (clients[i].recvWSBufLen <  4)){return;}
		u8  opcode  = clients[i].recvWSBuf[0];
		u8  masklen = clients[i].recvWSBuf[1];
		u64 mlen    = 0;
		u8  mask[4];
		uint ii=0;
		if(opcode != 0x82){
			fprintf(stderr,"oh noes: %X\n",opcode);
			fprintf(stderr,"Buf[%i]: ",clients[i].recvWSBufLen);
			for(uint iii = 0;iii<clients[i].recvWSBufLen;iii++){
				fprintf(stderr,"%X ",clients[i].recvWSBuf[iii]);
			}
			serverKill(i);
		}
		if((masklen&0x80) == 0){
			//See: https://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-17
			fprintf(stderr,"Clients MUST mask: %X\n",masklen);
			serverKill(i);
		}
		if((masklen&0x7F) == 0x7E){
			mlen = (clients[i].recvWSBuf[2]<<8) | clients[i].recvWSBuf[3];
			ii = 4;
		}else if((masklen&0x7F) == 0x7F){
			// > 4GB Messages are not supported, therefore the upper 32-bits are ignored
			mlen |= clients[i].recvWSBuf[6] << 24;
			mlen |= clients[i].recvWSBuf[7] << 16;
			mlen |= clients[i].recvWSBuf[8] <<  8;
			mlen |= clients[i].recvWSBuf[9]      ;
			ii = 10;
		}else{
			mlen = masklen&0x7F;
			ii = 2;
		}
		mask[0] = clients[i].recvWSBuf[ii++];
		mask[1] = clients[i].recvWSBuf[ii++];
		mask[2] = clients[i].recvWSBuf[ii++];
		mask[3] = clients[i].recvWSBuf[ii++];

		for(u64 iii=0;iii<mlen;iii++){
			clients[i].recvBuf[clients[i].recvBufLen++] = clients[i].recvWSBuf[ii+iii] ^ mask[iii&3];
		}

		ii += mlen;
		if(clients[i].recvWSBufLen != ii){
			for(unsigned int iii=0;iii < (clients[i].recvWSBufLen - ii);++iii){
				clients[i].recvWSBuf[iii] = clients[i].recvWSBuf[iii+ii];
			}
		}
		clients[i].recvWSBufLen -= ii;
	}
}

void serverParseWebSocketHeaderField(uint c,const char *key, const char *val){
	static SHA1_CTX ctx;
	static char buf[512];
	static u8 webSocketKeyHash[20];
	const char *b64hash;
	uint len=0;
	if(strncmp(key,"Sec-WebSocket-Key",18) != 0){return;}
	len = snprintf(buf,sizeof(buf),"%s%s",val,"258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	printf("Sec-WebSocket-Key = '%s'\nConcat: '%s'\n",val,buf);

	SHA1Init(&ctx);
	SHA1Update(&ctx,(unsigned char *)buf,len);
	SHA1Final(webSocketKeyHash,&ctx);
	b64hash = base64_encode(webSocketKeyHash,20,NULL);

	printf("SHA1 = ");
	for(int i=0;i<20;i++){
		printf("%X ",webSocketKeyHash[i]);
	}
	printf("\n");
	printf("B64 = %s\n",b64hash);

	len = snprintf(buf,sizeof(buf),"HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Protocol: binary\r\nSec-WebSocket-Accept: %s\r\n\r\n",b64hash);
	const uint ret = serverSendRaw(c, buf, len);
	if(ret < len){serverKill(c);return;}
	clients[c].flags |= CONNECTION_WEBSOCKET;
}

void serverParseWebSocketHeader(uint c,uint end){
	int lb=0;
	char *key=NULL;
	char *val=NULL;
	for(uint i=0;i<end;i++){
		if(clients[c].recvBuf[i] == '\n'){
			clients[c].recvBuf[i]=0;
			if(lb==5){
				serverParseWebSocketHeaderField(c,key,val);
				lb=0;
			}
			lb|=1;
			continue;
		}
		if(clients[c].recvBuf[i] == '\r'){
			clients[c].recvBuf[i]=0;
			if(lb==5){
				serverParseWebSocketHeaderField(c,key,val);
				lb=0;
			}
			lb|=2;
			continue;
		}
		if(lb==3){
			key = (char *) &clients[c].recvBuf[i];
			lb=4;
		}
		if(lb && (clients[c].recvBuf[i] == ':')){
			clients[c].recvBuf[i]=0;
			val = (char *) &clients[c].recvBuf[i+1];
			if(*val == ' '){val++;}
			lb=5;
		}
	}
}

int addWSMessagePrefix(u8 *d, uint len, uint maxlen){
	if(len < 126){
		if(maxlen <= 2){return 0;}
		*d++ = 0x82; // Opcode - Binary Data / Fin Bit
		*d++ = len;
		return 2;
	}else if(len < 0xFFFF){
		if(maxlen <= 4){return 0;}
		*d++ = 0x82; // Opcode - Binary Data / Fin Bit
		*d++ = 0x7E;
		*d++ = (len>>8) & 0xFF;
		*d++ = (len   ) & 0xFF;
		return 4;
	}else{
		if(maxlen < 10){return 0;}
		*d++ = 0x82; // Opcode - Binary Data / Fin Bit
		*d++ = 0x7F;
		*d++ = 0;
		*d++ = 0;
		*d++ = 0;
		*d++ = 0;
		*d++ = (len>>24) & 0xFF;
		*d++ = (len>>16) & 0xFF;
		*d++ = (len>> 8) & 0xFF;
		*d++ = (len    ) & 0xFF;
		return 10;
	}
}
