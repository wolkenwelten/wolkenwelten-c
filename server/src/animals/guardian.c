#include "guardian.h"

#include "../game/animal.h"
#include "../game/entity.h"
#include "../game/itemDrop.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/network/messages.h"
#include "../../../common/src/misc/misc.h"

#include <math.h>

static void animalAggresive(animal *e){
	character *cChar;
	float dist = animalClosestPlayer(e,&cChar);
	uint los   = 0;
	if(cChar != NULL){
		los = lineOfSightBlockCount(vecAdd(e->pos,vecNew(0.f,.5f,0.f)),cChar->pos,2);
	}

	if(e->state == ANIMAL_S_HEAT){
		if(--e->temp > 0){return;}
		vec caNorm = vecNorm(vecNew(cChar->pos.x - e->pos.x,cChar->pos.y - e->pos.y, cChar->pos.z - e->pos.z));
		vec caRot  = vecVecToDeg(caNorm);
		vec vrot   = e->rot;
		vrot.yaw   = -vrot.yaw;
		vrot.pitch = -vrot.pitch;
		vec bPos   = vecAdd(e->pos, vecMulS(vecDegToVec(vrot),256.f));

		int target = getClientByCharacter(cChar);
		int dmg    = 2;
		float yd   = (-e->rot.yaw   - caRot.yaw  );
		float pd   = (-e->rot.pitch - caRot.pitch);
		float d    = sqrtf(yd*yd + pd*pd);
		if(((d < 4) && (target >= 0)) || los){
			msgBeingDamage(target,dmg,2,beingCharacter(target),-1,e->pos);
		}
		e->state = ANIMAL_S_FLEE;
		e->vel   = vecAdd(e->vel,vecMulS(caNorm,-0.001f));
		e->grot  = vecZero();
		e->temp  = 10;
		msgFxBeamBlaster(-1,bPos,e->pos,4.f,4);
		addPriorityAnimal(e-animalList);
	}else if(e->state == ANIMAL_S_FIGHT){
		if((cChar == NULL) || (dist > 64.f) || los){
			e->state   =  ANIMAL_S_LOITER;
			addPriorityAnimal(e-animalList);
		}else{
			vec caNorm   = vecNorm(vecNew(cChar->pos.x - e->pos.x,cChar->pos.y - e->pos.y, cChar->pos.z - e->pos.z));
			vec caRot    = vecVecToDeg(caNorm);

			e->rot.yaw   = -caRot.yaw;
			e->rot.pitch = -caRot.pitch;
			e->grot      = e->rot;
			e->gvel.x    = 0;
			e->gvel.z    = 0;

			if(--e->temp == 0){
				e->state = ANIMAL_S_HEAT;
				e->temp = 3;
				msgFxBeamBlaster(-1,e->pos,cChar->pos,2.f,2);
			}
			addPriorityAnimal(e-animalList);
		}
	}else if(e->state == ANIMAL_S_FLEE){
		if((cChar == NULL) || (dist > 78.f) || (--e->temp == 0)){
			e->state = ANIMAL_S_LOITER;
			addPriorityAnimal(e-animalList);
		}else{
			vec caNorm  = vecNorm(vecNew(e->pos.x - cChar->pos.x,0.f, e->pos.z - cChar->pos.z));
			vec caVel   = vecMulS(caNorm,0.03f);
			vec caRot   = vecVecToDeg(caNorm);

			e->gvel.x   = caVel.x;
			e->gvel.z   = caVel.z;
			e->rot.yaw  = -caRot.yaw;
			e->grot.yaw = e->rot.yaw;
			if((dist < 3.f) && (rngValM(8)==0)){
				e->state = ANIMAL_S_FIGHT;
				e->flags |= ANIMAL_AGGRESIVE;
			}
			addPriorityAnimal(e-animalList);
		}
	}else{
		float fd = 48.f;
		if(e->state == ANIMAL_S_SLEEP){fd = 24.f;}
		if((cChar != NULL) && (dist < fd) && !los){
			e->state = ANIMAL_S_FIGHT;
			e->temp  = 10;
			addPriorityAnimal(e-animalList);
		}
	}
	e->temp = MIN(e->temp,64);
}

static void animalSLoiter(animal *e){
	if(rngValM( 8) == 0){
		e->grot.yaw = e->rot.yaw + ((rngValf()*2.f)-1.f)*4.f;
	}
	if(rngValM(16) == 0){
		e->gvel = vecZero();
	}
	if(rngValM(48) == 0){
		e->grot.pitch = ((rngValf()*2.f)-1.f)*10.f;
	}
	if(rngValM(64) == 0){
		e->grot.yaw = ((rngValf()*2.f)-1.f)*360.f;
	}
	if(rngValM(32) == 0){
		vec dir = vecMulS(vecDegToVec(vecNew(e->rot.yaw,0.f,0.f)),0.01f);
		e->gvel.x = dir.x;
		e->gvel.z = dir.z;
	}
}

void animalThinkGuardian(animal *e){
	animalCheckSuffocation(e);
	animalAggresive(e);

	switch(e->state){
	default:
		break;
	case ANIMAL_S_LOITER:
		animalSLoiter(e);
		break;
	}
}

void animalRDieGuardian(animal *e){
	item drop = itemNew(I_Bullet,rngValMM(20,40));
	itemDropNewP(e->pos,&drop);
}
