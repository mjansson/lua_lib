/* compile.c  -  Lua library  -  Public Domain  -  2016 Mattias Jansson / Rampant Pixels
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
} lua_compile_dump_t;

static FOUNDATION_NOINLINE int
lua_compile_dump_writer(lua_State* state, const void* buffer, size_t size, void* user_data) {
	lua_compile_dump_t* dump = user_data;

	FOUNDATION_UNUSED(state);

	if (size <= 0)
		return 0;

	dump->bytecode = (dump->bytecode ?
	                  memory_reallocate(dump->bytecode, dump->bytecode_size + size, 0, dump->bytecode_size, 0) :
	                  memory_allocate(HASH_LUA, size, 0, MEMORY_PERSISTENT));

	memcpy(dump->bytecode + dump->bytecode_size, buffer, size);
	dump->bytecode_size += size;

	return 0;
}

static resource_change_t*
resource_source_platform_reduce(resource_change_t* change, resource_change_t* best, void* data) {
	uint64_t** subplatforms = data;
	uint64_t platform = (*subplatforms)[0];
	size_t iplat, psize;
	FOUNDATION_UNUSED(best);
	if ((platform == RESOURCE_PLATFORM_ALL) ||
	        resource_platform_is_equal_or_more_specific(change->platform, platform)) {
		for (iplat = 1, psize = array_size(*subplatforms); iplat != psize; ++iplat) {
			if ((*subplatforms)[iplat] == change->platform)
				break;
		}
		if (iplat >= psize)
			array_push(*subplatforms, change->platform);
	}
	return 0;
}

int
lua_compile(const uuid_t uuid, uint64_t platform, resource_source_t* source,
            const uint256_t source_hash, const char* type, size_t type_length) {
	int result = 0;
	uint64_t* subplatforms = 0;
	size_t iplat, psize;
	hashmap_fixed_t fixedmap;
	hashmap_t* map = (hashmap_t*)&fixedmap;
	hash_t resource_type_hash;

	resource_type_hash = hash(type, type_length);

	if (resource_type_hash != HASH_LUA)
		return -1;

	uint64_t restore_platform = lua_resource_platform();
	lua_resource_set_platform(platform);

	error_context_declare_local(
	    char uuidbuf[40];
	    const string_t uuidstr = string_from_uuid(uuidbuf, sizeof(uuidbuf), uuid);
	);
	error_context_push(STRING_CONST("compiling module"), STRING_ARGS(uuidstr));

	array_push(subplatforms, platform);

	hashmap_initialize(map, sizeof(fixedmap.bucket) / sizeof(fixedmap.bucket[0]), 8);
	resource_source_map_all(source, map, false);
	resource_source_map_reduce(source, map, &subplatforms, resource_source_platform_reduce);
	resource_source_map_clear(map);

	resource_platform_t platform_decl = resource_platform_decompose(platform);
	if (lua_arch_is_fr2(platform_decl.arch) && (array_size(subplatforms) == 1))
		array_push(subplatforms, platform);

	for (iplat = 1, psize = array_size(subplatforms); (iplat != psize) && (result == 0); ++iplat) {
		void* compiled_blob = 0;
		size_t compiled_size = 0;
		stream_t* stream;
		uint64_t subplatform = subplatforms[iplat];

		platform_decl = resource_platform_decompose(subplatform);
		bool need_fr2 = lua_arch_is_fr2(platform_decl.arch);

		//We cannot compile for a non-matching FR2 mode
		if (need_fr2 != lua_is_fr2()) {
			log_infof(HASH_RESOURCE, STRING_CONST("Unable to compile for non-matching FR2 mode, platform %"
			                                      PRIx64 " (FR2 %s)"),
			          subplatform, need_fr2 ? "true" : "false");
			result = -1;
			break;
		}

		log_debugf(HASH_RESOURCE, STRING_CONST("Compile for platform: %" PRIx64 " (FR2 %s)"),
		           subplatform, lua_is_fr2() ? "true" : "false");

		resource_change_t* sourcechange = resource_source_get(source, HASH_SOURCE, subplatform);
		if (sourcechange && (sourcechange->flags & RESOURCE_SOURCEFLAG_BLOB)) {
			size_t source_size = sourcechange->value.blob.size;
			if (source_size <= 0) {
				log_errorf(HASH_RESOURCE, ERROR_INVALID_VALUE,
				           STRING_CONST("No blob for platform %" PRIx64),
				           subplatform);
				result = -1;
				continue;
			}

			void* source_blob = memory_allocate(HASH_RESOURCE, source_size, 0, MEMORY_PERSISTENT);
			if (!resource_source_read_blob(uuid, HASH_SOURCE, sourcechange->platform,
			                               sourcechange->value.blob.checksum,
			                               source_blob, source_size)) {
				memory_deallocate(source_blob);
				log_errorf(HASH_RESOURCE, ERROR_SYSTEM_CALL_FAIL,
				           STRING_CONST("Unable to read blob for platform %" PRIx64),
				           subplatform);
				result = -1;
				continue;
			}

			lua_t* env;
			lua_State* state;
			lua_compile_dump_t dump;
			lua_readbuffer_t read_buffer = {
				.buffer = source_blob,
				.size = source_size,
				.offset = 0
			};

			FOUNDATION_UNUSED(uuid);

			env = lua_allocate();
			state = lua_state(env);
			memset(&dump, 0, sizeof(dump));

			if (lua_load(state, lua_read_buffer, &read_buffer, "compile") != 0) {
				const char* errstr = lua_tostring(state, -1);
				log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua load failed: %s"),
				           errstr ? errstr : "<no error>");
				lua_pop(state, 1);
				result = -1;
			}
			else {
				lua_dump(state, lua_compile_dump_writer, &dump);

				if (lua_pcall(state, 0, 0, 0) != 0) {
					const char* errstr = lua_tostring(state, -1);
					log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua pcall failed: %s"),
					           errstr ? errstr : "<no error>");
					lua_pop(state, 1);
					result = -1;
				}
				else {
					log_debug(HASH_LUA, STRING_CONST("Lua bytecode dump successful"));
					compiled_blob = dump.bytecode;
					compiled_size = dump.bytecode_size;
				}
			}

			lua_deallocate(env);

			memory_deallocate(source_blob);

			if (result < 0)
				memory_deallocate(dump.bytecode);
		}

		if (compiled_size <= 0)
			continue;

		stream = resource_local_create_static(uuid, subplatform);
		if (stream) {
			const uint32_t version = LUA_RESOURCE_MODULE_VERSION;
			resource_header_t header = {
				.type = resource_type_hash,
				.version = version,
				.source_hash = source_hash
			};
			resource_stream_write_header(stream, header);
			string_const_t streampath = stream_path(stream);
			log_debugf(HASH_RESOURCE, STRING_CONST("Wrote static resource stream: %.*s"),
			           STRING_FORMAT(streampath));
			stream_deallocate(stream);

			if (compiled_size > 0) {
				stream = resource_local_create_dynamic(uuid, subplatform);
				if (stream) {
					stream_write_uint32(stream, version);
					stream_write_uint64(stream, compiled_size);
					stream_write(stream, compiled_blob, compiled_size);
					streampath = stream_path(stream);
					log_debugf(HASH_RESOURCE, STRING_CONST("Wrote dynamic resource stream: %.*s"),
					           STRING_FORMAT(streampath));
					stream_deallocate(stream);
				}
				else {
					log_errorf(HASH_RESOURCE, ERROR_SYSTEM_CALL_FAIL,
					           STRING_CONST("Unable to create dynamic resource stream"));
					result = -1;
				}
			}
		}
		else {
			log_errorf(HASH_RESOURCE, ERROR_SYSTEM_CALL_FAIL,
			           STRING_CONST("Unable to create static resource stream"));
			result = -1;
		}

		memory_deallocate(compiled_blob);
	}

	array_deallocate(subplatforms);
	hashmap_finalize(map);

	error_context_pop();

	lua_resource_set_platform(restore_platform);

	return result;
}

#else

int
lua_compile(const uuid_t uuid, uint64_t platform, resource_source_t* source,
            const uint256_t source_hash, const char* type, size_t type_length) {
	FOUNDATION_UNUSED(uuid);
	FOUNDATION_UNUSED(platform);
	FOUNDATION_UNUSED(source);
	FOUNDATION_UNUSED(source_hash);
	FOUNDATION_UNUSED(type);
	FOUNDATION_UNUSED(type_length);
	return -1;
}

#endif
