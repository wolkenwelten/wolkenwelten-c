#pragma once
#include "../../../common/src/common.h"
#include "api_v1.h"

void  modsInit                ();

int         damageDispatch          (const item *cItem);
int         blockDamageDispatch     (const item *cItem, blockCategory blockCat);
mesh       *getMeshDispatch         (const item *cItem);
bool        primaryActionDispatch   (      item *cItem, character *cChar, int to);
bool        hasPrimaryAction        (const item *cItem);
bool        secondaryActionDispatch (      item *cItem, character *cChar, int to);
bool        hasSecondaryAction      (const item *cItem);
bool        tertiaryActionDispatch  (      item *cItem, character *cChar, int to);
bool        hasTertiaryAction       (const item *cItem);
float       getInaccuracyDispatch   (const item *cItem);
int         getAmmunitionDispatch   (const item *cItem);
int         getStackSizeDispatch    (const item *cItem);
int         getMagSizeDispatch      (const item *cItem);
bool        hasGetMagSize           (const item *cItem);
bool        itemDropCallbackDispatch(const item *cItem, float x, float y, float z);
bool        hasItemDropCallback     (const item *cItem);

int         damageDefault           (const item *cItem);
int         blockDamageDefault      (const item *cItem, blockCategory blockCat);
mesh       *getMeshDefault          (const item *cItem);
bool        primaryActionDefault    (      item *cItem, character *cChar, int to);
bool        secondaryActionDefault  (      item *cItem, character *cChar, int to);
bool        tertiaryActionDefault   (      item *cItem, character *cChar, int to);
float       getInaccuracyDefault    (const item *cItem);
int         getAmmunitionDefault    (const item *cItem);
int         getStackSizeDefault     (const item *cItem);
int         getMagSizeDefault       (const item *cItem);
bool        itemDropCallbackDefault (const item *cItem, float x, float y, float z);
