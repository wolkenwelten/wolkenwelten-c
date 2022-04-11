#pragma once
#include "../../../../common/src/common.h"
#include "../../../../common/src/game/character.h"

extern blockId consHighlightBlock;

void  characterCalcMVP           (const character *c, float out[16]);
void  characterDraw              (const character *c);
void  characterDrawFirstPerson   (const character *c);
void  characterDrawConsHighlight (const character *c);
void  characterDrawAll           ();
void  characterUpdateAnimation   (      character *c);
