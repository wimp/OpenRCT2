#pragma region Copyright (c) 2014-2016 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#include "../addresses.h"
#include "../input.h"
#include "../interface/widget.h"
#include "../interface/window.h"
#include "../interface/viewport.h"
#include "../localisation/localisation.h"
#include "../sprites.h"
#include "../world/map.h"
#include "../game.h"
#include "../interface/themes.h"

#define MINIMUM_TOOL_SIZE 1
#define MAXIMUM_TOOL_SIZE 64

enum WINDOW_WATER_WIDGET_IDX {
	WIDX_BACKGROUND,
	WIDX_TITLE,
	WIDX_CLOSE,
	WIDX_PREVIEW,
	WIDX_DECREMENT,
	WIDX_INCREMENT,
	WIDX_BUY_LAND_RIGHTS,
	WIDX_BUY_CONSTRUCTION_RIGHTS
};

static rct_widget window_land_rights_widgets[] = {
	{ WWT_FRAME,	0,	0,	97,	0,	93,	0xFFFFFFFF,									STR_NONE },							    // panel / background
	{ WWT_CAPTION,	0,	1,	96,	1,	14,	STR_LAND_RIGHTS,							STR_WINDOW_TITLE_TIP },				    // title bar
	{ WWT_CLOSEBOX,	0,	85,	95,	2,	13,	STR_CLOSE_X,								STR_CLOSE_WINDOW_TIP },				    // close x button
	{ WWT_IMGBTN,	0,	27,	70,	17,	48,	SPR_LAND_TOOL_SIZE_0,						STR_NONE },							    // preview box
	{ WWT_TRNBTN,	2,	28,	43,	18,	33,	0x20000000 | SPR_LAND_TOOL_DECREASE,		STR_ADJUST_SMALLER_LAND_RIGHTS_TIP },	// decrement size
	{ WWT_TRNBTN,	2,	54,	69,	32,	47,	0x20000000 | SPR_LAND_TOOL_INCREASE,		STR_ADJUST_LARGER_LAND_RIGHTS_TIP },	// increment size
	{ WWT_FLATBTN,	2,	22, 45, 53, 76, 0x20000000 | SPR_BUY_LAND_RIGHTS,			STR_BUY_LAND_RIGHTS_TIP },			    // land rights
	{ WWT_FLATBTN,	2,	52, 75, 53, 76, 0x20000000 | SPR_BUY_CONSTRUCTION_RIGHTS,	STR_BUY_CONSTRUCTION_RIGHTS_TIP },	    // construction rights
	{ WIDGETS_END },
};

static int window_land_rights_should_close();

static void window_land_rights_close(rct_window *w);
static void window_land_rights_mouseup(rct_window *w, int widgetIndex);
static void window_land_rights_update(rct_window *w);
static void window_land_rights_invalidate(rct_window *w);
static void window_land_rights_paint(rct_window *w, rct_drawpixelinfo *dpi);
static void window_land_rights_textinput(rct_window *w, int widgetIndex, char *text);
static void window_land_rights_inputsize(rct_window *w);

static rct_window_event_list window_land_rights_events = {
	window_land_rights_close,
	window_land_rights_mouseup,
	NULL,
	NULL,
	NULL,
	NULL,
	window_land_rights_update,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	window_land_rights_textinput,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	window_land_rights_invalidate,
	window_land_rights_paint,
	NULL
};

void window_land_rights_open()
{
	rct_window* window;

	// Check if window is already open
	if (window_find_by_class(WC_LAND_RIGHTS) != NULL)
		return;

	window = window_create(gScreenWidth - 98, 29, 98, 94, &window_land_rights_events, WC_LAND_RIGHTS, 0);
	window->widgets = window_land_rights_widgets;
	window->enabled_widgets = (1 << WIDX_CLOSE) | (1 << WIDX_DECREMENT) | (1 << WIDX_INCREMENT) | (1 << WIDX_PREVIEW) |
		(1 << WIDX_BUY_LAND_RIGHTS) | (1 << WIDX_BUY_CONSTRUCTION_RIGHTS);
	window_init_scroll_widgets(window);
	window_push_others_below(window);

	LandRightsMode = true;
	window->pressed_widgets = (1 << WIDX_BUY_LAND_RIGHTS);

	gWaterToolRaiseCost = MONEY32_UNDEFINED;
	gWaterToolLowerCost = MONEY32_UNDEFINED;

	show_land_rights();
}

static void window_land_rights_close(rct_window *w)
{
	// If the tool wasn't changed, turn tool off
	if (!window_land_rights_should_close())
		tool_cancel();
}

static void window_land_rights_mouseup(rct_window *w, int widgetIndex)
{
	switch (widgetIndex) {
	case WIDX_CLOSE:
		window_close(w);
		break;
	case WIDX_DECREMENT:
		// Decrement land rights tool size
		gLandToolSize = max(MINIMUM_TOOL_SIZE, gLandToolSize-1);

		// Invalidate the window
		window_invalidate(w);
		break;
	case WIDX_INCREMENT:
		// Decrement land rights tool size
		gLandToolSize = min(MAXIMUM_TOOL_SIZE, gLandToolSize+1);

		// Invalidate the window
		window_invalidate(w);
		break;
	case WIDX_PREVIEW:
		window_land_rights_inputsize(w);
		break;
	case WIDX_BUY_LAND_RIGHTS:
		if (!LandRightsMode) {
			LandRightsMode = true;
			hide_construction_rights();
			show_land_rights();
			window_invalidate(w);
		}
		break;
	case WIDX_BUY_CONSTRUCTION_RIGHTS:
		if (LandRightsMode) {
			LandRightsMode = false;
			hide_land_rights();
			show_construction_rights();
			window_invalidate(w);
		}
		break;
	}
}

static void window_land_rights_textinput(rct_window *w, int widgetIndex, char *text)
{
	int size;
	char* end;

	if (widgetIndex != WIDX_PREVIEW || text == NULL)
		return;

	size = strtol(text, &end, 10);
	if (*end == '\0') {
		size = max(MINIMUM_TOOL_SIZE,size);
		size = min(MAXIMUM_TOOL_SIZE,size);
		gLandToolSize = size;
		window_invalidate(w);
	}
}

static void window_land_rights_inputsize(rct_window *w)
{
	TextInputDescriptionArgs[0] = MINIMUM_TOOL_SIZE;
	TextInputDescriptionArgs[1] = MAXIMUM_TOOL_SIZE;
	window_text_input_open(w, WIDX_PREVIEW, STR_SELECTION_SIZE, STR_ENTER_SELECTION_SIZE, STR_NONE, STR_NONE, 3);
}

static void window_land_rights_update(rct_window *w)
{
	// Close window if another tool is open
	if (window_land_rights_should_close())
		window_close(w);
}

static void window_land_rights_invalidate(rct_window *w)
{
	colour_scheme_update(w);

	// Set the preview image button to be pressed down
	w->pressed_widgets |= (1 << WIDX_PREVIEW) | (1 << (LandRightsMode ? WIDX_BUY_LAND_RIGHTS : WIDX_BUY_CONSTRUCTION_RIGHTS));
	w->pressed_widgets &= ~(1 << (!LandRightsMode ? WIDX_BUY_LAND_RIGHTS : WIDX_BUY_CONSTRUCTION_RIGHTS));

	// Update the preview image
	// TODO: Don't apply addition to images
	window_land_rights_widgets[WIDX_PREVIEW].image = gLandToolSize <= 7 ?
		SPR_LAND_TOOL_SIZE_0 + gLandToolSize :
		0xFFFFFFFF;
}

static void window_land_rights_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
	int x, y;

	x = w->x + (window_land_rights_widgets[WIDX_PREVIEW].left + window_land_rights_widgets[WIDX_PREVIEW].right) / 2;
	y = w->y + (window_land_rights_widgets[WIDX_PREVIEW].top + window_land_rights_widgets[WIDX_PREVIEW].bottom) / 2;

	window_draw_widgets(w, dpi);
	// Draw number for tool sizes bigger than 7
	if (gLandToolSize > 7) {
		gfx_draw_string_centred(dpi, STR_LAND_TOOL_SIZE_VALUE, x, y - 2, 0, &gLandToolSize);
	}
	y = w->y + window_land_rights_widgets[WIDX_PREVIEW].bottom + 5;

	// Draw cost amount
	x = (window_land_rights_widgets[WIDX_PREVIEW].left + window_land_rights_widgets[WIDX_PREVIEW].right) / 2 + w->x;
	y = window_land_rights_widgets[WIDX_PREVIEW].bottom + w->y + 32;
	if (RCT2_GLOBAL(0x00F1AD62, uint32) != MONEY32_UNDEFINED && RCT2_GLOBAL(0x00F1AD62, uint32) != 0)
		gfx_draw_string_centred(dpi, STR_COST_AMOUNT, x, y, 0, (void*)0x00F1AD62);
}

static int window_land_rights_should_close()
{
	if (!(gInputFlags & INPUT_FLAG_TOOL_ACTIVE))
		return 1;
	if (gCurrentToolWidget.window_classification != WC_PARK_INFORMATION)
		return 1;
	if (gCurrentToolWidget.widget_index != 14)
		return 1;
	return 0;
}
