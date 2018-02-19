/* window.c  -  Lua library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
 *
 * This library provides a cross-platform lua library in C11 for games and applications
 * based on out foundation library. The latest source code is always available at
 *
 * https://github.com/rampantpixels/lua_lib
 *
 * This library is put in the public domain; you can redistribute it and/or modify it without
 * any restrictions.
 *
 * The LuaJIT library is released under the MIT license. For more information about LuaJIT, see
 * http://luajit.org/
 */

#include <lua/lua.h>

#include <foundation/foundation.h>
#include <window/window.h>

static bool _symbols_loaded;

#if FOUNDATION_PLATFORM_MACOS || FOUNDATION_PLATFORM_IOS || FOUNDATION_PLATFORM_ANDROID

static window_t*
window_create(unsigned int adapter, const char* title, size_t length, int width,
              int height, bool show) {
	FOUNDATION_UNUSED(adapter);
	FOUNDATION_UNUSED(title);
	FOUNDATION_UNUSED(length);
	FOUNDATION_UNUSED(width);
	FOUNDATION_UNUSED(height);
	FOUNDATION_UNUSED(show);
	return 0;
}

#endif

#if !FOUNDATION_PLATFORM_WINDOWS

static void*
window_hwnd(window_t* window) {
	FOUNDATION_UNUSED(window);
	return 0;
}

static void*
window_hinstance(window_t* window) {
	FOUNDATION_UNUSED(window);
	return 0;
}

static void*
window_hdc(window_t* window) {
	FOUNDATION_UNUSED(window);
	return 0;	
}

static void
window_release_hdc(void* hwnd, void* hdc) {
	FOUNDATION_UNUSED(hwnd);
	FOUNDATION_UNUSED(hdc);
}

#endif

#if !FOUNDATION_PLATFORM_LINUX

static void*
window_display(window_t* window) {
	FOUNDATION_UNUSED(window);
	return 0;
}

static int
window_screen(window_t* window) {
	FOUNDATION_UNUSED(window);
	return 0;
}

static unsigned long
window_drawable(window_t* window) {
	FOUNDATION_UNUSED(window);
	return 0;
}

static void*
window_visual(window_t* window) {
	FOUNDATION_UNUSED(window);
	return 0;
}

#endif

#if !FOUNDATION_PLATFORM_MACOS && !FOUNDATION_PLATFORM_IOS

static void*
window_view(window_t* window, unsigned int tag) {
	FOUNDATION_UNUSED(window);
	FOUNDATION_UNUSED(tag);
	return 0;
}

#endif

#if !FOUNDATION_PLATFORM_IOS

static void*
window_layer(window_t* window, void* view) {
	FOUNDATION_UNUSED(window);
	FOUNDATION_UNUSED(view);
	return 0;
}

static int
window_view_width(window_t* window, void* view) {
	FOUNDATION_UNUSED(window);
	FOUNDATION_UNUSED(view);
	return 0;
}

static int
window_view_height(window_t* window, void* view) {
	FOUNDATION_UNUSED(window);
	FOUNDATION_UNUSED(view);
	return 0;
}

static void
window_add_displaylink(window_t* window, window_draw_fn drawfn) {
	FOUNDATION_UNUSED(window);
	FOUNDATION_UNUSED(drawfn);
}

#endif

#if !FOUNDATION_PLATFORM_IOS && !FOUNDATION_PLATFORM_ANDROID

static void
window_show_keyboard(window_t* window) {
	FOUNDATION_UNUSED(window);
}

static void
window_hide_keyboard(window_t* window) {
	FOUNDATION_UNUSED(window);
}

#endif

void
lua_symbol_load_window(void) {
	hashmap_t* map = lua_symbol_lookup_map();

	if (_symbols_loaded)
		return;
	_symbols_loaded = true;

#define RESOURCE_SYM(fn, name) hashmap_insert(map, HASH_SYM_##name, (void*)(uintptr_t)fn)

	RESOURCE_SYM(window_module_initialize, WINDOW_MODULE_INITIALIZE);
	RESOURCE_SYM(window_module_finalize, WINDOW_MODULE_FINALIZE);
	RESOURCE_SYM(window_module_is_initialized, WINDOW_MODULE_IS_INITIALIZED);
	RESOURCE_SYM(window_module_version, WINDOW_MODULE_VERSION);

	RESOURCE_SYM(window_create, WINDOW_CREATE);
	RESOURCE_SYM(window_finalize, WINDOW_FINALIZE);
	RESOURCE_SYM(window_deallocate, WINDOW_DEALLOCATE);
	RESOURCE_SYM(window_adapter, WINDOW_ADAPTER);
	RESOURCE_SYM(window_maximize, WINDOW_MAXIMIZE);
	RESOURCE_SYM(window_minimize, WINDOW_MINIMIZE);
	RESOURCE_SYM(window_restore, WINDOW_RESTORE);
	RESOURCE_SYM(window_resize, WINDOW_RESIZE);
	RESOURCE_SYM(window_move, WINDOW_MOVE);
	RESOURCE_SYM(window_is_open, WINDOW_IS_OPEN);
	RESOURCE_SYM(window_is_visible, WINDOW_IS_VISIBLE);
	RESOURCE_SYM(window_is_maximized, WINDOW_IS_MAXIMIZED);
	RESOURCE_SYM(window_is_minimized, WINDOW_IS_MINIMIZED);
	RESOURCE_SYM(window_has_focus, WINDOW_HAS_FOCUS);
	RESOURCE_SYM(window_show_cursor, WINDOW_SHOW_CURSOR);
	RESOURCE_SYM(window_set_cursor_pos, WINDOW_SET_CURSOR_POS);
	RESOURCE_SYM(window_is_cursor_locked, WINDOW_IS_CURSOR_LOCKED);
	RESOURCE_SYM(window_set_title, WINDOW_SET_TITLE);
	RESOURCE_SYM(window_width, WINDOW_WIDTH);
	RESOURCE_SYM(window_height, WINDOW_HEIGHT);
	RESOURCE_SYM(window_position_x, WINDOW_POSITION_X);
	RESOURCE_SYM(window_position_y, WINDOW_POSITION_Y);
	RESOURCE_SYM(window_fit_to_screen, WINDOW_FIT_TO_SCREEN);

	RESOURCE_SYM(window_hwnd, WINDOW_HWND);
	RESOURCE_SYM(window_hinstance, WINDOW_HINSTANCE);
	RESOURCE_SYM(window_hdc, WINDOW_HDC);
	RESOURCE_SYM(window_release_hdc, WINDOW_RELEASE_HDC);

	RESOURCE_SYM(window_display, WINDOW_DISPLAY);
	RESOURCE_SYM(window_drawable, WINDOW_DRAWABLE);
	RESOURCE_SYM(window_visual, WINDOW_VISUAL);
	RESOURCE_SYM(window_screen, WINDOW_SCREEN);
	RESOURCE_SYM(window_view, WINDOW_VIEW);
	RESOURCE_SYM(window_layer, WINDOW_LAYER);

	RESOURCE_SYM(window_add_displaylink, WINDOW_ADD_DISPLAYLINK);

	RESOURCE_SYM(window_show_keyboard, WINDOW_SHOW_KEYBOARD);
	RESOURCE_SYM(window_hide_keyboard, WINDOW_HIDE_KEYBOARD);

	RESOURCE_SYM(window_screen_width, WINDOW_SCREEN_WIDTH);
	RESOURCE_SYM(window_screen_height, WINDOW_SCREEN_HEIGHT);

	RESOURCE_SYM(window_view_width, WINDOW_VIEW_WIDTH);
	RESOURCE_SYM(window_view_height, WINDOW_VIEW_HEIGHT);
}
