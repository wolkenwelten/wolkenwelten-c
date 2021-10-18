#pragma once
#include "../../../common/src/common.h"

typedef enum {
	animalUnused=0,
	animalBunny,
	animalGuardian,
	animalWerebunny
} animalType;

extern animal  animalList[1<<12];
extern uint    animalListMax;
extern uint    animalCount;

void        animalDeleteAll        ();
animal     *animalNew              (const vec pos , int type, int gender);
void        animalDel              (uint i);
void        animalDelChungus       (const chungus *c);
void        animalReset            (      animal *e);
float       animalDistance         (const animal *e,const character *c);
const char *animalGetStateName     (const animal *e);
int         animalGetMaxHealth     (const animal *e);
float       animalGetWeight        (const animal *e);
int         animalUpdate           (      animal *e);
void        animalThink            (      animal *e);
float       animalClosestAnimal    (const animal *e, animal **cAnim, int typeFilter, uint flagsMask, uint flagsCompare);
float       animalClosestPlayer    (const animal *e, character **cChar);
void        animalCheckSuffocation (      animal *e);
void        animalCheckTarget      (      animal *e);
void        animalRDie             (      animal *e);
void        animalRHit             (      animal *e, being culprit, u8 cause);
void        animalRBurn            (      animal *e);
void        animalSync             (u8 c, u16 i);
void        animalEmptySync        (u8 c);
void        animalSyncInactive     (u8 c, u16 i);
void        animalDoDamage         (animal *a,i16 hp, u8 cause, float knockbackMult, being culprit, const vec pos);
being       animalFindFOFTarget    (const animal *e);

void        animalUpdateAll        ();
void        animalThinkAll         ();
void        animalNeedsAll         ();
void        animalCheckBurnAll     ();


animal *animalGetByBeing(being b);
being   animalGetBeing  (const animal *h);
animal *animalClosest   (const vec pos, float maxDistance);
int     animalHitCheck  (const vec pos, float mdd, int dmg, int cause, u16 iteration, being source);

const char *animalGetName(const animal *a);
