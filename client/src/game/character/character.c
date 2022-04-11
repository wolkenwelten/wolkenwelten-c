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
#include "character.h"
#include "draw.h"
#include "hook.h"
#include "network.h"
#include "../animal.h"
#include "../entity.h"
#include "../projectile.h"
#include "../weather/weather.h"
#include "../../sdl/sdl.h"
#include "../../gfx/gfx.h"
#include "../../gfx/particle.h"
#include "../../gui/overlay.h"
#include "../../misc/lisp.h"
#include "../../sfx/sfx.h"
#include "../../tmp/objs.h"
#include "../../voxel/bigchungus.h"
#include "../../../../common/src/misc/profiling.h"
#include "../../../../common/src/game/being.h"
#include "../../../../common/src/game/hook.h"
#include "../../../../common/src/network/messages.h"

character *player;

#include <string.h>

void characterInit(character *c){
	if(c == NULL){return;}
	characterFreeHook(c);
	memset(c,0,sizeof(character));

	c->breathing      = rngValM(1024);
	c->maxhp = c->hp  = 20;
	c->blockMiningX   = c->blockMiningY = c->blockMiningZ = -1;
	c->pos            = vecZero();
	c->controls       = vecZero();
	c->rot            = vecNew(135.f,15.f,0.f);
	c->eMesh          = meshPear;
	c->goalZoomFactor = c->zoomFactor = 1.f;

	if(c == player){
		c->flags = CHAR_SPAWNING;
	}
}

static void characterUpdateInaccuracy(character *c){
	float minInaccuracy = 4.f;

	if(c->shake > c->inaccuracy){c->inaccuracy = c->shake;}
	c->inaccuracy = MINMAX(minInaccuracy,128.f,c->inaccuracy - 0.5f);
}

static void characterUpdateYOff(character *c){
	if(!(c->flags & CHAR_FALLING) && ((fabsf(c->gvel.x) > 0.001f) || (fabsf(c->gvel.z) > 0.001f))){
		if(getTicks() > (c->stepTimeout + 200)){
			c->stepTimeout = getTicks();
			if(c->gyoff > -0.1f){
				c->gyoff = -0.2f;
				sfxPlay(sfxStep,1.f);
			}else{
				c->gyoff =  0.0f;
			}
		}
	}else{
		c->gyoff =  0.0f;
	}

	if(fabsf(c->gyoff - c->yoff) < 0.001f){
		c->yoff = c->gyoff;
	}else if(c->gyoff < c->yoff){
		c->yoff = c->yoff - (0.03f * ( c->yoff - c->gyoff));
	}else{
		c->yoff = c->yoff + (0.03f * (c->gyoff -  c->yoff));
	}
}

static bool characterUpdateJumping(character *c){
	if((c->controls.y > 0) && !(c->flags & (CHAR_FALLING | CHAR_JUMPING)) && ((c->hook == NULL) || (!hookGetHooked(c->hook)))){
		if((rngValA(15))==0){
			sfxPlay(sfxYahoo,1.f);
		}else{
			if((rngValA(15))==0){
				sfxPlay(sfxHoo,1.f);
			}else{
				sfxPlay(sfxHoho,1.f);
			}
		}
		c->inaccuracy += 24.f;
		return 1;
	}
	c->flags &= ~CHAR_JUMP_NEXT;
	return 0;
}

static void characterUpdateDamage(character *c, int damage){
	if(damage <= 0){ return; }
	if(damage > 8){
		sfxPlay(sfxImpact,1.f);
		sfxPlay(sfxUngh,1.f);
		setOverlayColor(0xA03020F0,0);
		c->flags &= ~CHAR_GLIDE;
		if(characterHP(c,damage / -8)){
			characterDyingMessage(characterGetBeing(c),0,deathCausePhysics);
			setOverlayColor(0xFF000000,0);
			commitOverlayColor();
		}
	}else{
		sfxPlay(sfxStomp,1.f);
	}
}

void characterHit(character *c){
	static uint iteration=0;
	iteration--;

	const float range  = 4.f;
	const vec pos      = vecAdd(c->pos,vecDegToVec(c->rot));
	const being source = beingCharacter(playerID);
	const int damage   = 1;
	characterHitCheck(pos,range,damage,2,iteration, source);
	animalHitCheck   (pos,range,damage,2,iteration, source);

	characterStartAnimation(c,animationHit,240);
	characterSetCooldown(c,320);
}

void characterDoPrimary(character *c){
	const vec los = characterLOSBlock(c,0);
	if(los.x < 0){
		if(c->actionTimeout >= 0){characterHit(c);}
		c->blockMiningX = -1;
		c->blockMiningY = -1;
		c->blockMiningZ = -1;
		return;
	}
	c->blockMiningX = los.x;
	c->blockMiningY = los.y;
	c->blockMiningZ = los.z;
	if(c->actionTimeout >= 0){
		sfxPlay(sfxTock,1.f);
		vibrate(0.3f);
		characterHit(c);
	}
}

void characterMineStop(character *c){
	c->blockMiningX = c->blockMiningY = c->blockMiningZ = -1;
}

void characterDie(character *c){
	if(c != player)             {return;}
	if(c->flags & CHAR_SPAWNING){return;}
	if(c->flags & CHAR_NOCLIP)  {return;}

	setOverlayColor(0xFF000000,0);
	characterInit(c);
	msgRequestPlayerSpawnPos();
	c->flags |= CHAR_SPAWNING;
	printf("Character Died\n");
	lispCallFunc("on-spawn-fire", NULL);
}

static void characterUpdateGlide(character *c){
	if(!(c->flags & CHAR_FALLING)){
		c->flags &= ~CHAR_GLIDE;
	}
	if(!(c->flags & CHAR_JUMPING) && (c->controls.y > 0) && (c->flags & CHAR_FALLING) && (c->hook == NULL)){
		characterToggleGlider(c);
		c->flags |= CHAR_JUMPING;
	}
	if(c->gliderFade < 0.01f){return;}
	const vec   dir = vecDegToVec(c->rot);
	      vec   vel = c->vel;
	const vec  vdeg = vecVecToDeg(vecNorm(vel));

	float aoa   = fabsf(vdeg.y - c->rot.pitch);
	float drag  = fabsf(sinf(aoa*PI180)) * 0.98f + 0.02f;

	float speed = vecMag(vel);
	if((speed < 0.1f) && (c->flags & CHAR_FALLING)){
		float pd = 0.1f - speed;
		pd  = pd * 6.f;
		pd  = pd * pd;
		c->rot.pitch += pd;
	}

	vec  vdrg    = vecMulS(vecInvert(vel),drag * 0.1f);
	float mag    = vecMag(vdrg);
	c->shake     = MAX(c->shake,mag*16.f + speed * c->gliderFade);
	const vec nv = vecAdd(vecAdd(vdrg,vecMulS(dir,mag*0.99f)),vecMulS(windVel,0.025f));

	c->vel = vecAdd(vel,vecMulS(nv,c->gliderFade));
}

static void characterUpdateCons(character *c, uint oldCol, const vec oldPos){
	if(!(c->flags & CHAR_CONS_MODE)){return;}
	uint col = characterCollision(c->pos);
	if(col & 0xF){return;}
	if((oldCol & 0xF) == 0){return;}
	c->pos = oldPos;
	c->vel = vecZero();

	if(oldCol & 3){
		c->pos.x = oldPos.x;
	}
	if(oldCol & (4|8)){
		c->pos.z = oldPos.z;
	}
}

static void characterGoStepUp(character *c){
	u32 ncol = characterCollision(vecAdd(c->pos,vecNew(0,1,0)));
	if(ncol){return;}
	c->pos.y += 0.1f;
	c->vel.y = 0.03f;
}

static float characterBlockRepulsion(character *c, float *vel){
	return blockRepulsion(c->pos,vel,80.f,characterCollisionBlock);
}

// ToDo: Check blockRepulsion and also set Positions */
static void characterBeingCollisionCallback(vec pos, being b, being source){
	if(beingType(b) != BEING_ANIMAL){return;}
	const vec beingPos = beingGetPos(b);
	const vec delta = vecSub(pos, beingPos);
	//const float power = (2.f - vecMag(delta));
	const vec deltaS = vecMulS(vecNorm(delta),0.0001f);
	const float weightDistribution = beingGetWeight(source) / beingGetWeight(b);
	beingAddVel(b,vecMulS(deltaS,weightDistribution));
	msgBeingMove(b,vecZero(),vecMulS(deltaS,-weightDistribution));
	beingAddVel(source,vecMulS(deltaS,(-1.f + weightDistribution)));
}

static void characterCheckForAnimalCollision(character *c){
	beingGetInSphere(c->pos, 2.f, characterGetBeing(c), characterBeingCollisionCallback);
}

static int characterPhysics(character *c){
	int ret = 0;
	u32 col;

	vec oldPos = c->pos;
	uint oldCol = characterCollision(c->pos);

	c->pos = vecAdd(c->pos,c->vel);
	c->shake = MAX(0.f,c->shake-0.1f);
	if(c->flags & CHAR_NOCLIP){
		col = characterCollision(c->pos);
		if(col){ c->flags |= CHAR_COLLIDE; }
		return 0;
	}

	c->vel.y -= 0.0005f;
	c->vel.x -= (c->vel.x * c->vel.x * c->vel.x) * 0.01f;
	c->vel.y -= (c->vel.y * c->vel.y * c->vel.y) * 0.01f;
	c->vel.z -= (c->vel.z * c->vel.z * c->vel.z) * 0.01f;

	c->flags |=  CHAR_FALLING;
	c->flags &= ~CHAR_COLLIDE;
	col = characterCollision(c->pos);
	if(col){ c->flags |= CHAR_COLLIDE; }
	if((col&0x3110) && (c->vel.x < 0.f)){
		ret += characterBlockRepulsion(c,&c->vel.x);
		if(((col&0x3111) == 0x0101) && (fabsf(c->vel.y) < 0.001f)){
			characterGoStepUp(c);
		}
	}
	if((col&0xC220) && (c->vel.x > 0.f)){
		ret += characterBlockRepulsion(c,&c->vel.x);
		if(((col&0xC222) == 0x0202) && (fabsf(c->vel.y) < 0.001f)){
			characterGoStepUp(c);
		}
	}
	if((col&0xA880) && (c->vel.z > 0.f)){
		ret += characterBlockRepulsion(c,&c->vel.z);
		if(((col&0x5888) == 0x0808) && (fabsf(c->vel.y) < 0.001f)){
			characterGoStepUp(c);
		}
	}
	if((col&0x5440) && (c->vel.z < 0.f)){
		ret += characterBlockRepulsion(c,&c->vel.z);
		if(((col&0xA444) == 0x0404) && (fabsf(c->vel.y) < 0.001f)){
			characterGoStepUp(c);
		}
	}
	if((col&0xF0F0) && (c->vel.y > 0.f)){
		ret += characterBlockRepulsion(c,&c->vel.y);
	}
	if((col&0x000F) && (c->vel.y < 0.f)){
		c->flags &= ~CHAR_FALLING;
		if(c->vel.y < -0.02f){
			c->yoff += MAX(-.1f,c->vel.y * 10.f);
		}
		if(c->vel.y > -0.01f) {
			c->vel = vecMul(c->vel,vecNew(0.98f,0,0.98f));
		}else{
			ret += characterBlockRepulsion(c,&c->vel.y);
		}
	}

	if(isInClouds(c->pos)){
		int cloudyness = vecMag(c->vel) * 1024.f;
		c->cloudyness = MAX(c->cloudyness,cloudyness);
	}
	if(c->cloudyness > 0){
		--c->cloudyness;
		newParticle(c->pos.x,c->pos.y,c->pos.z,0.f,0.f,0.f,78.f,2.5f,cloudCT[c->cloudyness&0x1F]|0xFF000000, MAX(32,MIN(768,c->cloudyness*2)));
	}

	characterCheckForAnimalCollision(c);
	characterUpdateCons(c,oldCol,oldPos);
	characterUpdateGlide(c);
	return ret;
}

static void characterUpdateBooster(character *c){
	if(!(c->flags & CHAR_BOOSTING)
	 || (c->flags & CHAR_CONS_MODE)){
		sfxLoop(sfxJet,0.f);
		return;
	}
	const vec rot = c->rot;
	float speed   = 0.0002f / MAX(0.1,vecMag(c->vel));
	const vec nv  = vecMulS(vecDegToVec(rot),speed);
	c->vel = vecAdd(c->vel,nv);
	c->shake = MAX(c->shake,1.25f + speed);
	if(c == player){
		sfxLoop(sfxJet,1.f);
		if(rngValA(15) == 0){
			const vec ppos = vecSub(c->pos,vecMulS(vecDegToVec(rot),2.f));
			projectileNew(ppos,vecAdd(rot,vecInvert(vecMulS(vecRng(),10.f))),0,characterGetBeing(player),1,0.01f);
		}
	}

	const vec adir = vecMulS(vecDegToVec(vecAdd(rot,vecMulS(vecRng(),10.f))),-0.07f * (rngValf()+.5f));
	const vec apos = vecAdd(c->pos,vecAdd(adir,vecMulS(vecRng(),0.1f)));
	newParticleV(apos, adir, 96.f, 0.1f, 0xE643B0F8,  192);
	const vec bdir = vecMulS(vecDegToVec(vecAdd(rot,vecMulS(vecRng(),10.f))),-0.03f * (rngValf()+.5f));
	const vec bpos = vecAdd(c->pos,vecAdd(adir,vecMulS(vecRng(),0.1f)));
	newParticleV(bpos, bdir, 48.f, 0.2f, 0xC42370FA,  386);
	const vec cdir = vecMulS(vecDegToVec(vecAdd(rot,vecMulS(vecRng(), 4.f))),-0.01f * (rngValf()+.5f));
	const vec cpos = vecAdd(c->pos,vecAdd(adir,vecMulS(vecRng(),0.1f)));
	newParticleV(cpos, cdir, 24.f+rngValf()*24.f, 0.4f+rngValf()*0.4f, 0xD0203040 | rngValA(0x1F1F0F0F), 1536+rngValA(511));
	if(rngValM(6)==0){
		const vec ddir = vecMulS(vecDegToVec(vecAdd(rot,vecMulS(vecRng(), 4.f))),-0.001f * (rngValf()+.5f));
		const vec dpos = vecAdd(c->pos,vecAdd(adir,vecMulS(vecRng(),0.1f)));
		newSparticleV(dpos, ddir, 16.f+rngValf()*32.f, 0.2f+rngValf()*0.1f, 0xD0101820 | rngValA(0x1F0F0707), 3072+rngValA(1023));
	}
}

static void characterUpdateFalling(character *c){
	if(c != player){return;}
	if(c->flags & CHAR_NOCLIP){return;}
	if(c->pos.y < -32){
		setOverlayColor(0xFF000000,1000);
		if(!(c->flags & CHAR_FALLINGSOUND)){
			c->flags |= CHAR_FALLINGSOUND;
			sfxPlay(sfxFalling,1.f);
		}
	}
	if(c->pos.y < -512){
		characterDie(c);
		characterDyingMessage(characterGetBeing(player),0,deathCauseAbyss);
	}
}

static void characterUpdateAim(character *c){
	float diff = c->goalZoomFactor - c->zoomFactor;
	if(fabsf(diff) < 0.001f){
		c->zoomFactor = c->goalZoomFactor;
		return;
	}
	c->zoomFactor += diff * 0.03f;
}

void characterUpdate(character *c){
	float walkFactor = 1.f;
	vec nvel;

	if(c->flags & CHAR_SPAWNING){return;}
	if((c->flags & CHAR_FALLINGSOUND) && (c->pos.y > -32)){ c->flags &= ~CHAR_FALLINGSOUND; }
	characterUpdateAim(c);
	characterUpdateFalling(c);
	if(c->rot.pitch < -90.f){
		 c->rot.pitch = -90.f;
	}else if(c->rot.pitch >  90.f){
		 c->rot.pitch =  90.f;
	}
	if(c->rot.yaw < 0.f){
		 c->rot.yaw += 360.f;
	}else if(c->rot.yaw >  360.f){
		 c->rot.yaw -= 360.f;
	}

	if(c->actionTimeout < 0){ c->actionTimeout += msPerTick; }
	if(c->flags & CHAR_NOCLIP){
		c->vel = c->gvel;
		characterUpdateHook(c);
		characterUpdateAnimation(c);
		characterUpdateInaccuracy(c);
		characterPhysics(c);
		return;
	}
	characterUpdateBooster(c);
	if((c->flags & CHAR_JUMPING) && (c->controls.y < 0.001f)){
		c->flags &= ~CHAR_JUMPING;
	}
	if((c->flags & (CHAR_GLIDE | CHAR_FALLING)) == (CHAR_GLIDE | CHAR_FALLING)){
		characterUpdateHook(c);
		characterUpdateAnimation(c);
		characterUpdateInaccuracy(c);
		characterUpdateDamage(c,characterPhysics(c));
		return;
	}
	nvel = c->vel;
	uint col = characterCollision(c->pos);

	if(c->flags & CHAR_FALLING){ walkFactor = 0.2f; }
	if(c->gvel.x < nvel.x){
		if((col&0x3110) == 0){
			nvel.x -= 0.05f * (nvel.x - c->gvel.x) * walkFactor;
			if(nvel.x < c->gvel.x){
				nvel.x = c->gvel.x;
			}
		}
	}else if(c->gvel.x > nvel.x){
		if((col&0xC220) == 0){
			nvel.x += 0.05f * (c->gvel.x - nvel.x) * walkFactor;
			if(nvel.x > c->gvel.x){
				nvel.x = c->gvel.x;
			}
		}
	}
	if(c->gvel.z < nvel.z){
		if((col&0x5440) == 0){
			nvel.z -= 0.05f*(nvel.z - c->gvel.z) * walkFactor;
			if(nvel.z < c->gvel.z){
				nvel.z = c->gvel.z;
			}
		}
	}else if(c->gvel.z > nvel.z){
		if((col&0xA880) == 0){
			nvel.z += 0.05f * (c->gvel.z - nvel.z) * walkFactor;
			if(nvel.z > c->gvel.z){
				nvel.z = c->gvel.z;
			}
		}
	}
	if((c->hook != NULL) && (hookGetHooked(c->hook))){
		if(fabsf(c->gvel.x) < 0.001)                 {nvel.x=c->vel.x;}
		if(fabsf(c->gvel.z) < 0.001)                 {nvel.z=c->vel.z;}
		if((c->gvel.x < -0.001)&&(nvel.x > c->vel.x)){nvel.x=c->vel.x;}
		if((c->gvel.x >  0.001)&&(nvel.x < c->vel.x)){nvel.x=c->vel.x;}
		if((c->gvel.z < -0.001)&&(nvel.z > c->vel.z)){nvel.z=c->vel.z;}
		if((c->gvel.z >  0.001)&&(nvel.z < c->vel.z)){nvel.z=c->vel.z;}
	}
	if(characterUpdateJumping(c)){
		nvel.y = 0.055f;
		c->flags |= CHAR_JUMPING;
	}
	c->vel = nvel;

	const int damage = characterPhysics(c);
	if(c == player){
		characterUpdateDamage(c,damage);
		if((nvel.y < -0.2f) && c->vel.y > -0.01f){
			sfxPlay(sfxImpact,1.f);
		} else if((nvel.y < -0.05f) && c->vel.y > -0.01f){
			sfxPlay(sfxStomp,1.f);
		}
		if((damage > 0) && (hookGetHooked(c->hook))){
			hookReturnHook(c->hook);
		}
		characterUpdateHook(c);
	}
	characterUpdateInaccuracy(c);
	characterUpdateYOff(c);
	characterUpdateAnimation(c);
	characterCheckHealth(c);
}

void characterUpdateAll(){
	PROFILE_START();

	for(uint i=0;i<characterCount;i++){
		characterUpdate(&characterList[i]);
	}

	PROFILE_STOP();
}

float characterFirstBlockDist (const character *c){
	const float maxLen = characterGetMaxHookLen(c);
	vec pos = vecAdd(vecNew(0,1,0),c->pos);
	vec vel = vecAdd(vecMulS(vecDegToVec(c->rot),1.3f),c->vel);
	for(int i=0;i<1024;i++){
		pos = vecAdd(pos,vel);
		const vec dis = vecSub(pos,c->pos);
		const float d = vecMag(dis);
		if(d > maxLen){return -1.f;}
		if(worldGetB(pos.x,pos.y,pos.z) != 0){return d;}
	}
	return -1.f;
}

int characterHitCheck(const vec pos, float mdd, int damage, int cause, u16 iteration, being source){
	int hits = 0;
	for(int i=0;i<32;i++){
		if(playerList[i] == NULL  )          {continue;}
		if(playerList[i]->temp == iteration) {continue;}
		if(beingCharacter(i) == source)      {continue;}
		vec dis = vecSub(pos,playerList[i]->pos);

		if(vecDot(dis,dis) < mdd){
			msgBeingDamage(0,damage,cause,0.01f,beingCharacter(i),0,pos);
			playerList[i]->temp = iteration;
			hits++;
		}
	}
	return hits;
}

bool characterDamage(character *c, int hp){
	sfxPlay(sfxImpact,1.f);
	sfxPlay(sfxUngh,1.f);
	setOverlayColor(0xA03020F0,0);
	bool ret = characterHP(c,-hp);
	if(ret){
		setOverlayColor(0xFF000000,0);
		commitOverlayColor();
	}
	return ret;
}
