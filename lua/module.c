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
#include <lua/hashstrings.h>
#include <lua/read.h>

#include <foundation/hash.h>
#include <foundation/hashmap.h>
#include <foundation/memory.h>
#include <foundation/array.h>
#include <foundation/stream.h>
#include <foundation/log.h>

#include <resource/stream.h>
#include <resource/platform.h>

#undef LUA_API
#define LUA_HAS_LUA_STATE_TYPE

#include "luajit/src/lua.h"
#include "luajit/src/lauxlib.h"
#include "luajit/src/lualib.h"

LUA_EXTERN void* lj_clib_getsym_registry(lua_State*, const char*, size_t);

static hashmap_t* _lua_registry;

struct lua_module_t {
	void* bytecode;
	size_t size;
};
typedef struct lua_module_t lua_module_t;

struct lua_registry_entry_t {
	hash_t name;
	uuid_t uuid;
	lua_fn loader;
	lua_preload_fn preload;
	lua_module_t module;
};
typedef struct lua_registry_entry_t lua_registry_entry_t;

LUA_EXTERN int
lua_registry_initialize(void);

LUA_EXTERN void
lua_registry_finalize(void);

#if FOUNDATION_COMPILER_GCC
#  pragma GCC diagnostic ignored "-Wpedantic"
#endif

//This is our custom callback point from luajit lib_package loader
int
lj_cf_package_loader_registry(lua_State* state) {
	size_t length = 0;
	const char* name = luaL_checklstring(state, 1, &length);
	hash_t namehash = hash(name, length);
	lua_registry_entry_t* entry = hashmap_lookup(_lua_registry, namehash);
	if (entry) {
		lua_pushlightuserdata(state, entry);
		lua_pushcclosure(state, entry->loader, 1);
	}
	else {
		lua_pushfstring(state, "\n\tno registry entry '%s'", name);
	}
	return 1;
}

int
lua_registry_initialize(void) {
	_lua_registry = hashmap_allocate(13, 7);
	return 0;
}

void
lua_registry_finalize(void) {
	for (size_t ibucket = 0; ibucket < _lua_registry->num_buckets; ++ibucket) {
		hashmap_node_t* bucket = _lua_registry->bucket[ibucket];
		for (size_t inode = 0, nsize = array_size(bucket); inode < nsize; ++inode ) {
			hashmap_node_t* node = bucket + inode;
			lua_registry_entry_t* entry = node->value;
			memory_deallocate(entry->module.bytecode);
			memory_deallocate(entry);
		}
	}
	hashmap_deallocate(_lua_registry);
}

void
lua_module_register(const char* name, size_t length, uuid_t uuid, lua_fn loader, lua_preload_fn preload) {
	hash_t namehash = hash(name, length);
	lua_registry_entry_t* entry = hashmap_lookup(_lua_registry, namehash);
	if (!entry) {
		entry = memory_allocate(HASH_LUA, sizeof(lua_registry_entry_t), 0, MEMORY_PERSISTENT);
		hashmap_insert(_lua_registry, hash(name, length), entry);
	}
	else {
		memory_deallocate(entry->module.bytecode);
	}
	entry->name = namehash;
	entry->uuid = uuid;
	entry->loader = loader;
	entry->preload = preload;
	entry->module.bytecode = 0;
	entry->module.size = 0;
}

static lua_module_t
lua_module_load(const uuid_t uuid) {
	lua_module_t module = {0, 0};
	bool success = false;
	const uint32_t expected_version = 1;
	uint64_t platform = 0;
	stream_t* stream;

	stream = resource_stream_open_static(uuid, platform);
	if (stream) {
		resource_header_t header = resource_stream_read_header(stream);
		if ((header.type == HASH_LUA) && (header.version == expected_version)) {
			success = true;
		}
		else {
			log_warnf(HASH_LUA, WARNING_INVALID_VALUE,
			          STRING_CONST("Got unexpected type/version when loading Lua module: %" PRIx64 " : %u"),
			          header.type, header.version);
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
			}
		}
		else {
			log_warnf(HASH_LUA, WARNING_INVALID_VALUE,
			          STRING_CONST("Got unexpected type/version when loading Lua module blob: %u"),
			          version);
		}
		stream_deallocate(stream);
		stream = nullptr;
	}

	return module;	
}

int
lua_module_loader(lua_State* state) {
	lua_registry_entry_t* entry = lua_touserdata(state, lua_upvalueindex(1));
	if (!entry)
		return 0;

	if (entry->preload)
		entry->preload();

	if (!entry->module.size) {
		entry->module = lua_module_load(entry->uuid);
		if (!entry->module.size) {
			string_const_t uuidstr = string_from_uuid_static(entry->uuid);
			lua_pushfstring(state, "unable to load module '%s'", uuidstr.str);
			lua_error(state);
			goto exit;
		}
	}

	string_const_t errmsg = {0, 0};
	lua_readbuffer_t read_buffer = {
		.buffer = entry->module.bytecode,
		.size   = entry->module.size,
		.offset = 0
	};
	int stacksize;

	log_debugf(HASH_LUA, STRING_CONST("Loading %u bytes of module bytecode"),
	           read_buffer.size);

	stacksize = lua_gettop(state);

	if (lua_load(state, lua_read_buffer, &read_buffer, "module") != 0) {
		errmsg.str = lua_tolstring(state, -1, &errmsg.length);
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua load failed (module): %.*s"),
		           STRING_FORMAT(errmsg));
		lua_pop(state, 1);
		goto exit;
	}

	if (lua_pcall(state, 0, 1, 0) != 0) {
		errmsg.str = lua_tolstring(state, -1, &errmsg.length);
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua pcall failed (module): %.*s"),
		           STRING_FORMAT(errmsg));
		goto exit;
	}

	log_debug(HASH_LUA, STRING_CONST("Loaded module"));

exit:
	return lua_gettop(state) - stacksize;
}
