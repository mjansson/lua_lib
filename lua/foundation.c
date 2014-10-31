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


#if !BUILD_ENABLE_LOG
static void  _log_debugf_disabled( uint64_t context, const char* format, ... ) {}
#endif

#if !BUILD_ENABLE_ERROR_CONTEXT
static void  _error_context_push_disabled( const char* name, const char* data ) {}
static void  _error_context_pop_disabled() {}
#endif

static void*        _array_allocate_pointer( int size ) { char** arr = 0; array_grow( arr, size ); return arr; }	
static void         _array_deallocate( void** arr ) { array_deallocate( arr ); }
static int          _array_size( const void* arr ) { return ( arr ? array_size( arr ) : 0 ); }
static const void*  _array_element_pointer( const void* arr, int pos ) { return ( arr && ( pos >= 0 ) && ( pos < array_size( arr ) ) ) ? ((const void* const*)arr)[pos] : 0; }
static void         _array_set_element_pointer( const void** arr, int pos, void* ptr ) { arr[pos] = ptr; }


static NOINLINE void lua_load_foundation_builtins( lua_State* state )
{
	lua_t* env = lua_from_state( state );
	hashmap_t* map = lua_lookup_map( env );
	
	hashmap_insert( map, HASH_SYM_HASH,                   (void*)(uintptr_t)hash );

#if BUILD_ENABLE_DEBUG_LOG
	hashmap_insert( map, HASH_SYM_LOG_DEBUGF,             (void*)(uintptr_t)log_debugf );
#else
	hashmap_insert( map, HASH_SYM_LOG_DEBUGF,             (void*)(uintptr_t)_log_debugf_disabled );
#endif
#if BUILD_ENABLE_LOG
	hashmap_insert( map, HASH_SYM_LOG_INFOF,              (void*)(uintptr_t)log_infof );
	hashmap_insert( map, HASH_SYM_LOG_WARNF,              (void*)(uintptr_t)log_warnf );
	hashmap_insert( map, HASH_SYM_LOG_ERRORF,             (void*)(uintptr_t)log_errorf );
	hashmap_insert( map, HASH_SYM_LOG_PANICF,             (void*)(uintptr_t)log_panicf );
#else
	hashmap_insert( map, HASH_SYM_LOG_INFOF,              (void*)(uintptr_t)_log_debugf_disabled );
	hashmap_insert( map, HASH_SYM_LOG_WARNF,              (void*)(uintptr_t)_log_debugf_disabled );
	hashmap_insert( map, HASH_SYM_LOG_ERRORF,             (void*)(uintptr_t)_log_debugf_disabled );
	hashmap_insert( map, HASH_SYM_LOG_PANICF,             (void*)(uintptr_t)_log_debugf_disabled );
#endif
	hashmap_insert( map, HASH_SYM_LOG_ENABLE_PREFIX,      (void*)(uintptr_t)log_enable_prefix );
	hashmap_insert( map, HASH_SYM_LOG_ENABLE_STDOUT,      (void*)(uintptr_t)log_enable_stdout );
	hashmap_insert( map, HASH_SYM_LOG_SET_SUPPRESS,       (void*)(uintptr_t)log_set_suppress );
	hashmap_insert( map, HASH_SYM_LOG_SUPPRESS,           (void*)(uintptr_t)log_suppress );

	hashmap_insert( map, HASH_SYM_ARRAY_ALLOCATE_POINTER,     (void*)(uintptr_t)_array_allocate_pointer );
	hashmap_insert( map, HASH_SYM_ARRAY_SIZE,                 (void*)(uintptr_t)_array_size );
	hashmap_insert( map, HASH_SYM_ARRAY_ELEMENT_POINTER,      (void*)(uintptr_t)_array_element_pointer );
	hashmap_insert( map, HASH_SYM_ARRAY_SET_ELEMENT_POINTER,  (void*)(uintptr_t)_array_set_element_pointer );
	hashmap_insert( map, HASH_SYM_ARRAY_DEALLOCATE,           (void*)(uintptr_t)_array_deallocate );
	
	hashmap_insert( map, HASH_SYM_ERROR,                  (void*)(uintptr_t)error );
	hashmap_insert( map, HASH_SYM_ERROR_REPORT,           (void*)(uintptr_t)error_report );
#if BUILD_ENABLE_ERROR_CONTEXT
	hashmap_insert( map, HASH_SYM_ERROR_CONTEXT_PUSH,     (void*)(uintptr_t)_error_context_push );
	hashmap_insert( map, HASH_SYM_ERROR_CONTEXT_POP,      (void*)(uintptr_t)_error_context_pop );
#else
	hashmap_insert( map, HASH_SYM_ERROR_CONTEXT_PUSH,     (void*)(uintptr_t)_error_context_push_disabled );
	hashmap_insert( map, HASH_SYM_ERROR_CONTEXT_POP,      (void*)(uintptr_t)_error_context_pop_disabled );
#endif

	hashmap_insert( map, HASH_SYM_STRING_DEALLOCATE,                          (void*)(uintptr_t)string_deallocate );
	hashmap_insert( map, HASH_SYM_STRING_LENGTH,                              (void*)(uintptr_t)string_length );
	hashmap_insert( map, HASH_SYM_STRING_GLYPHS,                              (void*)(uintptr_t)string_glyphs );
	hashmap_insert( map, HASH_SYM_STRING_MERGE,                               (void*)(uintptr_t)string_merge );
	hashmap_insert( map, HASH_SYM_STRING_EXPLODE,                             (void*)(uintptr_t)string_explode );
	hashmap_insert( map, HASH_SYM_STRING_FROM_UINT128_STATIC,                 (void*)(uintptr_t)string_from_uint128_static );
	hashmap_insert( map, HASH_SYM_STRING_FROM_TIME_STATIC,                    (void*)(uintptr_t)string_from_time_static );
	hashmap_insert( map, HASH_SYM_STRING_FROM_UUID_STATIC,                    (void*)(uintptr_t)string_from_uuid_static );
	hashmap_insert( map, HASH_SYM_STRING_TO_UINT64,                           (void*)(uintptr_t)string_to_uint64 );
	hashmap_insert( map, HASH_SYM_STRING_TO_UINT128,                          (void*)(uintptr_t)string_to_uint128 );
	hashmap_insert( map, HASH_SYM_STRING_TO_UUID,                             (void*)(uintptr_t)string_to_uint128 );
	hashmap_insert( map, HASH_SYM_STRING_IS_VALID_EMAIL_ADDRESS,              (void*)(uintptr_t)string_is_valid_email_address );

	hashmap_insert( map, HASH_SYM_ENVIRONMENT_COMMAND_LINE,                   (void*)(uintptr_t)environment_command_line );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_EXECUTABLE_NAME,                (void*)(uintptr_t)environment_executable_name );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_EXECUTABLE_DIRECTORY,           (void*)(uintptr_t)environment_executable_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_INITIAL_WORKING_DIRECTORY,      (void*)(uintptr_t)environment_initial_working_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_CURRENT_WORKING_DIRECTORY,      (void*)(uintptr_t)environment_current_working_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_SET_CURRENT_WORKING_DIRECTORY,  (void*)(uintptr_t)environment_set_current_working_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_HOME_DIRECTORY,                 (void*)(uintptr_t)environment_home_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_TEMPORARY_DIRECTORY,            (void*)(uintptr_t)environment_temporary_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_ENVIRONMENT_VARIABLE,           (void*)(uintptr_t)environment_variable );
	
	hashmap_insert( map, HASH_SYM_STREAM_OPEN,                    (void*)(uintptr_t)stream_open );
	hashmap_insert( map, HASH_SYM_STREAM_CLONE,                   (void*)(uintptr_t)stream_clone );
	hashmap_insert( map, HASH_SYM_STREAM_DEALLOCATE,              (void*)(uintptr_t)stream_deallocate );
	hashmap_insert( map, HASH_SYM_STREAM_TELL,                    (void*)(uintptr_t)stream_tell );
	hashmap_insert( map, HASH_SYM_STREAM_SEEK,                    (void*)(uintptr_t)stream_seek );
	hashmap_insert( map, HASH_SYM_STREAM_EOS,                     (void*)(uintptr_t)stream_eos );
	hashmap_insert( map, HASH_SYM_STREAM_SET_BYTEORDER,           (void*)(uintptr_t)stream_set_byteorder );
	hashmap_insert( map, HASH_SYM_STREAM_SET_BINARY,              (void*)(uintptr_t)stream_set_binary );
	hashmap_insert( map, HASH_SYM_STREAM_IS_SEQUENTIAL,           (void*)(uintptr_t)stream_is_sequential );
	hashmap_insert( map, HASH_SYM_STREAM_IS_RELIABLE,             (void*)(uintptr_t)stream_is_reliable );
	hashmap_insert( map, HASH_SYM_STREAM_IS_INORDER,              (void*)(uintptr_t)stream_is_inorder );
	hashmap_insert( map, HASH_SYM_STREAM_IS_SWAPPED,              (void*)(uintptr_t)stream_is_swapped );
	hashmap_insert( map, HASH_SYM_STREAM_BYTEORDER,               (void*)(uintptr_t)stream_byteorder );
	hashmap_insert( map, HASH_SYM_STREAM_LAST_MODIFIED,           (void*)(uintptr_t)stream_last_modified );
	hashmap_insert( map, HASH_SYM_STREAM_READ,                    (void*)(uintptr_t)stream_read );
	hashmap_insert( map, HASH_SYM_STREAM_READ_LINE,               (void*)(uintptr_t)stream_read_line );
	hashmap_insert( map, HASH_SYM_STREAM_SIZE,                    (void*)(uintptr_t)stream_size );
	hashmap_insert( map, HASH_SYM_STREAM_DETERMINE_BINARY_MODE,   (void*)(uintptr_t)stream_determine_binary_mode );
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
	hashmap_insert( map, HASH_SYM_STREAM_BUFFER_READ,             (void*)(uintptr_t)stream_buffer_read );
	hashmap_insert( map, HASH_SYM_STREAM_AVAILABLE_READ,          (void*)(uintptr_t)stream_available_read );
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
	hashmap_insert( map, HASH_SYM_STREAM_TRUNCATE,                (void*)(uintptr_t)stream_truncate );
	hashmap_insert( map, HASH_SYM_STREAM_FLUSH,                   (void*)(uintptr_t)stream_flush );
	hashmap_insert( map, HASH_SYM_STREAM_OPEN_STDOUT,             (void*)(uintptr_t)stream_open_stdout );
	hashmap_insert( map, HASH_SYM_STREAM_OPEN_STDERR,             (void*)(uintptr_t)stream_open_stderr );
	hashmap_insert( map, HASH_SYM_STREAM_OPEN_STDIN,              (void*)(uintptr_t)stream_open_stdin );

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
	hashmap_insert( map, HASH_SYM_SYSTEM_PROCESS_EVENTS,          (void*)(uintptr_t)system_process_events );
	hashmap_insert( map, HASH_SYM_SYSTEM_MESSAGE_BOX,             (void*)(uintptr_t)system_message_box );
	hashmap_insert( map, HASH_SYM_SYSTEM_EVENT_STREAM,            (void*)(uintptr_t)system_event_stream );
	hashmap_insert( map, HASH_SYM_SYSTEM_POST_EVENT,              (void*)(uintptr_t)system_post_event );

	hashmap_insert( map, HASH_SYM_BASE64_ENCODE,                  (void*)(uintptr_t)base64_encode );
	hashmap_insert( map, HASH_SYM_BASE64_DECODE,                  (void*)(uintptr_t)base64_decode );
 
	hashmap_insert( map, HASH_SYM_BLOWFISH_ALLOCATE,              (void*)(uintptr_t)blowfish_allocate );
	hashmap_insert( map, HASH_SYM_BLOWFISH_DEALLOCATE,            (void*)(uintptr_t)blowfish_deallocate );
	hashmap_insert( map, HASH_SYM_BLOWFISH_INITIALIZE,            (void*)(uintptr_t)blowfish_initialize );
	hashmap_insert( map, HASH_SYM_BLOWFISH_ENCRYPT,               (void*)(uintptr_t)blowfish_encrypt );
	hashmap_insert( map, HASH_SYM_BLOWFISH_DECRYPT,               (void*)(uintptr_t)blowfish_decrypt );

	hashmap_insert( map, HASH_SYM_CONFIG_BOOL,                    (void*)(uintptr_t)config_bool );
	hashmap_insert( map, HASH_SYM_CONFIG_INT,                     (void*)(uintptr_t)config_int );
	hashmap_insert( map, HASH_SYM_CONFIG_REAL,                    (void*)(uintptr_t)config_real );
	hashmap_insert( map, HASH_SYM_CONFIG_STRING,                  (void*)(uintptr_t)config_string );
	hashmap_insert( map, HASH_SYM_CONFIG_STRING_HASH,             (void*)(uintptr_t)config_string_hash );
	hashmap_insert( map, HASH_SYM_CONFIG_SET_BOOL,                (void*)(uintptr_t)config_set_bool );
	hashmap_insert( map, HASH_SYM_CONFIG_SET_INT,                 (void*)(uintptr_t)config_set_int );
	hashmap_insert( map, HASH_SYM_CONFIG_SET_REAL,                (void*)(uintptr_t)config_set_real );
	hashmap_insert( map, HASH_SYM_CONFIG_SET_STRING,              (void*)(uintptr_t)config_set_string );
	hashmap_insert( map, HASH_SYM_CONFIG_LOAD,                    (void*)(uintptr_t)config_load );
	hashmap_insert( map, HASH_SYM_CONFIG_PARSE,                   (void*)(uintptr_t)config_parse );
	hashmap_insert( map, HASH_SYM_CONFIG_WRITE,                   (void*)(uintptr_t)config_write );
	hashmap_insert( map, HASH_SYM_CONFIG_PARSE_COMMANDLINE,       (void*)(uintptr_t)config_parse_commandline );

	hashmap_insert( map, HASH_SYM_EVENT_POST,                     (void*)(uintptr_t)event_post );
	hashmap_insert( map, HASH_SYM_EVENT_NEXT,                     (void*)(uintptr_t)event_next );
	hashmap_insert( map, HASH_SYM_EVENT_PAYLOAD_SIZE,             (void*)(uintptr_t)event_payload_size );
	hashmap_insert( map, HASH_SYM_EVENT_STREAM_ALLOCATE,          (void*)(uintptr_t)event_stream_allocate );
	hashmap_insert( map, HASH_SYM_EVENT_STREAM_DEALLOCATE,        (void*)(uintptr_t)event_stream_deallocate );
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
	hashmap_insert( map, HASH_SYM_FS_FILES,                       (void*)(uintptr_t)fs_files );
	hashmap_insert( map, HASH_SYM_FS_SUBDIRS,                     (void*)(uintptr_t)fs_subdirs );
	hashmap_insert( map, HASH_SYM_FS_MONITOR,                     (void*)(uintptr_t)fs_monitor );
	hashmap_insert( map, HASH_SYM_FS_UNMONITOR,                   (void*)(uintptr_t)fs_unmonitor );
	hashmap_insert( map, HASH_SYM_FS_TEMPORARY_FILE,              (void*)(uintptr_t)fs_temporary_file );
	hashmap_insert( map, HASH_SYM_FS_POST_EVENT,                  (void*)(uintptr_t)fs_post_event );
	hashmap_insert( map, HASH_SYM_FS_EVENT_STREAM,                (void*)(uintptr_t)fs_event_stream );

	hashmap_insert( map, HASH_SYM_LIBRARY_LOAD,                   (void*)(uintptr_t)library_load );
	hashmap_insert( map, HASH_SYM_LIBRARY_UNLOAD,                 (void*)(uintptr_t)library_unload );
	hashmap_insert( map, HASH_SYM_LIBRARY_SYMBOL,                 (void*)(uintptr_t)library_symbol );
	hashmap_insert( map, HASH_SYM_LIBRARY_NAME,                   (void*)(uintptr_t)library_name );
	hashmap_insert( map, HASH_SYM_LIBRARY_VALID,                  (void*)(uintptr_t)library_valid );

	hashmap_insert( map, HASH_SYM_MD5_ALLOCATE,                   (void*)(uintptr_t)md5_allocate );
	hashmap_insert( map, HASH_SYM_MD5_DEALLOCATE,                 (void*)(uintptr_t)md5_deallocate );
	hashmap_insert( map, HASH_SYM_MD5_INITIALIZE,                 (void*)(uintptr_t)md5_initialize );
	hashmap_insert( map, HASH_SYM_MD5_DIGEST,                     (void*)(uintptr_t)md5_digest );
	hashmap_insert( map, HASH_SYM_MD5_DIGEST_RAW,                 (void*)(uintptr_t)md5_digest_raw );
	hashmap_insert( map, HASH_SYM_MD5_DIGEST_FINALIZE,            (void*)(uintptr_t)md5_digest_finalize );
	hashmap_insert( map, HASH_SYM_MD5_GET_DIGEST,                 (void*)(uintptr_t)md5_get_digest );
	hashmap_insert( map, HASH_SYM_MD5_GET_DIGEST_RAW,             (void*)(uintptr_t)md5_get_digest_raw );

	hashmap_insert( map, HASH_SYM_PIPE_ALLOCATE,                  (void*)(uintptr_t)pipe_allocate );
	hashmap_insert( map, HASH_SYM_PIPE_CLOSE_READ,                (void*)(uintptr_t)pipe_close_read );
	hashmap_insert( map, HASH_SYM_PIPE_CLOSE_WRITE,               (void*)(uintptr_t)pipe_close_write );

	hashmap_insert( map, HASH_SYM_PROCESS_ALLOCATE,               (void*)(uintptr_t)process_allocate );
	hashmap_insert( map, HASH_SYM_PROCESS_DEALLOCATE,             (void*)(uintptr_t)process_deallocate );
	hashmap_insert( map, HASH_SYM_PROCESS_SET_WORKING_DIRECTORY,  (void*)(uintptr_t)process_set_working_directory );
	hashmap_insert( map, HASH_SYM_PROCESS_SET_EXECUTABLE_PATH,    (void*)(uintptr_t)process_set_executable_path );
	hashmap_insert( map, HASH_SYM_PROCESS_SET_ARGUMENTS,          (void*)(uintptr_t)process_set_arguments );
	hashmap_insert( map, HASH_SYM_PROCESS_SET_FLAGS,              (void*)(uintptr_t)process_set_flags );
	hashmap_insert( map, HASH_SYM_PROCESS_SPAWN,                  (void*)(uintptr_t)process_spawn );
	hashmap_insert( map, HASH_SYM_PROCESS_STDOUT,                 (void*)(uintptr_t)process_stdout );
	hashmap_insert( map, HASH_SYM_PROCESS_STDIN,                  (void*)(uintptr_t)process_stdin );
	hashmap_insert( map, HASH_SYM_PROCESS_WAIT,                   (void*)(uintptr_t)process_wait );
	hashmap_insert( map, HASH_SYM_PROCESS_SET_VERB,               (void*)(uintptr_t)process_set_verb );
	hashmap_insert( map, HASH_SYM_PROCESS_EXIT_CODE,              (void*)(uintptr_t)process_exit_code );
	hashmap_insert( map, HASH_SYM_SET_EXIT_CODE,                  (void*)(uintptr_t)process_set_exit_code );
	hashmap_insert( map, HASH_SYM_PROCESS_EXIT,                   (void*)(uintptr_t)process_exit );

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

	hashmap_insert( map, HASH_SYM_RINGBUFFER_ALLOCATE,            (void*)(uintptr_t)ringbuffer_allocate );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_DEALLOCATE,          (void*)(uintptr_t)ringbuffer_deallocate );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_SIZE,                (void*)(uintptr_t)ringbuffer_size );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_RESET,               (void*)(uintptr_t)ringbuffer_reset );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_READ,                (void*)(uintptr_t)ringbuffer_read );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_WRITE,               (void*)(uintptr_t)ringbuffer_write );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_TOTAL_READ,          (void*)(uintptr_t)ringbuffer_total_read );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_TOTAL_WRITTEN,       (void*)(uintptr_t)ringbuffer_total_written );
	hashmap_insert( map, HASH_SYM_RINGBUFFER_STREAM_ALLOCATE,     (void*)(uintptr_t)ringbuffer_stream_allocate );

	hashmap_insert( map, HASH_SYM_TIME_CURRENT,                   (void*)(uintptr_t)time_current );
	hashmap_insert( map, HASH_SYM_TIME_DIFF,                      (void*)(uintptr_t)time_diff );
	hashmap_insert( map, HASH_SYM_TIME_ELAPSED,                   (void*)(uintptr_t)time_elapsed );
	hashmap_insert( map, HASH_SYM_TIME_ELAPSED_TICKS,             (void*)(uintptr_t)time_elapsed_ticks );
	hashmap_insert( map, HASH_SYM_TIME_TICKS_PER_SECOND,          (void*)(uintptr_t)time_ticks_per_second );
	hashmap_insert( map, HASH_SYM_TIME_TICKS_TO_SECONDS,          (void*)(uintptr_t)time_ticks_to_seconds );
	hashmap_insert( map, HASH_SYM_TIME_STARTUP,                   (void*)(uintptr_t)time_startup );
	hashmap_insert( map, HASH_SYM_TIME_SYSTEM,                    (void*)(uintptr_t)time_system );
}


int lua_load_foundation( lua_State* state )
{
	static unsigned char bytecode[] = {
		#include "bind.foundation.hex"
	};

	lua_readbuffer_t read_buffer = {
		.buffer = bytecode,
		.size   = sizeof( bytecode ),
		.offset = 0
	};

	log_debugf( HASH_LUA, "Loading foundation built-ins (%u bytes of bytecode)", read_buffer.size );

	lua_load_foundation_builtins( state );
	
	if( lua_load( state, lua_read_buffer, &read_buffer, "=eval" ) != 0 )
	{
		log_errorf( HASH_LUA, ERROR_INTERNAL_FAILURE, "Lua load failed (foundation): %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		return 0;
	}

	if( lua_pcall( state, 0, 0, 0 ) != 0 )
	{
		log_errorf( HASH_LUA, ERROR_INTERNAL_FAILURE, "Lua pcall failed (foundation): %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		return 0;
	}
	
	return 0;
}

	
