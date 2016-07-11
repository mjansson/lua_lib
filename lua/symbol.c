/* symbol.c  -  Lua library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <lua/symbol.h>

#include <foundation/hash.h>
#include <foundation/hashmap.h>
LUA_EXTERN void* lj_clib_getsym_registry(lua_State* state, const char* sym, size_t length);

static hashmap_t* _lua_symbols;

LUA_EXTERN int
lua_symbol_initialize(void);

LUA_EXTERN void
lua_symbol_finalize(void);

//This is our custom callback point from luajit lj_clib resolver
void*
lj_clib_getsym_registry(lua_State* state, const char* sym, size_t length) {
	hash_t symhash = hash(sym, length);
	void* fn = hashmap_lookup(_lua_symbols, symhash);
	FOUNDATION_UNUSED(state);
	//log_debugf(HASH_LUA, STRING_CONST("Built-in lookup: %.*s -> %p", (int)length, sym, fn);
	return fn;
}

hashmap_t*
lua_symbol_lookup_map(void) {
	return _lua_symbols;
}

int
lua_symbol_initialize(void) {
	_lua_symbols = hashmap_allocate(BUILD_SIZE_LUA_LOOKUP_BUCKETS, 8);
	return 0;
}

void
lua_symbol_finalize(void) {
	hashmap_deallocate(_lua_symbols);
}
