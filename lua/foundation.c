/* foundation.c  -  Lua library  -  MIT License  -  2013 Mattias Jansson / Rampant Pixels
 *
 * This library provides a fork of the LuaJIT library with custom modifications for projects
 * based on our foundation library.
 *
 * The latest source code maintained by Rampant Pixels is always available at
 * https://github.com/rampantpixels/lua_lib
 *
 * For more information about LuaJIT, see
 * http://luajit.org/
 *
 * The MIT License (MIT)
 * Copyright (c) 2013 Rampant Pixels AB
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <lua/lua.h>
#include <lua/read.h>
#include <lua/foundation.h>
#include <lua/hashstrings.h>

#include <foundation/foundation.h>

#undef LUA_API
#define LUA_HAS_LUA_STATE_TYPE

#include "luajit/src/lua.h"

#if !BUILD_ENABLE_DEBUG_LOG

static void  
_log_debugf_disabled( uint64_t context, const char* format, size_t format_size, ... ) {
	FOUNDATION_UNUSED( context );
	FOUNDATION_UNUSED( format );
	FOUNDATION_UNUSED( format_size );
}

#endif

#if !BUILD_ENABLE_LOG

static log_callback_fn
_log_callback(void) {
	return (log_callback_fn)nullptr;
}

static void
_log_set_callback(log_callback_fn callback) {
	FOUNDATION_UNUSED(callback);
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

static void _error_context_push_disabled( const char* name, size_t name_length, const char* data, size_t data_length ) {
	FOUNDATION_UNUSED( name );
	FOUNDATION_UNUSED( name_length );
	FOUNDATION_UNUSED( data );
	FOUNDATION_UNUSED( data_length );
}

static void _error_context_pop_disabled() {
}

#endif

static void*        
_array_allocate_pointer( int size ) {
	void** arr = 0;
	array_grow( arr, size );
	return arr;
}

static void         
_array_deallocate( void** arr ) {
	array_deallocate( arr );
}

static int          
_array_size( const void* arr ) { 
	return ( arr ? array_size( arr ) : 0 );
}

static const void*  
_array_element_pointer( const void* arr, int pos ) { 
	return ( arr && ( pos >= 0 ) && ( pos < (int)array_size( arr ) ) ) ?
		((const void* const*)arr)[pos] :
		0;
}

static void         
_array_set_element_pointer( const void** arr, int pos, void* ptr ) {
	arr[pos] = ptr;
}

#if !FOUNDATION_PLATFORM_ANDROID
static stream_t* 
asset_stream_open( const char* path, size_t path_length, unsigned int mode ) {
	FOUNDATION_UNUSED( path ); 
	FOUNDATION_UNUSED( path_length ); 
	FOUNDATION_UNUSED( mode );
	return 0;
}

static void      
thread_attach_jvm( void ) {
}

static void      
thread_detach_jvm( void ) {
}

#endif

static int 
system_size_real( void ) { 
	return FOUNDATION_SIZE_REAL;
}

static int 
system_size_pointer( void ) { 
	return FOUNDATION_SIZE_POINTER;
}

static int       
system_size_wchar( void ) { 
	return FOUNDATION_SIZE_WCHAR;
}

#if !BUILD_ENABLE_PROFILE

void 
_profile_initialize( const char* str, size_t length, void* ptr, uint64_t val ) { 
	FOUNDATION_UNUSED( str ); 
	FOUNDATION_UNUSED( length );
	FOUNDATION_UNUSED( ptr ); 
	FOUNDATION_UNUSED( val );
}

void 
_profile_void( void ) {
}

void 
_profile_bool( bool flag ) { 
	FOUNDATION_UNUSED( flag );
}

void 
_profile_fn( profile_write_fn fn ) { 
	FOUNDATION_UNUSED( fn );
}

void 
_profile_int( int val ) { 
	FOUNDATION_UNUSED( val );
}

void 
_profile_uint64( uint64_t val ) { 
	FOUNDATION_UNUSED( val );
}

void 
_profile_str( const char* str, size_t length ) { 
	FOUNDATION_UNUSED( str );
	FOUNDATION_UNUSED( length );
}

#endif

static void 
_string_array_deallocate( string_t* array ) { 
	string_array_deallocate( array );
}

static uuid_t uuid_dns( void ) { 
	return UUID_DNS;
}

static FOUNDATION_NOINLINE void 
lua_load_foundation_builtins( lua_State* state )
{
	lua_t* env = lua_from_state( state );
	hashmap_t* map = lua_lookup_map( env );

	hashmap_insert( map, HASH_SYM_ARRAY_ALLOCATE_POINTER,         (void*)(uintptr_t)_array_allocate_pointer );
	hashmap_insert( map, HASH_SYM_ARRAY_SIZE,                     (void*)(uintptr_t)_array_size );
	hashmap_insert( map, HASH_SYM_ARRAY_ELEMENT_POINTER,          (void*)(uintptr_t)_array_element_pointer );
	hashmap_insert( map, HASH_SYM_ARRAY_SET_ELEMENT_POINTER,      (void*)(uintptr_t)_array_set_element_pointer );
	hashmap_insert( map, HASH_SYM_ARRAY_DEALLOCATE,               (void*)(uintptr_t)_array_deallocate );

	hashmap_insert( map, HASH_SYM_ASSERT_HANDLER,                 (void*)(uintptr_t)assert_handler );
	hashmap_insert( map, HASH_SYM_ASSERT_SET_HANDLER,             (void*)(uintptr_t)assert_set_handler );
	hashmap_insert( map, HASH_SYM_ASSERT_REPORT,                  (void*)(uintptr_t)assert_report );

	hashmap_insert( map, HASH_SYM_ASSET_STREAM_OPEN,              (void*)(uintptr_t)asset_stream_open );

	hashmap_insert( map, HASH_SYM_BASE64_ENCODE,                  (void*)(uintptr_t)base64_encode );
	hashmap_insert( map, HASH_SYM_BASE64_DECODE,                  (void*)(uintptr_t)base64_decode );

	hashmap_insert( map, HASH_SYM_BITBUFFER_ALLOCATE_BUFFER,      (void*)(uintptr_t)bitbuffer_allocate_buffer );
	hashmap_insert( map, HASH_SYM_BITBUFFER_ALLOCATE_STREAM,      (void*)(uintptr_t)bitbuffer_allocate_stream );
	hashmap_insert( map, HASH_SYM_BITBUFFER_DEALLOCATE,           (void*)(uintptr_t)bitbuffer_deallocate );
	hashmap_insert( map, HASH_SYM_BITBUFFER_INITIALIZE_BUFFER,    (void*)(uintptr_t)bitbuffer_initialize_buffer );
	hashmap_insert( map, HASH_SYM_BITBUFFER_INITIALIZE_STREAM,    (void*)(uintptr_t)bitbuffer_initialize_stream );
	hashmap_insert( map, HASH_SYM_BITBUFFER_FINALIZE,             (void*)(uintptr_t)bitbuffer_finalize );
	hashmap_insert( map, HASH_SYM_BITBUFFER_READ32,               (void*)(uintptr_t)bitbuffer_read32 );
	hashmap_insert( map, HASH_SYM_BITBUFFER_READ64,               (void*)(uintptr_t)bitbuffer_read64 );
	hashmap_insert( map, HASH_SYM_BITBUFFER_READ128,              (void*)(uintptr_t)bitbuffer_read128 );
	hashmap_insert( map, HASH_SYM_BITBUFFER_READ_FLOAT32,         (void*)(uintptr_t)bitbuffer_read_float32 );
	hashmap_insert( map, HASH_SYM_BITBUFFER_READ_FLOAT64,         (void*)(uintptr_t)bitbuffer_read_float64 );
	hashmap_insert( map, HASH_SYM_BITBUFFER_WRITE32,              (void*)(uintptr_t)bitbuffer_write32 );
	hashmap_insert( map, HASH_SYM_BITBUFFER_WRITE64,              (void*)(uintptr_t)bitbuffer_write64 );
	hashmap_insert( map, HASH_SYM_BITBUFFER_WRITE128,             (void*)(uintptr_t)bitbuffer_write128 );
	hashmap_insert( map, HASH_SYM_BITBUFFER_WRITE_FLOAT32,        (void*)(uintptr_t)bitbuffer_write_float32 );
	hashmap_insert( map, HASH_SYM_BITBUFFER_WRITE_FLOAT64,        (void*)(uintptr_t)bitbuffer_write_float64 );
	hashmap_insert( map, HASH_SYM_BITBUFFER_ALIGN_READ,           (void*)(uintptr_t)bitbuffer_align_read );
	hashmap_insert( map, HASH_SYM_BITBUFFER_ALIGN_WRITE,          (void*)(uintptr_t)bitbuffer_align_write );
	hashmap_insert( map, HASH_SYM_BITBUFFER_DISCARD_READ,         (void*)(uintptr_t)bitbuffer_discard_read );
	hashmap_insert( map, HASH_SYM_BITBUFFER_DISCARD_WRITE,        (void*)(uintptr_t)bitbuffer_discard_write );

	hashmap_insert( map, HASH_SYM_BYTEORDER_SWAP16,               (void*)(uintptr_t)byteorder_swap16 );
	hashmap_insert( map, HASH_SYM_BYTEORDER_SWAP32,               (void*)(uintptr_t)byteorder_swap32 );
	hashmap_insert( map, HASH_SYM_BYTEORDER_SWAP64,               (void*)(uintptr_t)byteorder_swap64 );
	hashmap_insert( map, HASH_SYM_BYTEORDER_SWAP,                 (void*)(uintptr_t)byteorder_swap );
	hashmap_insert( map, HASH_SYM_BYTEORDER_BIGENDIAN16,          (void*)(uintptr_t)byteorder_bigendian16 );
	hashmap_insert( map, HASH_SYM_BYTEORDER_BIGENDIAN32,          (void*)(uintptr_t)byteorder_bigendian32 );
	hashmap_insert( map, HASH_SYM_BYTEORDER_BIGENDIAN64,          (void*)(uintptr_t)byteorder_bigendian64 );
	hashmap_insert( map, HASH_SYM_BYTEORDER_BIGENDIAN,            (void*)(uintptr_t)byteorder_bigendian );
	hashmap_insert( map, HASH_SYM_BYTEORDER_LITTLEENDIAN16,       (void*)(uintptr_t)byteorder_littleendian16 );
	hashmap_insert( map, HASH_SYM_BYTEORDER_LITTLEENDIAN32,       (void*)(uintptr_t)byteorder_littleendian32 );
	hashmap_insert( map, HASH_SYM_BYTEORDER_LITTLEENDIAN64,       (void*)(uintptr_t)byteorder_littleendian64 );
	hashmap_insert( map, HASH_SYM_BYTEORDER_LITTLEENDIAN,         (void*)(uintptr_t)byteorder_littleendian );

	hashmap_insert( map, HASH_SYM_BLOWFISH_ALLOCATE,              (void*)(uintptr_t)blowfish_allocate );
	hashmap_insert( map, HASH_SYM_BLOWFISH_DEALLOCATE,            (void*)(uintptr_t)blowfish_deallocate );
	hashmap_insert( map, HASH_SYM_BLOWFISH_INITIALIZE,            (void*)(uintptr_t)blowfish_initialize );
	hashmap_insert( map, HASH_SYM_BLOWFISH_FINALIZE,              (void*)(uintptr_t)blowfish_finalize );
	hashmap_insert( map, HASH_SYM_BLOWFISH_ENCRYPT,               (void*)(uintptr_t)blowfish_encrypt );
	hashmap_insert( map, HASH_SYM_BLOWFISH_DECRYPT,               (void*)(uintptr_t)blowfish_decrypt );

	hashmap_insert( map, HASH_SYM_BUFFER_STREAM_ALLOCATE,         (void*)(uintptr_t)buffer_stream_allocate );
	hashmap_insert( map, HASH_SYM_BUFFER_STREAM_INITIALIZE,       (void*)(uintptr_t)buffer_stream_initialize );

	hashmap_insert( map, HASH_SYM_CONFIG_BOOL,                    (void*)(uintptr_t)config_bool );
	hashmap_insert( map, HASH_SYM_CONFIG_INT,                     (void*)(uintptr_t)config_int );
	hashmap_insert( map, HASH_SYM_CONFIG_REAL,                    (void*)(uintptr_t)config_real );
	hashmap_insert( map, HASH_SYM_CONFIG_STRING,                  (void*)(uintptr_t)config_string );
	hashmap_insert( map, HASH_SYM_CONFIG_HASH,                    (void*)(uintptr_t)config_hash );
	hashmap_insert( map, HASH_SYM_CONFIG_SET_BOOL,                (void*)(uintptr_t)config_set_bool );
	hashmap_insert( map, HASH_SYM_CONFIG_SET_INT,                 (void*)(uintptr_t)config_set_int );
	hashmap_insert( map, HASH_SYM_CONFIG_SET_REAL,                (void*)(uintptr_t)config_set_real );
	hashmap_insert( map, HASH_SYM_CONFIG_SET_STRING,              (void*)(uintptr_t)config_set_string );
	hashmap_insert( map, HASH_SYM_CONFIG_LOAD,                    (void*)(uintptr_t)config_load );
	hashmap_insert( map, HASH_SYM_CONFIG_PARSE,                   (void*)(uintptr_t)config_parse );
	hashmap_insert( map, HASH_SYM_CONFIG_WRITE,                   (void*)(uintptr_t)config_write );
	hashmap_insert( map, HASH_SYM_CONFIG_PARSE_COMMANDLINE,       (void*)(uintptr_t)config_parse_commandline );

	hashmap_insert( map, HASH_SYM_ENVIRONMENT_COMMAND_LINE,                   (void*)(uintptr_t)environment_command_line );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_EXECUTABLE_NAME,                (void*)(uintptr_t)environment_executable_name );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_EXECUTABLE_DIRECTORY,           (void*)(uintptr_t)environment_executable_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_EXECUTABLE_PATH,                (void*)(uintptr_t)environment_executable_path );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_INITIAL_WORKING_DIRECTORY,      (void*)(uintptr_t)environment_initial_working_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_CURRENT_WORKING_DIRECTORY,      (void*)(uintptr_t)environment_current_working_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_SET_CURRENT_WORKING_DIRECTORY,  (void*)(uintptr_t)environment_set_current_working_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_HOME_DIRECTORY,                 (void*)(uintptr_t)environment_home_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_TEMPORARY_DIRECTORY,            (void*)(uintptr_t)environment_temporary_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_VARIABLE,                       (void*)(uintptr_t)environment_variable );

	hashmap_insert( map, HASH_SYM_ERROR,                          (void*)(uintptr_t)error );
	hashmap_insert( map, HASH_SYM_ERROR_REPORT,                   (void*)(uintptr_t)error_report );
	hashmap_insert( map, HASH_SYM_ERROR_CALLBACK,                 (void*)(uintptr_t)error_callback );
	hashmap_insert( map, HASH_SYM_ERROR_SET_CALLBACK,             (void*)(uintptr_t)error_set_callback );
#if BUILD_ENABLE_ERROR_CONTEXT
	hashmap_insert( map, HASH_SYM_ERROR_CONTEXT_PUSH,             (void*)(uintptr_t)_error_context_push );
	hashmap_insert( map, HASH_SYM_ERROR_CONTEXT_POP,              (void*)(uintptr_t)_error_context_pop );
#else
	hashmap_insert( map, HASH_SYM_ERROR_CONTEXT_PUSH,             (void*)(uintptr_t)_error_context_push_disabled );
	hashmap_insert( map, HASH_SYM_ERROR_CONTEXT_POP,              (void*)(uintptr_t)_error_context_pop_disabled );
#endif

	hashmap_insert( map, HASH_SYM_EVENT_POST,                     (void*)(uintptr_t)event_post );
	hashmap_insert( map, HASH_SYM_EVENT_NEXT,                     (void*)(uintptr_t)event_next );
	hashmap_insert( map, HASH_SYM_EVENT_PAYLOAD_SIZE,             (void*)(uintptr_t)event_payload_size );
	hashmap_insert( map, HASH_SYM_EVENT_STREAM_ALLOCATE,          (void*)(uintptr_t)event_stream_allocate );
	hashmap_insert( map, HASH_SYM_EVENT_STREAM_DEALLOCATE,        (void*)(uintptr_t)event_stream_deallocate );
	hashmap_insert( map, HASH_SYM_EVENT_STREAM_INITIALIZE,        (void*)(uintptr_t)event_stream_initialize );
	hashmap_insert( map, HASH_SYM_EVENT_STREAM_FINALIZE,          (void*)(uintptr_t)event_stream_finalize );
	hashmap_insert( map, HASH_SYM_EVENT_STREAM_PROCESS,           (void*)(uintptr_t)event_stream_process );

	hashmap_insert( map, HASH_SYM_FS_OPEN_FILE,                   (void*)(uintptr_t)fs_open_file );
	hashmap_insert( map, HASH_SYM_FS_COPY_FILE,                   (void*)(uintptr_t)fs_copy_file );
	hashmap_insert( map, HASH_SYM_FS_REMOVE_FILE,                 (void*)(uintptr_t)fs_remove_file );
	hashmap_insert( map, HASH_SYM_FS_IS_FILE,                     (void*)(uintptr_t)fs_is_file );
	hashmap_insert( map, HASH_SYM_FS_MAKE_DIRECTORY,              (void*)(uintptr_t)fs_make_directory );
	hashmap_insert( map, HASH_SYM_FS_REMOVE_DIRECTORY,            (void*)(uintptr_t)fs_remove_directory );
	hashmap_insert( map, HASH_SYM_FS_IS_DIRECTORY,                (void*)(uintptr_t)fs_is_directory );
	hashmap_insert( map, HASH_SYM_FS_LAST_MODIFIED,               (void*)(uintptr_t)fs_last_modified );
	hashmap_insert( map, HASH_SYM_FS_TOUCH,                       (void*)(uintptr_t)fs_touch );
	hashmap_insert( map, HASH_SYM_FS_MD5,                         (void*)(uintptr_t)fs_md5 );
	hashmap_insert( map, HASH_SYM_FS_MATCHING_FILES,              (void*)(uintptr_t)fs_matching_files );
	hashmap_insert( map, HASH_SYM_FS_MATCHING_FILES_REGEX,        (void*)(uintptr_t)fs_matching_files_regex );
	hashmap_insert( map, HASH_SYM_FS_FILES,                       (void*)(uintptr_t)fs_files );
	hashmap_insert( map, HASH_SYM_FS_SUBDIRS,                     (void*)(uintptr_t)fs_subdirs );
	hashmap_insert( map, HASH_SYM_FS_MONITOR,                     (void*)(uintptr_t)fs_monitor );
	hashmap_insert( map, HASH_SYM_FS_UNMONITOR,                   (void*)(uintptr_t)fs_unmonitor );
	hashmap_insert( map, HASH_SYM_FS_TEMPORARY_FILE,              (void*)(uintptr_t)fs_temporary_file );
	hashmap_insert( map, HASH_SYM_FS_POST_EVENT,                  (void*)(uintptr_t)fs_post_event );
	hashmap_insert( map, HASH_SYM_FS_EVENT_STREAM,                (void*)(uintptr_t)fs_event_stream );

	hashmap_insert( map, HASH_SYM_FOUNDATION_VERSION,             (void*)(uintptr_t)foundation_version );

	hashmap_insert( map, HASH_SYM_HASH,                           (void*)(uintptr_t)hash );
	hashmap_insert( map, HASH_SYM_HASH_TO_STRING,                 (void*)(uintptr_t)hash_to_string );

	hashmap_insert( map, HASH_SYM_HASHMAP_ALLOCATE,               (void*)(uintptr_t)hashmap_allocate );
	hashmap_insert( map, HASH_SYM_HASHMAP_DEALLOCATE,             (void*)(uintptr_t)hashmap_deallocate );
	hashmap_insert( map, HASH_SYM_HASHMAP_INITIALIZE,             (void*)(uintptr_t)hashmap_initialize );
	hashmap_insert( map, HASH_SYM_HASHMAP_FINALIZE,               (void*)(uintptr_t)hashmap_finalize );
	hashmap_insert( map, HASH_SYM_HASHMAP_INSERT,                 (void*)(uintptr_t)hashmap_insert );
	hashmap_insert( map, HASH_SYM_HASHMAP_ERASE,                  (void*)(uintptr_t)hashmap_erase );
	hashmap_insert( map, HASH_SYM_HASHMAP_LOOKUP,                 (void*)(uintptr_t)hashmap_lookup );
	hashmap_insert( map, HASH_SYM_HASHMAP_HAS_KEY,                (void*)(uintptr_t)hashmap_has_key );
	hashmap_insert( map, HASH_SYM_HASHMAP_SIZE,                   (void*)(uintptr_t)hashmap_size );
	hashmap_insert( map, HASH_SYM_HASHMAP_CLEAR,                  (void*)(uintptr_t)hashmap_clear );

#if FOUNDATION_SIZE_POINTER == 4
	hashmap_insert( map, HASH_SYM_HASHTABLE_ALLOCATE,             (void*)(uintptr_t)hashtable32_allocate );
	hashmap_insert( map, HASH_SYM_HASHTABLE_DEALLOCATE,           (void*)(uintptr_t)hashtable32_deallocate );
	hashmap_insert( map, HASH_SYM_HASHTABLE_INITIALIZE,           (void*)(uintptr_t)hashtable32_initialize );
	hashmap_insert( map, HASH_SYM_HASHTABLE_FINALIZE,             (void*)(uintptr_t)hashtable32_finalize );
	hashmap_insert( map, HASH_SYM_HASHTABLE_SET,                  (void*)(uintptr_t)hashtable32_set );
	hashmap_insert( map, HASH_SYM_HASHTABLE_ERASE,                (void*)(uintptr_t)hashtable32_erase );
	hashmap_insert( map, HASH_SYM_HASHTABLE_GET,                  (void*)(uintptr_t)hashtable32_get );
	hashmap_insert( map, HASH_SYM_HASHTABLE_SIZE,                 (void*)(uintptr_t)hashtable32_size );
	hashmap_insert( map, HASH_SYM_HASHTABLE_CLEAR,                (void*)(uintptr_t)hashtable32_clear );
#else
	hashmap_insert( map, HASH_SYM_HASHTABLE_ALLOCATE,             (void*)(uintptr_t)hashtable64_allocate );
	hashmap_insert( map, HASH_SYM_HASHTABLE_DEALLOCATE,           (void*)(uintptr_t)hashtable64_deallocate );
	hashmap_insert( map, HASH_SYM_HASHTABLE_INITIALIZE,           (void*)(uintptr_t)hashtable64_initialize );
	hashmap_insert( map, HASH_SYM_HASHTABLE_FINALIZE,             (void*)(uintptr_t)hashtable64_finalize );
	hashmap_insert( map, HASH_SYM_HASHTABLE_SET,                  (void*)(uintptr_t)hashtable64_set );
	hashmap_insert( map, HASH_SYM_HASHTABLE_ERASE,                (void*)(uintptr_t)hashtable64_erase );
	hashmap_insert( map, HASH_SYM_HASHTABLE_GET,                  (void*)(uintptr_t)hashtable64_get );
	hashmap_insert( map, HASH_SYM_HASHTABLE_SIZE,                 (void*)(uintptr_t)hashtable64_size );
	hashmap_insert( map, HASH_SYM_HASHTABLE_CLEAR,                (void*)(uintptr_t)hashtable64_clear );
#endif
	hashmap_insert( map, HASH_SYM_HASHTABLE32_ALLOCATE,           (void*)(uintptr_t)hashtable32_allocate );
	hashmap_insert( map, HASH_SYM_HASHTABLE32_DEALLOCATE,         (void*)(uintptr_t)hashtable32_deallocate );
	hashmap_insert( map, HASH_SYM_HASHTABLE32_INITIALIZE,         (void*)(uintptr_t)hashtable32_initialize );
	hashmap_insert( map, HASH_SYM_HASHTABLE32_FINALIZE,           (void*)(uintptr_t)hashtable32_finalize );
	hashmap_insert( map, HASH_SYM_HASHTABLE32_SET,                (void*)(uintptr_t)hashtable32_set );
	hashmap_insert( map, HASH_SYM_HASHTABLE32_ERASE,              (void*)(uintptr_t)hashtable32_erase );
	hashmap_insert( map, HASH_SYM_HASHTABLE32_GET,                (void*)(uintptr_t)hashtable32_get );
	hashmap_insert( map, HASH_SYM_HASHTABLE32_CLEAR,              (void*)(uintptr_t)hashtable32_size );
	hashmap_insert( map, HASH_SYM_HASHTABLE32_SIZE,               (void*)(uintptr_t)hashtable32_clear );
	hashmap_insert( map, HASH_SYM_HASHTABLE64_ALLOCATE,           (void*)(uintptr_t)hashtable64_allocate );
	hashmap_insert( map, HASH_SYM_HASHTABLE64_DEALLOCATE,         (void*)(uintptr_t)hashtable64_deallocate );
	hashmap_insert( map, HASH_SYM_HASHTABLE64_INITIALIZE,         (void*)(uintptr_t)hashtable64_initialize );
	hashmap_insert( map, HASH_SYM_HASHTABLE64_FINALIZE,           (void*)(uintptr_t)hashtable64_finalize );
	hashmap_insert( map, HASH_SYM_HASHTABLE64_SET,                (void*)(uintptr_t)hashtable64_set );
	hashmap_insert( map, HASH_SYM_HASHTABLE64_ERASE,              (void*)(uintptr_t)hashtable64_erase );
	hashmap_insert( map, HASH_SYM_HASHTABLE64_GET,                (void*)(uintptr_t)hashtable64_get );
	hashmap_insert( map, HASH_SYM_HASHTABLE64_SIZE,               (void*)(uintptr_t)hashtable64_size );
	hashmap_insert( map, HASH_SYM_HASHTABLE64_CLEAR,              (void*)(uintptr_t)hashtable64_clear );

	hashmap_insert( map, HASH_SYM_LIBRARY_LOAD,                   (void*)(uintptr_t)library_load );
	hashmap_insert( map, HASH_SYM_LIBRARY_REF,                    (void*)(uintptr_t)library_ref );
	hashmap_insert( map, HASH_SYM_LIBRARY_UNLOAD,                 (void*)(uintptr_t)library_unload );
	hashmap_insert( map, HASH_SYM_LIBRARY_SYMBOL,                 (void*)(uintptr_t)library_symbol );
	hashmap_insert( map, HASH_SYM_LIBRARY_NAME,                   (void*)(uintptr_t)library_name );
	hashmap_insert( map, HASH_SYM_LIBRARY_VALID,                  (void*)(uintptr_t)library_valid );

#if BUILD_ENABLE_DEBUG_LOG
	hashmap_insert( map, HASH_SYM_LOG_DEBUGF,                     (void*)(uintptr_t)log_debugf );
#else
	hashmap_insert( map, HASH_SYM_LOG_DEBUGF,                     (void*)(uintptr_t)_log_debugf_disabled );
#endif
#if BUILD_ENABLE_LOG
	hashmap_insert( map, HASH_SYM_LOG_INFOF,                      (void*)(uintptr_t)log_infof );
	hashmap_insert( map, HASH_SYM_LOG_WARNF,                      (void*)(uintptr_t)log_warnf );
	hashmap_insert( map, HASH_SYM_LOG_ERRORF,                     (void*)(uintptr_t)log_errorf );
	hashmap_insert( map, HASH_SYM_LOG_PANICF,                     (void*)(uintptr_t)log_panicf );
	hashmap_insert( map, HASH_SYM_LOG_ENABLE_PREFIX,              (void*)(uintptr_t)log_enable_prefix );
	hashmap_insert( map, HASH_SYM_LOG_ENABLE_STDOUT,              (void*)(uintptr_t)log_enable_stdout );
	hashmap_insert( map, HASH_SYM_LOG_CALLBACK,                   (void*)(uintptr_t)log_callback );
	hashmap_insert( map, HASH_SYM_LOG_SET_CALLBACK,               (void*)(uintptr_t)log_set_callback );
	hashmap_insert( map, HASH_SYM_LOG_SET_SUPPRESS,               (void*)(uintptr_t)log_set_suppress );
	hashmap_insert( map, HASH_SYM_LOG_SUPPRESS,                   (void*)(uintptr_t)log_suppress );
	hashmap_insert( map, HASH_SYM_LOG_SUPPRESS_CLEAR,             (void*)(uintptr_t)log_suppress_clear );
#else
	hashmap_insert( map, HASH_SYM_LOG_INFOF,                      (void*)(uintptr_t)_log_debugf_disabled );
	hashmap_insert( map, HASH_SYM_LOG_WARNF,                      (void*)(uintptr_t)_log_debugf_disabled );
	hashmap_insert( map, HASH_SYM_LOG_ERRORF,                     (void*)(uintptr_t)_log_debugf_disabled );
	hashmap_insert( map, HASH_SYM_LOG_PANICF,                     (void*)(uintptr_t)_log_debugf_disabled );
	hashmap_insert( map, HASH_SYM_LOG_ENABLE_PREFIX,              (void*)(uintptr_t)_log_enable );
	hashmap_insert( map, HASH_SYM_LOG_ENABLE_STDOUT,              (void*)(uintptr_t)_log_enable );
	hashmap_insert( map, HASH_SYM_LOG_CALLBACK,                   (void*)(uintptr_t)_log_callback );
	hashmap_insert( map, HASH_SYM_LOG_SET_CALLBACK,               (void*)(uintptr_t)_log_set_callback );
	hashmap_insert( map, HASH_SYM_LOG_SET_SUPPRESS,               (void*)(uintptr_t)_log_set_suppress );
	hashmap_insert( map, HASH_SYM_LOG_SUPPRESS,                   (void*)(uintptr_t)_log_suppress );
	hashmap_insert( map, HASH_SYM_LOG_SUPPRESS_CLEAR,             (void*)(uintptr_t)_log_suppress_clear );
#endif

	hashmap_insert( map, HASH_SYM_MD5_ALLOCATE,                   (void*)(uintptr_t)md5_allocate );
	hashmap_insert( map, HASH_SYM_MD5_DEALLOCATE,                 (void*)(uintptr_t)md5_deallocate );
	hashmap_insert( map, HASH_SYM_MD5_INITIALIZE,                 (void*)(uintptr_t)md5_initialize );
	hashmap_insert( map, HASH_SYM_MD5_FINALIZE,                   (void*)(uintptr_t)md5_finalize );
	hashmap_insert( map, HASH_SYM_MD5_DIGEST,                     (void*)(uintptr_t)md5_digest );
	hashmap_insert( map, HASH_SYM_MD5_DIGEST_FINALIZE,            (void*)(uintptr_t)md5_digest_finalize );
	hashmap_insert( map, HASH_SYM_MD5_GET_DIGEST,                 (void*)(uintptr_t)md5_get_digest );
	hashmap_insert( map, HASH_SYM_MD5_GET_DIGEST_RAW,             (void*)(uintptr_t)md5_get_digest_raw );

	hashmap_insert( map, HASH_SYM_MEMORY_ALLOCATE,                (void*)(uintptr_t)memory_allocate );
	hashmap_insert( map, HASH_SYM_MEMORY_REALLOCATE,              (void*)(uintptr_t)memory_reallocate );
	hashmap_insert( map, HASH_SYM_MEMORY_DEALLOCATE,              (void*)(uintptr_t)memory_deallocate );
	hashmap_insert( map, HASH_SYM_MEMORY_CONTEXT_PUSH,            (void*)(uintptr_t)memory_context_push );
	hashmap_insert( map, HASH_SYM_MEMORY_CONTEXT_POP,             (void*)(uintptr_t)memory_context_pop );
	hashmap_insert( map, HASH_SYM_MEMORY_CONTEXT,                 (void*)(uintptr_t)memory_context );

	hashmap_insert( map, HASH_SYM_MUTEX_ALLOCATE,                 (void*)(uintptr_t)mutex_allocate );
	hashmap_insert( map, HASH_SYM_MUTEX_DEALLOCATE,               (void*)(uintptr_t)mutex_deallocate );
	hashmap_insert( map, HASH_SYM_MUTEX_NAME,                     (void*)(uintptr_t)mutex_name );
	hashmap_insert( map, HASH_SYM_MUTEX_TRY_LOCK,                 (void*)(uintptr_t)mutex_try_lock );
	hashmap_insert( map, HASH_SYM_MUTEX_LOCK,                     (void*)(uintptr_t)mutex_lock );
	hashmap_insert( map, HASH_SYM_MUTEX_UNLOCK,                   (void*)(uintptr_t)mutex_unlock );
	hashmap_insert( map, HASH_SYM_MUTEX_WAIT,                     (void*)(uintptr_t)mutex_wait );
	hashmap_insert( map, HASH_SYM_MUTEX_SIGNAL,                   (void*)(uintptr_t)mutex_signal );

	hashmap_insert( map, HASH_SYM_OBJECTMAP_ALLOCATE,             (void*)(uintptr_t)objectmap_allocate );
	hashmap_insert( map, HASH_SYM_OBJECTMAP_DEALLOCATE,           (void*)(uintptr_t)objectmap_deallocate );
	hashmap_insert( map, HASH_SYM_OBJECTMAP_INITIALIZE,           (void*)(uintptr_t)objectmap_initialize );
	hashmap_insert( map, HASH_SYM_OBJECTMAP_FINALIZE,             (void*)(uintptr_t)objectmap_finalize );
	hashmap_insert( map, HASH_SYM_OBJECTMAP_SIZE,                 (void*)(uintptr_t)objectmap_size );
	hashmap_insert( map, HASH_SYM_OBJECTMAP_RESERVE,              (void*)(uintptr_t)objectmap_reserve );
	hashmap_insert( map, HASH_SYM_OBJECTMAP_FREE,                 (void*)(uintptr_t)objectmap_free );
	hashmap_insert( map, HASH_SYM_OBJECTMAP_SET,                  (void*)(uintptr_t)objectmap_set );
	hashmap_insert( map, HASH_SYM_OBJECTMAP_RAW_LOOKUP,           (void*)(uintptr_t)objectmap_raw_lookup );
	hashmap_insert( map, HASH_SYM_OBJECTMAP_LOOKUP_REF,           (void*)(uintptr_t)objectmap_lookup_ref );
	hashmap_insert( map, HASH_SYM_OBJECTMAP_LOOKUP_UNREF,         (void*)(uintptr_t)objectmap_lookup_unref );
	hashmap_insert( map, HASH_SYM_OBJECTMAP_LOOKUP,               (void*)(uintptr_t)objectmap_lookup );

	hashmap_insert( map, HASH_SYM_PATH_BASE_FILE_NAME,            (void*)(uintptr_t)path_base_file_name);
	hashmap_insert( map, HASH_SYM_PATH_BASE_FILE_NAME_WITH_DIRECTORY, (void*)(uintptr_t)path_base_file_name_with_directory);
	hashmap_insert( map, HASH_SYM_PATH_FILE_EXTENSION,            (void*)(uintptr_t)path_file_extension);
	hashmap_insert( map, HASH_SYM_PATH_FILE_NAME,                 (void*)(uintptr_t)path_file_name);
	hashmap_insert( map, HASH_SYM_PATH_DIRECTORY_NAME,            (void*)(uintptr_t)path_directory_name);
	hashmap_insert( map, HASH_SYM_PATH_SUBDIRECTORY_NAME,         (void*)(uintptr_t)path_subdirectory_name);
	hashmap_insert( map, HASH_SYM_PATH_PROTOCOL,                  (void*)(uintptr_t)path_protocol);
	hashmap_insert( map, HASH_SYM_PATH_ALLOCATE_CONCAT,           (void*)(uintptr_t)path_allocate_concat);
	hashmap_insert( map, HASH_SYM_PATH_CONCAT,                    (void*)(uintptr_t)path_concat);
	hashmap_insert( map, HASH_SYM_PATH_APPEND,                    (void*)(uintptr_t)path_append);
	hashmap_insert( map, HASH_SYM_PATH_PREPEND,                   (void*)(uintptr_t)path_prepend);
	hashmap_insert( map, HASH_SYM_PATH_ABSOLUTE,                  (void*)(uintptr_t)path_absolute);
	hashmap_insert( map, HASH_SYM_PATH_ALLOCATE_ABSOLUTE,         (void*)(uintptr_t)path_allocate_absolute);
	hashmap_insert( map, HASH_SYM_PATH_CLEAN,                     (void*)(uintptr_t)path_clean);
	hashmap_insert( map, HASH_SYM_PATH_IS_ABSOLUTE,               (void*)(uintptr_t)path_is_absolute);
	hashmap_insert( map, HASH_SYM_PATH_MAKE_TEMPORARY,            (void*)(uintptr_t)path_make_temporary);

	hashmap_insert( map, HASH_SYM_PIPE_ALLOCATE,                  (void*)(uintptr_t)pipe_allocate );
	hashmap_insert( map, HASH_SYM_PIPE_INITIALIZE,                (void*)(uintptr_t)pipe_initialize );
	hashmap_insert( map, HASH_SYM_PIPE_CLOSE_READ,                (void*)(uintptr_t)pipe_close_read );
	hashmap_insert( map, HASH_SYM_PIPE_CLOSE_WRITE,               (void*)(uintptr_t)pipe_close_write );

	hashmap_insert( map, HASH_SYM_PROCESS_ALLOCATE,               (void*)(uintptr_t)process_allocate );
	hashmap_insert( map, HASH_SYM_PROCESS_DEALLOCATE,             (void*)(uintptr_t)process_deallocate );
	hashmap_insert( map, HASH_SYM_PROCESS_INITIALIZE,             (void*)(uintptr_t)process_initialize );
	hashmap_insert( map, HASH_SYM_PROCESS_FINALIZE,               (void*)(uintptr_t)process_finalize );
	hashmap_insert( map, HASH_SYM_PROCESS_SET_WORKING_DIRECTORY,  (void*)(uintptr_t)process_set_working_directory );
	hashmap_insert( map, HASH_SYM_PROCESS_SET_EXECUTABLE_PATH,    (void*)(uintptr_t)process_set_executable_path );
	hashmap_insert( map, HASH_SYM_PROCESS_SET_ARGUMENTS,          (void*)(uintptr_t)process_set_arguments );
	hashmap_insert( map, HASH_SYM_PROCESS_SET_FLAGS,              (void*)(uintptr_t)process_set_flags );
	hashmap_insert( map, HASH_SYM_PROCESS_SET_VERB,               (void*)(uintptr_t)process_set_verb );
	hashmap_insert( map, HASH_SYM_PROCESS_SET_EXIT_CODE,          (void*)(uintptr_t)process_set_exit_code );
	hashmap_insert( map, HASH_SYM_PROCESS_SPAWN,                  (void*)(uintptr_t)process_spawn );
	hashmap_insert( map, HASH_SYM_PROCESS_STDOUT,                 (void*)(uintptr_t)process_stdout );
	hashmap_insert( map, HASH_SYM_PROCESS_STDIN,                  (void*)(uintptr_t)process_stdin );
	hashmap_insert( map, HASH_SYM_PROCESS_WAIT,                   (void*)(uintptr_t)process_wait );
	hashmap_insert( map, HASH_SYM_PROCESS_EXIT_CODE,              (void*)(uintptr_t)process_exit_code );
	hashmap_insert( map, HASH_SYM_PROCESS_EXIT,                   (void*)(uintptr_t)process_exit );

#if BUILD_ENABLE_PROFILE
	hashmap_insert( map, HASH_SYM_PROFILE_INITIALIZE,             (void*)(uintptr_t)profile_initialize );
	hashmap_insert( map, HASH_SYM_PROFILE_FINALIZE,               (void*)(uintptr_t)profile_finalize );
	hashmap_insert( map, HASH_SYM_PROFILE_ENABLE,                 (void*)(uintptr_t)profile_enable );
	hashmap_insert( map, HASH_SYM_PROFILE_SET_OUTPUT,             (void*)(uintptr_t)profile_set_output );
	hashmap_insert( map, HASH_SYM_PROFILE_SET_OUTPUT_WAIT,        (void*)(uintptr_t)profile_set_output_wait );
	hashmap_insert( map, HASH_SYM_PROFILE_END_FRAME,              (void*)(uintptr_t)profile_end_frame );
	hashmap_insert( map, HASH_SYM_PROFILE_BEGIN_BLOCK,            (void*)(uintptr_t)profile_begin_block );
	hashmap_insert( map, HASH_SYM_PROFILE_UPDATE_BLOCK,           (void*)(uintptr_t)profile_update_block );
	hashmap_insert( map, HASH_SYM_PROFILE_END_BLOCK,              (void*)(uintptr_t)profile_end_block );
	hashmap_insert( map, HASH_SYM_PROFILE_LOG,                    (void*)(uintptr_t)profile_log );
	hashmap_insert( map, HASH_SYM_PROFILE_TRYLOCK,                (void*)(uintptr_t)profile_trylock );
	hashmap_insert( map, HASH_SYM_PROFILE_LOCK,                   (void*)(uintptr_t)profile_lock );
	hashmap_insert( map, HASH_SYM_PROFILE_UNLOCK,                 (void*)(uintptr_t)profile_unlock );
	hashmap_insert( map, HASH_SYM_PROFILE_WAIT,                   (void*)(uintptr_t)profile_wait );
	hashmap_insert( map, HASH_SYM_PROFILE_SIGNAL,                 (void*)(uintptr_t)profile_signal );
#else
	hashmap_insert( map, HASH_SYM_PROFILE_INITIALIZE,             (void*)(uintptr_t)_profile_initialize );
	hashmap_insert( map, HASH_SYM_PROFILE_SHUTDOWN,               (void*)(uintptr_t)_profile_void );
	hashmap_insert( map, HASH_SYM_PROFILE_ENABLE,                 (void*)(uintptr_t)_profile_bool );
	hashmap_insert( map, HASH_SYM_PROFILE_SET_OUTPUT,             (void*)(uintptr_t)_profile_fn );
	hashmap_insert( map, HASH_SYM_PROFILE_SET_OUTPUT_WAIT,        (void*)(uintptr_t)_profile_int );
	hashmap_insert( map, HASH_SYM_PROFILE_END_FRAME,              (void*)(uintptr_t)_profile_uint64 );
	hashmap_insert( map, HASH_SYM_PROFILE_BEGIN_BLOCK,            (void*)(uintptr_t)_profile_str );
	hashmap_insert( map, HASH_SYM_PROFILE_UPDATE_BLOCK,           (void*)(uintptr_t)_profile_void );
	hashmap_insert( map, HASH_SYM_PROFILE_END_BLOCK,              (void*)(uintptr_t)_profile_void );
	hashmap_insert( map, HASH_SYM_PROFILE_LOG,                    (void*)(uintptr_t)_profile_str );
	hashmap_insert( map, HASH_SYM_PROFILE_TRYLOCK,                (void*)(uintptr_t)_profile_str );
	hashmap_insert( map, HASH_SYM_PROFILE_LOCK,                   (void*)(uintptr_t)_profile_str );
	hashmap_insert( map, HASH_SYM_PROFILE_UNLOCK,                 (void*)(uintptr_t)_profile_str );
	hashmap_insert( map, HASH_SYM_PROFILE_WAIT,                   (void*)(uintptr_t)_profile_str );
	hashmap_insert( map, HASH_SYM_PROFILE_SIGNAL,                 (void*)(uintptr_t)_profile_str );
#endif

	hashmap_insert( map, HASH_SYM_RADIXSORT_ALLOCATE,             (void*)(uintptr_t)radixsort_allocate );
	hashmap_insert( map, HASH_SYM_RADIXSORT_DEALLOCATE,           (void*)(uintptr_t)radixsort_deallocate );
	hashmap_insert( map, HASH_SYM_RADIXSORT_INITIALIZE,           (void*)(uintptr_t)radixsort_initialize );
	hashmap_insert( map, HASH_SYM_RADIXSORT_FINALIZE,             (void*)(uintptr_t)radixsort_finalize );
	hashmap_insert( map, HASH_SYM_RADIXSORT_SORT,                 (void*)(uintptr_t)radixsort_sort );

	hashmap_insert( map, HASH_SYM_RANDOM32,                       (void*)(uintptr_t)random32 );
	hashmap_insert( map, HASH_SYM_RANDOM32_RANGE,                 (void*)(uintptr_t)random32_range );
	hashmap_insert( map, HASH_SYM_RANDOM64,                       (void*)(uintptr_t)random64 );
	hashmap_insert( map, HASH_SYM_RANDOM64_RANGE,                 (void*)(uintptr_t)random64_range );
	hashmap_insert( map, HASH_SYM_RANDOM_NORMALIZED,              (void*)(uintptr_t)random_normalized );
	hashmap_insert( map, HASH_SYM_RANDOM_RANGE,                   (void*)(uintptr_t)random_range );
	hashmap_insert( map, HASH_SYM_RANDOM32_GAUSSIAN_RANGE,        (void*)(uintptr_t)random32_gaussian_range );
	hashmap_insert( map, HASH_SYM_RANDOM_GAUSSIAN_RANGE,          (void*)(uintptr_t)random_gaussian_range );
	hashmap_insert( map, HASH_SYM_RANDOM32_TRIANGLE_RANGE,        (void*)(uintptr_t)random32_triangle_range );
	hashmap_insert( map, HASH_SYM_RANDOM_TRIANGLE_RANGE,          (void*)(uintptr_t)random_triangle_range );
	hashmap_insert( map, HASH_SYM_RANDOM32_WEIGHTED,              (void*)(uintptr_t)random32_weighted );

	hashmap_insert( map, HASH_SYM_REGEX_COMPILE,                  (void*)(uintptr_t)regex_compile );
	hashmap_insert( map, HASH_SYM_REGEX_MATCH,                    (void*)(uintptr_t)regex_match );
	hashmap_insert( map, HASH_SYM_REGEX_DEALLOCATE,               (void*)(uintptr_t)regex_deallocate );

	hashmap_insert( map, HASH_SYM_RINGBUFFER_ALLOCATE,            (void*)(uintptr_t)ringbuffer_allocate );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_DEALLOCATE,          (void*)(uintptr_t)ringbuffer_deallocate );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_INITIALIZE,          (void*)(uintptr_t)ringbuffer_initialize );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_FINALIZE,            (void*)(uintptr_t)ringbuffer_finalize );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_SIZE,                (void*)(uintptr_t)ringbuffer_size );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_RESET,               (void*)(uintptr_t)ringbuffer_reset );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_READ,                (void*)(uintptr_t)ringbuffer_read );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_WRITE,               (void*)(uintptr_t)ringbuffer_write );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_TOTAL_READ,          (void*)(uintptr_t)ringbuffer_total_read );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_TOTAL_WRITTEN,       (void*)(uintptr_t)ringbuffer_total_written );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_STREAM_ALLOCATE,     (void*)(uintptr_t)ringbuffer_stream_allocate );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_STREAM_INITIALIZE,   (void*)(uintptr_t)ringbuffer_stream_initialize );

	hashmap_insert( map, HASH_SYM_SEMAPHORE_INITIALIZE,           (void*)(uintptr_t)semaphore_initialize );
	hashmap_insert( map, HASH_SYM_SEMAPHORE_INITIALIZE_NAMED,     (void*)(uintptr_t)semaphore_initialize_named );
	hashmap_insert( map, HASH_SYM_SEMAPHORE_FINALIZE,             (void*)(uintptr_t)semaphore_finalize );
	hashmap_insert( map, HASH_SYM_SEMAPHORE_WAIT,                 (void*)(uintptr_t)semaphore_wait );
	hashmap_insert( map, HASH_SYM_SEMAPHORE_TRY_WAIT,             (void*)(uintptr_t)semaphore_try_wait );
	hashmap_insert( map, HASH_SYM_SEMAPHORE_POST,                 (void*)(uintptr_t)semaphore_post );

	hashmap_insert( map, HASH_SYM_STACKTRACE_CAPTURE,             (void*)(uintptr_t)stacktrace_capture );
	hashmap_insert( map, HASH_SYM_STACKTRACE_RESOLVE,             (void*)(uintptr_t)stacktrace_resolve );

	hashmap_insert( map, HASH_SYM_STREAM_OPEN,                    (void*)(uintptr_t)stream_open );
	hashmap_insert( map, HASH_SYM_STREAM_CLONE,                   (void*)(uintptr_t)stream_clone );
	hashmap_insert( map, HASH_SYM_STREAM_DEALLOCATE,              (void*)(uintptr_t)stream_deallocate );
	hashmap_insert( map, HASH_SYM_STREAM_INITIALIZE,              (void*)(uintptr_t)stream_initialize );
	hashmap_insert( map, HASH_SYM_STREAM_FINALIZE,                (void*)(uintptr_t)stream_finalize );
	hashmap_insert( map, HASH_SYM_STREAM_TELL,                    (void*)(uintptr_t)stream_tell );
	hashmap_insert( map, HASH_SYM_STREAM_SEEK,                    (void*)(uintptr_t)stream_seek );
	hashmap_insert( map, HASH_SYM_STREAM_EOS,                     (void*)(uintptr_t)stream_eos );
	hashmap_insert( map, HASH_SYM_STREAM_SIZE,                    (void*)(uintptr_t)stream_size );
	hashmap_insert( map, HASH_SYM_STREAM_SET_BYTEORDER,           (void*)(uintptr_t)stream_set_byteorder );
	hashmap_insert( map, HASH_SYM_STREAM_SET_BINARY,              (void*)(uintptr_t)stream_set_binary );
	hashmap_insert( map, HASH_SYM_STREAM_DETERMINE_BINARY_MODE,   (void*)(uintptr_t)stream_determine_binary_mode );
	hashmap_insert( map, HASH_SYM_STREAM_IS_BINARY,               (void*)(uintptr_t)stream_is_binary );
	hashmap_insert( map, HASH_SYM_STREAM_IS_SEQUENTIAL,           (void*)(uintptr_t)stream_is_sequential );
	hashmap_insert( map, HASH_SYM_STREAM_IS_RELIABLE,             (void*)(uintptr_t)stream_is_reliable );
	hashmap_insert( map, HASH_SYM_STREAM_IS_INORDER,              (void*)(uintptr_t)stream_is_inorder );
	hashmap_insert( map, HASH_SYM_STREAM_IS_SWAPPED,              (void*)(uintptr_t)stream_is_swapped );
	hashmap_insert( map, HASH_SYM_STREAM_BYTEORDER,               (void*)(uintptr_t)stream_byteorder );
	hashmap_insert( map, HASH_SYM_STREAM_PATH,                    (void*)(uintptr_t)stream_path );
	hashmap_insert( map, HASH_SYM_STREAM_LAST_MODIFIED,           (void*)(uintptr_t)stream_last_modified );
	hashmap_insert( map, HASH_SYM_STREAM_READ,                    (void*)(uintptr_t)stream_read );
	hashmap_insert( map, HASH_SYM_STREAM_READ_LINE_BUFFER,        (void*)(uintptr_t)stream_read_line_buffer );
	hashmap_insert( map, HASH_SYM_STREAM_READ_LINE,               (void*)(uintptr_t)stream_read_line );
	hashmap_insert( map, HASH_SYM_STREAM_READ_BOOL,               (void*)(uintptr_t)stream_read_bool );
	hashmap_insert( map, HASH_SYM_STREAM_READ_INT8,               (void*)(uintptr_t)stream_read_int8 );
	hashmap_insert( map, HASH_SYM_STREAM_READ_UINT8,              (void*)(uintptr_t)stream_read_uint8 );
	hashmap_insert( map, HASH_SYM_STREAM_READ_INT16,              (void*)(uintptr_t)stream_read_int16 );
	hashmap_insert( map, HASH_SYM_STREAM_READ_UINT16,             (void*)(uintptr_t)stream_read_uint16 );
	hashmap_insert( map, HASH_SYM_STREAM_READ_INT32,              (void*)(uintptr_t)stream_read_int32 );
	hashmap_insert( map, HASH_SYM_STREAM_READ_UINT32,             (void*)(uintptr_t)stream_read_uint32 );
	hashmap_insert( map, HASH_SYM_STREAM_READ_INT64,              (void*)(uintptr_t)stream_read_int64 );
	hashmap_insert( map, HASH_SYM_STREAM_READ_UINT64,             (void*)(uintptr_t)stream_read_uint64 );
	hashmap_insert( map, HASH_SYM_STREAM_READ_FLOAT32,            (void*)(uintptr_t)stream_read_float32 );
	hashmap_insert( map, HASH_SYM_STREAM_READ_FLOAT64,            (void*)(uintptr_t)stream_read_float64 );
	hashmap_insert( map, HASH_SYM_STREAM_READ_STRING,             (void*)(uintptr_t)stream_read_string );
	hashmap_insert( map, HASH_SYM_STREAM_READ_STRING_BUFFER,      (void*)(uintptr_t)stream_read_string_buffer );
	hashmap_insert( map, HASH_SYM_STREAM_WRITE,                   (void*)(uintptr_t)stream_write );
	hashmap_insert( map, HASH_SYM_STREAM_WRITE_BOOL,              (void*)(uintptr_t)stream_write_bool );
	hashmap_insert( map, HASH_SYM_STREAM_WRITE_INT8,              (void*)(uintptr_t)stream_write_int8 );
	hashmap_insert( map, HASH_SYM_STREAM_WRITE_UINT8,             (void*)(uintptr_t)stream_write_uint8 );
	hashmap_insert( map, HASH_SYM_STREAM_WRITE_INT16,             (void*)(uintptr_t)stream_write_int16 );
	hashmap_insert( map, HASH_SYM_STREAM_WRITE_UINT16,            (void*)(uintptr_t)stream_write_uint16 );
	hashmap_insert( map, HASH_SYM_STREAM_WRITE_INT32,             (void*)(uintptr_t)stream_write_int32 );
	hashmap_insert( map, HASH_SYM_STREAM_WRITE_UINT32,            (void*)(uintptr_t)stream_write_uint32 );
	hashmap_insert( map, HASH_SYM_STREAM_WRITE_INT64,             (void*)(uintptr_t)stream_write_int64 );
	hashmap_insert( map, HASH_SYM_STREAM_WRITE_UINT64,            (void*)(uintptr_t)stream_write_uint64 );
	hashmap_insert( map, HASH_SYM_STREAM_WRITE_FLOAT32,           (void*)(uintptr_t)stream_write_float32 );
	hashmap_insert( map, HASH_SYM_STREAM_WRITE_FLOAT64,           (void*)(uintptr_t)stream_write_float64 );
	hashmap_insert( map, HASH_SYM_STREAM_WRITE_STRING,            (void*)(uintptr_t)stream_write_string );
	hashmap_insert( map, HASH_SYM_STREAM_WRITE_ENDL,              (void*)(uintptr_t)stream_write_endl );
	hashmap_insert( map, HASH_SYM_STREAM_WRITE_FORMAT,            (void*)(uintptr_t)stream_write_format );
	hashmap_insert( map, HASH_SYM_STREAM_BUFFER_READ,             (void*)(uintptr_t)stream_buffer_read );
	hashmap_insert( map, HASH_SYM_STREAM_AVAILABLE_READ,          (void*)(uintptr_t)stream_available_read );
	hashmap_insert( map, HASH_SYM_STREAM_MD5,                     (void*)(uintptr_t)stream_md5 );
	hashmap_insert( map, HASH_SYM_STREAM_TRUNCATE,                (void*)(uintptr_t)stream_truncate );
	hashmap_insert( map, HASH_SYM_STREAM_FLUSH,                   (void*)(uintptr_t)stream_flush );
	hashmap_insert( map, HASH_SYM_STREAM_OPEN_STDOUT,             (void*)(uintptr_t)stream_open_stdout );
	hashmap_insert( map, HASH_SYM_STREAM_OPEN_STDERR,             (void*)(uintptr_t)stream_open_stderr );
	hashmap_insert( map, HASH_SYM_STREAM_OPEN_STDIN,              (void*)(uintptr_t)stream_open_stdin );
	hashmap_insert( map, HASH_SYM_STREAM_SET_PROTOCOL_HANDLER,    (void*)(uintptr_t)stream_set_protocol_handler );
	hashmap_insert( map, HASH_SYM_STREAM_PROTOCOL_HANDLER,        (void*)(uintptr_t)stream_protocol_handler );

	hashmap_insert( map, HASH_SYM_STRING_ALLOCATE,                (void*)(uintptr_t)string_allocate );
	hashmap_insert( map, HASH_SYM_STRING_DEALLOCATE,              (void*)(uintptr_t)string_deallocate );
	hashmap_insert( map, HASH_SYM_STRING_CLONE,                   (void*)(uintptr_t)string_clone );
	hashmap_insert( map, HASH_SYM_STRING_CLONE_STRING,            (void*)(uintptr_t)string_clone_string );
	hashmap_insert( map, HASH_SYM_STRING_NULL,                    (void*)(uintptr_t)string_null );
	hashmap_insert( map, HASH_SYM_STRING_EMPTY,                   (void*)(uintptr_t)string_empty );
	hashmap_insert( map, HASH_SYM_STRING,                         (void*)(uintptr_t)string );
	hashmap_insert( map, HASH_SYM_STRING_CONST,                   (void*)(uintptr_t)string_const );
	hashmap_insert( map, HASH_SYM_STRING_TO_CONST,                (void*)(uintptr_t)string_to_const );
	hashmap_insert( map, HASH_SYM_STRING_ALLOCATE_FORMAT,         (void*)(uintptr_t)string_allocate_format );
	hashmap_insert( map, HASH_SYM_STRING_FORMAT,                  (void*)(uintptr_t)string_format );
	hashmap_insert( map, HASH_SYM_STRING_LENGTH,                  (void*)(uintptr_t)string_length );
	hashmap_insert( map, HASH_SYM_STRING_GLYPHS,                  (void*)(uintptr_t)string_glyphs );
	hashmap_insert( map, HASH_SYM_STRING_HASH,                    (void*)(uintptr_t)string_hash );
	hashmap_insert( map, HASH_SYM_STRING_COPY,                    (void*)(uintptr_t)string_copy );
	hashmap_insert( map, HASH_SYM_STRING_RESIZE,                  (void*)(uintptr_t)string_resize );
	hashmap_insert( map, HASH_SYM_STRING_REPLACE,                 (void*)(uintptr_t)string_replace );
	hashmap_insert( map, HASH_SYM_STRING_ALLOCATE_CONCAT,         (void*)(uintptr_t)string_allocate_concat );
	hashmap_insert( map, HASH_SYM_STRING_CONCAT,                  (void*)(uintptr_t)string_concat );
	hashmap_insert( map, HASH_SYM_STRING_APPEND,                  (void*)(uintptr_t)string_append );
	hashmap_insert( map, HASH_SYM_STRING_PREPEND,                 (void*)(uintptr_t)string_prepend );
	hashmap_insert( map, HASH_SYM_STRING_SUBSTR,                  (void*)(uintptr_t)string_substr );
	hashmap_insert( map, HASH_SYM_STRING_STRIP,                   (void*)(uintptr_t)string_strip );
	hashmap_insert( map, HASH_SYM_STRING_FIND,                    (void*)(uintptr_t)string_find );
	hashmap_insert( map, HASH_SYM_STRING_FIND_STRING,             (void*)(uintptr_t)string_find_string );
	hashmap_insert( map, HASH_SYM_STRING_RFIND,                   (void*)(uintptr_t)string_rfind );
	hashmap_insert( map, HASH_SYM_STRING_RFIND_STRING,            (void*)(uintptr_t)string_rfind_string );
	hashmap_insert( map, HASH_SYM_STRING_FIND_FIRST_OF,           (void*)(uintptr_t)string_find_first_of );
	hashmap_insert( map, HASH_SYM_STRING_FIND_LAST_OF,            (void*)(uintptr_t)string_find_last_of );
	hashmap_insert( map, HASH_SYM_STRING_FIND_FIRST_NOT_OF,       (void*)(uintptr_t)string_find_first_not_of );
	hashmap_insert( map, HASH_SYM_STRING_FIND_LAST_NOT_OF,        (void*)(uintptr_t)string_find_last_not_of );
	hashmap_insert( map, HASH_SYM_STRING_ENDS_WITH,               (void*)(uintptr_t)string_ends_with );
	hashmap_insert( map, HASH_SYM_STRING_EQUAL,                   (void*)(uintptr_t)string_equal );
	hashmap_insert( map, HASH_SYM_STRING_EQUAL_NOCASE,            (void*)(uintptr_t)string_equal_nocase );
	hashmap_insert( map, HASH_SYM_STRING_EQUAL_SUBSTR,            (void*)(uintptr_t)string_equal_substr );
	hashmap_insert( map, HASH_SYM_STRING_EQUAL_SUBSTR_NOCASE,     (void*)(uintptr_t)string_equal_substr_nocase );
	hashmap_insert( map, HASH_SYM_STRING_MATCH_PATTERN,           (void*)(uintptr_t)string_match_pattern );
	hashmap_insert( map, HASH_SYM_STRING_SPLIT,                   (void*)(uintptr_t)string_split );
	hashmap_insert( map, HASH_SYM_STRING_EXPLODE,                 (void*)(uintptr_t)string_explode );
	hashmap_insert( map, HASH_SYM_STRING_MERGE,                   (void*)(uintptr_t)string_merge );
	hashmap_insert( map, HASH_SYM_STRING_GLYPH,                   (void*)(uintptr_t)string_glyph );
	hashmap_insert( map, HASH_SYM_STRING_ARRAY_FIND,              (void*)(uintptr_t)string_array_find );
	hashmap_insert( map, HASH_SYM_STRING_ARRAY_DEALLOCATE_ELEMENTS, (void*)(uintptr_t)string_array_deallocate_elements );
	hashmap_insert( map, HASH_SYM_STRING_ARRAY_DEALLOCATE,        (void*)(uintptr_t)_string_array_deallocate );
	hashmap_insert( map, HASH_SYM_WSTRING_ALLOCATE_FROM_STRING,   (void*)(uintptr_t)wstring_allocate_from_string );
	hashmap_insert( map, HASH_SYM_WSTRING_FROM_STRING,            (void*)(uintptr_t)wstring_from_string );
	hashmap_insert( map, HASH_SYM_WSTRING_DEALLOCATE,             (void*)(uintptr_t)wstring_deallocate );
	hashmap_insert( map, HASH_SYM_WSTRING_LENGTH,                 (void*)(uintptr_t)wstring_length );
	hashmap_insert( map, HASH_SYM_WSTRING_EQUAL,                  (void*)(uintptr_t)wstring_equal );
	hashmap_insert( map, HASH_SYM_STRING_ALLOCATE_FROM_WSTRING,   (void*)(uintptr_t)string_allocate_from_wstring );
	hashmap_insert( map, HASH_SYM_STRING_ALLOCATE_FROM_UTF16,     (void*)(uintptr_t)string_allocate_from_utf16 );
	hashmap_insert( map, HASH_SYM_STRING_ALLOCATE_FROM_UTF32,     (void*)(uintptr_t)string_allocate_from_utf32 );
	hashmap_insert( map, HASH_SYM_STRING_CONVERT_UTF16,           (void*)(uintptr_t)string_convert_utf16 );
	hashmap_insert( map, HASH_SYM_STRING_CONVERT_UTF32,           (void*)(uintptr_t)string_convert_utf32 );
	hashmap_insert( map, HASH_SYM_STRING_FROM_INT,                (void*)(uintptr_t)string_from_int );
	hashmap_insert( map, HASH_SYM_STRING_FROM_UINT,               (void*)(uintptr_t)string_from_uint );
	hashmap_insert( map, HASH_SYM_STRING_FROM_UINT128,            (void*)(uintptr_t)string_from_uint128 );
	hashmap_insert( map, HASH_SYM_STRING_FROM_REAL,               (void*)(uintptr_t)string_from_real );
	hashmap_insert( map, HASH_SYM_STRING_FROM_TIME,               (void*)(uintptr_t)string_from_time );
	hashmap_insert( map, HASH_SYM_STRING_FROM_UUID,               (void*)(uintptr_t)string_from_uuid );
	hashmap_insert( map, HASH_SYM_STRING_FROM_VERSION,            (void*)(uintptr_t)string_from_version );
	hashmap_insert( map, HASH_SYM_STRING_FROM_INT_STATIC,         (void*)(uintptr_t)string_from_int_static );
	hashmap_insert( map, HASH_SYM_STRING_FROM_UINT_STATIC,        (void*)(uintptr_t)string_from_uint_static );
	hashmap_insert( map, HASH_SYM_STRING_FROM_UINT128_STATIC,     (void*)(uintptr_t)string_from_uint128_static );
	hashmap_insert( map, HASH_SYM_STRING_FROM_REAL_STATIC,        (void*)(uintptr_t)string_from_real_static );
	hashmap_insert( map, HASH_SYM_STRING_FROM_TIME_STATIC,        (void*)(uintptr_t)string_from_time_static );
	hashmap_insert( map, HASH_SYM_STRING_FROM_UUID_STATIC,        (void*)(uintptr_t)string_from_uuid_static );
	hashmap_insert( map, HASH_SYM_STRING_FROM_VERSION_STATIC,     (void*)(uintptr_t)string_from_version_static );
	hashmap_insert( map, HASH_SYM_STRING_TO_INT,                  (void*)(uintptr_t)string_to_int );
	hashmap_insert( map, HASH_SYM_STRING_TO_UINT,                 (void*)(uintptr_t)string_to_uint );
	hashmap_insert( map, HASH_SYM_STRING_TO_INT64,                (void*)(uintptr_t)string_to_int64 );
	hashmap_insert( map, HASH_SYM_STRING_TO_UINT64,               (void*)(uintptr_t)string_to_uint64 );
	hashmap_insert( map, HASH_SYM_STRING_TO_UINT128,              (void*)(uintptr_t)string_to_uint128 );
	hashmap_insert( map, HASH_SYM_STRING_TO_FLOAT32,              (void*)(uintptr_t)string_to_float32 );
	hashmap_insert( map, HASH_SYM_STRING_TO_FLOAT64,              (void*)(uintptr_t)string_to_float64 );
	hashmap_insert( map, HASH_SYM_STRING_TO_REAL,                 (void*)(uintptr_t)string_to_real );
	hashmap_insert( map, HASH_SYM_STRING_TO_UUID,                 (void*)(uintptr_t)string_to_uuid );
	hashmap_insert( map, HASH_SYM_STRING_TO_VERSION,              (void*)(uintptr_t)string_to_version );
	hashmap_insert( map, HASH_SYM_STRING_THREAD_BUFFER,           (void*)(uintptr_t)string_thread_buffer );

	hashmap_insert( map, HASH_SYM_SYSTEM_ERROR,                   (void*)(uintptr_t)system_error );
	hashmap_insert( map, HASH_SYM_SYSTEM_ERROR_RESET,             (void*)(uintptr_t)system_error_reset );
	hashmap_insert( map, HASH_SYM_SYSTEM_ERROR_MESSAGE,           (void*)(uintptr_t)system_error_message );
	hashmap_insert( map, HASH_SYM_SYSTEM_PLATFORM,                (void*)(uintptr_t)system_platform );
	hashmap_insert( map, HASH_SYM_SYSTEM_ARCHITECTURE,            (void*)(uintptr_t)system_architecture );
	hashmap_insert( map, HASH_SYM_SYSTEM_BYTEORDER,               (void*)(uintptr_t)system_byteorder );
	hashmap_insert( map, HASH_SYM_SYSTEM_HARDWARE_THREADS,        (void*)(uintptr_t)system_hardware_threads );
	hashmap_insert( map, HASH_SYM_SYSTEM_HOSTNAME,                (void*)(uintptr_t)system_hostname );
	hashmap_insert( map, HASH_SYM_SYSTEM_HOSTID,                  (void*)(uintptr_t)system_hostid );
	hashmap_insert( map, HASH_SYM_SYSTEM_USERNAME,                (void*)(uintptr_t)system_username );
	hashmap_insert( map, HASH_SYM_SYSTEM_DEBUGGER_ATTACHED,       (void*)(uintptr_t)system_debugger_attached );
	hashmap_insert( map, HASH_SYM_SYSTEM_PAUSE,                   (void*)(uintptr_t)system_pause );
	hashmap_insert( map, HASH_SYM_SYSTEM_LANGUAGE,                (void*)(uintptr_t)system_language );
	hashmap_insert( map, HASH_SYM_SYSTEM_COUNTRY,                 (void*)(uintptr_t)system_country );
	hashmap_insert( map, HASH_SYM_SYSTEM_LOCALE,                  (void*)(uintptr_t)system_locale );
	hashmap_insert( map, HASH_SYM_SYSTEM_LOCALE_STRING,           (void*)(uintptr_t)system_locale_string );
	hashmap_insert( map, HASH_SYM_SYSTEM_SET_DEVICE_ORIENTATION,  (void*)(uintptr_t)system_set_device_orientation );
	hashmap_insert( map, HASH_SYM_SYSTEM_DEVICE_ORIENTATION,      (void*)(uintptr_t)system_device_orientation );
	hashmap_insert( map, HASH_SYM_SYSTEM_PROCESS_EVENTS,          (void*)(uintptr_t)system_process_events );
	hashmap_insert( map, HASH_SYM_SYSTEM_MESSAGE_BOX,             (void*)(uintptr_t)system_message_box );
	hashmap_insert( map, HASH_SYM_SYSTEM_EVENT_STREAM,            (void*)(uintptr_t)system_event_stream );
	hashmap_insert( map, HASH_SYM_SYSTEM_POST_EVENT,              (void*)(uintptr_t)system_post_event );
	hashmap_insert( map, HASH_SYM_SYSTEM_SIZE_REAL,               (void*)(uintptr_t)system_size_real );
	hashmap_insert( map, HASH_SYM_SYSTEM_SIZE_POINTER,            (void*)(uintptr_t)system_size_pointer );
	hashmap_insert( map, HASH_SYM_SYSTEM_SIZE_WCHAR,              (void*)(uintptr_t)system_size_wchar );

	hashmap_insert( map, HASH_SYM_THREAD_CREATE,                  (void*)(uintptr_t)thread_create );
	hashmap_insert( map, HASH_SYM_THREAD_REF,                     (void*)(uintptr_t)thread_ref );
	hashmap_insert( map, HASH_SYM_THREAD_DESTROY,                 (void*)(uintptr_t)thread_destroy );
	hashmap_insert( map, HASH_SYM_THREAD_START,                   (void*)(uintptr_t)thread_start );
	hashmap_insert( map, HASH_SYM_THREAD_TERMINATE,               (void*)(uintptr_t)thread_terminate );
	hashmap_insert( map, HASH_SYM_THREAD_IS_STARTED,              (void*)(uintptr_t)thread_is_started );
	hashmap_insert( map, HASH_SYM_THREAD_IS_RUNNING,              (void*)(uintptr_t)thread_is_running );
	hashmap_insert( map, HASH_SYM_THREAD_IS_THREAD,               (void*)(uintptr_t)thread_is_thread );
	hashmap_insert( map, HASH_SYM_THREAD_IS_MAIN,                 (void*)(uintptr_t)thread_is_main );
	hashmap_insert( map, HASH_SYM_THREAD_SHOULD_TERMINATE,        (void*)(uintptr_t)thread_should_terminate );
	hashmap_insert( map, HASH_SYM_THREAD_SET_MAIN,                (void*)(uintptr_t)thread_set_main );
	hashmap_insert( map, HASH_SYM_THREAD_SET_NAME,                (void*)(uintptr_t)thread_set_name );
	hashmap_insert( map, HASH_SYM_THREAD_SET_HARDWARE,            (void*)(uintptr_t)thread_set_hardware );
	hashmap_insert( map, HASH_SYM_THREAD_RESULT,                  (void*)(uintptr_t)thread_result );
	hashmap_insert( map, HASH_SYM_THREAD_SELF,                    (void*)(uintptr_t)thread_self );
	hashmap_insert( map, HASH_SYM_THREAD_NAME,                    (void*)(uintptr_t)thread_name );
	hashmap_insert( map, HASH_SYM_THREAD_ID,                      (void*)(uintptr_t)thread_id );
	hashmap_insert( map, HASH_SYM_THREAD_HARDWARE,                (void*)(uintptr_t)thread_hardware );
	hashmap_insert( map, HASH_SYM_THREAD_SLEEP,                   (void*)(uintptr_t)thread_sleep );
	hashmap_insert( map, HASH_SYM_THREAD_YIELD,                   (void*)(uintptr_t)thread_yield );
	hashmap_insert( map, HASH_SYM_THREAD_FINALIZE,                (void*)(uintptr_t)thread_finalize );
	hashmap_insert( map, HASH_SYM_THREAD_ATTACH_JVM,              (void*)(uintptr_t)thread_attach_jvm );
	hashmap_insert( map, HASH_SYM_THREAD_DETACH_JVM,              (void*)(uintptr_t)thread_detach_jvm );

	hashmap_insert( map, HASH_SYM_TIME_CURRENT,                   (void*)(uintptr_t)time_current );
	hashmap_insert( map, HASH_SYM_TIME_DIFF,                      (void*)(uintptr_t)time_diff );
	hashmap_insert( map, HASH_SYM_TIME_ELAPSED,                   (void*)(uintptr_t)time_elapsed );
	hashmap_insert( map, HASH_SYM_TIME_ELAPSED_TICKS,             (void*)(uintptr_t)time_elapsed_ticks );
	hashmap_insert( map, HASH_SYM_TIME_TICKS_PER_SECOND,          (void*)(uintptr_t)time_ticks_per_second );
	hashmap_insert( map, HASH_SYM_TIME_TICKS_TO_SECONDS,          (void*)(uintptr_t)time_ticks_to_seconds );
	hashmap_insert( map, HASH_SYM_TIME_STARTUP,                   (void*)(uintptr_t)time_startup );
	hashmap_insert( map, HASH_SYM_TIME_SYSTEM,                    (void*)(uintptr_t)time_system );

	hashmap_insert( map, HASH_SYM_UUID_GENERATE_TIME,             (void*)(uintptr_t)uuid_generate_time );
	hashmap_insert( map, HASH_SYM_UUID_GENERATE_NAME,             (void*)(uintptr_t)uuid_generate_name );
	hashmap_insert( map, HASH_SYM_UUID_GENERATE_RANDOM,           (void*)(uintptr_t)uuid_generate_random );
	hashmap_insert( map, HASH_SYM_UUID_GENERATE_EQUAL,            (void*)(uintptr_t)uuid_equal );
	hashmap_insert( map, HASH_SYM_UUID_NULL,                      (void*)(uintptr_t)uuid_null );
	hashmap_insert( map, HASH_SYM_UUID_IS_NULL,                   (void*)(uintptr_t)uuid_is_null );
	hashmap_insert( map, HASH_SYM_UUID_DNS,                       (void*)(uintptr_t)uuid_dns );
}

int 
lua_load_foundation( lua_State* state )
{
	//TODO: When implemented lua compiled bytecode libraries, load from library resource instaed
	static unsigned char bytecode[] = {
		#include "bind.foundation.hex"
	};

	lua_readbuffer_t read_buffer = {
		.buffer = bytecode,
		.size   = sizeof( bytecode ),
		.offset = 0
	};

	log_debugf( HASH_LUA, STRING_CONST("Loading foundation built-ins (%u bytes of bytecode)"), read_buffer.size );

	lua_load_foundation_builtins( state );

	if( lua_load( state, lua_read_buffer, &read_buffer, "=eval" ) != 0 )
	{
		log_errorf( HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua load failed (foundation): %s"), lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		return 0;
	}

	if( lua_pcall( state, 0, 0, 0 ) != 0 )
	{
		log_errorf( HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua pcall failed (foundation): %s"), lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		return 0;
	}

	return 0;
}


