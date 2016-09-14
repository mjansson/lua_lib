/* import.c  -  Lua library  -  Public Domain  -  2016 Mattias Jansson / Rampant Pixels
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

#include <foundation/foundation.h>
#include <lua/lua.h>
#include <resource/resource.h>

#if RESOURCE_ENABLE_LOCAL_SOURCE

LUA_API int
lua_load(lua_State* L, lua_Reader reader, void* dt, const char* chunkname);

LUA_API int
lua_dump(lua_State* L, lua_Writer writer, void* data);

LUA_API int
lua_pcall(lua_State* L, int nargs, int nresults, int errfunc);

typedef struct {
	char*   bytecode;
	size_t  bytecode_size;
} luaimport_dump_t;

static FOUNDATION_NOINLINE int
lua_import_dump_writer(lua_State* state, const void* buffer, size_t size, void* user_data) {
	luaimport_dump_t* dump = user_data;

	FOUNDATION_UNUSED(state);

	if (size <= 0)
		return 0;

	dump->bytecode = (dump->bytecode ?
	                  memory_reallocate(dump->bytecode, dump->bytecode_size + size, 0, dump->bytecode_size) :
	                  memory_allocate(HASH_LUA, size, 0, MEMORY_PERSISTENT));

	memcpy(dump->bytecode + dump->bytecode_size, buffer, size);
	dump->bytecode_size += size;

	return 0;
}

static int
lua_import_stream(stream_t* stream, const uuid_t uuid, luaimport_dump_t* dump) {
	lua_t* env;
	lua_State* state;
	int result = 0;
	lua_readstream_t read_stream = {
		.stream = stream,
	};

	FOUNDATION_UNUSED(uuid);

	env = lua_allocate();
	state = lua_state(env);

	if (lua_load(state, lua_read_stream, &read_stream, "import") != 0) {
		const char* errstr = lua_tostring(state, -1);
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua load failed: %s"),
		           errstr ? errstr : "<no error>");
		lua_pop(state, 1);
		result = -1;
		goto exit;
	}

	lua_dump(state, lua_import_dump_writer, dump);

	if (lua_pcall(state, 0, 0, 0) != 0) {
		const char* errstr = lua_tostring(state, -1);
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua pcall failed: %s"),
		           errstr ? errstr : "<no error>");
		lua_pop(state, 1);
		result = -1;
		goto exit;
	}

	log_debug(HASH_LUA, STRING_CONST("Lua bytecode dump successful"));

exit:

	lua_deallocate(env);

	return result;
}

static int
lua_import_output(const uuid_t uuid, const luaimport_dump_t* dump) {
	resource_source_t source;
	hash_t checksum;
	tick_t timestamp;
	uint64_t platform;
	string_const_t type = string_const(STRING_CONST("lua"));
	int ret = 0;

	resource_source_initialize(&source);
	resource_source_read(&source, uuid);

	timestamp = time_system();
	platform = 0;

	checksum = hash(dump->bytecode, dump->bytecode_size);
	if (resource_source_write_blob(uuid, timestamp, HASH_SOURCE,
	                               platform, checksum,
	                               dump->bytecode, dump->bytecode_size)) {
		resource_source_set_blob(&source, timestamp, HASH_SOURCE,
		                         platform, checksum, dump->bytecode_size);
	}
	else {
		string_const_t uuidstr = string_from_uuid_static(uuid);
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Failed to write resource source blob file: %.*s"),
		           STRING_FORMAT(uuidstr));
		ret = -1;
		goto finalize;
	}

	resource_source_set(&source, timestamp, HASH_RESOURCE_TYPE,
	                    0, STRING_ARGS(type));

	if (!resource_source_write(&source, uuid, false)) {
		string_const_t uuidstr = string_from_uuid_static(uuid);
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Failed to write resource source file: %.*s"),
		           STRING_FORMAT(uuidstr));
		ret = -1;
		goto finalize;
	}

finalize:

	resource_source_finalize(&source);

	return ret;
}

int
lua_import(stream_t* stream, const uuid_t uuid_given) {
	uuid_t uuid = uuid_given;
	string_const_t path;
	string_const_t extension;
	bool store_import = false;
	luaimport_dump_t dump = {0, 0};
	int ret;

	path = stream_path(stream);
	extension = path_file_extension(STRING_ARGS(path));
	if (!string_equal_nocase(STRING_ARGS(extension), STRING_CONST("lua")))
		return -1;

	if (uuid_is_null(uuid))
		uuid = resource_import_lookup(STRING_ARGS(path)).uuid;

	if (uuid_is_null(uuid)) {
		uuid = uuid_generate_random();
		store_import = true;
	}

	if (store_import) {
		uuid_t founduuid = resource_import_map_store(STRING_ARGS(path), uuid, uint256_null());
		if (uuid_is_null(founduuid)) {
			log_warn(HASH_RESOURCE, WARNING_SUSPICIOUS,
			         STRING_CONST("Unable to open import map file to store new resource"));
			return -1;
		}
		uuid = founduuid;
	}

	if ((ret = lua_import_stream(stream, uuid, &dump)) < 0)
		goto exit;

	if ((ret = lua_import_output(uuid, &dump)) < 0)
		goto exit;

	resource_import_map_store(STRING_ARGS(path), uuid, stream_sha256(stream));

exit:

	memory_deallocate(dump.bytecode);

	return ret;
}

#else

int
lua_import(stream_t* stream, const uuid_t uuid) {
	FOUNDATION_UNUSED(stream);
	FOUNDATION_UNUSED(uuid);
	return -1;
}

#endif
