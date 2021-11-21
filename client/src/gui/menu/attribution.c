/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "attribution.h"

#include "../gui.h"
#include "../widget.h"

extern uint txt_attribution_txt_len;
extern unsigned char txt_attribution_txt_data[];

widget *menuAttribution;

void initAttributions(){
	menuAttribution = widgetNewCPL(wTextScroller,rootMenu,0,0,-1,-1,(const char *)txt_attribution_txt_data);
	menuAttribution->flags |= WIDGET_HIDDEN;
	widgetBind(menuAttribution,"click",handlerRoot);
	widgetBind(menuAttribution,"blur",handlerRoot);
}

void openAttributions(){
	menuAttribution->flags &= ~WIDGET_HIDDEN;
	widgetFocus(menuAttribution);
}

void closeAttributions(){
	menuAttribution->flags |= WIDGET_HIDDEN;
}
