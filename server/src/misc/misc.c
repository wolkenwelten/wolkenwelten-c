#include "misc.h"

#include <math.h>
#include <stdio.h>
#include <sys/time.h>

unsigned long long int RNGValue = 1;

void seedRNG(unsigned int seed){
	RNGValue = seed;
}

unsigned int getRNGSeed(){
	return RNGValue;
}

unsigned int getTicks(){
	return 0;
}

float rngValf(){
	RNGValue = ((RNGValue * 1103515245)) + 12345;
	return (float)(((RNGValue&0xFFFF)<<16) | ((RNGValue>>16)&0xFFFF)) / ((float)0xffffffff);
}

unsigned int rngValR(){
	RNGValue = ((RNGValue * 1103515245)) + 12345;
	return ((RNGValue&0xFFFF)<<16) | ((RNGValue>>16)&0xFFFF);
}
unsigned int rngValM(unsigned int max){
	if(max == 0){return 0;}
	return rngValR() % max;
}
int rngValMM(int min,int max){
	return rngValM(max - min) + min;
}

uint64_t getMillis(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (tv.tv_usec / 1000) + (tv.tv_sec * 1000);
}

const char *base64_encode(const unsigned char *data,unsigned int input_length,const char *prefix) {
	static char encoded_data[128];
	unsigned int i=0,j=0,a,b,c,triple;
	unsigned int output_length = 4 * ((input_length + 2) / 3);
	const unsigned int mod_table[] = {0, 2, 1};
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
