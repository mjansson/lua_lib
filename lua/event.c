/* event.c  -  Lua library  -  Public Domain  -  2016 Mattias Jansson / Rampant Pixels
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

#include <lua/event.h>
#include <lua/module.h>

#include <resource/event.h>

void
lua_event_handle(lua_t* lua, const event_t* event) {
	if (event->id != RESOURCEEVENT_MODIFY)
		return;

	lua_module_reload(lua, resource_event_uuid(event));
}
