/* module.c  -  Lua library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <lua/module.h>

#include <foundation/hash.h>
#include <foundation/hashmap.h>

#undef LUA_API
#define LUA_HAS_LUA_STATE_TYPE

#include "luajit/src/lua.h"
#include "luajit/src/lauxlib.h"
#include "luajit/src/lualib.h"

LUA_EXTERN void* lj_clib_getsym_registry(lua_State*, const char*, size_t);

static hashmap_t* _lua_registry;

LUA_EXTERN int
lua_registry_initialize(void);

LUA_EXTERN void
lua_registry_finalize(void);

//This is our custom callback point from luajit lib_package loader
int
lj_cf_package_loader_registry(lua_State* state) {
	size_t length = 0;
	const char* name = luaL_checklstring(state, 1, &length);
	hash_t namehash = hash(name, length);
	lua_fn loader = hashmap_lookup(_lua_registry, namehash);
	if (loader)
		lua_pushcclosure(state, loader, 0);
	else
		lua_pushfstring(state, "\n\tno registry entry '%s'", name);
	return 1;
}

int
lua_registry_initialize(void) {
	_lua_registry = hashmap_allocate(13, 7);
	return 0;
}

void
lua_registry_finalize(void) {
	hashmap_deallocate(_lua_registry);
}

void
lua_module_register(const char* name, size_t length, lua_fn loader) {
	hashmap_insert(_lua_registry, hash(name, length), loader);
}
