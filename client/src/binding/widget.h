#pragma once
#include "../../../common/src/common.h"

#include "../misc/lisp.h"
#include "../gui/widget.h"

widget *widgetGet       (uint i);
void    widgetExport    (widget *w, const char *symbol);
void    lOperatorsWidget(lClosure *c);
widget *castToWidget    (lVal *v);
lVal   *lValW           (widget *w);
