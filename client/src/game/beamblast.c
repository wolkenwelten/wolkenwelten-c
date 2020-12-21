#include "beamblast.h"

#include "../game/animal.h"
#include "../game/character.h"
#include "../gfx/effects.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/network/messages.h"

#include <math.h>

void singleBeamblast(character *ent, const vec start, const vec rot, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft){
	static u16 iteration = 0;

	vec pos         = start;
	vec vel         = vecDegToVec(rot);
	vec tvel        = vecMulS(vel,1.f/8.f);
	const float mdd = beamSize * beamSize;
	const int dmg   = ((int)damageMultiplier)+1;
	--iteration;

	for(int ticksLeft = 0x1FFF; ticksLeft > 0; ticksLeft--){
		vec spos = pos;
		for(int i=0;i<8;i++){
			spos = vecAdd(spos,tvel);
			if(worldGetB(spos.x,spos.y,spos.z) != 0){
				worldBoxSphere(spos.x,spos.y,spos.z,beamSize*2.f,0);
				worldBoxSphereDirty(spos.x,spos.y,spos.z,beamSize*2.f);
				fxExplosionBlaster(spos,beamSize/2.f);
				if(--hitsLeft <= 0){break;}
			}
			characterHitCheck(spos, mdd, dmg, 1, iteration, 0);
			animalHitCheck   (spos, mdd, dmg, 1, iteration, 0);
		}
		pos = vecAdd(pos,vel);
	}
	fxBeamBlaster(start,pos,beamSize,damageMultiplier);
	msgFxBeamBlaster(0,start,pos,beamSize,damageMultiplier);

	ent->vel = vecAdd(ent->vel, vecMulS(vel,-0.75f*recoilMultiplier));
	ent->rot = vecAdd(ent->rot, vecNew((rngValf()-0.5f) * 64.f * recoilMultiplier, (rngValf()-.8f) * 64.f * recoilMultiplier, 0.f));
}

void beamblast(character *ent, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int shots, float inaccuracyInc, float inaccuracyMult){
	const float mx =  1.f - ent->aimFade;
	const float mz = -1.f;
	vec pos = ent->pos;
	pos.x += ((cos((ent->rot.yaw+90.f)*PI/180) * cos(ent->rot.pitch*PI/180))*mz) + cos((ent->rot.yaw)*PI/180)*mx;
	pos.y += (sin(ent->rot.pitch*PI/180)*mz);
	pos.z += ((sin((ent->rot.yaw+90.f)*PI/180) * cos(ent->rot.pitch*PI/180))*mz) + sin((ent->rot.yaw)*PI/180)*mx;

	for(int i=shots;i>0;i--){
		const float inacc = MIN(96.f,(ent->inaccuracy*inaccuracyMult)) / (1.f + (ent->aimFade * ent->zoomFactor));
		const float yaw   = ent->rot.yaw   + (rngValf()-0.5f)*inacc;
		const float pitch = ent->rot.pitch + (rngValf()-0.5f)*inacc;
		singleBeamblast(ent, pos, vecNew(yaw, pitch, 0.f), beamSize, damageMultiplier, recoilMultiplier, hitsLeft);
	}
	characterAddInaccuracy(ent,inaccuracyInc);
}
