#include "sfx.h"
#include "../misc/options.h"
#include "../tmp/assets.h"

#include <stdio.h>

int sfxEnable = 0;

sfx *sfxFalling;
sfx *sfxHoho;
sfx *sfxHoo;
sfx *sfxImpact;
sfx *sfxPhaser;
sfx *sfxBomb;
sfx *sfxTock;
sfx *sfxPock;
sfx *sfxStomp;
sfx *sfxStep;
sfx *sfxUngh;
sfx *sfxYahoo;
sfx *sfxHookFire;
sfx *sfxHookHit;
sfx *sfxHookReturned;
sfx *atmosfxHookRope;
sfx *atmosfxWind;

sfx sfxList[32];
int sfxCount=0;

bgm bgmList[8];
int bgmCount=0;

void sfxInit(){
	if(!sfxEnable){return;}

	sfxFalling      = sfxNew(sfx_falling_aif_data,      sfx_falling_aif_len      );
	sfxHoho         = sfxNew(sfx_hoho_aif_data,         sfx_hoho_aif_len         );
	sfxHoo          = sfxNew(sfx_hoo_aif_data,          sfx_hoo_aif_len          );
	sfxImpact       = sfxNew(sfx_impact_aif_data,       sfx_impact_aif_len       );
	sfxPhaser       = sfxNew(sfx_phaser_aif_data,       sfx_phaser_aif_len       );
	sfxBomb         = sfxNew(sfx_bomb_aif_data,         sfx_bomb_aif_len         );
	sfxTock         = sfxNew(sfx_tock_aif_data,         sfx_tock_aif_len         );
	sfxPock         = sfxNew(sfx_pock_aif_data,         sfx_pock_aif_len         );
	sfxStomp        = sfxNew(sfx_stomp_aif_data,        sfx_stomp_aif_len        );
	sfxStep         = sfxNew(sfx_step_aif_data,         sfx_step_aif_len         );
	sfxUngh         = sfxNew(sfx_ungh_aif_data,         sfx_ungh_aif_len         );
	sfxYahoo        = sfxNew(sfx_yahoo_aif_data,        sfx_yahoo_aif_len        );
	sfxHookFire     = sfxNew(sfx_hookfire_aif_data,     sfx_hookfire_aif_len     );
	sfxHookHit      = sfxNew(sfx_hookhit_aif_data,      sfx_hookhit_aif_len      );
	sfxHookReturned = sfxNew(sfx_hookreturned_aif_data, sfx_hookreturned_aif_len );
	atmosfxHookRope = sfxNew(sfx_hookrope_aif_data,     sfx_hookrope_aif_len     );
	atmosfxWind     = sfxNew(sfx_wind_aif_data,         sfx_wind_aif_len         );
}

void sfxFreeAll(){
	sfxFree(sfxFalling);
	sfxFree(sfxHoho);
	sfxFree(sfxHoo);
	sfxFree(sfxImpact);
	sfxFree(sfxPhaser);
	sfxFree(sfxBomb);
	sfxFree(sfxTock);
	sfxFree(sfxPock);
	sfxFree(sfxStomp);
	sfxFree(sfxStep);
	sfxFree(sfxUngh);
	sfxFree(sfxYahoo);
	sfxFree(sfxHookFire);
	sfxFree(sfxHookHit);
	sfxFree(sfxHookReturned);
	sfxFree(atmosfxHookRope);
	sfxFree(atmosfxWind);
}

bgm *bgmNew(unsigned char *data,size_t dataLen){
	bgm *b = &bgmList[bgmCount++];
	b->chan = -1;
	b->mixMusic = NULL;
	if(data == NULL){ return NULL; }
	b->mixMusic = Mix_LoadMUS_RW(SDL_RWFromConstMem(data,dataLen),0);
	//mixMusic=Mix_LoadMUS(file);
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

sfx *sfxNew(unsigned char *data,size_t dataLen){
	sfx *b = &sfxList[sfxCount++];

	b->chan = -1;
	b->mixChunk = NULL;
	if(data == NULL){ return NULL; }
	b->mixChunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(data,dataLen),0);
	//mixChunk=Mix_LoadWAV(file);
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
