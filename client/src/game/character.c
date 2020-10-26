#include "../game/character.h"

#include "../main.h"
#include "../game/animal.h"
#include "../game/itemDrop.h"
#include "../game/grapplingHook.h"

#include "../gfx/effects.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/shadow.h"
#include "../gfx/particle.h"
#include "../gui/gui.h"
#include "../gui/overlay.h"
#include "../network/chat.h"
#include "../network/client.h"
#include "../sdl/input_gamepad.h"
#include "../sdl/sdl.h"
#include "../sdl/sfx.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/mods/mods.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

character *player;
int        playerID = -1;
character  characterList[64];
int        characterCount = 0;
character *characterFirstFree = NULL;
character *playerList[32];
char       playerNames[32][32];

void characterInit(character *c){
	if(c->hook != NULL){
		grapplingHookFree(c->hook);
		c->hook = NULL;
	}
	memset(c,0,sizeof(character));

	c->breathing     = rngValM(1024);
	c->maxhp = c->hp = 20;
	c->blockMiningX  = c->blockMiningY = c->blockMiningZ = -1;
	c->pos           = vecNew(0,0,0);
	c->rot           = vecNew(135.f,15.f,0.f);
	c->eMesh         = meshPear;

	if(c == player){
		sfxLoop(sfxWind,0.f);
		sfxLoop(sfxHookRope,0.f);
		c->flags = CHAR_SPAWNING;
	}
}

character *characterNew(){
	character *c = NULL;
	if(characterFirstFree != NULL){
		c = characterFirstFree;
		characterFirstFree = c->nextFree;
	}
	if(c == NULL){
		if(characterCount >= (int)(sizeof(characterList) / sizeof(character))-1){
			fprintf(stderr,"characterList Overflow!\n");
			return NULL;
		}
		c = &characterList[characterCount++];
	}
	characterInit(c);
	return c;
}

void characterFree(character *c){
	c->eMesh = NULL;
	if(c->hook != NULL){
		grapplingHookFree(c->hook);
		c->hook = NULL;
	}
	c->nextFree = characterFirstFree;
	characterFirstFree = c;
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
			playerList[i]->hook = grapplingHookNew(playerList[i]);
		}
		playerList[i]->hook->hooked     = true;
		playerList[i]->hook->ent->flags = ENTITY_NOCLIP;
		playerList[i]->hook->ent->pos   = vecNewP(&p->v.f[16]);
		playerList[i]->hook->ent->vel   = vecZero();
	}else{
		if(playerList[i]->hook != NULL){
			grapplingHookFree(playerList[i]->hook);
			playerList[i]->hook = NULL;
		}
	}
	playerList[i]->hp                 = p->v.u16[23];
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

void characterUpdateInaccuracy(character *c){
	item *itm = &c->inventory[c->activeItem];
	float minInaccuracy = getInaccuracyDispatch(itm);

	if(c->shake > c->inaccuracy){c->inaccuracy = c->shake;}
	if(c->inaccuracy > 64.f){
		c->inaccuracy = 64.f;
	}else if(c->inaccuracy > minInaccuracy){
		c->inaccuracy -= 0.4f;
	}else{
		c->inaccuracy = minInaccuracy;
	}
}

void characterUpdateHook(character *c){
	if(c->hook == NULL){ return; }
	if(grapplingHookUpdate(c->hook)){
		grapplingHookFree(c->hook);
		c->hook = NULL;
		if(c == player){
			sfxLoop(sfxHookRope,0.f);
			sfxPlay(sfxHookReturned,1.f);
		}
		return;
	}
	if(c == player){
		if(c->hook->hooked){
			sfxLoop(sfxHookRope,0.f);
		}else{
			sfxLoop(sfxHookRope,1.f);
		}
	}

	if(grapplingHookGetHooked(c->hook)){
		const float gl     = grapplingHookGetGoalLength(c->hook);
		const float wspeed = characterGetHookWinchS(c);
		const float maxl   = characterGetMaxHookLen(c);
		if((c->gvel.y > 0) && (gl > 1.f)){
			grapplingHookSetGoalLength(c->hook,gl-wspeed);
		}
		if((c->flags & CHAR_SNEAK) && (gl < maxl)){
			grapplingHookSetGoalLength(c->hook,gl+wspeed);
		}
		if(grapplingHookGetLength(c->hook) > gl){
			grapplingHookPullTowards(c->hook,c);
		}
	}
}

void characterUpdateAnimation(character *c){
	c->animationTicksLeft -= MS_PER_TICK;
	c->breathing          += MS_PER_TICK;
	if(c->animationTicksLeft <= 0){
		c->animationTicksLeft = 0;
		c->animationTicksMax  = 0;
		c->animationIndex     = 0;
	}
	if(c->flags & CHAR_GLIDE){
		c->gliderFade += 0.03f;
		if(c->gliderFade > 1.f){c->gliderFade = 1.f;}
	}else{
		c->gliderFade -= 0.03f;
		if(c->gliderFade < 0.f){c->gliderFade = 0.f;}
	}
}

void characterUpdateWindVolume(const character *c){
	float windVol = vecMag(c->vel);
	if(windVol < 0.01f){
		sfxLoop(sfxWind,0.f);
	}else{
		windVol = MIN((windVol - 0.01f),1.0);
		if(windVol > 0.2){
			vibrate(windVol);
		}
		sfxLoop(sfxWind,windVol);
	}
}

int characterUpdateJumping(const character *c){
	if((c->gvel.y > 0) && !(c->flags & CHAR_FALLING) && ((c->hook == NULL) || (!grapplingHookGetHooked(c->hook)))){
		if((rngValR()&15)==0){
			sfxPlay(sfxYahoo,1.f);
		}else{
			if((rngValR()&1)==0){
				sfxPlay(sfxHoo,1.f);
			}else{
				sfxPlay(sfxHoho,1.f);
			}
		}
		return 1;
	}
	return 0;
}

void characterUpdateDamage(character *c, int damage){
	if(damage <= 0){ return; }
	if(damage > 8){
		sfxPlay(sfxImpact,1.f);
		sfxPlay(sfxUngh,1.f);
		setOverlayColor(0xA03020F0,0);
		if(characterHP(c,damage / -8)){
			msgSendDyingMessage("did not bounce", 65535);
			setOverlayColor(0xFF000000,0);
			commitOverlayColor();
		}
	}else{
		sfxPlay(sfxStomp,1.f);
	}
}

void characterUpdateYOff(character *c){
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

	const vec pos = vecAdd(c->pos,vecDegToVec(c->rot));
	characterHitCheck(pos,2.f,damageDispatch(itm),2,iteration);
	animalHitCheck   (pos,2.f,damageDispatch(itm),2,iteration);

	characterStartAnimation(c,0,240);
	characterAddCooldown(c,80);
}

void characterPrimary(character *c){
	item *itm = &c->inventory[c->activeItem];
	if(hasPrimaryAction(itm)){
		primaryActionDispatch(itm,c);
		return;
	}
	ivec los = characterLOSBlock(c,0);
	if(los.x < 0){
		if(c->actionTimeout >= 0){characterHit(c);}
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

void characterSecondary(character *c){
	item *cItem = characterGetItemBarSlot(c,c->activeItem);
	if(!itemIsEmpty(cItem)){
		secondaryActionDispatch(cItem,c);
	}
}

void characterTertiary(character *c){
	item *cItem = characterGetItemBarSlot(c,c->activeItem);
	if(!itemIsEmpty(cItem)){
		tertiaryActionDispatch(cItem,c);
	}
}

void characterDropItem(character *c, int i){
	item *cItem = characterGetItemBarSlot(c,i);
	if(cItem == NULL)       { return; }
	if(itemIsEmpty(cItem))  { return; }

	itemDropNewC(c, cItem);
	itemDiscard(cItem);
}

void characterDropSingleItem(character *c, int i){
	if(c->actionTimeout < 0){ return; }
	item *cItem = characterGetItemBarSlot(c,i);
	if(cItem == NULL)       { return; }
	if(itemIsEmpty(cItem))  { return; }
	item dItem = itemNew(cItem->ID,1);
	dItem.amount = itemDecStack(cItem,1);
	if(itemIsEmpty(&dItem)) { return; }
	characterAddCooldown(c,50);
	itemDropNewC(c, &dItem);
}

void characterDie(character *c){
	if(c != player)               { return; }
	if(c->flags & CHAR_SPAWNING)  { return; }
	if(c->flags & CHAR_NOCLIP)    { return; }
	for(int i=0;i<40;i++){
		itemDropNewD(c->pos, characterGetItemBarSlot(c,i));
	}
	for(int i=0;i<3;i++){
		itemDropNewD(c->pos, &c->equipment[i]);
	}
	characterInit(c);
	setOverlayColor(0xFF000000,0);
	msgRequestPlayerSpawnPos();
}

void updateGlide(character *c){
	if((c == player) && ((itemIsEmpty(&c->equipment[CHAR_EQ_GLIDER])) || (c->equipment[CHAR_EQ_GLIDER].ID != I_Glider))){
		c->flags &= ~CHAR_GLIDE;
		return;
	}
	if(!(c->flags & CHAR_GLIDE)){return;}
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

	vec  vdrg   = vecMulS(vecInvert(vel),drag * 0.1f);
	float mag   = vecMag(vdrg);
	c->shake    = MAX(c->shake,mag*16.f + speed);
	vel         = vecAdd(vel,vdrg);
	vel         = vecAdd(vel,vecMulS(dir,mag*0.98f));

	c->vel = vel;
}

int characterPhysics(character *c){
	int ret=0;
	u32 col;
	c->pos = vecAdd(c->pos,c->vel);
	c->shake = MAX(0.f,c->shake-0.1f);
	if(c->flags & CHAR_NOCLIP){
		col = characterCollision(c->pos);
		if(col){ c->flags |= CHAR_COLLIDE; }
		return 0;
	}

	c->vel.y -= 0.0005f;
	if(c->vel.y < -4.0f){c->vel.y+=0.005f;}
	if(c->vel.y >  4.0f){c->vel.y-=0.005f;}
	if(c->vel.x < -4.0f){c->vel.x+=0.005f;}
	if(c->vel.x >  4.0f){c->vel.x-=0.005f;}
	if(c->vel.z < -4.0f){c->vel.z+=0.005f;}
	if(c->vel.z >  4.0f){c->vel.z-=0.005f;}

	c->flags |=  CHAR_FALLING;
	c->flags &= ~CHAR_COLLIDE;
	col = characterCollision(c->pos);
	if(col){ c->flags |= CHAR_COLLIDE; }
	if((col&0x110) && (c->vel.x < 0.f)){
		if(c->vel.x < -0.1f){ ret += (int)(fabsf(c->vel.x)*512.f); }
		c->pos.x = MAX(c->pos.x,floor(c->pos.x)+0.3f);
		c->vel.x = c->vel.x*-0.3f;
	}
	if((col&0x220) && (c->vel.x > 0.f)){
		if(c->vel.x >  0.1f){ ret += (int)(fabsf(c->vel.x)*512.f); }
		c->pos.x = MIN(c->pos.x,floorf(c->pos.x)+0.7f);
		c->vel.x = c->vel.x*-0.3f;
	}
	if((col&0x880) && (c->vel.z > 0.f)){
		if(c->vel.z >  0.1f){ ret += (int)(fabsf(c->vel.z)*512.f); }
		c->pos.z = MIN(c->pos.z,floorf(c->pos.z)+0.7f);
		c->vel.z = c->vel.z*-0.3f;
	}
	if((col&0x440) && (c->vel.z < 0.f)){
		if(c->vel.z < -0.1f){ ret += (int)(fabsf(c->vel.z)*512.f); }
		c->pos.z = MAX(c->pos.z,floorf(c->pos.z)+0.3f);
		c->vel.z = c->vel.z*-0.3f;
	}
	if((col&0x0F0) && (c->vel.y > 0.f)){
		if(c->vel.y >  0.1f){ ret += (int)(fabsf(c->vel.y)*512.f); }
		c->pos.y = MIN(c->pos.y,floorf(c->pos.y)+0.5f);
		c->vel.y = c->vel.y*-0.3f;
	}
	if((col&0x00F) && (c->vel.y < 0.f)){
		c->flags &= ~CHAR_FALLING;
		if(c->vel.y < -0.15f){
			ret += (int)(fabsf(c->vel.y)*512.f);
		}
		if(c->vel.y < -0.02f){
			c->yoff += MAX(-.9f,c->vel.y * 10.f);
		}
		if(c->vel.y < -0.1f){
			c->vel.y = c->vel.y*-0.3f;
		}else{
			c->vel = vecMul(c->vel,vecNew(0.98f,0,0.98f));
		}
		c->pos.y = MAX(c->pos.y,floorf(c->pos.y)+.99f);
	}

	updateGlide(c);
	return ret;
}

void characterUpdateBooster(character *c){
	if(!(c->flags & CHAR_SNEAK)){
		sfxLoop(sfxJet,0.f);
		return;
	}
	if((itemIsEmpty(&c->equipment[CHAR_EQ_PACK])) || (c->equipment[CHAR_EQ_PACK].ID != I_Jetpack)){return;}
	const vec rot = c->rot;
	float speed    = 0.0002f / MAX(0.1,vecMag(c->vel));
	const vec nv   = vecMulS(vecDegToVec(rot),speed);
	c->vel = vecAdd(c->vel,nv);
	c->shake = MAX(c->shake,1.25f + speed);
	if(c == player){
		sfxLoop(sfxJet,1.f);
	}

	const vec adir = vecMulS(vecDegToVec(vecAdd(rot,vecMulS(vecRng(),10.f))),-0.07f * (rngValf()+.5f));
	const vec apos = vecAdd(c->pos,vecAdd(adir,vecMulS(vecRng(),0.1f)));
	newParticleV(apos, adir, vecZero(),96.f, 0.1f, 0xE643B0F8,  192);
	const vec bdir = vecMulS(vecDegToVec(vecAdd(rot,vecMulS(vecRng(),10.f))),-0.03f * (rngValf()+.5f));
	const vec bpos = vecAdd(c->pos,vecAdd(adir,vecMulS(vecRng(),0.1f)));
	newParticleV(bpos, bdir, vecZero(),48.f, 0.2f, 0xC42370FA,  386);
	const vec cdir = vecMulS(vecDegToVec(vecAdd(rot,vecMulS(vecRng(), 4.f))),-0.01f * (rngValf()+.5f));
	const vec cpos = vecAdd(c->pos,vecAdd(adir,vecMulS(vecRng(),0.1f)));
	newParticleV(cpos, cdir, vecNew(0,0.00001,0),24.f+rngValf()*24.f, 0.4f+rngValf()*0.4f, 0xC4233A4A, 1536+rngValM(512));
	if(rngValM(6)==0){
		const vec ddir = vecMulS(vecDegToVec(vecAdd(rot,vecMulS(vecRng(), 4.f))),-0.001f * (rngValf()+.5f));
		const vec dpos = vecAdd(c->pos,vecAdd(adir,vecMulS(vecRng(),0.1f)));
		newParticleV(dpos, ddir, vecNew(0,0.00001,0),16.f+rngValf()*32.f, 0.2f+rngValf()*0.1f, 0xC4131A24, 3072+rngValM(1024));
	}
}

void characterUpdateFalling(character *c){
	if(c != player){return;}
	if(c->pos.y < -512){
		characterDie(c);
		msgSendDyingMessage("fell into the abyss", 65535);
	}
	if(c->pos.y < -64){
		setOverlayColor(0xFF000000,1000);
		if(!(c->flags & CHAR_FALLINGSOUND)){
			c->flags |= CHAR_FALLINGSOUND;
			sfxPlay(sfxFalling,1.f);
		}
	}
}

void characterUpdate(character *c){
	float walkFactor = 1.f;
	vec nvel;

	if(c->flags & CHAR_SPAWNING){return;}
	if((c->flags & CHAR_FALLINGSOUND) && (c->pos.y > -32)){ c->flags &= ~CHAR_FALLINGSOUND; }
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

	if(c->actionTimeout < 0){ c->actionTimeout++; }
	if(c->flags & CHAR_NOCLIP){
		c->vel = c->gvel;
		characterUpdateHook(c);
		characterUpdateAnimation(c);
		characterUpdateInaccuracy(c);
		characterPhysics(c);
		return;
	}
	characterUpdateBooster(c);
	if((c->flags & (CHAR_GLIDE | CHAR_FALLING)) == (CHAR_GLIDE | CHAR_FALLING)){
		characterUpdateHook(c);
		characterUpdateAnimation(c);
		characterUpdateInaccuracy(c);
		characterUpdateDamage(c,characterPhysics(c));
		characterUpdateWindVolume(c);
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
	if((c->hook != NULL) && (grapplingHookGetHooked(c->hook))){
		if(fabsf(c->gvel.x) < 0.001)                 {nvel.x=c->vel.x;}
		if(fabsf(c->gvel.z) < 0.001)                 {nvel.z=c->vel.z;}
		if((c->gvel.x < -0.001)&&(nvel.x > c->vel.x)){nvel.x=c->vel.x;}
		if((c->gvel.x >  0.001)&&(nvel.x < c->vel.x)){nvel.x=c->vel.x;}
		if((c->gvel.z < -0.001)&&(nvel.z > c->vel.z)){nvel.z=c->vel.z;}
		if((c->gvel.z >  0.001)&&(nvel.z < c->vel.z)){nvel.z=c->vel.z;}
	}
	if(characterUpdateJumping(c)){ nvel.y = 0.044f;}
	c->vel = nvel;

	const int damage = characterPhysics(c);
	if(c == player){
		characterUpdateDamage(c,damage);
		if((nvel.y < -0.2f) && c->vel.y > -0.01f){
			sfxPlay(sfxImpact,1.f);
		} else if((nvel.y < -0.05f) && c->vel.y > -0.01f){
			sfxPlay(sfxStomp,1.f);
		}
		if((damage > 0) && (grapplingHookGetHooked(c->hook))){
			grapplingHookReturnHook(c->hook);
		}
		characterUpdateWindVolume(c);
		characterUpdateHook(c);
	}
	characterUpdateInaccuracy(c);
	characterUpdateYOff(c);
	characterUpdateAnimation(c);
	characterHP(c,0);
}

void charactersUpdate(){
	for(int i=0;i<characterCount;i++){
		characterUpdate(&characterList[i]);
	}
}

void characterFireHook(character *c){
	if(c->actionTimeout < 0){return;}
	characterAddCooldown(c,60);
	if(c->hook == NULL){
		c->hook = grapplingHookNew(c);
		sfxPlay(sfxHookFire,1.f);
		characterStartAnimation(c,1,350);
	}else{
		grapplingHookReturnHook(c->hook);
		characterStartAnimation(c,1,350);
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
		grapplingHookFree(c->hook);
		c->hook = NULL;
	}
}

void characterMoveDelta(character *c, const packet *p){
	c->vel   = vecAdd(c->vel,vecNewP(&p->v.f[0]));
	c->rot   = vecAdd(c->rot,vecNewP(&p->v.f[3]));
	c->shake = vecMag(c->vel)*8.f;
}

void characterShadesDraw(const character *c){
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

void characterGliderDraw(const character *c){
	static u64 ticks = 0;
	if(c->gliderFade < 0.01f){return;}
	const float breath = sinf((float)(c->breathing-384)/512.f)*4.f;
	float deg  = ((float)++ticks*0.4f);
	float yoff = cos(deg*2.1f)*player->shake;
	float xoff = sin(deg*1.3f)*player->shake;

	matMov(matMVP,matView);
	matMulTrans(matMVP,c->pos.x,c->pos.y+c->yoff,c->pos.z);
	matMulRotYX(matMVP,-c->rot.yaw+xoff,-(c->rot.pitch-((1.f - c->gliderFade)*90.f)-breath+yoff));
	matMulTrans(matMVP,0.f,0.4f,-0.2f);
	matMulScale(matMVP,c->gliderFade, c->gliderFade, c->gliderFade);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(meshGlider);
}

void characterActiveItemDraw(const character *c){
	const item *activeItem;
	mesh *aiMesh;
	float sneakOff = 0.f;
	if(c->flags & CHAR_SNEAK){sneakOff = 1.f;}

	activeItem = &c->inventory[c->activeItem];
	if(activeItem == NULL)     {return;}
	if(itemIsEmpty(activeItem)){return;}
	aiMesh = getMeshDispatch(activeItem);
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
		default:
			hitOff = animationInterpolation(c->animationTicksLeft,c->animationTicksMax,0.3f);
			y = iy+c->yoff-(hitOff/8);
			matMulTrans(matMVP,ix-hitOff*0.2f,y+(hitOff/3),iz - hitOff*0.5f);
			matMulRotYX(matMVP,hitOff*5.f,hitOff*-20.f);
		break;

		case 1:
			hitOff = animationInterpolation(c->animationTicksLeft,c->animationTicksMax,0.5f);
			matMulTrans(matMVP,ix,c->yoff+iy,iz + hitOff*0.3f);
			matMulRotYX(matMVP,hitOff*10.f,hitOff*45.f);
		break;

		case 2:
			hitOff = animationInterpolationSustain(c->animationTicksLeft,c->animationTicksMax,0.3f,0.5f);
			y = iy+c->yoff-(hitOff/8);
			matMulTrans(matMVP,ix-hitOff*0.5f,y-(hitOff*0.5f),iz - hitOff*0.2f);
			matMulRotYX(matMVP,hitOff*15.f,hitOff*-55.f);
		break;

		case 3:
			hitOff = animationInterpolation(c->animationTicksLeft,c->animationTicksMax,0.5f);
			matMulTrans(matMVP,ix,c->yoff+iy,iz + hitOff*0.1f);
			matMulRotYX(matMVP,hitOff*3.f,hitOff*9.f);
		break;

		case 4:
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

		case 5:
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
	if(c == player)     {return;}
	if(c->eMesh == NULL){return;}
	shadowAdd(c->pos,0.75f);

	const float breath = sinf((float)c->breathing/512.f)*6.f;

	matMov(matMVP,matView);
	matMulTrans(matMVP,c->pos.x,c->pos.y+c->yoff+breath/128.f,c->pos.z);
	matMulRotYX(matMVP,-c->rot.yaw,-c->rot.pitch/6.f + breath);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(c->eMesh);
	c->screenPos = matMulVec(matMVP,vecNew(0,0.5f,0));

	characterActiveItemDraw(c);
	characterShadesDraw(c);
	characterGliderDraw(c);
}

void characterDrawAll(){
	shaderBind(sMesh);
	for(int i=0;i<characterCount;i++){
		if(characterList[i].nextFree != NULL){ continue; }
		characterDraw(&characterList[i]);
	}
}

static void characterDyingMessage(u16 cause, u16 culprit){
	const char *messages[3] = {
		"died by command",
		"beamblasted",
		"clubbed"
	};
	if(cause > 3){ cause = 0; }
	msgSendDyingMessage(messages[cause], culprit);
}

void characterDamagePacket(character *c, const packet *p){
	const being target  = p->v.u32[1];
	const being culprit = p->v.u32[2];
	const u16 cause     = p->v.u16[1];
	const i16 hp        = p->v.u16[0];
	if(beingType(target) != BEING_CHARACTER){return;}
	if(beingID(target) != (uint)playerID)         {return;}

	if(cause == 2){
		sfxPlay(sfxImpact,1.f);
		sfxPlay(sfxUngh,  1.f);
		setOverlayColor(0xA03020F0,0);
		commitOverlayColor();
		vec pos = vecNewP(&p->v.f[3]);
		vec dis = vecNorm(vecSub(c->pos,pos));
		c->vel = vecAdd(c->vel,vecMulS(dis,0.04f));
	}
	if(characterDamage(c,hp)){
		characterDyingMessage(cause,culprit);
	}
}

void characterGotHitPacket(const packet *p){
	const being target  = p->v.u32[1];
	character *c = NULL;
	if(beingType(target) != BEING_CHARACTER){return;}
	if(beingID(target) == (uint)playerID){
		c = player;
	}else{
		if(beingID(target) > 32){return;}
		c = playerList[beingID(target)];
	}
	if(c == NULL){return;}
	fxBleeding(c->pos,target,p->v.i16[0],p->v.u16[1]);
}


void characterSetData(character *c, const packet *p){
	c->hp         = p->v.i16[0];
	c->activeItem = p->v.u16[1];
	c->flags      = p->v.u32[2];
	playerID      = p->v.u16[4];
	connectionState = 2;
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

int characterHitCheck(const vec pos, float mdd, int damage, int cause, u16 iteration){
	int hits = 0;
	for(int i=0;i<32;i++){
		if(playerList[i] == player)          {continue;}
		if(playerList[i] == NULL  )          {continue;}
		if(playerList[i]->temp == iteration) {continue;}
		vec dis = vecSub(pos,playerList[i]->pos);
		if(vecDot(dis,dis) < mdd){
			msgBeingDamage(0,damage,cause,beingCharacter(i),0,pos);
			playerList[i]->temp = iteration;
			hits++;
		}
	}
	return hits;
}
