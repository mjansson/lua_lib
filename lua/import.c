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

typedef struct {
	char* sourcecode;
	size_t sourcecode_size;
	tick_t timestamp;
} luaimport_source_t;

static int
lua_import_stream(stream_t* stream, luaimport_source_t* source) {
	int result = -1;

	source->sourcecode_size = stream_size(stream);
	source->sourcecode = memory_allocate(HASH_LUA, source->sourcecode_size, 0, MEMORY_PERSISTENT);
	source->timestamp = stream_last_modified(stream);

	if (stream_read(stream, source->sourcecode, source->sourcecode_size) == source->sourcecode_size)
		result = 0;

	return result;
}

static int
lua_import_output(const uuid_t uuid, const luaimport_source_t* import) {
	resource_source_t source;
	hash_t checksum;
	tick_t timestamp;
	uint64_t platform;
	string_const_t type = string_const(STRING_CONST("lua"));
	int ret = 0;

	resource_source_initialize(&source);
	resource_source_read(&source, uuid);

	timestamp = import->timestamp;
	platform = 0;

	checksum = hash(import->sourcecode, import->sourcecode_size);
	if (resource_source_write_blob(uuid, timestamp, HASH_SOURCE, platform, checksum,
	                               import->sourcecode, import->sourcecode_size)) {
		resource_source_set_blob(&source, timestamp, HASH_SOURCE, platform, checksum,
		                         import->sourcecode_size);
	} else {
		string_const_t uuidstr = string_from_uuid_static(uuid);
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE,
		           STRING_CONST("Failed to write resource source blob file: %.*s"),
		           STRING_FORMAT(uuidstr));
		ret = -1;
		goto finalize;
	}

	resource_source_set(&source, timestamp, HASH_RESOURCE_TYPE, 0, STRING_ARGS(type));

	if (!resource_source_write(&source, uuid, false)) {
		string_const_t uuidstr = string_from_uuid_static(uuid);
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE,
		           STRING_CONST("Failed to write resource source file: %.*s"),
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
	luaimport_source_t source = {0, 0};
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

	error_context_declare_local(char uuidbuf[40]; const string_t uuidstr = string_from_uuid(
	                                                  uuidbuf, sizeof(uuidbuf), uuid));
	error_context_push(STRING_CONST("importing module"), STRING_ARGS(uuidstr));

	if (store_import) {
		uuid_t founduuid = resource_import_map_store(STRING_ARGS(path), uuid, uint256_null());
		if (uuid_is_null(founduuid)) {
			log_warn(HASH_RESOURCE, WARNING_SUSPICIOUS,
			         STRING_CONST("Unable to open import map file to store new resource"));
			ret = -1;
			goto exit;
		}
		uuid = founduuid;
	}

	if ((ret = lua_import_stream(stream, &source)) < 0)
		goto exit;

	if ((ret = lua_import_output(uuid, &source)) < 0)
		goto exit;

	resource_import_map_store(STRING_ARGS(path), uuid, stream_sha256(stream));

exit:

	memory_deallocate(source.sourcecode);

	error_context_pop();

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
