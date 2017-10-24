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

#include <foundation/array.h>
#include <resource/event.h>

extern lua_t**
lua_instances(void);

void
lua_event_handle_resource(const event_t* event) {
	if ((event->id != RESOURCEEVENT_MODIFY) && (event->id != RESOURCEEVENT_DEPENDS))
		return;

	lua_t** instances = lua_instances();
	for (size_t ienv = 0, esize = array_size(instances); ienv < esize; ++ienv)
		lua_module_reload(instances[ienv], resource_event_uuid(event));
}
