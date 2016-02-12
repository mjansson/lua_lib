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

static resource_change_t*
resource_source_platform_reduce(resource_change_t* change, resource_change_t* best, void* data) {
	uint64_t** subplatforms = data;
	uint64_t platform = (*subplatforms)[0];
	size_t iplat, psize;
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
               const char* type, size_t type_length) {
	int result = 0;
	uint64_t* subplatforms = 0;
	size_t iplat, psize;
	hashmap_fixed_t fixedmap;
	hashmap_t* map = (hashmap_t*)&fixedmap;
	hash_t resource_type_hash;

	resource_type_hash = hash(type, type_length);

	if (resource_type_hash != HASH_LUA)
		return -1;

	array_push(subplatforms, platform);

	hashmap_initialize(map, sizeof(fixedmap.bucket) / sizeof(fixedmap.bucket[0]), 8);
	resource_source_map_all(source, map, false);
	resource_source_map_reduce(source, map, &subplatforms, resource_source_platform_reduce);
	resource_source_map_clear(map);

	for (iplat = 1, psize = array_size(subplatforms); (iplat != psize) && (result == 0); ++iplat) {
		void* compiled_blob = 0;
		size_t compiled_size = 0;
		stream_t* stream;
		uint64_t subplatform = subplatforms[iplat];

		resource_change_t* sourcechange = resource_source_get(source, HASH_SOURCE, subplatform);
		if (sourcechange && (sourcechange->flags & RESOURCE_SOURCEFLAG_BLOB)) {
			compiled_size = sourcechange->value.blob.size;
			compiled_blob = memory_allocate(HASH_RESOURCE, compiled_size, 0, MEMORY_PERSISTENT);
			if (!resource_source_read_blob(uuid, HASH_SOURCE, subplatform, sourcechange->value.blob.checksum,
			                               compiled_blob, compiled_size)) {
				memory_deallocate(compiled_blob);
				compiled_blob = 0;
				compiled_size = 0;
			}
		}

		if (compiled_size <= 0)
			continue;

		stream = resource_local_create_static(uuid, subplatform);
		if (stream) {
			uint32_t version = 1;
			stream_write_uint64(stream, resource_type_hash);
			stream_write_uint32(stream, version);
			stream_deallocate(stream);

			if (compiled_size > 0) {
				stream = resource_local_create_dynamic(uuid, subplatform);
				if (stream) {
					stream_write_uint32(stream, version);
					stream_write_uint64(stream, compiled_size);
					stream_write(stream, compiled_blob, compiled_size);
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

	return result;
}

#endif
