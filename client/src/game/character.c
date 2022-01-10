/*
 * Woslkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
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

#include "../game/character.h"

#include "../main.h"
#include "../game/animal.h"
#include "../game/being.h"
#include "../game/blockType.h"
#include "../game/entity.h"
#include "../game/itemDrop.h"
#include "../game/projectile.h"
#include "../game/throwable.h"
#include "../game/weather/weather.h"
#include "../gfx/effects.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/particle.h"
#include "../gfx/shader.h"
#include "../gfx/shadow.h"
#include "../gfx/sky.h"
#include "../gui/gui.h"
#include "../gui/overlay.h"
#include "../gui/menu/inventory.h"
#include "../misc/options.h"
#include "../network/chat.h"
#include "../network/client.h"
#include "../sdl/sdl.h"
#include "../sfx/sfx.h"
#include "../tmp/objs.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/game/hook.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/game/weather/weather.h"
#include "../../../common/src/network/messages.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

character *player;
int        playerID = -1;
character *playerList[32];
char       playerNames[32][32];

void characterInit(character *c){
	if(c == NULL){return;}
	if(c->hook != NULL){
		hookFree(c->hook);
		c->hook = NULL;
	}
	memset(c,0,sizeof(character));

	c->breathing      = rngValM(1024);
	c->maxhp = c->hp  = 20;
	c->inventorySize  = 20;
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

	if(packetLen(p) >= 19*4){
		if(playerList[i]->hook == NULL){
			playerList[i]->hook = hookNew(playerList[i]);
		}
		playerList[i]->hook->hooked     = true;
		playerList[i]->hook->ent->flags = ENTITY_NOCLIP;
		playerList[i]->hook->ent->pos   = vecNewP(&p->v.f[16]);
		playerList[i]->hook->ent->vel   = vecZero();
	}else{
		if(playerList[i]->hook != NULL){
			hookFree(playerList[i]->hook);
			playerList[i]->hook = NULL;
		}
	}
	playerList[i]->hp           = p->v.u16[23];
	playerList[i]->inventory[0] = itemNew(p->v.u16[24],1);
	playerList[i]->activeItem   = 0;

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

static void characterUpdateInaccuracy(character *c){
	item *itm = &c->inventory[c->activeItem];
	float minInaccuracy = itemGetInaccuracy(itm);

	if(c->shake > c->inaccuracy){c->inaccuracy = c->shake;}
	c->inaccuracy = MINMAX(minInaccuracy,128.f,c->inaccuracy - 0.5f);
}

static void characterUpdateHook(character *c){
	if(c->hook == NULL){ return; }
	if(hookUpdate(c->hook)){
		hookFree(c->hook);
		c->hook = NULL;
		if(c == player){
			sfxPlay(sfxHookReturned,1.f);
		}
		return;
	}

	if(hookGetHooked(c->hook)){
		const float gl     = hookGetGoalLength(c->hook);
		const float wspeed = characterGetHookWinchS(c);
		const float maxl   = characterGetMaxHookLen(c);
		if(c->controls.y > 0){
			if(gl > 2.f){
				hookSetGoalLength(c->hook,gl-wspeed);
				c->flags |= CHAR_JUMPING;
			}else{
				hookReturnHook(c->hook);
				c->vel = vecMul(c->vel,vecNew(0.5f,1.f,0.5));
				c->vel.y += 0.05f;
				c->flags |= CHAR_JUMPING;
			}
		}
		if((c->flags & CHAR_SNEAK) && (gl < maxl)){
			hookSetGoalLength(c->hook,gl+wspeed);
		}
	}
}

void characterAddHookLength(character *c, float d){
	if((c == NULL) || (c->hook == NULL)){return;}
	const float gl     = hookGetGoalLength(c->hook);
	const float wspeed = characterGetHookWinchS(c);
	const float maxl   = characterGetMaxHookLen(c);
	const float ngl    = MAX(1.f,MIN(maxl,gl+wspeed*d));
	hookSetGoalLength(c->hook,ngl);
}

float characterGetHookLength(const character *c){
	if((c == NULL) || (c->hook == NULL)){return 0.f;}
	return hookGetGoalLength(c->hook);
}

float characterGetRopeLength(const character *c){
	if((c == NULL) || (c->hook == NULL)){return 0.f;}
	return hookGetRopeLength(c->hook);
}

static void characterUpdateAnimation(character *c){
	c->animationTicksLeft -= msPerTick;
	c->breathing          += msPerTick;
	if(c->animationTicksLeft <= 0){
		c->animationTicksLeft = 0;
		c->animationTicksMax  = 0;
		c->animationIndex     = 0;
	}
	if(c->flags & CHAR_GLIDE){
		c->gliderFade += 0.02f;
	}else{
		c->gliderFade -= 0.02f;
	}
	c->gliderFade = MINMAX(0.f,1.f,c->gliderFade);
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

void characterHit(character *c){
	static uint iteration=0;
	item *itm = &c->inventory[c->activeItem];
	iteration--;

	const float range = 4.f;
	const vec pos = vecAdd(c->pos,vecDegToVec(c->rot));
	const being source = beingCharacter(playerID);
	characterHitCheck(pos,range,itemGetDamage(itm,0),2,iteration, source);
	animalHitCheck   (pos,range,itemGetDamage(itm,0),2,iteration, source);

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

void characterStopMining(character *c){
	c->blockMiningX = c->blockMiningY = c->blockMiningZ = -1;
}

void characterDropItem(character *c, int i){
	item *cItem = characterGetItemBarSlot(c,i);
	if(cItem == NULL)       { return; }
	if(itemIsEmpty(cItem))  { return; }
	characterStopAim(c);

	itemDropNewC(c, cItem);
	itemDiscard(cItem);
}

void characterDropSingleItem(character *c, int i){
	if(c->actionTimeout < 0){ return; }
	item *cItem = characterGetItemBarSlot(c,i);
	if(cItem == NULL)       { return; }
	if(itemIsEmpty(cItem))  { return; }
	if(itemGetStackSize(cItem) <= 1){
		itemDropNewC(c, cItem);
		*cItem = itemEmpty();
	}else{
		item dItem = itemNew(cItem->ID,1);
		dItem.amount = itemDecStack(cItem,1);
		if(itemIsEmpty(&dItem)) { return; }
		itemDropNewC(c, &dItem);
	}
	characterStopAim(c);
	characterSetCooldown(c,200);
}

void characterDie(character *c){
	if(c != player)             {return;}
	if(c->flags & CHAR_SPAWNING){return;}
	if(c->flags & CHAR_NOCLIP)  {return;}
	for(int i=0;i<CHAR_INV_MAX;i++){
		itemDropNewP(c->pos, characterGetItemBarSlot(c,i),-1);
	}
	for(int i=0;i<CHAR_EQ_MAX;i++){
		itemDropNewP(c->pos, &c->equipment[i],-1);
	}
	itemDropNewC(player, &inventoryCurrentPickup);
	itemDiscard(&inventoryCurrentPickup);

	setOverlayColor(0xFF000000,0);
	characterInit(c);
	msgRequestPlayerSpawnPos();
	c->flags |= CHAR_SPAWNING;
	printf("Character Died\n");
	lispEval("[event-fire \"on-spawn\"]",false);
}

static void updateGlide(character *c){
	if((c == player) && ((itemIsEmpty(&c->equipment[CHAR_EQ_GLIDER])) || (c->equipment[CHAR_EQ_GLIDER].ID != I_Glider))){
		c->flags &= ~CHAR_GLIDE;
	}
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
	if(c->vel.y < -1.0f){c->vel.y += 0.0007f;}
	if(c->vel.y >  1.0f){c->vel.y -= 0.0007f;}
	if(c->vel.x < -1.0f){c->vel.x += 0.0007f;}
	if(c->vel.x >  1.0f){c->vel.x -= 0.0007f;}
	if(c->vel.z < -1.0f){c->vel.z += 0.0007f;}
	if(c->vel.z >  1.0f){c->vel.z -= 0.0007f;}

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
	updateGlide(c);
	return ret;
}

static void characterUpdateBooster(character *c){
	if(!(c->flags & CHAR_BOOSTING)
	 || ((c == player) && ((itemIsEmpty(&c->equipment[CHAR_EQ_PACK])) || (c->equipment[CHAR_EQ_PACK].ID != I_Jetpack)))
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

	if(c->flags & CHAR_FALLING){ walkFactor = 0.2f; }
	if(c->gvel.x < nvel.x){
		nvel.x -= 0.05f * (nvel.x - c->gvel.x) * walkFactor;
		if(nvel.x < c->gvel.x){ nvel.x = c->gvel.x; }
	}else if(c->gvel.x > nvel.x){
		nvel.x += 0.05f * (c->gvel.x - nvel.x) * walkFactor;
		if(nvel.x > c->gvel.x){ nvel.x = c->gvel.x; }
	}
	if(c->gvel.z < nvel.z){
		nvel.z -= 0.05f*(nvel.z - c->gvel.z) * walkFactor;
		if(nvel.z < c->gvel.z){ nvel.z = c->gvel.z; }
	}else if(c->gvel.z > nvel.z){
		nvel.z += 0.05f * (c->gvel.z - nvel.z) * walkFactor;
		if(nvel.z > c->gvel.z){ nvel.z = c->gvel.z; }
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

void charactersUpdate(){
	PROFILE_START();

	for(uint i=0;i<characterCount;i++){
		characterUpdate(&characterList[i]);
	}

	PROFILE_STOP();
}

void characterFireHook(character *c){
	if(c->actionTimeout < 0){return;}
	if(!playerChunkActive)  {return;}
	characterCloseGlider(c);
	c->flags |= CHAR_JUMPING;
	characterSetCooldown(c,240);
	if(c->hook == NULL){
		c->hook = hookNew(c);
		sfxPlay(sfxHookFire,1.f);
		characterStartAnimation(c,animationFire,350);
	}else{
		hookReturnHook(c->hook);
		characterStartAnimation(c,animationFire,350);
	}
}

float characterCanHookHit(const character *c){
	const float maxLen = characterGetMaxHookLen(c);
	vec pos = vecAdd(vecNew(0,1,0),c->pos);
	vec vel = vecAdd(vecMulS(vecDegToVec(c->rot),1.3f),c->vel);
	for(int i=0;i<1024;i++){
		pos = vecAdd(pos,vel);
		const vec dis = vecSub(pos,c->pos);
		const float d = vecMag(dis);
		if(d > maxLen){return -1.f;}
		if(checkCollision(pos.x,pos.y,pos.z)){return d;}
		vel.y -= 0.0005f;
	}
	return -1.f;
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

void characterFreeHook(character *c){
	if(c->hook != NULL){
		hookFree(c->hook);
		c->hook = NULL;
	}
}

void characterMoveDelta(character *c, const packet *p){
	c->vel   = vecAdd(c->vel,vecNewP(&p->v.f[0]));
	c->rot   = vecAdd(c->rot,vecNewP(&p->v.f[3]));
	c->shake = vecMag(c->vel)*8.f;
}

static void characterShadesDraw(const character *c){
	float sneakOff = 0.f;
	if(c->flags & CHAR_SNEAK){sneakOff = 1.f;}
	const float breath = sinf((float)(c->breathing-256)/512.f)*6.f;

	matMov(matMVP,matView);
	matMulTrans(matMVP,c->pos.x,c->pos.y+c->yoff+breath/128.f,c->pos.z);
	matMulRotYX(matMVP,-c->rot.yaw,-(c->rot.pitch+sneakOff*30.f)/3.f + breath);
	matMulTrans(matMVP,0.f,0.1f,-0.2f);
	matMulScale(matMVP,0.5f, 0.5f, 0.5f);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(meshSunglasses);
}

static void characterGliderDraw(const character *c){
	static u64 ticks = 0;
	if(c->gliderFade < 0.01f){return;}
	const float breath = sinf((float)(c->breathing-384)/512.f)*4.f;
	float deg  = ((float)++ticks*0.4f);
	float yoff = cosf(deg*2.1f)*player->shake;
	float xoff = sinf(deg*1.3f)*player->shake;

	matMov(matMVP,matView);
	matMulTrans(matMVP,c->pos.x,c->pos.y+c->yoff,c->pos.z);
	matMulRotYX(matMVP,-c->rot.yaw+xoff,-(c->rot.pitch-((1.f - c->gliderFade)*90.f)-breath+yoff));
	matMulTrans(matMVP,0.f,0.4f,-0.2f);
	matMulScale(matMVP,c->gliderFade, c->gliderFade, c->gliderFade);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(meshGlider);
}

static void characterActiveItemDraw(const character *c){
	const item *activeItem;
	mesh *aiMesh;
	float sneakOff = 0.f;
	if(c->flags & CHAR_SNEAK){sneakOff = 1.f;}

	activeItem = &c->inventory[c->activeItem];
	if(activeItem == NULL)     {return;}
	if(itemIsEmpty(activeItem)){return;}
	aiMesh = itemGetMesh(activeItem);
	if(aiMesh == NULL)         {return;}

	const float breath = cosf((float)c->breathing/512.f)*4.f;

	matMov(matMVP,matView);
	matMulTrans(matMVP,c->pos.x,c->pos.y+c->yoff,c->pos.z);
	matMulRotYX(matMVP,-c->rot.yaw+(15.f*sneakOff),-c->rot.pitch+breath);

	const float ix =  0.4f - (sneakOff/20.f);
	const float iy = -0.2f;
	const float iz = -0.3f;
	float hitOff,y;

	switch(c->animationIndex){
	case animationHit:
		hitOff = animationInterpolation(c->animationTicksLeft,c->animationTicksMax,0.3f);
		y = iy+c->yoff-(hitOff/8);
		matMulTrans(matMVP,ix-hitOff*0.2f,y+(hitOff/3),iz - hitOff*0.5f);
		matMulRotYX(matMVP,hitOff*5.f,hitOff*-20.f);
	break;

	case animationFire:
		hitOff = animationInterpolation(c->animationTicksLeft,c->animationTicksMax,0.5f);
		matMulTrans(matMVP,ix,c->yoff+iy,iz + hitOff*0.3f);
		matMulRotYX(matMVP,hitOff*10.f,hitOff*45.f);
	break;

	case animationReload:
		hitOff = animationInterpolationSustain(c->animationTicksLeft,c->animationTicksMax,0.3f,0.5f);
		y = iy+c->yoff-(hitOff/8);
		matMulTrans(matMVP,ix-hitOff*0.5f,y-(hitOff*0.5f),iz - hitOff*0.2f);
		matMulRotYX(matMVP,hitOff*15.f,hitOff*-55.f);
	break;

	case animationEmpty:
		hitOff = animationInterpolation(c->animationTicksLeft,c->animationTicksMax,0.5f);
		matMulTrans(matMVP,ix,c->yoff+iy,iz + hitOff*0.1f);
		matMulRotYX(matMVP,hitOff*3.f,hitOff*9.f);
	break;

	case animationEat:
		hitOff = animationInterpolation(c->animationTicksLeft,c->animationTicksMax,1.f)*3.f;
		if(hitOff < 1.f){
			matMulTrans(matMVP,ix-hitOff*0.4,c->yoff+iy,iz - hitOff*0.2f);
			matMulRotYX(matMVP,hitOff*20.f,hitOff*40.f);
		}else if(hitOff < 2.f){
			hitOff = hitOff-1.f;
			matMulTrans(matMVP,ix-0.4f,c->yoff+iy-hitOff*0.2f,iz - 0.2f);
			matMulRotYX(matMVP,hitOff*60.f+20.f,hitOff*120.f+40.f);
			matMulScale(matMVP, 1.f-hitOff, 1.f-hitOff, 1.f-hitOff);
		}else if(hitOff < 3.f){
			hitOff = 1.f-(hitOff-2.f);
			matMulTrans(matMVP,ix-hitOff*0.4,c->yoff+iy,iz + hitOff*0.4f);
			matMulRotYX(matMVP,hitOff*20.f,hitOff*40.f);
		}
	break;

	case animationSwitch:
		hitOff = (float)c->animationTicksLeft / (float)c->animationTicksMax;
		y = iy+c->yoff-(hitOff/8);
		matMulTrans(matMVP,ix-hitOff*0.5f,y-(hitOff*0.5f),iz - hitOff*0.2f);
		matMulRotYX(matMVP,hitOff*30.f,hitOff*-70.f);
	break;
	};

	matMulScale(matMVP,0.5f, 0.5f, 0.5f);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(aiMesh);
}

void characterDraw(character *c){
	if(c == NULL)       {return;}
	if(c->eMesh == NULL){return;}
	if((c == player) && (!optionThirdPerson)){return;}
	shadowAdd(c->pos,0.75f);

	const float breath = sinf((float)c->breathing/512.f)*6.f;
	if(c->effectValue){
		const float effectMult = 1.f - (--c->effectValue / 31.f);
		const float lowBrightness = worldBrightness * effectMult * effectMult;
		shaderColor(sMesh, worldBrightness, lowBrightness, lowBrightness, 1.f);
	}else{
		shaderColor(sMesh, worldBrightness, worldBrightness, worldBrightness, 1.f);
	}
	matMov(matMVP,matView);
	matMulTrans(matMVP,c->pos.x,c->pos.y+c->yoff+breath/128.f,c->pos.z);
	matMulRotYX(matMVP,-c->rot.yaw,-c->rot.pitch/6.f + breath);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(c->eMesh);
	c->screenPos = matMulVec(matMVP,vecNew(0,0.5f,0));
	shaderColor(sMesh, worldBrightness, worldBrightness, worldBrightness, 1.f);

	characterActiveItemDraw(c);
	characterShadesDraw(c);
	characterGliderDraw(c);
}

void characterDrawAll(){
	shaderBind(sMesh);
	for(uint i=0;i<characterCount;i++){
		if(characterList[i].nextFree != NULL){ continue; }
		characterDraw(&characterList[i]);
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
	msgSendRawMessage(tmp);
}

void characterDamagePacket(character *c, const packet *p){
	const being target  = p->v.u32[1];
	const being culprit = p->v.u32[2];
	const i16 hp        = p->v.u16[0];
	const u8 cause      = p->v.u8[2];
	const float knockbackMult = ((float)p->v.u8[3])/16.f;
	if(beingType(target) != BEING_CHARACTER){return;}
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
	if(beingType(target) != BEING_CHARACTER){return;}
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
	c->activeItem = p->v.u16[1];
	playerID      = p->v.u16[2];
	c->flags      = p->v.u32[2];
	if(playerList[playerID] == NULL){
		playerList[playerID] = player;
	}
	connectionState = 2;
	c->flags &= ~CHAR_SPAWNING;
	lispEval("[event-fire \"on-join\"]",false);
}

void characterSetName(const packet *p){
	if(p->v.u16[0] >= 32){return;}
	memcpy(playerNames[p->v.u16[0]],&p->v.u8[2],32);
}

void characterPickupPacket(character *c, const packet *p){
	const u16 ID     = p->v.u16[0];
	const i16 amount = p->v.i16[1];
	int a = characterPickupItem(c,ID,amount);
	if(a ==  0){return;}
	if(a == -1){a=0;}
	item drop = itemNew(ID,amount-a);
	itemDropNewC(c, &drop);
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

void characterDrawConsHighlight(const character *c){
	static uint counter = 0;
	item *activeItem = &player->inventory[player->activeItem];
	if((activeItem == NULL) || itemIsEmpty(activeItem) || (!(player->flags & CHAR_CONS_MODE))){return;}
	const u16 id = activeItem->ID;
	if(id < 256){
		vec los = characterLOSBlock(c,true);
		if(los.x < 0){return;}
		const float a = 0.7f + cosf((++counter&0x7F)/128.f*PI*2)*0.15f;
		blockTypeDraw(id, vecNew(los.x+0.5f,los.y+0.5f,los.z+0.5f),a,0);
	}else{
		vec los = characterLOSBlock(c,false);
		if(los.x < 0){return;}
		const float a = 0.5f + cosf((++counter&0x7F)/128.f*PI*2)*0.15f;
		blockTypeDraw(I_Marble_Block, vecNew(los.x+0.5f,los.y+0.5f,los.z+0.5f),a,-4);
	}
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

const char *characterGetName(const character *c) {
	if(c == NULL){return "Unknown Player";}
	uint id = beingID(characterGetBeing(c));
	if(id > 31){return "Out of Bounds Player";}
	return playerNames[id];
}
