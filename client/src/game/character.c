#include "../game/character.h"
#include "../main.h"
#include "../game/blockType.h"
#include "../game/itemDrop.h"
#include "../game/grapplingHook.h"
#include "../game/blockMining.h"
#include "../gui/gui.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gui/overlay.h"
#include "../tmp/objs.h"
#include "../gfx/shader.h"
#include "../gfx/particle.h"
#include "../gfx/texture.h"
#include "../network/chat.h"
#include "../misc/options.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/mods/mods.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"
#include "../sdl/sdl.h"
#include "../sdl/sfx.h"
#include "../sdl/input_gamepad.h"
#include "../voxel/bigchungus.h"

#include <math.h>

character *player;
character  characterList[64];
int        characterCount = 0;
character *characterFirstFree = NULL;
character *playerList[32];

void characterInit(character *c){
	if(c->hook != NULL){
		grapplingHookFree(c->hook);
		c->hook = NULL;
	}
	memset(c,0,sizeof(character));

	c->breathing     = rngValM(1024);
	c->maxhp = c->hp = 20;
	c->blockMiningX  = c->blockMiningY = c->blockMiningZ = -1;
	c->pos           = vecNew(1024,1024,1024);
	c->rot           = vecNew(135.f,15.f,0.f);
	c->eMesh         = meshPear;

	if(c == player){
		sfxLoop(sfxWind,0.f);
		sfxLoop(sfxHookRope,0.f);
		msgRequestPlayerSpawnPos();
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

void characterSetPlayerPos(const packet *p){
	const int i = p->val.i[19];
	if(playerList[i] == NULL){
		playerList[i] = characterNew();
	}
	playerList[i]->pos  = vecNewP(&p->val.f[0]);
	playerList[i]->rot  = vecNewP(&p->val.f[3]);
	playerList[i]->vel  = vecNewP(&p->val.f[6]);
	playerList[i]->yoff = p->val.f[9];

	if(p->val.i[10]){
		if(playerList[i]->hook == NULL){
			playerList[i]->hook = grapplingHookNew(playerList[i]);
		}
		playerList[i]->hook->hooked     = true;
		playerList[i]->hook->ent->flags = ENTITY_NOCLIP;
		playerList[i]->hook->ent->pos   = vecNewP(&p->val.f[11]);
		playerList[i]->hook->ent->vel   = vecZero();
	}else{
		if(playerList[i]->hook != NULL){
			grapplingHookFree(playerList[i]->hook);
			playerList[i]->hook = NULL;
		}
	}
	playerList[i]->inventory[0] = itemNew(p->val.u[17],1);
	playerList[i]->activeItem   = 0;

	playerList[i]->animationIndex     = p->val.i[18];
	playerList[i]->animationTicksMax  = p->val.i[20];
	playerList[i]->animationTicksLeft = p->val.i[21];
	playerList[i]->flags              = p->val.i[22];
	playerList[i]->hp                 = p->val.i[23];
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
		float gl = grapplingHookGetGoalLength(c->hook);
		if((c->gvel.y > 0) && (gl > 1.f)){
			grapplingHookSetGoalLength(c->hook,gl-0.1f);
		}
		if((c->flags & CHAR_SNEAK) && (gl < 96.f)){
			grapplingHookSetGoalLength(c->hook,gl+0.1f);
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

// TODO: knochback direction gets calculated from the center of the hit sphere and NOT the attackign character, leading to unintuitive knocback directions
void characterHitCheck(character *c, int origin, const vec pos, const vec rot, int pwr){
	const vec dist = vecSub(vecAdd(pos,vecDegToVec(rot)),c->pos);
	const float d  = vecMag(dist);

	if(d < 1.f){
		sfxPlay(sfxImpact,1.f);
		sfxPlay(sfxUngh,  1.f);
		setOverlayColor(0xA03020F0,0);
		if(characterHP(c,-pwr)){
			msgSendDyingMessage("clubbed",origin);
			setOverlayColor(0xFF000000,0);
		}
		commitOverlayColor();
		msgCharacterGotHit(-1,pwr);

		c->vel = vecAdd(c->vel,vecMulS(dist,sqrtf((4.f*pwr)/d) * -0.01f));
	}
}

void characterGotHitBroadcast(int i,int pwr){
	if(playerList[i] == NULL){return;}
	character *c   = playerList[i];
	const float d  = vecMag(vecSub(player->pos,c->pos));
	if(d > 128.f){return;}
	const float vol = (128.f-d)/128.f;

	sfxPlay(sfxImpact,vol);
	sfxPlay(sfxUngh,vol);
	(void)pwr;
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

void characterPrimary(character *c){
	int cx,cy,cz;
	item *itm = &c->inventory[c->activeItem];
	if(hasPrimaryAction(itm)){
		primaryActionDispatch(itm,c,c->actionTimeout);
	}else{
		if(characterLOSBlock(c,&cx,&cy,&cz,0)){
			c->blockMiningX = cx;
			c->blockMiningY = cy;
			c->blockMiningZ = cz;
			if(c->actionTimeout >= 0){
				sfxPlay(sfxTock,1.f);
				vibrate(0.3f);
				characterStartAnimation(c,0,300);
				characterAddCooldown(c,80);
				msgCharacterHit(-1,c->pos,c->rot,damageDispatch(itm));
			}
		}else if(c->actionTimeout >= 0){
			msgCharacterHit(-1,c->pos,c->rot,damageDispatch(itm));
			characterStartAnimation(c,0,240);
			characterAddCooldown(c,80);
		}
	}
}

void characterStopMining(character *c){
	c->blockMiningX = c->blockMiningY = c->blockMiningZ = -1;
}

void characterSecondary(character *c){
	item *cItem = characterGetItemBarSlot(c,c->activeItem);
	if(!itemIsEmpty(cItem)){
		secondaryActionDispatch(cItem,c,c->actionTimeout);
	}
}

void characterTertiary(character *c){
	item *cItem = characterGetItemBarSlot(c,c->activeItem);
	if(!itemIsEmpty(cItem)){
		tertiaryActionDispatch(cItem,c,c->actionTimeout);
	}
}

void characterDropItem(character *c, int i){
	item *cItem;
	if(c->actionTimeout < 0){ return; }
	cItem = characterGetItemBarSlot(c,i);
	if(cItem == NULL)       { return; }
	if(itemIsEmpty(cItem))  { return; }
	characterAddCooldown(c,50);

	itemDropNewC(c, cItem);
	itemDiscard(cItem);
}

void characterDie(character *c){
	if(c != player)               { return; }
	if(c->flags & CHAR_SPAWNING)  { return; }
	for(int i=0;i<40;i++){
		item *cItem = characterGetItemBarSlot(c,i);
		if(cItem == NULL)     { continue; }
		if(itemIsEmpty(cItem)){ continue; }
		itemDropNewD(vecAdd(c->pos,vecNew(0,3,0)), cItem);
	}
	characterInit(c);
	setOverlayColor(0xFF000000,0);
}

void updateGlide(character *c){
	vec   dir   = vecDegToVec(c->rot);
	vec   vel   = c->vel;
	vec  vdeg   = vecVecToDeg(vecNorm(vel));

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
			c->yoff = -0.8f;
			ret += (int)(fabsf(c->vel.y)*512.f);
		}else if(c->vel.y < -0.07f){
			c->yoff += -0.4f;
		}else if(c->vel.y < -0.04f){
			c->yoff += -0.2f;
		}
		c->vel = vecMul(c->vel,vecNew(0.97f,0,0.97f));
	}

	if(c->flags & CHAR_GLIDE){ updateGlide(c); }
	return ret;
}

void characterUpdateBooster(character *c){
	if(!(c->flags & CHAR_SNEAK)){
		sfxLoop(sfxJet,0.f);
		return;
	}
	const vec rot = c->rot;
	//if(!(c->flags & CHAR_SNEAK) && (c->gvel.y > 0.9) && !(c->flags & CHAR_GLIDE)){rot.pitch = -90.f;}
	float speed    = 0.0001f / MAX(0.1,vecMag(c->vel));
	const vec nv   = vecMulS(vecDegToVec(rot),speed);
	c->vel = vecAdd(c->vel,nv);
	c->shake = MAX(c->shake,1.25f + speed);
	sfxLoop(sfxJet,1.f);

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

void characterUpdate(character *c){
	float walkFactor = 1.f;
	vec nvel;

	if(c->flags & CHAR_SPAWNING){return;}

	if((c->flags & CHAR_FALLINGSOUND) && (c->pos.y > -32)){ c->flags &= ~CHAR_FALLINGSOUND; }
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
		characterUpdateWindVolume(c);
		characterUpdateHook(c);
	}
	characterUpdateInaccuracy(c);
	characterUpdateYOff(c);
	characterUpdateAnimation(c);
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

void characterFreeHook(character *c){
	if(c->hook != NULL){
		grapplingHookFree(c->hook);
		c->hook = NULL;
	}
}

void characterMoveDelta(character *c, const packet *p){
	c->vel   = vecAdd(c->vel,vecNewP(&p->val.f[0]));
	c->rot   = vecAdd(c->rot,vecNewP(&p->val.f[3]));
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

void characterDraw(const character *c){
	if(c == NULL)       {return;}
	if(c == player)     {return;}
	if(c->eMesh == NULL){return;}

	const float breath = sinf((float)c->breathing/512.f)*6.f;

	matMov(matMVP,matView);
	matMulTrans(matMVP,c->pos.x,c->pos.y+c->yoff+breath/128.f,c->pos.z);
	matMulRotYX(matMVP,-c->rot.yaw,-c->rot.pitch/6.f + breath);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(c->eMesh);

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

void characterDamagePacket(character *c, const packet *p){
	if(characterDamage(c,p->val.i[0])){
		msgSendDyingMessage("died by command", 65535);
	}
}

bool itemPlaceBlock(item *i,character *chr, int to){
	int cx,cy,cz;
	if(to < 0){return false;}
	if(characterLOSBlock(chr,&cx,&cy,&cz,true)){
		if((characterCollision(chr->pos)&0xFF0)){ return false; }
		if(!itemDecStack(i,1)){ return false; }
		worldSetB(cx,cy,cz,i->ID);
		if((characterCollision(chr->pos)&0xFF0) != 0){
			worldSetB(cx,cy,cz,0);
			itemIncStack(i,1);
			return false;
		} else {
			msgPlaceBlock(cx,cy,cz,i->ID);
			sfxPlay(sfxPock,1.f);
			characterStartAnimation(chr,0,240);
			characterAddCooldown(chr,50);
			return true;
		}
	}
	return false;
}

void characterSetData(character *c, const packet *p){
	c->hp         = p->val.i[0];
	c->activeItem = p->val.i[1];
	c->flags      = p->val.u[2];
}
