#pragma once
#include "../../../common/src/common.h"
#include "api_v1.h"

void        modsInit                ();

bool        primaryActionDispatch   (      item *cItem, character *cChar);
bool        hasPrimaryAction        (const item *cItem);
bool        secondaryActionDispatch (      item *cItem, character *cChar);
bool        hasSecondaryAction      (const item *cItem);
bool        tertiaryActionDispatch  (      item *cItem, character *cChar);
bool        throwActionDispatch     (      item *cItem, character *cChar);
bool        hasTertiaryAction       (const item *cItem);
int         getMagSizeDispatch      (const item *cItem);
bool        hasGetMagSize           (const item *cItem);
int         itemDropCallbackDispatch(const item *cItem, float x, float y, float z);

bool        primaryActionDefault    (      item *cItem, character *cChar);
bool        secondaryActionDefault  (      item *cItem, character *cChar);
bool        tertiaryActionDefault   (      item *cItem, character *cChar);
int         getMagSizeDefault       (const item *cItem);
int         itemDropCallbackDefault (const item *cItem, float x, float y, float z);
int         throwActionDefault      (      item *cItem, character *cChar);

int         itemDropBurnUpDispatch  (      itemDrop *id);
int         itemDropBurnUpDefault   (      itemDrop *id);
