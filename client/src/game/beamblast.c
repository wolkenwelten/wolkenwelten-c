#include "beamblast.h"

#include "../game/animal.h"
#include "../game/character.h"
#include "../gfx/effects.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/network/messages.h"

#include <math.h>

void singleBeamblast(character *ent, const vec start, const vec rot, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft){
	static u16 iteration = 0;

	float speed     = 0.1f;
	vec pos         = start;
	vec vel         = vecMulS(vecDegToVec(rot),speed);
	const float mdd = beamSize * beamSize;
	const int dmg   = ((int)damageMultiplier)+1;
	--iteration;

	for(int ticksLeft = 0x1FFF; ticksLeft > 0; ticksLeft--){
		pos = vecAdd(pos,vel);
		if(worldGetB(pos.x,pos.y,pos.z) != 0){
			worldBoxSphere(pos.x,pos.y,pos.z,beamSize*2.f,0);
			worldBoxSphereDirty(pos.x,pos.y,pos.z,beamSize*2.f);
			fxExplosionBlaster(pos,beamSize/2.f);
			if(--hitsLeft <= 0){break;}
		}
		characterHitCheck(pos, mdd, dmg, 1, iteration, 0);
		animalHitCheck   (pos, mdd, dmg, 1, iteration, 0);

	}
	fxBeamBlaster(start,pos,beamSize,damageMultiplier);
	msgFxBeamBlaster(0,start,pos,beamSize,damageMultiplier);

	ent->vel = vecAdd(ent->vel, vecMulS(vel,-0.75f*recoilMultiplier));
	ent->rot = vecAdd(ent->rot, vecNew((rngValf()-0.5f) * 64.f * recoilMultiplier, (rngValf()-.8f) * 64.f * recoilMultiplier, 0.f));
}

void beamblast(character *ent, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int shots, float inaccuracyInc, float inaccuracyMult){
	const float mx =  1.f;
	const float mz = -1.f;
	vec pos = ent->pos;
	pos.x += ((cos((ent->rot.yaw+90.f)*PI/180) * cos(ent->rot.pitch*PI/180))*mz) + cos((ent->rot.yaw)*PI/180)*mx;
	pos.y += (sin(ent->rot.pitch*PI/180)*mz);
	pos.z += ((sin((ent->rot.yaw+90.f)*PI/180) * cos(ent->rot.pitch*PI/180))*mz) + sin((ent->rot.yaw)*PI/180)*mx;

	for(int i=shots;i>0;i--){
		const float yaw   = ent->rot.yaw   + (rngValf()-0.5f)*ent->inaccuracy*inaccuracyMult;
		const float pitch = ent->rot.pitch + (rngValf()-0.5f)*ent->inaccuracy*inaccuracyMult;
		singleBeamblast(ent, pos, vecNew(yaw, pitch, 0.f), beamSize, damageMultiplier, recoilMultiplier, hitsLeft);
	}
	characterAddInaccuracy(ent,inaccuracyInc);
}
