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
#include "../gfx/vec.h"
#include "../tmp/objs.h"
#include "../gfx/shader.h"
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
character characterList[64];
int characterCount = 0;
character *characterFirstFree = NULL;
character *playerList[32];

void characterInit(character *c){
	int sx=-1024,sy=1024,sz=-1024;
	c->gyoff = 0.f;
	c->gvx = c->gvy = c->gvz = 0.f;
	c->shake = c->inaccuracy = 0.f;

	c->flags = 0;
	c->gliderFade = 0.f;
	c->animationIndex = c->animationTicksMax = c->animationTicksLeft = 0;

	c->actionTimeout = 0;
	c->stepTimeout   = 0;

	c->maxhp = c->hp = 20;
	c->activeItem = 0;
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
		sfxLoop(sfxWind,0.f);
		sfxLoop(sfxHookRope,0.f);
		msgRequestPlayerSpawnPos();
	}

	characterEmptyInventory(c);
	if(optionDebugInfo){
		c->inventory[0] = itemNew(261, 1);
		c->inventory[1] = itemNew(262, 1);
		c->inventory[2] = itemNew(263, 1);
		c->inventory[3] = itemNew(264, 1);
		c->inventory[4] = itemNew(270, 1);
		c->inventory[5] = itemNew(271, 1);
		c->inventory[6] = itemNew(256,99);
		c->inventory[7] = itemNew(258,99);
		c->inventory[8] = itemNew(265,999);
		c->inventory[9] = itemNew(  1,99);
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

	if(p->val.i[10]){
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
	playerList[i]->inventory[0] = itemNew(p->val.u[17],1);
	playerList[i]->activeItem = 0;

	playerList[i]->animationIndex     = p->val.i[18];
	playerList[i]->animationTicksMax  = p->val.i[20];
	playerList[i]->animationTicksLeft = p->val.i[21];
	playerList[i]->flags              = p->val.i[22];
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
		if((c->gvy > 0) && (gl > 1.f)){
			grapplingHookSetGoalLength(c->hook,gl-0.05f);
		}
		if((c->flags & CHAR_SNEAK) && (gl < 96.f)){
			grapplingHookSetGoalLength(c->hook,gl+0.05f);
		}
		if(grapplingHookGetLength(c->hook) > gl){
			grapplingHookPullTowards(c->hook,c);
		}
	}
}

void characterUpdateAnimation(character *c){
	c->animationTicksLeft -= MS_PER_TICK;
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
void characterHitCheck(character *c, int origin, float x, float y, float z, float yaw, float pitch, float roll, int pwr){
	const float vx = cos((yaw-90.f)*PI/180) * cos((-pitch)*PI/180);
	const float vy = sin((-pitch)*PI/180);
	const float vz = sin((yaw-90.f)*PI/180) * cos((-pitch)*PI/180);
	float dx = (x+vx) - c->x;
	float dy = (y+vy) - c->y;
	float dz = (z+vz) - c->z;
	float d  = (dx*dx)+(dy*dy)+(dz*dz);

	if(d < 1.f){
		sfxPlay(sfxImpact,1.f);
		sfxPlay(sfxUngh,  1.f);
		setOverlayColor(0xA03020F0,0);
		if(characterHP(c,-pwr)){
			msgSendDyingMessage("clubbed",origin);
			setOverlayColor(0x00000000,0);
		}
		commitOverlayColor();
		msgCharacterGotHit(-1,pwr);

		float              dm = fabsf(dx);
		if(fabsf(dy) > dm){dm = fabsf(dy);}
		if(fabsf(dz) > dm){dm = fabsf(dz);}

		dx /= dm;
		dy /= dm;
		dz /= dm;
		dm = sqrtf((4.f*pwr)/dm);
		c->vx += dx * dm * -0.01f;
		c->vy += dy * dm * -0.01f;
		c->vz += dz * dm * -0.01f;
	}

	(void)roll;
}

void characterGotHitBroadcast(int i,int pwr){
	if(playerList[i] == NULL){return;}
	character *c = playerList[i];
	const float dx = player->x - c->x;
	const float dy = player->y - c->y;
	const float dz = player->z - c->z;
	const float d  = sqrtf((dx*dx)+(dy*dy)+(dz*dz));
	if(d > 128.f){return;}
	const float vol = (128.f-d)/128.f;

	sfxPlay(sfxImpact,vol);
	sfxPlay(sfxUngh,vol);
	(void)pwr;
}

void characterUpdateWindVolume(character *c, float wvx, float wvy, float wvz){
	(void)c;

	float windVol = fabsf(wvx)+fabsf(wvy)+fabsf(wvz);
	if(windVol < 0.1f){
		sfxLoop(sfxWind,0.f);
	}else{
		windVol = (windVol - 0.1f)/4.f;
		if(windVol > 1.f){windVol = 1.f;}
		vibrate(windVol);
		sfxLoop(sfxWind,windVol);
	}
}


int characterUpdateJumping(character *c){
	if((c->gvy > 0) && !(c->flags & CHAR_FALLING) && ((c->hook == NULL) || (!grapplingHookGetHooked(c->hook)))){
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
	if(!(c->flags & CHAR_FALLING) && ((fabsf(c->gvx) > 0.001f) || (fabsf(c->gvz) > 0.001f))){
		if(c->flags & CHAR_SNEAK){
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
		if(c->flags & CHAR_SNEAK){
			c->yoff = c->yoff - (0.015f * (c->yoff - c->gyoff));
		}else{
			c->yoff = c->yoff - (0.03f * (c->yoff - c->gyoff));
		}
	}else{
		if(c->flags & CHAR_SNEAK){
			c->yoff = c->yoff + (0.015f*(c->gyoff - c->yoff));
		}else{
			c->yoff = c->yoff + (0.03f*(c->gyoff - c->yoff));
		}
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
				msgCharacterHit(-1,c->x,c->y,c->z,c->yaw,c->pitch,c->roll,damageDispatch(itm));
			}
		}else if(c->actionTimeout >= 0){
			msgCharacterHit(-1,c->x,c->y,c->z,c->yaw,c->pitch,c->roll,damageDispatch(itm));
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
	if(cItem == NULL)      { return; }
	if(itemIsEmpty(cItem)) { return; }
	characterAddCooldown(c,50);

	itemDropNewC(c, cItem);
	itemDiscard(cItem);
}

void characterDie(character *c){
	if(c != player){return;}
	for(int i=0;i<40;i++){
		item *cItem = characterGetItemBarSlot(c,i);
		if(cItem == NULL)     { continue; }
		if(itemIsEmpty(cItem)){ continue; }
		itemDropNewD(c->x,c->y+3.f,c->z, cItem);
	}
	characterInit(c);
	setOverlayColor(0xFF000000,0);
}

void updateGlideWeird(character *c){
	float vm = fabsf(c->vx);
	if(fabsf(c->vy) > vm){vm = fabsf(c->vy);}
	if(fabsf(c->vz) > vm){vm = fabsf(c->vz);}
	float nx = c->vx / vm;
	float ny = c->vy / vm;
	float nz = c->vz / vm;

	float fx = cos((c->yaw-90.f)*PI/180) * cos((-c->pitch)*PI/180);
	float fy = sin((-c->pitch)*PI/180);
	float fz = sin((c->yaw-90.f)*PI/180) * cos((-c->pitch)*PI/180);
	float  v = ((fabsf(fx-nx)+fabsf(fy-ny)+fabsf(fz-nz))/6.f)*0.006f;

	c->vx += cos((c->yaw-90.f)*PI/180) * cos((-c->pitch)*PI/180)*v;
	c->vy += sin((-c->pitch)*PI/180)*v;
	c->vz += sin((c->yaw-90.f)*PI/180) * cos((-c->pitch)*PI/180)*v;

	c->vx -= c->vx*0.001f;
	c->vy -= c->vy*0.001f;
	c->vz -= c->vz*0.001f;
}

float cdrag;
float clift;

void updateGlide(character *c){
	vec  dir = vecDegToVec(vecNew(c->yaw,c->pitch,c->roll));
	vec  vel = vecNew(c->vx,c->vy,c->vz);

	vec vdeg = vecVecToDeg(vecNorm(vel));

	float aoa  = fabsf(vdeg.y - c->pitch);
	float drag = fabsf(sinf(aoa*PI180)) * 0.9f + 0.1f;
	cdrag = drag;

	vec vdrg = vecMulS(vecInvert(vel),drag * 0.03f);
	vel = vecAdd(vel,vdrg);
	vel = vecAdd(vel,vecMulS(dir,vecMag(vdrg)*0.95f));

	c->vx = vel.x;
	c->vy = vel.y;
	c->vz = vel.z;
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
	if(c->flags & CHAR_NOCLIP){
		col = characterCollision(c,c->x,c->y,c->z,0.3f);
		if(col){ c->flags |= CHAR_COLLIDE; }
		return 0;
	}

	c->vy -= 0.0005f;
	if(c->vy < -1.0f){c->vy+=0.005f;}
	if(c->vy >  1.0f){c->vy-=0.005f;}
	if(c->vx < -1.0f){c->vx+=0.005f;}
	if(c->vx >  1.0f){c->vx-=0.005f;}
	if(c->vz < -1.0f){c->vz+=0.005f;}
	if(c->vz >  1.0f){c->vz-=0.005f;}

	c->flags |=  CHAR_FALLING;
	c->flags &= ~CHAR_COLLIDE;
	col = characterCollision(c,c->x,c->y,c->z,0.3f);
	if(col){ c->flags |= CHAR_COLLIDE; }
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
		c->flags &= ~CHAR_FALLING;
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

	if(c->flags & CHAR_GLIDE){
		updateGlide(c);
	}
	return ret;
}

void characterUpdate(character *c){
	float walkFactor = 1.f;
	float nvx,nvy,nvz;
	uint32_t col,wcl;


	if((c->flags & CHAR_FALLINGSOUND) && (c->y > -32)){ c->flags &= ~CHAR_FALLINGSOUND; }
	if(c->y < -500){
		characterDie(c);
		msgSendDyingMessage("fell into the abyss", 65535);
	}
	if(c->y < -64){
		setOverlayColor(0xFF000000,1000);
		if(!(c->flags & CHAR_FALLINGSOUND)){
			c->flags |= CHAR_FALLINGSOUND;
			sfxPlay(sfxFalling,1.f);
		}
	}

	if(c->actionTimeout < 0){ c->actionTimeout++; }
	if(c->flags & CHAR_NOCLIP){
		c->vx = c->gvx;
		c->vy = c->gvy;
		c->vz = c->gvz;
		characterUpdateHook(c);
		characterUpdateAnimation(c);
		characterUpdateInaccuracy(c);
		characterPhysics(c);
		return;
	}
	if((c->flags & (CHAR_GLIDE | CHAR_FALLING)) == (CHAR_GLIDE | CHAR_FALLING)){
		characterUpdateHook(c);
		characterUpdateAnimation(c);
		characterUpdateInaccuracy(c);
		const int damage = characterPhysics(c);
		characterUpdateDamage(c,damage);
		characterUpdateWindVolume(c,c->vx,c->vy,c->vz);
		return;
	}
	nvx = c->vx;
	nvy = c->vy;
	nvz = c->vz;

	if(c->flags & CHAR_FALLING){ walkFactor = 0.2f; }
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

	if(c->flags & CHAR_SNEAK){
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

	const int damage = characterPhysics(c);
	if(c == player){
		characterUpdateDamage(c,damage);
		if((nvy < -0.2f) && c->vy > -0.01f){
			sfxPlay(sfxImpact,1.f);
		} else if((nvy < -0.05f) && c->vy > -0.01f){
			sfxPlay(sfxStomp,1.f);
		}
		characterUpdateWindVolume(c,c->vx,c->vy,c->vz);
	}

	characterUpdateInaccuracy(c);
	characterUpdateYOff(c);
	characterUpdateHook(c);
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

void characterMoveDelta(character *c, packet *p){
	c->vx    += p->val.f[0];
	c->vy    += p->val.f[1];
	c->vz    += p->val.f[2];
	c->yaw   += p->val.f[3];
	c->pitch += p->val.f[4];
	c->roll  += p->val.f[5];
	c->shake  = (fabsf(p->val.f[0]) + fabsf(p->val.f[1]) + fabsf(p->val.f[2]))*96.f;
}

void characterShadesDraw(character *c){
	float matMVP[16];
	float sneakOff = 0.f;
	if(c->flags & CHAR_SNEAK){sneakOff = 1.f;}

	matMov(matMVP,matView);
	matMulTrans(matMVP,c->x,c->y+c->yoff,c->z);
	matMulRotYX(matMVP,-c->yaw,-(c->pitch+sneakOff*30.f)/3.f);
	matMulTrans(matMVP,0.f,0.1f,-0.2f);
	matMulScale(matMVP,0.5f, 0.5f, 0.5f);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(meshSunglasses);
}

void characterGliderDraw(character *c){
	float matMVP[16];
	if(c->gliderFade < 0.01f){return;}

	matMov(matMVP,matView);
	matMulTrans(matMVP,c->x,c->y+c->yoff,c->z);
	matMulRotYX(matMVP,-c->yaw,-(c->pitch-((1.f - c->gliderFade)*90.f)));
	matMulTrans(matMVP,0.f,0.4f,-0.2f);
	matMulScale(matMVP,c->gliderFade, c->gliderFade, c->gliderFade);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(meshGlider);
}

void characterActiveItemDraw(character *c){
	float matMVP[16];
	item *activeItem;
	mesh *aiMesh;
	float sneakOff = 0.f;
	if(c->flags & CHAR_SNEAK){sneakOff = 1.f;}

	activeItem = &c->inventory[c->activeItem];
	if(activeItem == NULL)     {return;}
	if(itemIsEmpty(activeItem)){return;}
	aiMesh = getMeshDispatch(activeItem);
	if(aiMesh == NULL)         {return;}

	matMov(matMVP,matView);
	matMulTrans(matMVP,c->x,c->y+c->yoff,c->z);
	matMulRotYX(matMVP,-c->yaw+(15.f*sneakOff),-c->pitch);

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
	float matMVP[16];
	if(c == NULL)       {return;}
	if(c == player)     {return;}
	if(c->eMesh == NULL){return;}

	matMov(matMVP,matView);
	matMulTrans(matMVP,c->x,c->y+c->yoff,c->z);
	matMulRotYX(matMVP,-c->yaw,-c->pitch/6.f);
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

void characterDamagePacket(character *c, packet *p){
	if(characterDamage(c,p->val.i[0])){
		msgSendDyingMessage("died by command", 65535);
	}
}

bool itemPlaceBlock(item *i, character *chr, int to){
	int cx,cy,cz;
	if(to < 0){return false;}
	if(characterLOSBlock(chr,&cx,&cy,&cz,true)){
		if((characterCollision(chr,chr->x,chr->y,chr->z,0.3f)&0xFF0)){ return false; }
		if(!itemDecStack(i,1)){ return false; }
		worldSetB(cx,cy,cz,i->ID);
		if((characterCollision(chr,chr->x,chr->y,chr->z,0.3f)&0xFF0) != 0){
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
