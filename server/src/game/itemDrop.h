#pragma once
#include "../../../common/src/common.h"

extern uint  itemDropCount;

      void   itemDropNewP         (const vec pos, const item *itm);
      void   itemDropNewC         (const packet *p);
      void   itemDropUpdate       ();
      void   itemDropDel          (uint d);
      void   itemDropIntro        (uint c);
      void  *itemDropSaveChungus  (const chungus *c, void *buf);
      void   itemDropDelChungus   (const chungus *c);
const void  *itemDropLoad         (const void *buf);
uint         itemDropUpdatePlayer (uint c, uint offset);
