#pragma once
#include "../../../../common/src/common.h"
#include "../../../../common/src/game/character.h"

void  characterDraw              (character *c);
void  characterDrawAll           ();
void  characterDrawConsHighlight (const character *c);
void  characterUpdateAnimation   (character *c);
