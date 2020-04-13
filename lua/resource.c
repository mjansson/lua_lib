/* resource.c  -  Lua library  -  Public Domain  -  2013 Mattias Jansson
 *
 * This library provides a cross-platform lua library in C11 for games and applications
 * based on out foundation library. The latest source code is always available at
 *
 * https://github.com/mjansson/lua_lib
 *
 * This library is put in the public domain; you can redistribute it and/or modify it without
 * any restrictions.
 *
 * The LuaJIT library is released under the MIT license. For more information about LuaJIT, see
 * http://luajit.org/
 */

#include <lua/lua.h>

#include <foundation/foundation.h>
#include <resource/resource.h>

static bool _symbols_loaded;
static uint64_t _resource_platform;

uint64_t
lua_resource_platform(void) {
	if (_resource_platform)
		return _resource_platform;
	architecture_t arch = system_architecture();
	if (lua_arch_is_fr2((int)arch)) {
		resource_platform_t decl;
		memset(&decl, 0xFF, sizeof(decl));
		decl.arch = (1 << 7);
		return resource_platform(decl);
	}
	return 0;
}

void
lua_resource_set_platform(uint64_t platform) {
	resource_platform_t decl = resource_platform_decompose(platform);
	if (lua_arch_is_fr2(decl.arch)) {
		decl.arch |= (1 << 7);
		platform = resource_platform(decl);
	}
	_resource_platform = platform;
}

void
lua_symbol_load_resource(void) {
	hashmap_t* map = lua_symbol_lookup_map();

	if (_symbols_loaded)
		return;
	_symbols_loaded = true;

#define RESOURCE_SYM(fn, name) hashmap_insert(map, HASH_SYM_##name, (void*)(uintptr_t)fn)

	RESOURCE_SYM(resource_bundle_load, RESOURCE_BUNDLE_LOAD);
	RESOURCE_SYM(resource_bundle_stream, RESOURCE_BUNDLE_STREAM);

	RESOURCE_SYM(resource_change_is_value, RESOURCE_CHANGE_IS_VALUE);
	RESOURCE_SYM(resource_change_is_blob, RESOURCE_CHANGE_IS_BLOB);
	RESOURCE_SYM(resource_change_data_allocate, RESOURCE_CHANGE_DATA_ALLOCATE);
	RESOURCE_SYM(resource_change_data_deallocate, RESOURCE_CHANGE_DATA_DEALLOCATE);
	RESOURCE_SYM(resource_change_data_initialize, RESOURCE_CHANGE_DATA_INITIALIZE);
	RESOURCE_SYM(resource_change_data_finalize, RESOURCE_CHANGE_DATA_FINALIZE);
	RESOURCE_SYM(resource_change_block_allocate, RESOURCE_CHANGE_BLOCK_ALLOCATE);
	RESOURCE_SYM(resource_change_block_deallocate, RESOURCE_CHANGE_BLOCK_DEALLOCATE);
	RESOURCE_SYM(resource_change_block_initialize, RESOURCE_CHANGE_BLOCK_INITIALIZE);
	RESOURCE_SYM(resource_change_block_finalize, RESOURCE_CHANGE_BLOCK_FINALIZE);

	RESOURCE_SYM(resource_compile_need_update, RESOURCE_COMPILE_NEED_UPDATE);
	RESOURCE_SYM(resource_compile, RESOURCE_COMPILE);
	RESOURCE_SYM(resource_compile_register, RESOURCE_COMPILE_REGISTER);
	RESOURCE_SYM(resource_compile_register_path, RESOURCE_COMPILE_REGISTER_PATH);
	RESOURCE_SYM(resource_compile_unregister, RESOURCE_COMPILE_UNREGISTER);
	RESOURCE_SYM(resource_compile_unregister_path, RESOURCE_COMPILE_UNREGISTER_PATH);

	RESOURCE_SYM(resource_event_uuid, RESOURCE_EVENT_UUID);
	RESOURCE_SYM(resource_event_token, RESOURCE_EVENT_TOKEN);
	RESOURCE_SYM(resource_event_post, RESOURCE_EVENT_POST);
	RESOURCE_SYM(resource_event_stream, RESOURCE_EVENT_STREAM);
	RESOURCE_SYM(resource_event_handle, RESOURCE_EVENT_HANDLE);

	RESOURCE_SYM(resource_import_base_path, RESOURCE_IMPORT_BASE_PATH);
	RESOURCE_SYM(resource_import_set_base_path, RESOURCE_IMPORT_SET_BASE_PATH);
	RESOURCE_SYM(resource_import, RESOURCE_IMPORT);
	RESOURCE_SYM(resource_import_register, RESOURCE_IMPORT_REGISTER);
	RESOURCE_SYM(resource_import_register_path, RESOURCE_IMPORT_REGISTER_PATH);
	RESOURCE_SYM(resource_import_unregister, RESOURCE_IMPORT_UNREGISTER);
	RESOURCE_SYM(resource_import_unregister_path, RESOURCE_IMPORT_UNREGISTER_PATH);
	RESOURCE_SYM(resource_import_lookup, RESOURCE_IMPORT_LOOKUP);
	RESOURCE_SYM(resource_import_map_store, RESOURCE_IMPORT_MAP_STORE);
	RESOURCE_SYM(resource_import_map_purge, RESOURCE_IMPORT_MAP_PURGE);

	RESOURCE_SYM(resource_autoimport, RESOURCE_AUTOIMPORT);
	RESOURCE_SYM(resource_autoimport_need_update, RESOURCE_AUTOIMPORT_NEED_UPDATE);
	RESOURCE_SYM(resource_autoimport_watch, RESOURCE_AUTOIMPORT_WATCH);
	RESOURCE_SYM(resource_autoimport_unwatch, RESOURCE_AUTOIMPORT_UNWATCH);
	RESOURCE_SYM(resource_autoimport_clear, RESOURCE_AUTOIMPORT_CLEAR);
	RESOURCE_SYM(resource_autoimport_event_handle, RESOURCE_AUTOIMPORT_EVENT_HANDLE);

	RESOURCE_SYM(resource_local_paths, RESOURCE_LOCAL_PATHS);
	RESOURCE_SYM(resource_local_set_paths, RESOURCE_LOCAL_SET_PATHS);
	RESOURCE_SYM(resource_local_add_path, RESOURCE_LOCAL_ADD_PATH);
	RESOURCE_SYM(resource_local_remove_path, RESOURCE_LOCAL_REMOVE_PATH);
	RESOURCE_SYM(resource_local_clear_paths, RESOURCE_LOCAL_CLEAR_PATHS);
	RESOURCE_SYM(resource_local_open_static, RESOURCE_LOCAL_OPEN_STATIC);
	RESOURCE_SYM(resource_local_open_dynamic, RESOURCE_LOCAL_OPEN_DYNAMIC);
	RESOURCE_SYM(resource_local_create_static, RESOURCE_LOCAL_CREATE_STATIC);
	RESOURCE_SYM(resource_local_create_dynamic, RESOURCE_LOCAL_CREATE_DYNAMIC);

	RESOURCE_SYM(resource_platform, RESOURCE_PLATFORM);
	RESOURCE_SYM(resource_platform_decompose, RESOURCE_PLATFORM_DECOMPOSE);
	RESOURCE_SYM(resource_platform_is_equal_or_more_specific, RESOURCE_PLATFORM_IS_EQUAL_OR_MORE_SPECIFIC);
	RESOURCE_SYM(resource_platform_reduce, RESOURCE_PLATFORM_REDUCE);

	RESOURCE_SYM(resource_remote_sourced, RESOURCE_REMOTE_SOURCED);
	RESOURCE_SYM(resource_remote_sourced_connect, RESOURCE_REMOTE_SOURCED_CONNECT);
	RESOURCE_SYM(resource_remote_sourced_disconnect, RESOURCE_REMOTE_SOURCED_DISCONNECT);
	RESOURCE_SYM(resource_remote_sourced_is_connected, RESOURCE_REMOTE_SOURCED_IS_CONNECTED);
	RESOURCE_SYM(resource_remote_sourced_lookup, RESOURCE_REMOTE_SOURCED_LOOKUP);
	RESOURCE_SYM(resource_remote_sourced_hash, RESOURCE_REMOTE_SOURCED_HASH);
	RESOURCE_SYM(resource_remote_sourced_dependencies, RESOURCE_REMOTE_SOURCED_DEPENDENCIES);
	RESOURCE_SYM(resource_remote_sourced_read, RESOURCE_REMOTE_SOURCED_READ);
	RESOURCE_SYM(resource_remote_sourced_read_blob, RESOURCE_REMOTE_SOURCED_READ_BLOB);
	RESOURCE_SYM(resource_remote_compiled, RESOURCE_REMOTE_COMPILED);
	RESOURCE_SYM(resource_remote_compiled_connect, RESOURCE_REMOTE_COMPILED_CONNECT);
	RESOURCE_SYM(resource_remote_compiled_disconnect, RESOURCE_REMOTE_COMPILED_DISCONNECT);
	RESOURCE_SYM(resource_remote_open_static, RESOURCE_REMOTE_OPEN_STATIC);
	RESOURCE_SYM(resource_remote_open_dynamic, RESOURCE_REMOTE_OPEN_DYNAMIC);

	RESOURCE_SYM(resource_module_initialize, RESOURCE_MODULE_INITIALIZE);
	RESOURCE_SYM(resource_module_finalize, RESOURCE_MODULE_FINALIZE);
	RESOURCE_SYM(resource_module_is_initialized, RESOURCE_MODULE_IS_INITIALIZED);
	RESOURCE_SYM(resource_module_version, RESOURCE_MODULE_VERSION);
	RESOURCE_SYM(resource_module_config, RESOURCE_MODULE_CONFIG);
	RESOURCE_SYM(resource_module_parse_config, RESOURCE_MODULE_PARSE_CONFIG);

	RESOURCE_SYM(resource_source_path, RESOURCE_SOURCE_PATH);
	RESOURCE_SYM(resource_source_set_path, RESOURCE_SOURCE_SET_PATH);
	RESOURCE_SYM(resource_source_hash, RESOURCE_SOURCE_HASH);
	RESOURCE_SYM(resource_source_allocate, RESOURCE_SOURCE_ALLOCATE);
	RESOURCE_SYM(resource_source_deallocate, RESOURCE_SOURCE_DEALLOCATE);
	RESOURCE_SYM(resource_source_initialize, RESOURCE_SOURCE_INITIALIZE);
	RESOURCE_SYM(resource_source_finalize, RESOURCE_SOURCE_FINALIZE);
	RESOURCE_SYM(resource_source_read, RESOURCE_SOURCE_READ);
	RESOURCE_SYM(resource_source_write, RESOURCE_SOURCE_WRITE);
	RESOURCE_SYM(resource_source_set, RESOURCE_SOURCE_SET);
	RESOURCE_SYM(resource_source_unset, RESOURCE_SOURCE_UNSET);
	RESOURCE_SYM(resource_source_get, RESOURCE_SOURCE_GET);
	RESOURCE_SYM(resource_source_set_blob, RESOURCE_SOURCE_SET_BLOB);
	RESOURCE_SYM(resource_source_read_blob, RESOURCE_SOURCE_READ_BLOB);
	RESOURCE_SYM(resource_source_write_blob, RESOURCE_SOURCE_WRITE_BLOB);
	RESOURCE_SYM(resource_source_collapse_history, RESOURCE_SOURCE_COLLAPSE_HISTORY);
	RESOURCE_SYM(resource_source_clear_blob_history, RESOURCE_SOURCE_CLEAR_BLOB_HISTORY);
	RESOURCE_SYM(resource_source_map, RESOURCE_SOURCE_MAP);
	RESOURCE_SYM(resource_source_map_all, RESOURCE_SOURCE_MAP_ALL);
	RESOURCE_SYM(resource_source_map_iterate, RESOURCE_SOURCE_MAP_ITERATE);
	RESOURCE_SYM(resource_source_map_reduce, RESOURCE_SOURCE_MAP_REDUCE);
	RESOURCE_SYM(resource_source_map_clear, RESOURCE_SOURCE_MAP_CLEAR);
	RESOURCE_SYM(resource_source_dependencies_count, RESOURCE_SOURCE_DEPENDENCIES_COUNT);
	RESOURCE_SYM(resource_source_dependencies, RESOURCE_SOURCE_DEPENDENCIES);
	RESOURCE_SYM(resource_source_set_dependencies, RESOURCE_SOURCE_SET_DEPENDENCIES);
	RESOURCE_SYM(resource_source_reverse_dependencies_count, RESOURCE_SOURCE_REVERSE_DEPENDENCIES_COUNT);
	RESOURCE_SYM(resource_source_reverse_dependencies, RESOURCE_SOURCE_REVERSE_DEPENDENCIES);
	RESOURCE_SYM(resource_source_add_reverse_dependency, RESOURCE_SOURCE_ADD_REVERSE_DEPENDENCY);
	RESOURCE_SYM(resource_source_remove_reverse_dependency, RESOURCE_SOURCE_REMOVE_REVERSE_DEPENDENCY);

	RESOURCE_SYM(resource_stream_open_static, RESOURCE_STREAM_OPEN_STATIC);
	RESOURCE_SYM(resource_stream_open_dynamic, RESOURCE_STREAM_OPEN_DYNAMIC);
	RESOURCE_SYM(resource_stream_make_path, RESOURCE_STREAM_MAKE_PATH);
	RESOURCE_SYM(resource_stream_write_header, RESOURCE_STREAM_WRITE_HEADER);
	RESOURCE_SYM(resource_stream_read_header, RESOURCE_STREAM_READ_HEADER);
}
