#pragma once
#include "../../../common/src/common.h"
#include "api_v1.h"

void        modsInit                ();

int         damageDispatch          (const item *cItem);
int         blockDamageDispatch     (const item *cItem, blockCategory blockCat);
mesh       *getMeshDispatch         (const item *cItem);
bool        primaryActionDispatch   (      item *cItem, character *cChar);
bool        hasPrimaryAction        (const item *cItem);
bool        secondaryActionDispatch (      item *cItem, character *cChar);
bool        hasSecondaryAction      (const item *cItem);
bool        tertiaryActionDispatch  (      item *cItem, character *cChar);
bool        hasTertiaryAction       (const item *cItem);
float       getInaccuracyDispatch   (const item *cItem);
int         getAmmunitionDispatch   (const item *cItem);
int         getStackSizeDispatch    (const item *cItem);
int         getMagSizeDispatch      (const item *cItem);
bool        hasGetMagSize           (const item *cItem);
int         itemDropCallbackDispatch(const item *cItem, float x, float y, float z);

int         damageDefault           (const item *cItem);
int         blockDamageDefault      (const item *cItem, blockCategory blockCat);
mesh       *getMeshDefault          (const item *cItem);
bool        primaryActionDefault    (      item *cItem, character *cChar);
bool        secondaryActionDefault  (      item *cItem, character *cChar);
bool        tertiaryActionDefault   (      item *cItem, character *cChar);
float       getInaccuracyDefault    (const item *cItem);
int         getAmmunitionDefault    (const item *cItem);
int         getStackSizeDefault     (const item *cItem);
int         getMagSizeDefault       (const item *cItem);
int         itemDropCallbackDefault (const item *cItem, float x, float y, float z);
