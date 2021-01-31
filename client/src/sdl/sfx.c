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

#include "sfx.h"
#include "../game/character.h"
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

sfx *sfxNew(const void *data,size_t dataLen, const char *lName){
	sfx *b = &sfxList[sfxCount++];

	b->chan = -1;
	b->mixChunk = NULL;
	if(data == NULL){ return NULL; }
	b->mixChunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(data,dataLen),0);
	if(!b->mixChunk) { return NULL; }
	lispDefineInt(lName,b-sfxList);

	return b;
}

void sfxFree(sfx *b){
	if(b == NULL)          {return;}
	if(b->mixChunk == NULL){return;}
	Mix_FreeChunk(b->mixChunk);
}

void sfxPlay(sfx *b,float volume){
	if(b == NULL)       {return;}
	if(volume < 0.001f) {return;}
	b->chan = Mix_PlayChannel(-1,b->mixChunk,0);
	Mix_Volume(b->chan,(int)(MIX_MAX_VOLUME*(optionSoundVolume*volume)));
}

void sfxPlayPos(sfx *b,float volume, const vec pos){
	if(b == NULL)       {return;}
	if(volume < 0.001f) {return;}
	const float d = vecMag(vecSub(pos,player->pos));
	volume = volume * (MAX(0.0001f,128.f-d)/128.f);
	if(volume <= 0.001f){return;}
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
