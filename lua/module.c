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

#define LUA_USE_INTERNAL_HEADER

#include <lua/lua.h>

#include <foundation/foundation.h>

#include <resource/stream.h>
#include <resource/compile.h>
#include <resource/platform.h>

#undef LUA_API
#define LUA_HAS_LUA_STATE_TYPE

#include "luajit/src/lua.h"
#include "luajit/src/lauxlib.h"
#include "luajit/src/lualib.h"

#undef LUA_API

LUA_EXTERN int
lj_cf_package_loader_registry(lua_State* state);

static hashmap_t* _lua_modulemap;
static mutex_t* _lua_modulemap_lock;

struct lua_module_t {
	void* bytecode;
	size_t size;
};
typedef struct lua_module_t lua_module_t;

LUA_EXTERN int
lua_modulemap_initialize(void);

LUA_EXTERN void
lua_modulemap_finalize(void);

LUA_EXTERN void
lua_module_registry_initialize(lua_State* state);

LUA_EXTERN void
lua_module_registry_finalize(lua_State* state);

#if FOUNDATION_COMPILER_GCC
#  pragma GCC diagnostic ignored "-Wpedantic"
#endif

#define LOADED_MODULES_ENTRY_ARRAY "entry"

static void
lua_module_registry_add(lua_State* state, lua_modulemap_entry_t* entry) {
	lua_pushlstring(state, STRING_CONST(BUILD_REGISTRY_LOADED_MODULES));
	lua_gettable(state, LUA_REGISTRYINDEX);

	lua_pushlstring(state, STRING_CONST(LOADED_MODULES_ENTRY_ARRAY));
	lua_gettable(state, -2);

	lua_modulemap_entry_t** entryarr = lua_touserdata(state, -1);
	array_push(entryarr, entry);

	lua_pushlstring(state, STRING_CONST(LOADED_MODULES_ENTRY_ARRAY));
	lua_pushlightuserdata(state, entryarr);
	lua_settable(state, -4);

	lua_pop(state, 2);
}

static lua_modulemap_entry_t*
lua_module_registry_lookup(lua_State* state, const uuid_t uuid) {
	lua_pushlstring(state, STRING_CONST(BUILD_REGISTRY_LOADED_MODULES));
	lua_gettable(state, LUA_REGISTRYINDEX);

	lua_pushlstring(state, STRING_CONST(LOADED_MODULES_ENTRY_ARRAY));
	lua_gettable(state, -2);

	lua_modulemap_entry_t** entryarr = lua_touserdata(state, -1);

	lua_pop(state, 2);

	if (entryarr) {
		for (size_t ient = 0, esize = array_size(entryarr); ient != esize; ++ient) {
			if (uuid_equal(entryarr[ient]->uuid, uuid))
				return entryarr[ient];
		}
	}
	return nullptr;
}

void
lua_module_registry_initialize(lua_State* state) {
	lua_pushlstring(state, STRING_CONST(BUILD_REGISTRY_LOADED_MODULES));
	lua_newtable(state);
	lua_settable(state, LUA_REGISTRYINDEX);
}

void
lua_module_registry_finalize(lua_State* state) {
	lua_pushlstring(state, STRING_CONST(BUILD_REGISTRY_LOADED_MODULES));
	lua_gettable(state, LUA_REGISTRYINDEX);

	lua_pushlstring(state, STRING_CONST(LOADED_MODULES_ENTRY_ARRAY));
	lua_gettable(state, -2);

	lua_modulemap_entry_t** entryarr = lua_touserdata(state, -1);

	lua_pop(state, 2);

	if (entryarr)
		array_deallocate(entryarr);
}

//This is our custom callback point from luajit lib_package loader
int
lj_cf_package_loader_registry(lua_State* state) {
	size_t length = 0;
	const char* name = luaL_checklstring(state, 1, &length);

	mutex_lock(_lua_modulemap_lock);

	hash_t namehash = hash(name, length);
	lua_modulemap_entry_t* entry = hashmap_lookup(_lua_modulemap, namehash);

	mutex_unlock(_lua_modulemap_lock);

	if (entry) {
		lua_pushlstring(state, name, length);
		lua_pushlightuserdata(state, entry);
		lua_pushcclosure(state, entry->loader, 2);
	}
	else {
		lua_pushfstring(state, "\n\tno modulemap entry '%s'", name);
	}
	return 1;
}

int
lua_modulemap_initialize(void) {
	_lua_modulemap = hashmap_allocate(13, 7);
	_lua_modulemap_lock = mutex_allocate(STRING_CONST("lua-modulemap"));
	return 0;
}

void
lua_modulemap_finalize(void) {
	for (size_t ibucket = 0; ibucket < _lua_modulemap->num_buckets; ++ibucket) {
		hashmap_node_t* bucket = _lua_modulemap->bucket[ibucket];
		for (size_t inode = 0, nsize = array_size(bucket); inode < nsize; ++inode)
			memory_deallocate(bucket[inode].value);
	}
	hashmap_deallocate(_lua_modulemap);
	mutex_deallocate(_lua_modulemap_lock);
}

void
lua_module_register(const char* name, size_t length, const uuid_t uuid, lua_fn loader,
                    lua_preload_fn preload) {
	hash_t namehash = hash(name, length);

	mutex_lock(_lua_modulemap_lock);

	lua_modulemap_entry_t* entry = hashmap_lookup(_lua_modulemap, namehash);
	if (!entry) {
		entry = memory_allocate(HASH_LUA, sizeof(lua_modulemap_entry_t), 0, MEMORY_PERSISTENT);
		entry->name = namehash;
		entry->uuid = uuid;
		entry->loader = loader;
		entry->preload = preload;
		hashmap_insert(_lua_modulemap, hash(name, length), entry);
	}

	mutex_unlock(_lua_modulemap_lock);
}

static lua_module_t
lua_module_load_resource(const uuid_t uuid) {
	lua_module_t module = {0, 0};
	bool success = false;
	const uint32_t expected_version = LUA_RESOURCE_MODULE_VERSION;
	uint64_t platform = 0;
	stream_t* stream;
	bool recompile = false;
	bool recompiled = false;

	error_context_declare_local(
	    char uuidbuf[40];
	    const string_t uuidstr = string_from_uuid(uuidbuf, sizeof(uuidbuf), uuid);
	);
	error_context_push(STRING_CONST("loading module"), STRING_ARGS(uuidstr));

retry:

	stream = resource_stream_open_static(uuid, platform);
	if (stream) {
		resource_header_t header = resource_stream_read_header(stream);
		if ((header.type == HASH_LUA) && (header.version == expected_version)) {
			success = true;
		}
		else {
			log_warnf(HASH_LUA, WARNING_INVALID_VALUE,
			          STRING_CONST("Got unexpected type/version: %" PRIx64 " : %u"),
			          header.type, header.version);
			recompile = true;
		}
		stream_deallocate(stream);
		stream = nullptr;
	}
	if (success)
		stream = resource_stream_open_dynamic(uuid, platform);
	if (stream) {
		uint32_t version = stream_read_uint32(stream);
		size_t size = (size_t)stream_read_uint64(stream);
		if (version == expected_version) {
			void* buffer = memory_allocate(HASH_LUA, size, 0, MEMORY_TEMPORARY);
			if (stream_read(stream, buffer, size) == size) {
				module.bytecode = buffer;
				module.size = size;
			}
			else {
				memory_deallocate(buffer);
				log_warn(HASH_LUA, WARNING_SYSTEM_CALL_FAIL, STRING_CONST("Unable to read module data"));
			}
		}
		else {
			log_warnf(HASH_LUA, WARNING_INVALID_VALUE,
			          STRING_CONST("Got unexpected type/version: %u"),
			          version);
			recompile = true;
		}
		stream_deallocate(stream);
		stream = nullptr;
	}

	if (recompile && !recompiled) {
		recompiled = resource_compile(uuid, platform);
		if (recompiled)
			goto retry;
	}

	error_context_pop();

	return module;
}

/* Module garbage collection not enabled
static int
lua_module_gc(lua_State* state) {
	lua_modulemap_entry_t* entry = lua_touserdata(state, lua_upvalueindex(1));
	if (!entry)
		return 0;
	log_debug(HASH_LUA, STRING_CONST("Module garbage collected"));
	return 0;
}*/

static int
lua_module_upload(lua_State* state, const void* bytecode, size_t size) {
	string_const_t errmsg = {0, 0};
	lua_readbuffer_t read_buffer = {
		.buffer = bytecode,
		.size   = size,
		.offset = 0
	};

	log_debugf(HASH_LUA, STRING_CONST("Loading %" PRIsize " bytes of module bytecode"),
	           read_buffer.size);

	if (lua_load(state, lua_read_buffer, &read_buffer, "module") != 0) {
		errmsg.str = lua_tolstring(state, -1, &errmsg.length);
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua load failed (module): %.*s"),
		           STRING_FORMAT(errmsg));
		lua_pop(state, 1);
		return -1;
	}

	if (lua_pcall(state, 0, 1, 0) != 0) {
		errmsg.str = lua_tolstring(state, -1, &errmsg.length);
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua pcall failed (module): %.*s"),
		           STRING_FORMAT(errmsg));
		return -1;
	}

	return 0;
}

int
lua_module_loader(lua_State* state) {
	lua_modulemap_entry_t* entry = lua_touserdata(state, lua_upvalueindex(2));
	if (!entry)
		return 0;

	size_t name_length = 0;
	const char* name = luaL_checklstring(state, lua_upvalueindex(1), &name_length);

	int stacksize = lua_gettop(state);

	if (entry->preload)
		entry->preload();

	lua_module_t module = lua_module_load_resource(entry->uuid);
	if (module.size) {
		if (lua_module_upload(state, module.bytecode, module.size) == 0) {
			lua_module_registry_add(state, entry);
			if (lua_istable(state, -1)) {
				/* Modules are never garbage collected anyway, since lib_package keeps a global
				   registry reference in _LOADED table, and we keep a reference in loaded-modules
				   table for reload functionality
				lua_pushlstring(state, STRING_CONST("__gcproxy")); //Proxy table index
				lua_newuserdata(state, 8); //New proxy object
				lua_newtable(state); //New metatable for proxy
				lua_pushlstring(state, STRING_CONST("__gc"));
				lua_pushlightuserdata(state, entry);
				lua_pushcclosure(state, lua_module_gc, 1);
				lua_rawset(state, -3); //Set __gc metatable method
				lua_setmetatable(state, -2); //Set metatable on proxy
				lua_rawset(state, -3); //Set proxy in table index __gcproxy */

				lua_pushlstring(state, STRING_CONST(BUILD_REGISTRY_LOADED_MODULES));
				lua_gettable(state, LUA_REGISTRYINDEX);

				lua_pushlightuserdata(state, entry); //entry is key
				lua_pushvalue(state, -3); //copy the loaded module table as value

				lua_pushlstring(state, STRING_CONST("__modulename"));
				lua_pushlstring(state, name, name_length);
				lua_settable(state, -3); //set moduletable["__modulename"] = name

				lua_settable(state, -3); //set table loaded-modules[entry] = moduletable

				lua_pop(state, 1);
			}
		}
		memory_deallocate(module.bytecode);
	}
	else {
		string_const_t uuidstr = string_from_uuid_static(entry->uuid);
		lua_pushfstring(state, "unable to load module '%s'", uuidstr.str);
		lua_error(state);
	}

	return lua_gettop(state) - stacksize;
}

bool
lua_module_is_loaded(lua_t* lua, const uuid_t uuid) {
	return lua_module_registry_lookup(lua_state(lua), uuid) != nullptr;
}

int
lua_module_reload(lua_t* lua, const uuid_t uuid) {
	int ret = -1;
	lua_State* state = lua_state(lua);
	lua_modulemap_entry_t* entry = lua_module_registry_lookup(state, uuid);
	if (entry) {
		int stacksize = lua_gettop(state);

		lua_module_t module = lua_module_load_resource(uuid);
		if (module.size) {
			string_const_t uuidstr = string_from_uuid_static(uuid);
			log_debugf(HASH_LUA, STRING_CONST("Reloading module: %.*s"), STRING_FORMAT(uuidstr));
			if (lua_module_upload(state, module.bytecode, module.size) == 0) {
				//Check if module loaded as a table
				if (lua_istable(state, -1)) {
					lua_pushlstring(state, STRING_CONST(BUILD_REGISTRY_LOADED_MODULES));
					lua_gettable(state, LUA_REGISTRYINDEX);

					//Get and replace the old loaded module table
					lua_pushlightuserdata(state, entry);
					lua_gettable(state, -2);
					lua_replace(state, -2); //Get rid of loaded-modules registry table from stack
					if (lua_istable(state, -1)) {
						lua_pushlstring(state, STRING_CONST("__modulename"));
						lua_gettable(state, -2);

						size_t name_length = 0;
						const char* modulename = lua_isnil(state, -1)
						                         ? nullptr
						                         : luaL_checklstring(state, -1, &name_length);
						log_debugf(HASH_LUA, STRING_CONST("Replacing module table: %.*s (%.*s)"),
						           STRING_FORMAT(uuidstr), (int)name_length, modulename);
						lua_pop(state, 1); //Get rid of name from stack

						//Clear previous loaded-modules table
						lua_pushnil(state);
						while (lua_next(state, -2) != 0) {
							lua_pop(state, 1); //Old value
							lua_pushnil(state); //Replace with nil
							lua_settable(state, -3); //Erase in previous loaded-modules table
							lua_pushnil(state); //Restart lua_next
						}

						//Copy new module table to previous loaded-modules table
						lua_pushnil(state);
						while (lua_next(state, -3) != 0) { //Lookup in new module table
							lua_pushvalue(state, -2); //Copy key
							lua_pushvalue(state, -2); //Copy value
							lua_settable(state, -5); //Set in previous loaded-modules table
							lua_pop(state, 1); //Pop value, leaving key for lua_next iteration
						}

						lua_pushlstring(state, STRING_CONST("__modulename"));
						lua_pushlstring(state, modulename, name_length);
						lua_settable(state, -3);
					}
				}
				ret = 0;
			}
			else {
				uuidstr = string_from_uuid_static(uuid);
				log_warnf(HASH_LUA, WARNING_RESOURCE, STRING_CONST("Unable to reload module '%.*s'"),
				          STRING_FORMAT(uuidstr));
			}
		}
		else {
			string_const_t uuidstr = string_from_uuid_static(uuid);
			log_warnf(HASH_LUA, WARNING_RESOURCE, STRING_CONST("Unable to load module '%.*s'"),
			          STRING_FORMAT(uuidstr));
		}

		lua_settop(state, stacksize);
	}
	else {
		string_const_t uuidstr = string_from_uuid_static(uuid);
		log_debugf(HASH_LUA, STRING_CONST("Reloading module ignored, not loaded: %.*s"),
		           STRING_FORMAT(uuidstr));
	}
	return ret;
}
