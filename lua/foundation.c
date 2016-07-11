/* foundation.c  -  Lua library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <lua/lua.h>

#include <foundation/foundation.h>

#if !BUILD_ENABLE_DEBUG_LOG

static void
_log_debugf_disabled(uint64_t context, const char* format, size_t format_size, ...) {
	FOUNDATION_UNUSED(context);
	FOUNDATION_UNUSED(format);
	FOUNDATION_UNUSED(format_size);
}

#endif

#if !BUILD_ENABLE_LOG

static log_handler_fn
_log_handler(void) {
	return (log_handler_fn)nullptr;
}

static void
_log_set_handler(log_handler_fn handler) {
	FOUNDATION_UNUSED(handler);
}

static void
_log_enable(bool enable) {
	FOUNDATION_UNUSED(enable);
}

static void
_log_set_suppress(hash_t context, error_level_t level) {
	FOUNDATION_UNUSED(context);
	FOUNDATION_UNUSED(level);
}

static error_level_t
_log_suppress(hash_t context) {
	FOUNDATION_UNUSED(context);
	return ERRORLEVEL_NONE;
}

static void
_log_suppress_clear(void) {
}

#endif

#if !BUILD_ENABLE_ERROR_CONTEXT

static void _error_context_push_disabled(const char* name, size_t name_length, const char* data,
                                         size_t data_length) {
	FOUNDATION_UNUSED(name);
	FOUNDATION_UNUSED(name_length);
	FOUNDATION_UNUSED(data);
	FOUNDATION_UNUSED(data_length);
}

static void _error_context_pop_disabled() {
}

static error_context_t* _error_context_disabled() {
	return 0;
}

static string_t
_error_context_buffer_disabled(char* str, size_t length) {
	return string_copy(str, length, 0, 0);
}

#endif

#if !BUILD_ENABLE_MEMORY_CONTEXT

#undef memory_context_push
#undef memory_context_pop
#undef memory_context

#endif

#if !BUILD_ENABLE_STATIC_HASH_DEBUG

#undef hash_to_string

#endif

static void*
_array_allocate_pointer(int size) {
	void** arr = 0;
	array_grow(arr, size);
	return arr;
}

static void*
_array_allocate_string(int size) {
	string_t* arr = 0;
	array_grow(arr, size);
	return arr;
}

static void
_array_deallocate(void** arr) {
	array_deallocate(arr);
}

static unsigned int
_array_size(const void* arr) {
	return (arr ? array_size(arr) : 0);
}

static const void*
_array_element_pointer(const void* arr, int pos) {
	return (arr && (pos >= 0) && (pos < (int)array_size(arr))) ?
	       ((const void* const*)arr)[pos] :
	       0;
}

static void
_array_set_element_pointer(const void** arr, int pos, void* ptr) {
	if (arr && (pos >= 0) && (pos < (int)array_size(arr)))
		arr[pos] = ptr;
}

static string_const_t
_array_element_string(const void* arr, int pos) {
	return (arr && (pos >= 0) && (pos < (int)array_size(arr))) ?
	       ((const string_const_t*)arr)[pos] :
	       string_empty();
}

static void
_array_set_element_string(void* arr, int pos, string_const_t str) {
	if (arr && (pos >= 0) && (pos < (int)array_size(arr))) {
		string_t* strarr = (string_t*)arr;
		string_deallocate(strarr[pos].str);
		strarr[pos] = string_clone_string(str);
	}
}

#if !FOUNDATION_PLATFORM_ANDROID
static stream_t*
asset_stream_open(const char* path, size_t path_length, unsigned int mode) {
	FOUNDATION_UNUSED(path);
	FOUNDATION_UNUSED(path_length);
	FOUNDATION_UNUSED(mode);
	return 0;
}

static void
thread_attach_jvm(void) {
}

static void
thread_detach_jvm(void) {
}

#endif

static int
system_size_size_t(void) {
	return sizeof(size_t);
}

static int
system_size_real(void) {
	return FOUNDATION_SIZE_REAL;
}

static int
system_size_pointer(void) {
	return FOUNDATION_SIZE_POINTER;
}

static int
system_size_wchar(void) {
	return FOUNDATION_SIZE_WCHAR;
}

#if !BUILD_ENABLE_PROFILE

static void
_profile_initialize(const char* str, size_t length, void* ptr, uint64_t val) {
	FOUNDATION_UNUSED(str);
	FOUNDATION_UNUSED(length);
	FOUNDATION_UNUSED(ptr);
	FOUNDATION_UNUSED(val);
}

static void
_profile_void(void) {
}

static void
_profile_bool(bool flag) {
	FOUNDATION_UNUSED(flag);
}

static void
_profile_fn(profile_write_fn fn) {
	FOUNDATION_UNUSED(fn);
}

static void
_profile_int(int val) {
	FOUNDATION_UNUSED(val);
}

static void
_profile_uint64(uint64_t val) {
	FOUNDATION_UNUSED(val);
}

static void
_profile_str(const char* str, size_t length) {
	FOUNDATION_UNUSED(str);
	FOUNDATION_UNUSED(length);
}

#endif

static void
_string_array_deallocate(string_t* array) {
	string_array_deallocate(array);
}

static uuid_t uuid_dns(void) {
	return UUID_DNS;
}

static size_t
type_size_t(unsigned int val) {
	return val;
}

static ssize_t
type_ssize_t(int val) {
	return val;
}

static bool _symbols_loaded;

void
lua_symbol_load_foundation(void) {
	hashmap_t* map = lua_symbol_lookup_map();

	if (_symbols_loaded)
		return;
	_symbols_loaded = true;

#define FOUNDATION_SYM(fn, name) hashmap_insert(map, HASH_SYM_##name, (void*)(uintptr_t)fn)

	FOUNDATION_SYM(_array_allocate_pointer, ARRAY_ALLOCATE_POINTER);
	FOUNDATION_SYM(_array_allocate_string, ARRAY_ALLOCATE_STRING);
	FOUNDATION_SYM(_array_size, ARRAY_SIZE);
	FOUNDATION_SYM(_array_element_pointer, ARRAY_ELEMENT_POINTER);
	FOUNDATION_SYM(_array_set_element_pointer, ARRAY_SET_ELEMENT_POINTER);
	FOUNDATION_SYM(_array_element_string, ARRAY_ELEMENT_STRING);
	FOUNDATION_SYM(_array_set_element_string, ARRAY_SET_ELEMENT_STRING);
	FOUNDATION_SYM(_array_deallocate, ARRAY_DEALLOCATE);

	FOUNDATION_SYM(assert_handler, ASSERT_HANDLER);
	FOUNDATION_SYM(assert_set_handler, ASSERT_SET_HANDLER);
	FOUNDATION_SYM(assert_report, ASSERT_REPORT);

	FOUNDATION_SYM(asset_stream_open, ASSET_STREAM_OPEN);

	FOUNDATION_SYM(base64_encode, BASE64_ENCODE);
	FOUNDATION_SYM(base64_decode, BASE64_DECODE);

	FOUNDATION_SYM(beacon_allocate, BEACON_ALLOCATE);
	FOUNDATION_SYM(beacon_initialize, BEACON_INITIALIZE);
	FOUNDATION_SYM(beacon_finalize, BEACON_FINALIZE);
	FOUNDATION_SYM(beacon_deallocate, BEACON_DEALLOCATE);
	FOUNDATION_SYM(beacon_wait, BEACON_WAIT);
	FOUNDATION_SYM(beacon_try_wait, BEACON_TRY_WAIT);
	FOUNDATION_SYM(beacon_fire, BEACON_FIRE);

	FOUNDATION_SYM(bitbuffer_allocate_buffer, BITBUFFER_ALLOCATE_BUFFER);
	FOUNDATION_SYM(bitbuffer_allocate_stream, BITBUFFER_ALLOCATE_STREAM);
	FOUNDATION_SYM(bitbuffer_deallocate, BITBUFFER_DEALLOCATE);
	FOUNDATION_SYM(bitbuffer_initialize_buffer, BITBUFFER_INITIALIZE_BUFFER);
	FOUNDATION_SYM(bitbuffer_initialize_stream, BITBUFFER_INITIALIZE_STREAM);
	FOUNDATION_SYM(bitbuffer_finalize, BITBUFFER_FINALIZE);
	FOUNDATION_SYM(bitbuffer_read32, BITBUFFER_READ32);
	FOUNDATION_SYM(bitbuffer_read64, BITBUFFER_READ64);
	FOUNDATION_SYM(bitbuffer_read128, BITBUFFER_READ128);
	FOUNDATION_SYM(bitbuffer_read_float32, BITBUFFER_READ_FLOAT32);
	FOUNDATION_SYM(bitbuffer_read_float64, BITBUFFER_READ_FLOAT64);
	FOUNDATION_SYM(bitbuffer_write32, BITBUFFER_WRITE32);
	FOUNDATION_SYM(bitbuffer_write64, BITBUFFER_WRITE64);
	FOUNDATION_SYM(bitbuffer_write128, BITBUFFER_WRITE128);
	FOUNDATION_SYM(bitbuffer_write_float32, BITBUFFER_WRITE_FLOAT32);
	FOUNDATION_SYM(bitbuffer_write_float64, BITBUFFER_WRITE_FLOAT64);
	FOUNDATION_SYM(bitbuffer_align_read, BITBUFFER_ALIGN_READ);
	FOUNDATION_SYM(bitbuffer_align_write, BITBUFFER_ALIGN_WRITE);
	FOUNDATION_SYM(bitbuffer_discard_read, BITBUFFER_DISCARD_READ);
	FOUNDATION_SYM(bitbuffer_discard_write, BITBUFFER_DISCARD_WRITE);

	FOUNDATION_SYM(byteorder_swap16, BYTEORDER_SWAP16);
	FOUNDATION_SYM(byteorder_swap32, BYTEORDER_SWAP32);
	FOUNDATION_SYM(byteorder_swap64, BYTEORDER_SWAP64);
	FOUNDATION_SYM(byteorder_swap, BYTEORDER_SWAP);
	FOUNDATION_SYM(byteorder_bigendian16, BYTEORDER_BIGENDIAN16);
	FOUNDATION_SYM(byteorder_bigendian32, BYTEORDER_BIGENDIAN32);
	FOUNDATION_SYM(byteorder_bigendian64, BYTEORDER_BIGENDIAN64);
	FOUNDATION_SYM(byteorder_bigendian, BYTEORDER_BIGENDIAN);
	FOUNDATION_SYM(byteorder_littleendian16, BYTEORDER_LITTLEENDIAN16);
	FOUNDATION_SYM(byteorder_littleendian32, BYTEORDER_LITTLEENDIAN32);
	FOUNDATION_SYM(byteorder_littleendian64, BYTEORDER_LITTLEENDIAN64);
	FOUNDATION_SYM(byteorder_littleendian, BYTEORDER_LITTLEENDIAN);

	FOUNDATION_SYM(blowfish_allocate, BLOWFISH_ALLOCATE);
	FOUNDATION_SYM(blowfish_deallocate, BLOWFISH_DEALLOCATE);
	FOUNDATION_SYM(blowfish_initialize, BLOWFISH_INITIALIZE);
	FOUNDATION_SYM(blowfish_finalize, BLOWFISH_FINALIZE);
	FOUNDATION_SYM(blowfish_encrypt, BLOWFISH_ENCRYPT);
	FOUNDATION_SYM(blowfish_decrypt, BLOWFISH_DECRYPT);

	FOUNDATION_SYM(buffer_stream_allocate, BUFFER_STREAM_ALLOCATE);
	FOUNDATION_SYM(buffer_stream_initialize, BUFFER_STREAM_INITIALIZE);

	FOUNDATION_SYM(environment_command_line, ENVIRONMENT_COMMAND_LINE);
	FOUNDATION_SYM(environment_executable_name, ENVIRONMENT_EXECUTABLE_NAME);
	FOUNDATION_SYM(environment_executable_directory, ENVIRONMENT_EXECUTABLE_DIRECTORY);
	FOUNDATION_SYM(environment_executable_path, ENVIRONMENT_EXECUTABLE_PATH);
	FOUNDATION_SYM(environment_initial_working_directory, ENVIRONMENT_INITIAL_WORKING_DIRECTORY);
	FOUNDATION_SYM(environment_current_working_directory, ENVIRONMENT_CURRENT_WORKING_DIRECTORY);
	FOUNDATION_SYM(environment_set_current_working_directory,
	               ENVIRONMENT_SET_CURRENT_WORKING_DIRECTORY);
	FOUNDATION_SYM(environment_application_directory, ENVIRONMENT_APPLICATION_DIRECTORY);
	FOUNDATION_SYM(environment_temporary_directory, ENVIRONMENT_TEMPORARY_DIRECTORY);
	FOUNDATION_SYM(environment_variable, ENVIRONMENT_VARIABLE);

	FOUNDATION_SYM(error, ERROR);
	FOUNDATION_SYM(error_report, ERROR_REPORT);
	FOUNDATION_SYM(error_handler, ERROR_HANDLER);
	FOUNDATION_SYM(error_set_handler, ERROR_SET_HANDLER);
#if BUILD_ENABLE_ERROR_CONTEXT
	FOUNDATION_SYM(_error_context_push, ERROR_CONTEXT_PUSH);
	FOUNDATION_SYM(_error_context_pop, ERROR_CONTEXT_POP);
	FOUNDATION_SYM(_error_context_clear, ERROR_CONTEXT_CLEAR);
	FOUNDATION_SYM(_error_context_buffer, ERROR_CONTEXT_BUFFER);
	FOUNDATION_SYM(_error_context, ERROR_CONTEXT);
#else
	FOUNDATION_SYM(_error_context_push_disabled, ERROR_CONTEXT_PUSH);
	FOUNDATION_SYM(_error_context_pop_disabled, ERROR_CONTEXT_POP);
	FOUNDATION_SYM(_error_context_pop_disabled, ERROR_CONTEXT_CLEAR);
	FOUNDATION_SYM(_error_context_buffer_disabled, ERROR_CONTEXT_BUFFER);
	FOUNDATION_SYM(_error_context_disabled, ERROR_CONTEXT);
#endif

	FOUNDATION_SYM(event_post_varg, EVENT_POST);
	FOUNDATION_SYM(event_next, EVENT_NEXT);
	FOUNDATION_SYM(event_payload_size, EVENT_PAYLOAD_SIZE);
	FOUNDATION_SYM(event_stream_allocate, EVENT_STREAM_ALLOCATE);
	FOUNDATION_SYM(event_stream_deallocate, EVENT_STREAM_DEALLOCATE);
	FOUNDATION_SYM(event_stream_initialize, EVENT_STREAM_INITIALIZE);
	FOUNDATION_SYM(event_stream_finalize, EVENT_STREAM_FINALIZE);
	FOUNDATION_SYM(event_stream_process, EVENT_STREAM_PROCESS);
	FOUNDATION_SYM(event_stream_set_beacon, EVENT_STREAM_SET_BEACON);

	FOUNDATION_SYM(fs_open_file, FS_OPEN_FILE);
	FOUNDATION_SYM(fs_copy_file, FS_COPY_FILE);
	FOUNDATION_SYM(fs_remove_file, FS_REMOVE_FILE);
	FOUNDATION_SYM(fs_is_file, FS_IS_FILE);
	FOUNDATION_SYM(fs_make_directory, FS_MAKE_DIRECTORY);
	FOUNDATION_SYM(fs_remove_directory, FS_REMOVE_DIRECTORY);
	FOUNDATION_SYM(fs_is_directory, FS_IS_DIRECTORY);
	FOUNDATION_SYM(fs_last_modified, FS_LAST_MODIFIED);
	FOUNDATION_SYM(fs_touch, FS_TOUCH);
	FOUNDATION_SYM(fs_md5, FS_MD5);
	FOUNDATION_SYM(fs_matching_files, FS_MATCHING_FILES);
	FOUNDATION_SYM(fs_matching_files_regex, FS_MATCHING_FILES_REGEX);
	FOUNDATION_SYM(fs_files, FS_FILES);
	FOUNDATION_SYM(fs_subdirs, FS_SUBDIRS);
	FOUNDATION_SYM(fs_monitor, FS_MONITOR);
	FOUNDATION_SYM(fs_unmonitor, FS_UNMONITOR);
	FOUNDATION_SYM(fs_temporary_file, FS_TEMPORARY_FILE);
	FOUNDATION_SYM(fs_event_post, FS_EVENT_POST);
	FOUNDATION_SYM(fs_event_path, FS_EVENT_PATH);
	FOUNDATION_SYM(fs_event_stream, FS_EVENT_STREAM);

	FOUNDATION_SYM(foundation_version, FOUNDATION_VERSION);

	FOUNDATION_SYM(hash, HASH);
	FOUNDATION_SYM(hash_to_string, HASH_TO_STRING);

	FOUNDATION_SYM(hashmap_allocate, HASHMAP_ALLOCATE);
	FOUNDATION_SYM(hashmap_deallocate, HASHMAP_DEALLOCATE);
	FOUNDATION_SYM(hashmap_initialize, HASHMAP_INITIALIZE);
	FOUNDATION_SYM(hashmap_finalize, HASHMAP_FINALIZE);
	FOUNDATION_SYM(hashmap_insert, HASHMAP_INSERT);
	FOUNDATION_SYM(hashmap_erase, HASHMAP_ERASE);
	FOUNDATION_SYM(hashmap_lookup, HASHMAP_LOOKUP);
	FOUNDATION_SYM(hashmap_has_key, HASHMAP_HAS_KEY);
	FOUNDATION_SYM(hashmap_size, HASHMAP_SIZE);
	FOUNDATION_SYM(hashmap_clear, HASHMAP_CLEAR);

#if FOUNDATION_SIZE_POINTER == 4
	FOUNDATION_SYM(hashtable32_allocate, HASHTABLE_ALLOCATE);
	FOUNDATION_SYM(hashtable32_deallocate, HASHTABLE_DEALLOCATE);
	FOUNDATION_SYM(hashtable32_initialize, HASHTABLE_INITIALIZE);
	FOUNDATION_SYM(hashtable32_finalize, HASHTABLE_FINALIZE);
	FOUNDATION_SYM(hashtable32_set, HASHTABLE_SET);
	FOUNDATION_SYM(hashtable32_erase, HASHTABLE_ERASE);
	FOUNDATION_SYM(hashtable32_get, HASHTABLE_GET);
	FOUNDATION_SYM(hashtable32_size, HASHTABLE_SIZE);
	FOUNDATION_SYM(hashtable32_clear, HASHTABLE_CLEAR);
#else
	FOUNDATION_SYM(hashtable64_allocate, HASHTABLE_ALLOCATE);
	FOUNDATION_SYM(hashtable64_deallocate, HASHTABLE_DEALLOCATE);
	FOUNDATION_SYM(hashtable64_initialize, HASHTABLE_INITIALIZE);
	FOUNDATION_SYM(hashtable64_finalize, HASHTABLE_FINALIZE);
	FOUNDATION_SYM(hashtable64_set, HASHTABLE_SET);
	FOUNDATION_SYM(hashtable64_erase, HASHTABLE_ERASE);
	FOUNDATION_SYM(hashtable64_get, HASHTABLE_GET);
	FOUNDATION_SYM(hashtable64_size, HASHTABLE_SIZE);
	FOUNDATION_SYM(hashtable64_clear, HASHTABLE_CLEAR);
#endif
	FOUNDATION_SYM(hashtable32_allocate, HASHTABLE32_ALLOCATE);
	FOUNDATION_SYM(hashtable32_deallocate, HASHTABLE32_DEALLOCATE);
	FOUNDATION_SYM(hashtable32_initialize, HASHTABLE32_INITIALIZE);
	FOUNDATION_SYM(hashtable32_finalize, HASHTABLE32_FINALIZE);
	FOUNDATION_SYM(hashtable32_set, HASHTABLE32_SET);
	FOUNDATION_SYM(hashtable32_erase, HASHTABLE32_ERASE);
	FOUNDATION_SYM(hashtable32_get, HASHTABLE32_GET);
	FOUNDATION_SYM(hashtable32_size, HASHTABLE32_CLEAR);
	FOUNDATION_SYM(hashtable32_clear, HASHTABLE32_SIZE);
	FOUNDATION_SYM(hashtable64_allocate, HASHTABLE64_ALLOCATE);
	FOUNDATION_SYM(hashtable64_deallocate, HASHTABLE64_DEALLOCATE);
	FOUNDATION_SYM(hashtable64_initialize, HASHTABLE64_INITIALIZE);
	FOUNDATION_SYM(hashtable64_finalize, HASHTABLE64_FINALIZE);
	FOUNDATION_SYM(hashtable64_set, HASHTABLE64_SET);
	FOUNDATION_SYM(hashtable64_erase, HASHTABLE64_ERASE);
	FOUNDATION_SYM(hashtable64_get, HASHTABLE64_GET);
	FOUNDATION_SYM(hashtable64_size, HASHTABLE64_SIZE);
	FOUNDATION_SYM(hashtable64_clear, HASHTABLE64_CLEAR);

	FOUNDATION_SYM(json_parse, JSON_PARSE);
	FOUNDATION_SYM(sjson_parse, SJSON_PARSE);
	FOUNDATION_SYM(json_token_identifier, JSON_TOKEN_IDENTIFIER);
	FOUNDATION_SYM(json_token_value, JSON_TOKEN_VALUE);
	FOUNDATION_SYM(json_unescape, JSON_UNESCAPE);
	FOUNDATION_SYM(json_escape, JSON_ESCAPE);

	FOUNDATION_SYM(library_load, LIBRARY_LOAD);
	FOUNDATION_SYM(library_ref, LIBRARY_REF);
	FOUNDATION_SYM(library_unload, LIBRARY_UNLOAD);
	FOUNDATION_SYM(library_symbol, LIBRARY_SYMBOL);
	FOUNDATION_SYM(library_name, LIBRARY_NAME);
	FOUNDATION_SYM(library_valid, LIBRARY_VALID);

#if BUILD_ENABLE_DEBUG_LOG
	FOUNDATION_SYM(log_debugf, LOG_DEBUGF);
#else
	FOUNDATION_SYM(_log_debugf_disabled, LOG_DEBUGF);
#endif
#if BUILD_ENABLE_LOG
	FOUNDATION_SYM(log_infof, LOG_INFOF);
	FOUNDATION_SYM(log_warnf, LOG_WARNF);
	FOUNDATION_SYM(log_errorf, LOG_ERRORF);
	FOUNDATION_SYM(log_panicf, LOG_PANICF);
	FOUNDATION_SYM(log_enable_prefix, LOG_ENABLE_PREFIX);
	FOUNDATION_SYM(log_enable_stdout, LOG_ENABLE_STDOUT);
	FOUNDATION_SYM(log_handler, LOG_HANDLER);
	FOUNDATION_SYM(log_set_handler, LOG_SET_HANDLER);
	FOUNDATION_SYM(log_set_suppress, LOG_SET_SUPPRESS);
	FOUNDATION_SYM(log_suppress, LOG_SUPPRESS);
	FOUNDATION_SYM(log_suppress_clear, LOG_SUPPRESS_CLEAR);
#else
	FOUNDATION_SYM(_log_debugf_disabled, LOG_INFOF);
	FOUNDATION_SYM(_log_debugf_disabled, LOG_WARNF);
	FOUNDATION_SYM(_log_debugf_disabled, LOG_ERRORF);
	FOUNDATION_SYM(_log_debugf_disabled, LOG_PANICF);
	FOUNDATION_SYM(_log_enable, LOG_ENABLE_PREFIX);
	FOUNDATION_SYM(_log_enable, LOG_ENABLE_STDOUT);
	FOUNDATION_SYM(_log_handler, LOG_HANDLER);
	FOUNDATION_SYM(_log_set_handler, LOG_SET_HANDLER);
	FOUNDATION_SYM(_log_set_suppress, LOG_SET_SUPPRESS);
	FOUNDATION_SYM(_log_suppress, LOG_SUPPRESS);
	FOUNDATION_SYM(_log_suppress_clear, LOG_SUPPRESS_CLEAR);
#endif

	FOUNDATION_SYM(md5_allocate, MD5_ALLOCATE);
	FOUNDATION_SYM(md5_deallocate, MD5_DEALLOCATE);
	FOUNDATION_SYM(md5_initialize, MD5_INITIALIZE);
	FOUNDATION_SYM(md5_finalize, MD5_FINALIZE);
	FOUNDATION_SYM(md5_digest, MD5_DIGEST);
	FOUNDATION_SYM(md5_digest_finalize, MD5_DIGEST_FINALIZE);
	FOUNDATION_SYM(md5_get_digest, MD5_GET_DIGEST);
	FOUNDATION_SYM(md5_get_digest_raw, MD5_GET_DIGEST_RAW);

	FOUNDATION_SYM(memory_allocate, MEMORY_ALLOCATE);
	FOUNDATION_SYM(memory_reallocate, MEMORY_REALLOCATE);
	FOUNDATION_SYM(memory_deallocate, MEMORY_DEALLOCATE);
	FOUNDATION_SYM(memory_context_push, MEMORY_CONTEXT_PUSH);
	FOUNDATION_SYM(memory_context_pop, MEMORY_CONTEXT_POP);
	FOUNDATION_SYM(memory_context, MEMORY_CONTEXT);

	FOUNDATION_SYM(mutex_allocate, MUTEX_ALLOCATE);
	FOUNDATION_SYM(mutex_deallocate, MUTEX_DEALLOCATE);
	FOUNDATION_SYM(mutex_name, MUTEX_NAME);
	FOUNDATION_SYM(mutex_try_lock, MUTEX_TRY_LOCK);
	FOUNDATION_SYM(mutex_lock, MUTEX_LOCK);
	FOUNDATION_SYM(mutex_unlock, MUTEX_UNLOCK);
	FOUNDATION_SYM(mutex_wait, MUTEX_WAIT);
	FOUNDATION_SYM(mutex_signal, MUTEX_SIGNAL);

	FOUNDATION_SYM(objectmap_allocate, OBJECTMAP_ALLOCATE);
	FOUNDATION_SYM(objectmap_deallocate, OBJECTMAP_DEALLOCATE);
	FOUNDATION_SYM(objectmap_initialize, OBJECTMAP_INITIALIZE);
	FOUNDATION_SYM(objectmap_finalize, OBJECTMAP_FINALIZE);
	FOUNDATION_SYM(objectmap_size, OBJECTMAP_SIZE);
	FOUNDATION_SYM(objectmap_reserve, OBJECTMAP_RESERVE);
	FOUNDATION_SYM(objectmap_free, OBJECTMAP_FREE);
	FOUNDATION_SYM(objectmap_set, OBJECTMAP_SET);
	FOUNDATION_SYM(objectmap_raw_lookup, OBJECTMAP_RAW_LOOKUP);
	FOUNDATION_SYM(objectmap_lookup_ref, OBJECTMAP_LOOKUP_REF);
	FOUNDATION_SYM(objectmap_lookup_unref, OBJECTMAP_LOOKUP_UNREF);
	FOUNDATION_SYM(objectmap_lookup, OBJECTMAP_LOOKUP);

	FOUNDATION_SYM(path_base_file_name, PATH_BASE_FILE_NAME);
	FOUNDATION_SYM(path_base_file_name_with_directory, PATH_BASE_FILE_NAME_WITH_DIRECTORY);
	FOUNDATION_SYM(path_file_extension, PATH_FILE_EXTENSION);
	FOUNDATION_SYM(path_file_name, PATH_FILE_NAME);
	FOUNDATION_SYM(path_directory_name, PATH_DIRECTORY_NAME);
	FOUNDATION_SYM(path_subpath, PATH_SUBPATH);
	FOUNDATION_SYM(path_protocol, PATH_PROTOCOL);
	FOUNDATION_SYM(path_strip_protocol, PATH_STRIP_PROTOCOL);
	FOUNDATION_SYM(path_allocate_concat_varg, PATH_ALLOCATE_CONCAT);
	FOUNDATION_SYM(path_concat_varg, PATH_CONCAT);
	FOUNDATION_SYM(path_append_varg, PATH_APPEND);
	FOUNDATION_SYM(path_prepend_varg, PATH_PREPEND);
	FOUNDATION_SYM(path_absolute, PATH_ABSOLUTE);
	FOUNDATION_SYM(path_allocate_absolute, PATH_ALLOCATE_ABSOLUTE);
	FOUNDATION_SYM(path_clean, PATH_CLEAN);
	FOUNDATION_SYM(path_is_absolute, PATH_IS_ABSOLUTE);
	FOUNDATION_SYM(path_make_temporary, PATH_MAKE_TEMPORARY);

	FOUNDATION_SYM(pipe_allocate, PIPE_ALLOCATE);
	FOUNDATION_SYM(pipe_initialize, PIPE_INITIALIZE);
	FOUNDATION_SYM(pipe_close_read, PIPE_CLOSE_READ);
	FOUNDATION_SYM(pipe_close_write, PIPE_CLOSE_WRITE);

	FOUNDATION_SYM(process_allocate, PROCESS_ALLOCATE);
	FOUNDATION_SYM(process_deallocate, PROCESS_DEALLOCATE);
	FOUNDATION_SYM(process_initialize, PROCESS_INITIALIZE);
	FOUNDATION_SYM(process_finalize, PROCESS_FINALIZE);
	FOUNDATION_SYM(process_set_working_directory, PROCESS_SET_WORKING_DIRECTORY);
	FOUNDATION_SYM(process_set_executable_path, PROCESS_SET_EXECUTABLE_PATH);
	FOUNDATION_SYM(process_set_arguments, PROCESS_SET_ARGUMENTS);
	FOUNDATION_SYM(process_set_flags, PROCESS_SET_FLAGS);
	FOUNDATION_SYM(process_set_verb, PROCESS_SET_VERB);
	FOUNDATION_SYM(process_set_exit_code, PROCESS_SET_EXIT_CODE);
	FOUNDATION_SYM(process_spawn, PROCESS_SPAWN);
	FOUNDATION_SYM(process_stdout, PROCESS_STDOUT);
	FOUNDATION_SYM(process_stdin, PROCESS_STDIN);
	FOUNDATION_SYM(process_wait, PROCESS_WAIT);
	FOUNDATION_SYM(process_exit_code, PROCESS_EXIT_CODE);
	FOUNDATION_SYM(process_exit, PROCESS_EXIT);

#if BUILD_ENABLE_PROFILE
	FOUNDATION_SYM(profile_initialize, PROFILE_INITIALIZE);
	FOUNDATION_SYM(profile_finalize, PROFILE_FINALIZE);
	FOUNDATION_SYM(profile_enable, PROFILE_ENABLE);
	FOUNDATION_SYM(profile_set_output, PROFILE_SET_OUTPUT);
	FOUNDATION_SYM(profile_set_output_wait, PROFILE_SET_OUTPUT_WAIT);
	FOUNDATION_SYM(profile_end_frame, PROFILE_END_FRAME);
	FOUNDATION_SYM(profile_begin_block, PROFILE_BEGIN_BLOCK);
	FOUNDATION_SYM(profile_update_block, PROFILE_UPDATE_BLOCK);
	FOUNDATION_SYM(profile_end_block, PROFILE_END_BLOCK);
	FOUNDATION_SYM(profile_log, PROFILE_LOG);
	FOUNDATION_SYM(profile_trylock, PROFILE_TRYLOCK);
	FOUNDATION_SYM(profile_lock, PROFILE_LOCK);
	FOUNDATION_SYM(profile_unlock, PROFILE_UNLOCK);
	FOUNDATION_SYM(profile_wait, PROFILE_WAIT);
	FOUNDATION_SYM(profile_signal, PROFILE_SIGNAL);
#else
	FOUNDATION_SYM(_profile_initialize, PROFILE_INITIALIZE);
	FOUNDATION_SYM(_profile_void, PROFILE_FINALIZE);
	FOUNDATION_SYM(_profile_bool, PROFILE_ENABLE);
	FOUNDATION_SYM(_profile_fn, PROFILE_SET_OUTPUT);
	FOUNDATION_SYM(_profile_int, PROFILE_SET_OUTPUT_WAIT);
	FOUNDATION_SYM(_profile_uint64, PROFILE_END_FRAME);
	FOUNDATION_SYM(_profile_str, PROFILE_BEGIN_BLOCK);
	FOUNDATION_SYM(_profile_void, PROFILE_UPDATE_BLOCK);
	FOUNDATION_SYM(_profile_void, PROFILE_END_BLOCK);
	FOUNDATION_SYM(_profile_str, PROFILE_LOG);
	FOUNDATION_SYM(_profile_str, PROFILE_TRYLOCK);
	FOUNDATION_SYM(_profile_str, PROFILE_LOCK);
	FOUNDATION_SYM(_profile_str, PROFILE_UNLOCK);
	FOUNDATION_SYM(_profile_str, PROFILE_WAIT);
	FOUNDATION_SYM(_profile_str, PROFILE_SIGNAL);
#endif

	FOUNDATION_SYM(radixsort_allocate, RADIXSORT_ALLOCATE);
	FOUNDATION_SYM(radixsort_deallocate, RADIXSORT_DEALLOCATE);
	FOUNDATION_SYM(radixsort_initialize, RADIXSORT_INITIALIZE);
	FOUNDATION_SYM(radixsort_finalize, RADIXSORT_FINALIZE);
	FOUNDATION_SYM(radixsort_sort, RADIXSORT_SORT);

	FOUNDATION_SYM(random32, RANDOM32);
	FOUNDATION_SYM(random32_range, RANDOM32_RANGE);
	FOUNDATION_SYM(random64, RANDOM64);
	FOUNDATION_SYM(random64_range, RANDOM64_RANGE);
	FOUNDATION_SYM(random_normalized, RANDOM_NORMALIZED);
	FOUNDATION_SYM(random_range, RANDOM_RANGE);
	FOUNDATION_SYM(random32_gaussian_range, RANDOM32_GAUSSIAN_RANGE);
	FOUNDATION_SYM(random_gaussian_range, RANDOM_GAUSSIAN_RANGE);
	FOUNDATION_SYM(random32_triangle_range, RANDOM32_TRIANGLE_RANGE);
	FOUNDATION_SYM(random_triangle_range, RANDOM_TRIANGLE_RANGE);
	FOUNDATION_SYM(random32_weighted, RANDOM32_WEIGHTED);

	FOUNDATION_SYM(regex_compile, REGEX_COMPILE);
	FOUNDATION_SYM(regex_match, REGEX_MATCH);
	FOUNDATION_SYM(regex_deallocate, REGEX_DEALLOCATE);

	FOUNDATION_SYM(ringbuffer_allocate, RINGBUFFER_ALLOCATE);
	FOUNDATION_SYM(ringbuffer_deallocate, RINGBUFFER_DEALLOCATE);
	FOUNDATION_SYM(ringbuffer_initialize, RINGBUFFER_INITIALIZE);
	FOUNDATION_SYM(ringbuffer_finalize, RINGBUFFER_FINALIZE);
	FOUNDATION_SYM(ringbuffer_size, RINGBUFFER_SIZE);
	FOUNDATION_SYM(ringbuffer_reset, RINGBUFFER_RESET);
	FOUNDATION_SYM(ringbuffer_read, RINGBUFFER_READ);
	FOUNDATION_SYM(ringbuffer_write, RINGBUFFER_WRITE);
	FOUNDATION_SYM(ringbuffer_total_read, RINGBUFFER_TOTAL_READ);
	FOUNDATION_SYM(ringbuffer_total_written, RINGBUFFER_TOTAL_WRITTEN);
	FOUNDATION_SYM(ringbuffer_stream_allocate, RINGBUFFER_STREAM_ALLOCATE);
	FOUNDATION_SYM(ringbuffer_stream_initialize, RINGBUFFER_STREAM_INITIALIZE);

	FOUNDATION_SYM(sha256_allocate, SHA256_ALLOCATE);
	FOUNDATION_SYM(sha256_deallocate, SHA256_DEALLOCATE);
	FOUNDATION_SYM(sha256_digest, SHA256_DIGEST);
	FOUNDATION_SYM(sha256_digest_finalize, SHA256_DIGEST_FINALIZE);
	FOUNDATION_SYM(sha256_get_digest, SHA256_GET_DIGEST);
	FOUNDATION_SYM(sha256_get_digest_raw, SHA256_GET_DIGEST_RAW);

	FOUNDATION_SYM(sha512_allocate, SHA512_ALLOCATE);
	FOUNDATION_SYM(sha512_deallocate, SHA512_DEALLOCATE);
	FOUNDATION_SYM(sha512_digest, SHA512_DIGEST);
	FOUNDATION_SYM(sha512_digest_finalize, SHA512_DIGEST_FINALIZE);
	FOUNDATION_SYM(sha512_get_digest, SHA512_GET_DIGEST);
	FOUNDATION_SYM(sha512_get_digest_raw, SHA512_GET_DIGEST_RAW);

	FOUNDATION_SYM(semaphore_initialize, SEMAPHORE_INITIALIZE);
	FOUNDATION_SYM(semaphore_initialize_named, SEMAPHORE_INITIALIZE_NAMED);
	FOUNDATION_SYM(semaphore_finalize, SEMAPHORE_FINALIZE);
	FOUNDATION_SYM(semaphore_wait, SEMAPHORE_WAIT);
	FOUNDATION_SYM(semaphore_try_wait, SEMAPHORE_TRY_WAIT);
	FOUNDATION_SYM(semaphore_post, SEMAPHORE_POST);

	FOUNDATION_SYM(stacktrace_capture, STACKTRACE_CAPTURE);
	FOUNDATION_SYM(stacktrace_resolve, STACKTRACE_RESOLVE);

	FOUNDATION_SYM(stream_open, STREAM_OPEN);
	FOUNDATION_SYM(stream_clone, STREAM_CLONE);
	FOUNDATION_SYM(stream_deallocate, STREAM_DEALLOCATE);
	FOUNDATION_SYM(stream_initialize, STREAM_INITIALIZE);
	FOUNDATION_SYM(stream_finalize, STREAM_FINALIZE);
	FOUNDATION_SYM(stream_tell, STREAM_TELL);
	FOUNDATION_SYM(stream_seek, STREAM_SEEK);
	FOUNDATION_SYM(stream_eos, STREAM_EOS);
	FOUNDATION_SYM(stream_size, STREAM_SIZE);
	FOUNDATION_SYM(stream_set_byteorder, STREAM_SET_BYTEORDER);
	FOUNDATION_SYM(stream_set_binary, STREAM_SET_BINARY);
	FOUNDATION_SYM(stream_determine_binary_mode, STREAM_DETERMINE_BINARY_MODE);
	FOUNDATION_SYM(stream_is_binary, STREAM_IS_BINARY);
	FOUNDATION_SYM(stream_is_sequential, STREAM_IS_SEQUENTIAL);
	FOUNDATION_SYM(stream_is_reliable, STREAM_IS_RELIABLE);
	FOUNDATION_SYM(stream_is_inorder, STREAM_IS_INORDER);
	FOUNDATION_SYM(stream_is_swapped, STREAM_IS_SWAPPED);
	FOUNDATION_SYM(stream_byteorder, STREAM_BYTEORDER);
	FOUNDATION_SYM(stream_path, STREAM_PATH);
	FOUNDATION_SYM(stream_last_modified, STREAM_LAST_MODIFIED);
	FOUNDATION_SYM(stream_read, STREAM_READ);
	FOUNDATION_SYM(stream_read_line_buffer, STREAM_READ_LINE_BUFFER);
	FOUNDATION_SYM(stream_read_line, STREAM_READ_LINE);
	FOUNDATION_SYM(stream_read_bool, STREAM_READ_BOOL);
	FOUNDATION_SYM(stream_read_int8, STREAM_READ_INT8);
	FOUNDATION_SYM(stream_read_uint8, STREAM_READ_UINT8);
	FOUNDATION_SYM(stream_read_int16, STREAM_READ_INT16);
	FOUNDATION_SYM(stream_read_uint16, STREAM_READ_UINT16);
	FOUNDATION_SYM(stream_read_int32, STREAM_READ_INT32);
	FOUNDATION_SYM(stream_read_uint32, STREAM_READ_UINT32);
	FOUNDATION_SYM(stream_read_int64, STREAM_READ_INT64);
	FOUNDATION_SYM(stream_read_uint64, STREAM_READ_UINT64);
	FOUNDATION_SYM(stream_read_float32, STREAM_READ_FLOAT32);
	FOUNDATION_SYM(stream_read_float64, STREAM_READ_FLOAT64);
	FOUNDATION_SYM(stream_read_string, STREAM_READ_STRING);
	FOUNDATION_SYM(stream_read_string_buffer, STREAM_READ_STRING_BUFFER);
	FOUNDATION_SYM(stream_write, STREAM_WRITE);
	FOUNDATION_SYM(stream_write_bool, STREAM_WRITE_BOOL);
	FOUNDATION_SYM(stream_write_int8, STREAM_WRITE_INT8);
	FOUNDATION_SYM(stream_write_uint8, STREAM_WRITE_UINT8);
	FOUNDATION_SYM(stream_write_int16, STREAM_WRITE_INT16);
	FOUNDATION_SYM(stream_write_uint16, STREAM_WRITE_UINT16);
	FOUNDATION_SYM(stream_write_int32, STREAM_WRITE_INT32);
	FOUNDATION_SYM(stream_write_uint32, STREAM_WRITE_UINT32);
	FOUNDATION_SYM(stream_write_int64, STREAM_WRITE_INT64);
	FOUNDATION_SYM(stream_write_uint64, STREAM_WRITE_UINT64);
	FOUNDATION_SYM(stream_write_float32, STREAM_WRITE_FLOAT32);
	FOUNDATION_SYM(stream_write_float64, STREAM_WRITE_FLOAT64);
	FOUNDATION_SYM(stream_write_string, STREAM_WRITE_STRING);
	FOUNDATION_SYM(stream_write_endl, STREAM_WRITE_ENDL);
	FOUNDATION_SYM(stream_write_format, STREAM_WRITE_FORMAT);
	FOUNDATION_SYM(stream_buffer_read, STREAM_BUFFER_READ);
	FOUNDATION_SYM(stream_available_read, STREAM_AVAILABLE_READ);
	FOUNDATION_SYM(stream_md5, STREAM_MD5);
	FOUNDATION_SYM(stream_truncate, STREAM_TRUNCATE);
	FOUNDATION_SYM(stream_flush, STREAM_FLUSH);
	FOUNDATION_SYM(stream_open_stdout, STREAM_OPEN_STDOUT);
	FOUNDATION_SYM(stream_open_stderr, STREAM_OPEN_STDERR);
	FOUNDATION_SYM(stream_open_stdin, STREAM_OPEN_STDIN);
	FOUNDATION_SYM(stream_set_protocol_handler, STREAM_SET_PROTOCOL_HANDLER);
	FOUNDATION_SYM(stream_protocol_handler, STREAM_PROTOCOL_HANDLER);

	FOUNDATION_SYM(string_allocate, STRING_ALLOCATE);
	FOUNDATION_SYM(string_deallocate, STRING_DEALLOCATE);
	FOUNDATION_SYM(string_clone, STRING_CLONE);
	FOUNDATION_SYM(string_clone_string, STRING_CLONE_STRING);
	FOUNDATION_SYM(string_null, STRING_NULL);
	FOUNDATION_SYM(string_empty, STRING_EMPTY);
	FOUNDATION_SYM(string, STRING);
	FOUNDATION_SYM(string_const, STRING_CONST);
	FOUNDATION_SYM(string_to_const, STRING_TO_CONST);
	FOUNDATION_SYM(string_allocate_format, STRING_ALLOCATE_FORMAT);
	FOUNDATION_SYM(string_format, STRING_FORMAT);
	FOUNDATION_SYM(string_length, STRING_LENGTH);
	FOUNDATION_SYM(string_glyphs, STRING_GLYPHS);
	FOUNDATION_SYM(string_hash, STRING_HASH);
	FOUNDATION_SYM(string_copy, STRING_COPY);
	FOUNDATION_SYM(string_resize, STRING_RESIZE);
	FOUNDATION_SYM(string_replace, STRING_REPLACE);
	FOUNDATION_SYM(string_allocate_concat_varg, STRING_ALLOCATE_CONCAT);
	FOUNDATION_SYM(string_concat_varg, STRING_CONCAT);
	FOUNDATION_SYM(string_append_varg, STRING_APPEND);
	FOUNDATION_SYM(string_prepend_varg, STRING_PREPEND);
	FOUNDATION_SYM(string_substr, STRING_SUBSTR);
	FOUNDATION_SYM(string_strip, STRING_STRIP);
	FOUNDATION_SYM(string_find, STRING_FIND);
	FOUNDATION_SYM(string_find_string, STRING_FIND_STRING);
	FOUNDATION_SYM(string_rfind, STRING_RFIND);
	FOUNDATION_SYM(string_rfind_string, STRING_RFIND_STRING);
	FOUNDATION_SYM(string_find_first_of, STRING_FIND_FIRST_OF);
	FOUNDATION_SYM(string_find_last_of, STRING_FIND_LAST_OF);
	FOUNDATION_SYM(string_find_first_not_of, STRING_FIND_FIRST_NOT_OF);
	FOUNDATION_SYM(string_find_last_not_of, STRING_FIND_LAST_NOT_OF);
	FOUNDATION_SYM(string_ends_with, STRING_ENDS_WITH);
	FOUNDATION_SYM(string_equal, STRING_EQUAL);
	FOUNDATION_SYM(string_equal_nocase, STRING_EQUAL_NOCASE);
	FOUNDATION_SYM(string_equal_substr, STRING_EQUAL_SUBSTR);
	FOUNDATION_SYM(string_equal_substr_nocase, STRING_EQUAL_SUBSTR_NOCASE);
	FOUNDATION_SYM(string_match_pattern, STRING_MATCH_PATTERN);
	FOUNDATION_SYM(string_split, STRING_SPLIT);
	FOUNDATION_SYM(string_explode, STRING_EXPLODE);
	FOUNDATION_SYM(string_merge, STRING_MERGE);
	FOUNDATION_SYM(string_glyph, STRING_GLYPH);
	FOUNDATION_SYM(string_array_find, STRING_ARRAY_FIND);
	FOUNDATION_SYM(string_array_deallocate_elements, STRING_ARRAY_DEALLOCATE_ELEMENTS);
	FOUNDATION_SYM(_string_array_deallocate, STRING_ARRAY_DEALLOCATE);
	FOUNDATION_SYM(wstring_allocate_from_string, WSTRING_ALLOCATE_FROM_STRING);
	FOUNDATION_SYM(wstring_from_string, WSTRING_FROM_STRING);
	FOUNDATION_SYM(wstring_deallocate, WSTRING_DEALLOCATE);
	FOUNDATION_SYM(wstring_length, WSTRING_LENGTH);
	FOUNDATION_SYM(wstring_equal, WSTRING_EQUAL);
	FOUNDATION_SYM(string_allocate_from_wstring, STRING_ALLOCATE_FROM_WSTRING);
	FOUNDATION_SYM(string_allocate_from_utf16, STRING_ALLOCATE_FROM_UTF16);
	FOUNDATION_SYM(string_allocate_from_utf32, STRING_ALLOCATE_FROM_UTF32);
	FOUNDATION_SYM(string_convert_utf16, STRING_CONVERT_UTF16);
	FOUNDATION_SYM(string_convert_utf32, STRING_CONVERT_UTF32);
	FOUNDATION_SYM(string_from_int, STRING_FROM_INT);
	FOUNDATION_SYM(string_from_uint, STRING_FROM_UINT);
	FOUNDATION_SYM(string_from_uint128, STRING_FROM_UINT128);
	FOUNDATION_SYM(string_from_real, STRING_FROM_REAL);
	FOUNDATION_SYM(string_from_time, STRING_FROM_TIME);
	FOUNDATION_SYM(string_from_uuid, STRING_FROM_UUID);
	FOUNDATION_SYM(string_from_version, STRING_FROM_VERSION);
	FOUNDATION_SYM(string_from_int_static, STRING_FROM_INT_STATIC);
	FOUNDATION_SYM(string_from_uint_static, STRING_FROM_UINT_STATIC);
	FOUNDATION_SYM(string_from_uint128_static, STRING_FROM_UINT128_STATIC);
	FOUNDATION_SYM(string_from_real_static, STRING_FROM_REAL_STATIC);
	FOUNDATION_SYM(string_from_time_static, STRING_FROM_TIME_STATIC);
	FOUNDATION_SYM(string_from_uuid_static, STRING_FROM_UUID_STATIC);
	FOUNDATION_SYM(string_from_version_static, STRING_FROM_VERSION_STATIC);
	FOUNDATION_SYM(string_to_int, STRING_TO_INT);
	FOUNDATION_SYM(string_to_uint, STRING_TO_UINT);
	FOUNDATION_SYM(string_to_int64, STRING_TO_INT64);
	FOUNDATION_SYM(string_to_uint64, STRING_TO_UINT64);
	FOUNDATION_SYM(string_to_uint128, STRING_TO_UINT128);
	FOUNDATION_SYM(string_to_float32, STRING_TO_FLOAT32);
	FOUNDATION_SYM(string_to_float64, STRING_TO_FLOAT64);
	FOUNDATION_SYM(string_to_real, STRING_TO_REAL);
	FOUNDATION_SYM(string_to_uuid, STRING_TO_UUID);
	FOUNDATION_SYM(string_to_version, STRING_TO_VERSION);
	FOUNDATION_SYM(string_thread_buffer, STRING_THREAD_BUFFER);

	FOUNDATION_SYM(system_error, SYSTEM_ERROR);
	FOUNDATION_SYM(system_error_reset, SYSTEM_ERROR_RESET);
	FOUNDATION_SYM(system_error_message, SYSTEM_ERROR_MESSAGE);
	FOUNDATION_SYM(system_platform, SYSTEM_PLATFORM);
	FOUNDATION_SYM(system_architecture, SYSTEM_ARCHITECTURE);
	FOUNDATION_SYM(system_byteorder, SYSTEM_BYTEORDER);
	FOUNDATION_SYM(system_hardware_threads, SYSTEM_HARDWARE_THREADS);
	FOUNDATION_SYM(system_hostname, SYSTEM_HOSTNAME);
	FOUNDATION_SYM(system_hostid, SYSTEM_HOSTID);
	FOUNDATION_SYM(system_username, SYSTEM_USERNAME);
	FOUNDATION_SYM(system_debugger_attached, SYSTEM_DEBUGGER_ATTACHED);
	FOUNDATION_SYM(system_pause, SYSTEM_PAUSE);
	FOUNDATION_SYM(system_language, SYSTEM_LANGUAGE);
	FOUNDATION_SYM(system_country, SYSTEM_COUNTRY);
	FOUNDATION_SYM(system_locale, SYSTEM_LOCALE);
	FOUNDATION_SYM(system_locale_string, SYSTEM_LOCALE_STRING);
	FOUNDATION_SYM(system_set_device_orientation, SYSTEM_SET_DEVICE_ORIENTATION);
	FOUNDATION_SYM(system_device_orientation, SYSTEM_DEVICE_ORIENTATION);
	FOUNDATION_SYM(system_process_events, SYSTEM_PROCESS_EVENTS);
	FOUNDATION_SYM(system_message_box, SYSTEM_MESSAGE_BOX);
	FOUNDATION_SYM(system_event_stream, SYSTEM_EVENT_STREAM);
	FOUNDATION_SYM(system_post_event, SYSTEM_POST_EVENT);
	FOUNDATION_SYM(system_size_size_t, SYSTEM_SIZE_SIZE_T);
	FOUNDATION_SYM(system_size_real, SYSTEM_SIZE_REAL);
	FOUNDATION_SYM(system_size_pointer, SYSTEM_SIZE_POINTER);
	FOUNDATION_SYM(system_size_wchar, SYSTEM_SIZE_WCHAR);

	FOUNDATION_SYM(thread_allocate, THREAD_ALLOCATE);
	FOUNDATION_SYM(thread_initialize, THREAD_INITIALIZE);
	FOUNDATION_SYM(thread_finalize, THREAD_FINALIZE);
	FOUNDATION_SYM(thread_deallocate, THREAD_DEALLOCATE);
	FOUNDATION_SYM(thread_start, THREAD_START);
	FOUNDATION_SYM(thread_join, THREAD_JOIN);
	FOUNDATION_SYM(thread_signal, THREAD_SIGNAL);
	FOUNDATION_SYM(thread_wait, THREAD_WAIT);
	FOUNDATION_SYM(thread_try_wait, THREAD_TRY_WAIT);
	FOUNDATION_SYM(thread_is_started, THREAD_IS_STARTED);
	FOUNDATION_SYM(thread_is_running, THREAD_IS_RUNNING);
	FOUNDATION_SYM(thread_is_main, THREAD_IS_MAIN);
	FOUNDATION_SYM(thread_set_main, THREAD_SET_MAIN);
	FOUNDATION_SYM(thread_set_name, THREAD_SET_NAME);
	FOUNDATION_SYM(thread_set_hardware, THREAD_SET_HARDWARE);
	FOUNDATION_SYM(thread_self, THREAD_SELF);
	FOUNDATION_SYM(thread_name, THREAD_NAME);
	FOUNDATION_SYM(thread_id, THREAD_ID);
	FOUNDATION_SYM(thread_hardware, THREAD_HARDWARE);
	FOUNDATION_SYM(thread_sleep, THREAD_SLEEP);
	FOUNDATION_SYM(thread_yield, THREAD_YIELD);
	FOUNDATION_SYM(thread_finalize, THREAD_FINALIZE);
	FOUNDATION_SYM(thread_attach_jvm, THREAD_ATTACH_JVM);
	FOUNDATION_SYM(thread_detach_jvm, THREAD_DETACH_JVM);

	FOUNDATION_SYM(time_current, TIME_CURRENT);
	FOUNDATION_SYM(time_diff, TIME_DIFF);
	FOUNDATION_SYM(time_elapsed, TIME_ELAPSED);
	FOUNDATION_SYM(time_elapsed_ticks, TIME_ELAPSED_TICKS);
	FOUNDATION_SYM(time_ticks_per_second, TIME_TICKS_PER_SECOND);
	FOUNDATION_SYM(time_ticks_to_seconds, TIME_TICKS_TO_SECONDS);
	FOUNDATION_SYM(time_startup, TIME_STARTUP);
	FOUNDATION_SYM(time_system, TIME_SYSTEM);

	FOUNDATION_SYM(uuid_generate_time, UUID_GENERATE_TIME);
	FOUNDATION_SYM(uuid_generate_name, UUID_GENERATE_NAME);
	FOUNDATION_SYM(uuid_generate_random, UUID_GENERATE_RANDOM);
	FOUNDATION_SYM(uuid_equal, UUID_GENERATE_EQUAL);
	FOUNDATION_SYM(uuid_null, UUID_NULL);
	FOUNDATION_SYM(uuid_is_null, UUID_IS_NULL);
	FOUNDATION_SYM(uuid_dns, UUID_DNS);

	FOUNDATION_SYM(type_size_t, TYPE_SIZE_T);
	FOUNDATION_SYM(type_ssize_t, TYPE_SSIZE_T);
}
