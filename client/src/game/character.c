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
#include "../gfx/objs.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../../../common/src/misc.h"
#include "../network/chat.h"
#include "../network/packet.h"
#include "../network/messages.h"
#include "../sdl/sdl.h"
#include "../sdl/sfx.h"
#include "../sdl/input_gamepad.h"
#include "../voxel/bigchungus.h"

#include <math.h>

character *player;
character characterList[128];
int characterCount = 0;
character *characterFirstFree = NULL;
character *playerList[32];

uint32_t characterCollision(character *c, float cx, float cy, float cz,float wd){
	(void)c;
	uint32_t col = 0;
	const float WD=wd*2.f;

	if(checkCollision(cx-wd,cy+0.5f,cz   )){col |=  0x10;}
	if(checkCollision(cx+wd,cy+0.5f,cz   )){col |=  0x20;}
	if(checkCollision(cx   ,cy+0.5f,cz-wd)){col |=  0x40;}
	if(checkCollision(cx   ,cy+0.5f,cz+wd)){col |=  0x80;}

	if(checkCollision(cx-wd,cy-1.f ,cz   )){col |=   0x1;}
	if(checkCollision(cx+wd,cy-1.f ,cz   )){col |=   0x2;}
	if(checkCollision(cx   ,cy-1.f ,cz-wd)){col |=   0x4;}
	if(checkCollision(cx   ,cy-1.f ,cz+wd)){col |=   0x8;}

	if(checkCollision(cx-WD,cy-0.7f,cz   )){col |= 0x100;}
	if(checkCollision(cx+WD,cy-0.7f,cz   )){col |= 0x200;}
	if(checkCollision(cx   ,cy-0.7f,cz-WD)){col |= 0x400;}
	if(checkCollision(cx   ,cy-0.7f,cz+WD)){col |= 0x800;}

	return col;
}

void characterSetPlayerPos(int i, packetLarge *p){
	if(playerList[i] == NULL){
		playerList[i] = characterNew();
	}
	playerList[i]->x     = p->val.f[0];
	playerList[i]->y     = p->val.f[1];
	playerList[i]->z     = p->val.f[2];
	playerList[i]->yaw   = p->val.f[3];
	playerList[i]->pitch = p->val.f[4];
	playerList[i]->roll  = p->val.f[5];
	playerList[i]->vx    = p->val.f[6];
	playerList[i]->vy    = p->val.f[7];
	playerList[i]->vz    = p->val.f[8];
	playerList[i]->yoff  = p->val.f[9];

	if(p->val.i[10] > 0){
		if(playerList[i]->hook == NULL){
			playerList[i]->hook = grapplingHookNew(playerList[i]);
		}
		playerList[i]->hook->hooked  = true;
		playerList[i]->hook->ent->noClip = 1;
		playerList[i]->hook->ent->x  = p->val.f[11];
		playerList[i]->hook->ent->y  = p->val.f[12];
		playerList[i]->hook->ent->z  = p->val.f[13];
		playerList[i]->hook->ent->vx = 0.f;
		playerList[i]->hook->ent->vy = 0.f;
		playerList[i]->hook->ent->vz = 0.f;
	}else{
		if(playerList[i]->hook != NULL){
			grapplingHookFree(playerList[i]->hook);
			playerList[i]->hook = NULL;
		}
	}
}

void characterSetPos(character *c, float x, float y, float z){
	c->x = x;
	c->y = y;
	c->z = z;
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

void characterEmptyInventory(character *c){
	for(unsigned int i=0;i<40;i++){
		c->inventory[i] = itemEmpty();
	}
	c->inventory[0] = itemNew(261, 1);
	c->inventory[1] = itemNew(259, 1);
	c->inventory[2] = itemNew(260, 1);
	c->inventory[3] = itemNew(256,99);
	c->inventory[4] = itemNew(257,99);
	c->inventory[5] = itemNew(258,99);
	c->inventory[6] = itemNew(  1,99);
	c->inventory[7] = itemNew(  2,99);
	c->inventory[8] = itemNew(  3,99);
}

void characterUpdateHook(character *c){
	if(c->hook == NULL){ return; }
	if(grapplingHookUpdate(c->hook)){
		grapplingHookFree(c->hook);
		c->hook = NULL;
		return;
	}
	if(grapplingHookGetHooked(c->hook)){
		float gl = grapplingHookGetGoalLength(c->hook);
		if((c->gvy > 0) && (gl > 1.f)){
			grapplingHookSetGoalLength(c->hook,gl-0.05f);
		}
		if((c->sneak) && (gl < 96.f)){
			grapplingHookSetGoalLength(c->hook,gl+0.05f);
		}
		if(grapplingHookGetLength(c->hook) > gl){
			grapplingHookPullTowards(c->hook,c);
		}
	}
}

void characterUpdateHitOff(character *c){
	if(c->hasHit){
		c->hitOff += (c->hitOff*0.1f)+0.05f;
		if(c->hitOff > 1.f){
			c->hasHit = false;
		}
	}else if(c->hitOff > 0.f){
		c->hitOff -= c->hitOff*0.02f;
		if(c->hitOff < 0.f){
			c->hitOff = 0.f;
		}
	}
}

void characterUpdateWindVolume(character *c, float wvx, float wvy, float wvz){
	(void)c;

	float windVol = fabsf(wvx)+fabsf(wvy)+fabsf(wvz);
	if(windVol < 0.1f){
		sfxLoop(atmosfxWind,0.f);
	}else{
		windVol = (windVol - 0.1f)/4.f;
		if(windVol > 1.f){windVol = 1.f;}
		vibrate(windVol);
		sfxLoop(atmosfxWind,windVol);
	}
}


int characterUpdateJumping(character *c){
	if((c->gvy > 0) && !c->falling && ((c->hook == NULL) || (!grapplingHookGetHooked(c->hook)))){
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
	if(damage > 0){
		if(damage > 8){
			sfxPlay(sfxImpact,1.f);
			sfxPlay(sfxUngh,1.f);
			setOverlayColor(0xA03020F0,0);
			if(characterHP(c,damage / -8)){
				msgSendDyingMessage("did not bounce", 65535);
				setOverlayColor(0x00000000,0);
				commitOverlayColor();
			}
		}else{
			sfxPlay(sfxStomp,1.f);
		}
	}
}

void characterUpdateYOff(character *c){
	if(!c->falling && ((fabsf(c->gvx) > 0.001f) || (fabsf(c->gvz) > 0.001f))){
		if(c->sneak){
			if(getTicks() > (c->stepTimeout + 600)){
				c->stepTimeout = getTicks();
				if(c->gyoff > -0.1f){
					c->gyoff = -0.2f;
					sfxPlay(sfxStep,0.6f);
				}else{
					c->gyoff =  0.0f;
				}
			}
		}else{
			if(getTicks() > (c->stepTimeout + 200)){
				c->stepTimeout = getTicks();
				if(c->gyoff > -0.1f){
					c->gyoff = -0.2f;
					sfxPlay(sfxStep,1.f);
				}else{
					c->gyoff =  0.0f;
				}
			}
		}
	}else{
		c->gyoff =  0.0f;
	}

	if(fabsf(c->gyoff - c->yoff) < 0.001f){
		c->yoff = c->gyoff;
	}else if(c->gyoff < c->yoff){
		if(c->sneak){
			c->yoff = c->yoff - (0.015f * (c->yoff - c->gyoff));
		}else{
			c->yoff = c->yoff - (0.03f * (c->yoff - c->gyoff));
		}
	}else{
		if(c->sneak){
			c->yoff = c->yoff + (0.015f*(c->gyoff - c->yoff));
		}else{
			c->yoff = c->yoff + (0.03f*(c->gyoff - c->yoff));
		}
	}
}

void characterInit(character *c){
	int sx=-1024,sy=1024,sz=-1024;
	c->gyoff = 0.f;
	c->gvx = c->gvy = c->gvz = 0.f;

	c->collide      = false;
	c->noClip       = false;
	c->falling      = false;
	c->fallingSound = false;
	c->sneak        = false;
	c->hasHit       = false;

	c->actionTimeout = 0;
	c->stepTimeout   = 0;

	c->maxhp = c->hp = 20;
	c->activeItem = 0;
	c->hitOff     = 0.f;
	c->blockMiningX = c->blockMiningY = c->blockMiningZ = -1;

	c->x     = sx+0.5f;
	c->y     = sy+1.0f;
	c->z     = sz+0.5f;
	c->yaw   = 135.f;
	c->pitch = 0.f;
	c->roll  = 0.f;
	c->yoff  = 0.f;
	c->eMesh = meshPear;
	c->vx = c->vy = c->vz = 0.f;

	if(c->hook != NULL){
		grapplingHookFree(c->hook);
		c->hook = NULL;
	}
	if(c == player){
		sfxLoop(atmosfxWind,0.f);
		sfxLoop(atmosfxHookRope,0.f);
		msgRequestPlayerSpawnPos();
	}

	characterEmptyInventory(c);
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

bool characterLOSBlock(character *c, int *retX, int *retY, int *retZ, int returnBeforeBlock) {
	float cvx,cvy,cvz;
	float cx,cy,cz;
	float lx,ly,lz;
	int   lastBlock=-1;
	const float yaw   = c->yaw;
	const float pitch = c->pitch;

	cvx = (cos((yaw-90.f)*PI/180) * cos((-pitch)*PI/180))*0.1f;
	cvy = (sin((-pitch)*PI/180))*0.1f;
	cvz = (sin((yaw-90.f)*PI/180) * cos((-pitch)*PI/180))*0.1f;

	cx = lx = c->x;
	cy = ly = c->y+0.5f;
	cz = lz = c->z;

	for(int i=0;i<50;i++){
		if((lastBlock == -1) || (fabsf(lx - floorf(cx)) > 0.1f) || (fabsf(ly - floorf(cy)) > 0.1f) || (fabsf(lz - floorf(cz)) > 0.1f)){
			lastBlock = worldGetB(cx,cy,cz);
			if(lastBlock > 0){
				if(returnBeforeBlock){
					*retX = lx;
					*retY = ly;
					*retZ = lz;
				}else{
					*retX = cx;
					*retY = cy;
					*retZ = cz;
				}
				return true;
			}
			lx = floorf(cx);
			ly = floorf(cy);
			lz = floorf(cz);
		}

		cx += cvx;
		cy += cvy;
		cz += cvz;
	}
	return false;
}

void characterMineBlock(character *c){
	int cx,cy,cz;
	item *itm = &c->inventory[c->activeItem];
	if(itemHasMineAction(itm)){
		if(itemMineAction(itm,c,c->actionTimeout)){
			c->hasHit = true;
			c->actionTimeout = 0;
		}
	}else if(characterLOSBlock(c,&cx,&cy,&cz,0)){
		c->blockMiningX = cx;
		c->blockMiningY = cy;
		c->blockMiningZ = cz;
		if(c->actionTimeout >= 60){
			c->hasHit = true;
			c->actionTimeout = 0;
			sfxPlay(sfxTock,1.f);
			vibrate(0.3f);
		}
	}
}

void characterStopMining(character *c){
	c->blockMiningX = c->blockMiningY = c->blockMiningZ = -1;
}

void characterPlaceBlock(character *c){
	item *cItem;
	if(c->actionTimeout <= 50){ return; }
	cItem = characterGetItemBarSlot(c,c->activeItem);
	if(!itemIsEmpty(cItem) && itemActivate(cItem,c)){
		c->actionTimeout=0;
		c->hasHit = true;
	}
}

void characterDropItem(character *c, int i){
	item *cItem;
	if(c->actionTimeout <= 50){ return; }
	cItem = characterGetItemBarSlot(c,i);
	if(cItem == NULL)      { return; }
	if(itemIsEmpty(cItem)) { return; }
	c->hasHit = true;
	c->actionTimeout=0;

	itemDropNewC(c, cItem);
	itemDiscard(cItem);
}

void characterDie(character *c){
	characterInit(c);
	setOverlayColor(0xFF000000,0);
}

item *characterGetItemBarSlot(character *c, int i){
	if(i <  0){return NULL;}
	if(i > 40){return NULL;}
	return &c->inventory[i];
}
void characterSetActiveItem(character *c, int i){
	if(i > 9){i = 0;}
	if(i < 0){i = 9;}
	c->activeItem = i;
}
void characterSwapItemSlots(character *c, int a,int b){
	if(a <  0){return;}
	if(b <  0){return;}
	if(a > 39){return;}
	if(b > 39){return;}

	item tmp = c->inventory[a];
	c->inventory[a] = c->inventory[b];
	c->inventory[b] = tmp;
}

int characterGetItemAmount(character *c, uint16_t itemID){
	int amount = 0;
	for(unsigned int i=0;i<40;i++){
		if(c->inventory[i].ID == itemID){
			amount += c->inventory[i].amount;
		}
	}
	return amount;
}

bool characterDecItemAmount(character *c, uint16_t itemID,int amount){
	for(unsigned int i=0;i<40;i++){
		if(amount == 0){return true;}
		if(c->inventory[i].ID == itemID){
			if(c->inventory[i].amount > amount){
				itemDecStack(&c->inventory[i],amount);
				return true;
			}else{
				amount -= c->inventory[i].amount;
				c->inventory[i] = itemEmpty();
			}
		}
	}
	if(amount == 0){return true;}
	return false;
}

bool characterPickupItem(character *c, uint16_t itemID,int amount){
	for(unsigned int i=0;i<40;i++){
		if(itemCanStack(&c->inventory[i],itemID)){
			if(itemIncStack(&c->inventory[i],amount)){
				sfxPlay(sfxPock,.8f);
				return true;
			}
		}
	}
	for(unsigned int i=0;i<40;i++){
		if(itemIsEmpty(&c->inventory[i])){
			c->inventory[i] = itemNew(itemID,amount);
			sfxPlay(sfxPock,.8f);
			return true;
		}
	}
	return false;
}


bool characterHP(character *c, int addhp){
	c->hp += addhp;
	if(c->hp <= 0){
		characterDie(c);
		return true;
	}
	if(c->hp > c->maxhp){
		c->hp = c->maxhp;
	}
	return false;
}


int characterPhysics(character *c){
	int ret=0;
	uint32_t col;
	c->x += c->vx;
	c->y += c->vy;
	c->z += c->vz;
	if(c->shake > 0.f){
		c->shake -= 0.1f;
	}else if(c->shake < 0.f){
		c->shake = 0.f;
	}
	if(c->noClip){
		c->collide = characterCollision(c,c->x,c->y,c->z,0.3f);
		return 0;
	}

	c->vy -= 0.0005f;
	if(c->vy < -1.0f){c->vy+=0.005f;}
	if(c->vy >  1.0f){c->vy-=0.005f;}

	c->falling = true;
	c->collide = false;
	col = characterCollision(c,c->x,c->y,c->z,0.3f);
	if(col){ c->collide = true; }
	if((col&0x110) && (c->vx < 0.f)){
		if(c->vx < -0.1f){ ret += (int)(fabsf(c->vx)*512.f); }
		const float nx = floor(c->x)+0.3f;
		if(nx > c->x){c->x = nx;}
		c->vx = c->vx*-0.3f;
	}
	if((col&0x220) && (c->vx > 0.f)){
		if(c->vx >  0.1f){ ret += (int)(fabsf(c->vx)*512.f); }
		const float nx = floorf(c->x)+0.7f;
		if(nx < c->x){c->x = nx;}
		c->x = floorf(c->x)+0.7f;
		c->vx = c->vx*-0.3f;
	}
	if((col&0x880) && (c->vz > 0.f)){
		if(c->vz >  0.1f){ ret += (int)(fabsf(c->vz)*512.f); }
		const float nz = floorf(c->z)+0.7f;
		if(nz < c->z){c->z = nz;}
		c->vz = c->vz*-0.3f;
	}
	if((col&0x440) && (c->vz < 0.f)){
		if(c->vz < -0.1f){ ret += (int)(fabsf(c->vz)*512.f); }
		const float nz = floorf(c->z)+0.3f;
		if(nz > c->z){c->z = nz;}
		c->vz = c->vz*-0.3f;
	}
	if((col&0x0F0) && (c->vy > 0.f)){
		if(c->vy >  0.1f){ ret += (int)(fabsf(c->vy)*512.f); }
		const float ny = floorf(c->y)+0.5f;
		if(ny < c->y){c->y = ny;}
		c->vy = c->vy*-0.3f;
	}
	if((col&0x00F) && (c->vy < 0.f)){
		c->falling=false;
		if(c->vy < -0.15f){
			c->yoff = -0.8f;
			ret += (int)(fabsf(c->vy)*512.f);
		}else if(c->vy < -0.07f){
			c->yoff += -0.4f;
		}else if(c->vy < -0.04f){
			c->yoff += -0.2f;
		}
		c->vx *= 0.97f;
		c->vy  = 0.f;
		c->vz *= 0.97f;
	}

	return ret;
}

void characterUpdate(character *c){
	float walkFactor = 1.f;
	float nvx,nvy,nvz;
	uint32_t col,wcl;

	if(c->actionTimeout < 1024){ c->actionTimeout++; }
	if(c->noClip){
		c->vx = c->gvx;
		c->vy = c->gvy;
		c->vz = c->gvz;
		characterUpdateHook(c);
		characterUpdateHitOff(c);
		characterPhysics(c);
		return;
	}
	nvx = c->vx;
	nvy = c->vy;
	nvz = c->vz;

	if(c->falling){ walkFactor = 0.2f; }
	if(c->gvx < nvx){
		nvx -= 0.05f * (nvx - c->gvx) * walkFactor;
		if(nvx < c->gvx){ nvx = c->gvx; }
	}else if(c->gvx > nvx){
		nvx += 0.05f * (c->gvx - nvx) * walkFactor;
		if(nvx > c->gvx){ nvx = c->gvx; }
	}
	if(c->gvz < nvz){
		nvz -= 0.05f*(nvz - c->gvz) * walkFactor;
		if(nvz < c->gvz){ nvz = c->gvz; }
	}else if(c->gvz > nvz){
		nvz += 0.05f * (c->gvz - nvz) * walkFactor;
		if(nvz > c->gvz){ nvz = c->gvz; }
	}
	if((c->hook != NULL) && (grapplingHookGetHooked(c->hook))){
		if(fabsf(c->gvx) < 0.001)             { nvx = c->vx; }
		if(fabsf(c->gvz) < 0.001)             { nvz = c->vz; }
		if((c->gvx < -0.001) && (nvx > c->vx)){ nvx = c->vx; }
		if((c->gvx >  0.001) && (nvx < c->vx)){ nvx = c->vx; }
		if((c->gvz < -0.001) && (nvz > c->vz)){ nvz = c->vz; }
		if((c->gvz >  0.001) && (nvz < c->vz)){ nvz = c->vz; }
	}

	if(c->sneak){
		wcl = characterCollision(c,c->x,c->y,c->z,0.6f);
		col = characterCollision(c,c->x,c->y,c->z,-0.2f);
		if(!(col&0x1) && (wcl&0x2) && (nvx < 0.f)){ nvx =  0.001f;}
		if(!(col&0x2) && (wcl&0x1) && (nvx > 0.f)){ nvx = -0.001f;}
		if(!(col&0x4) && (wcl&0x8) && (nvz < 0.f)){ nvz =  0.001f;}
		if(!(col&0x8) && (wcl&0x4) && (nvz > 0.f)){ nvz = -0.001f;}
	}

	if(characterUpdateJumping(c)){ nvy = 0.044f;}
	c->vx = nvx;
	c->vy = nvy;
	c->vz = nvz;

	if(c->fallingSound && (c->y > -32)){ c->fallingSound = false; }
	if(c->y < -500){ 
		characterDie(c);
		msgSendDyingMessage("fell into the abyss", 65535);
	}
	if(c->y < -64){
		setOverlayColor(0xFF000000,1000);
		if(!c->fallingSound){
			c->fallingSound = true;
			sfxPlay(sfxFalling,1.f);
		}
	}
	

	const int damage = characterPhysics(c);
	characterUpdateDamage(c,damage);
	if((nvy < -0.2f) && c->vy > -0.01f){
		sfxPlay(sfxImpact,1.f);
	} else if((nvy < -0.05f) && c->vy > -0.01f){
		sfxPlay(sfxStomp,1.f);
	}
	characterUpdateYOff(c);
	characterUpdateHook(c);
	characterUpdateHitOff(c);
	characterUpdateWindVolume(c,c->vx,c->vy,c->vz);
}

void characterMove(character *c, float mx,float my,float mz){
	float speed;
	const float yaw   = c->yaw;
	const float pitch = c->pitch;

	if(c->noClip){
		if(c->sneak){
			speed = 1.f;
		}else{
			speed = 0.15f;
		}
		c->gvx = (cos((yaw+90.f)*PI/180) * cos(pitch*PI/180))*mz*speed;
		c->gvy = sin(pitch*PI/180)*mz*speed;
		c->gvz = (sin((yaw+90.f)*PI/180) * cos(pitch*PI/180))*mz*speed;

		c->gvx += cos((yaw)*PI/180)*mx*speed;
		c->gvz += sin((yaw)*PI/180)*mx*speed;
	}else{
		if(c->sneak){
			speed = 0.01f;
		}else{
			speed = 0.05f;
		}
		c->gvy = my;
		c->gvx = (cos((yaw+90)*PI/180)*mz)*speed;
		c->gvz = (sin((yaw+90)*PI/180)*mz)*speed;

		c->gvx += cos((yaw)*PI/180)*mx*speed;
		c->gvz += sin((yaw)*PI/180)*mx*speed;
	}
}

void characterRotate(character *c, float vYaw,float vPitch,float vRoll){
	float yaw   = c->yaw   +   vYaw;
	float pitch = c->pitch + vPitch;
	float roll  = c->roll  +  vRoll;

	if(yaw   > 360.f){yaw   -= 360.f;}
	if(yaw   <   0.f){yaw   += 360.f;}
	if(pitch >  90.f){pitch  =  90.f;}
	if(pitch < -90.f){pitch  = -90.f;}
	if(roll  > 360.f){roll  -= 360.f;}
	if(roll  <   0.f){roll  += 360.f;}

	c->yaw   = yaw;
	c->pitch = pitch;
	c->roll  = roll;
}

void characterFireHook(character *c){
	if(c->actionTimeout < 60){return;}
	c->actionTimeout = 0;
	if(c->hook == NULL){
		c->hook = grapplingHookNew(c);
		sfxPlay(sfxHookFire,1.f);
		c->hasHit = true;
	}else{
		grapplingHookReturnHook(c->hook);
		c->hasHit = true;
	}
}

void characterMoveDelta(character *c, packetMedium *p){
	c->vx    += p->val.f[0];
	c->vy    += p->val.f[1];
	c->vz    += p->val.f[2];
	c->yaw   += p->val.f[3];
	c->pitch += p->val.f[4];
	c->roll  += p->val.f[5];
	c->shake  = (fabsf(p->val.f[0]) + fabsf(p->val.f[1]) + fabsf(p->val.f[2]))*96.f;
}

void characterDraw(const character *c){
	float matMVP[16];
	if(c == NULL)       {return;}
	if(c == player)     {return;}
	if(c->eMesh == NULL){return;}

	matMov(matMVP,matView);
	matMulTrans(matMVP,c->x,c->y+c->yoff,c->z);
	matMulRotYX(matMVP,c->yaw,c->pitch);
	matMul(matMVP,matMVP,matProjection);

	shaderMatrix(sMesh,matMVP);
	meshDraw(c->eMesh);
}
void characterDrawAll(){
	shaderBind(sMesh);
	for(int i=0;i<characterCount;i++){
		if(characterList[i].nextFree != NULL){ continue; }
		characterDraw(&characterList[i]);
	}
}
