#include "sfx.h"
#include "../misc/options.h"

#include <stdio.h>

int sfxEnable = 0;

sfx sfxList[32];
int sfxCount=0;

bgm bgmList[8];
int bgmCount=0;

#include "../tmp/sfx.h"

bgm *bgmNew(const void *data,size_t dataLen){
	bgm *b = &bgmList[bgmCount++];
	b->chan = -1;
	b->mixMusic = NULL;
	if(data == NULL){ return NULL; }
	b->mixMusic = Mix_LoadMUS_RW(SDL_RWFromConstMem(data,dataLen),0);
	if(!b->mixMusic) { return NULL; }
	return b;
}

void bgmFree(bgm *b){
	if(b->mixMusic == NULL){return;}
	Mix_FreeMusic(b->mixMusic);
}

void bgmPlay(bgm *b, float volume){
	if(b == NULL){return;}
	Mix_VolumeMusic((int)(MIX_MAX_VOLUME*(optionMusicVolume*volume)));
	Mix_FadeInMusic(b->mixMusic,-1,5000);
}

sfx *sfxNew(const void *data,size_t dataLen){
	sfx *b = &sfxList[sfxCount++];

	b->chan = -1;
	b->mixChunk = NULL;
	if(data == NULL){ return NULL; }
	b->mixChunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(data,dataLen),0);
	if(!b->mixChunk) { return NULL; }

	return b;
}

void sfxFree(sfx *b){
	if(b->mixChunk == NULL){return;}
	Mix_FreeChunk(b->mixChunk);
}

void sfxPlay(sfx *b,float volume){
	if(b == NULL){return;}
	b->chan = Mix_PlayChannel(-1,b->mixChunk,0);
	Mix_Volume(b->chan,(int)(MIX_MAX_VOLUME*(optionSoundVolume*volume)));
}

void sfxLoop(sfx *b, float volume){
	if(b == NULL){return;}
	if((volume < 0.01f) && (b->chan >= 0)){
		Mix_HaltChannel(b->chan);
		b->chan = -1;
		return;
	}else if(b->chan < 0){
		b->chan = Mix_PlayChannel(b->chan,b->mixChunk,-1);
	}
	Mix_Volume(b->chan,(int)(MIX_MAX_VOLUME*(optionSoundVolume*volume)));
}

void sfxStopALl(){
	if(!sfxEnable){return;}
	for(int i=0;i<sfxCount;i++){
		sfxLoop(&sfxList[i],0);
	}
}
