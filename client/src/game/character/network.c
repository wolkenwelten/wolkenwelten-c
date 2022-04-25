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
#include "network.h"
#include "character.h"
#include "../../gfx/gfx.h"
#include "../../gfx/effects.h"
#include "../../gui/overlay.h"
#include "../../sfx/sfx.h"
#include "../../nujel/nujel.h"
#include "../../network/client.h"
#include "../../../../common/src/game/being.h"

int        playerID = -1;
character *playerList[32];
char       playerNames[32][32];

void characterUpdatePacket(const packet *p){
	const int i = p->v.u32[15];
	if(i > 32){return;}
	if(playerList[i] == NULL){
		playerList[i] = characterNew();
	}
	playerList[i]->pos      = vecNewP(&p->v.f[0]);
	playerList[i]->rot      = vecNewP(&p->v.f[3]);
	playerList[i]->rot.roll = 0;
	playerList[i]->yoff     = p->v.f[5];
	playerList[i]->vel      = vecNewP(&p->v.f[6]);
	playerList[i]->flags    = p->v.u32[9];
	playerList[i]->hp       = p->v.u16[23];

	playerList[i]->animationIndex     = p->v.u16[25];
	playerList[i]->animationTicksMax  = p->v.u16[26];
	playerList[i]->animationTicksLeft = p->v.u16[27];
}

void characterRemovePlayer(int c, int len){
	if(playerList[c] != NULL){
		characterFree(playerList[c]);
		playerList[c] = NULL;
		if(playerList[len] != NULL){
			playerList[c] = playerList[len];
			playerList[len] = NULL;
		}
	}
}

void characterDyingMessage(const being victim, const being culprit, deathCause cause){
	char tmp[4096];
	const char *victimName = beingGetName(victim);
	const char *culpritName = beingGetName(culprit);
	switch(cause){
	case deathCauseCommand:
		snprintf(tmp,sizeof(tmp),"%s died by command",victimName);
		break;
	case deathCauseBeamblast:
		snprintf(tmp,sizeof(tmp),"%s beamblasted %s",culpritName,victimName);
		break;
	case deathCauseMelee:
		snprintf(tmp,sizeof(tmp),"%s clubbed %s",culpritName,victimName);
		break;
	case deathCauseProjectile:
		snprintf(tmp,sizeof(tmp),"%s shot %s",culpritName,victimName);
		break;
	case deathCausePhysics:
		snprintf(tmp,sizeof(tmp),"%s didn't bounce",victimName);
		break;
	case deathCauseAbyss:
		snprintf(tmp,sizeof(tmp),"%s fell into the abyss",victimName);
		break;
	case deathCauseFire:
		snprintf(tmp,sizeof(tmp),"%s burned",victimName);
		break;
	case deathCauseGrenade:
		snprintf(tmp,sizeof(tmp),"%s bombed %s",culpritName,victimName);
		break;
	case deathCauseLightning:
		snprintf(tmp,sizeof(tmp),"%s got thunderstruck", victimName);
		break;
	}
	(void)tmp;
	//chatSendRaw(tmp);
}

void characterDamagePacket(character *c, const packet *p){
	const being target  = p->v.u32[1];
	const being culprit = p->v.u32[2];
	const i16 hp        = p->v.u16[0];
	const u8 cause      = p->v.u8[2];
	const float knockbackMult = ((float)p->v.u8[3])/16.f;
	if(beingType(target) != bkCharacter){return;}
	if(beingID(target) != (uint)playerID)   {return;}

	if(cause == 2){
		sfxPlay(sfxImpact,1.f);
		sfxPlay(sfxUngh,  1.f);
		setOverlayColor(0xA03020F0,0);
		commitOverlayColor();

		const vec pos = vecNewP(&p->v.f[3]);
		vec dis = vecNorm(vecSub(c->pos,pos));
		dis.y = MAX(0.1f,dis.y);
		c->vel = vecAdd(c->vel,vecMulS(dis,0.05f * knockbackMult));
	}
	if(characterDamage(c,hp)){
		characterDyingMessage(target,culprit,cause);
	}
}

void characterGotHitPacket(const packet *p){
	const being target  = p->v.u32[1];
	character *c = NULL;
	if(beingType(target) != bkCharacter){return;}
	if(beingID(target) == (uint)playerID){
		c = player;
		nextOverlayColor(0xA03020F0,0);
		const vec pos = vecNewP(&p->v.f[3]);
		vec dist = vecNorm(vecSub(player->pos,pos));
		dist.y = MAX(0.4f,dist.y);
		player->vel = vecAdd(player->vel,vecMulS(dist,0.02f));
	}else{
		if(beingID(target) > 32){return;}
		c = playerList[beingID(target)];
	}
	if(c == NULL){return;}
	const i16 hp   = p->v.i16[0];
	const u8 cause = p->v.u8[2];
	fxBleeding(c->pos,target,hp,cause);
	c->effectValue = 31;
}

void characterSetData(character *c, const packet *p){
	c->hp         = p->v.i16[0];
	playerID      = p->v.u16[1];
	c->flags      = p->v.u32[1];
	if(playerList[playerID] == NULL){
		playerList[playerID] = player;
	}
	connectionState = 2;
	c->flags &= ~CHAR_SPAWNING;
	lispCallFunc("on-join-fire", NULL);
}

void characterSetName(const packet *p){
	if(p->v.u16[0] >= 32){return;}
	memcpy(playerNames[p->v.u16[0]],&p->v.u8[2],32);
}

character *characterGetPlayer(uint i){
	if(i >= 32){return NULL;}
	if(playerList[i] == NULL){return NULL;}
	return playerList[i];
}

char *characterGetPlayerName(uint i){
	if(i >= 32){return NULL;}
	if(playerList[i] == NULL){return NULL;}
	return playerNames[i];
}

int characterGetPlayerHP(uint i){
	if(i >= 32){return 0;}
	if(playerList[i] == NULL){return 0;}
	return playerList[i]->hp;
}

vec characterGetPlayerDist(uint i){
	if(i >= 32){return vecZero();}
	if(playerList[i] == NULL){return vecZero();}
	return vecSub(player->pos,playerList[i]->pos);
}

const char *characterGetName(const character *c) {
	if(c == NULL){return "Unknown Player";}
	uint id = beingID(characterGetBeing(c));
	if(id > 31){return "Out of Bounds Player";}
	return playerNames[id];
}

void characterMoveDelta(character *c, const packet *p){
	c->vel   = vecAdd(c->vel,vecNewP(&p->v.f[0]));
	c->rot   = vecAdd(c->rot,vecNewP(&p->v.f[3]));
	c->shake = vecMag(c->vel)*8.f;
}
