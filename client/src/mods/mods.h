#pragma once
#include "../game/blockType.h"
#include "../game/item.h"
#include "../game/character.h"
#include "../gfx/mesh.h"

void  modsInit                ();

int   blockDamageDispatch     (item *cItem, blockCategory blockCat);
mesh *getMeshDispatch         (item *cItem);
bool  primaryActionDispatch   (item *cItem, character *cChar, int to);
bool  hasPrimaryAction        (item *cItem);
bool  secondaryActionDispatch (item *cItem, character *cChar, int to);
bool  hasSecondaryAction      (item *cItem);
bool  tertiaryActionDispatch  (item *cItem, character *cChar, int to);
bool  hasTertiaryAction       (item *cItem);
float getInaccuracyDispatch   (item *cItem);
int   getAmmunitionDispatch   (item *cItem);
int   getStackSizeDispatch    (item *cItem);
int   getMagSizeDispatch      (item *cItem);
bool  hasGetMagSize           (item *cItem);

int   blockDamageDefault      (item *cItem, blockCategory blockCat);
mesh *getMeshDefault          (item *cItem);
bool  primaryActionDefault    (item *cItem, character *cChar, int to);
bool  secondaryActionDefault  (item *cItem, character *cChar, int to);
bool  tertiaryActionDefault   (item *cItem, character *cChar, int to);
float getInaccuracyDefault    (item *cItem);
int   getAmmunitionDefault    (item *cItem);
int   getStackSizeDefault     (item *cItem); 
int   getMagSizeDefault       (item *cItem);