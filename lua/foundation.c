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

#include "lua.h"
#include "read.h"
#include "foundation.h"
#include "hashstrings.h"

#include <foundation/log.h>

#undef LUA_API
#define LUA_HAS_LUA_STATE_TYPE

#include "luajit/src/lua.h"


static void _log_debugf_disabled( const char* format, ... ) {}
static void _error_context_push_disabled( const char* name, const char* data ) {}
static void _error_context_pop_disabled() {} 

static NOINLINE void lua_load_foundation_builtins( lua_State* state )
{
	lua_t* env = lua_from_state( state );
	hashmap_t* map = lua_lookup_map( env );
	
#if BUILD_ENABLE_DEBUG_LOG
	hashmap_insert( map, HASH_SYM_LOG_DEBUGF,             (void*)(uintptr_t)log_debugf );
#else
	hashmap_insert( map, HASH_SYM_LOG_DEBUG,              (void*)(uintptr_t)_log_debugf_disabled );
#endif
#if BUILD_ENABLE_LOG
	hashmap_insert( map, HASH_SYM_LOG_INFOF,              (void*)(uintptr_t)log_infof );
#else
	hashmap_insert( map, HASH_SYM_LOG_DEBUG,              (void*)(uintptr_t)_log_debugf_disabled );
#endif
	hashmap_insert( map, HASH_SYM_LOG_WARNF,              (void*)(uintptr_t)log_warnf );
	hashmap_insert( map, HASH_SYM_LOG_ERRORF,             (void*)(uintptr_t)log_errorf );
	hashmap_insert( map, HASH_SYM_LOG_STDOUT,             (void*)(uintptr_t)log_stdout );
	hashmap_insert( map, HASH_SYM_LOG_ENABLE_PREFIX,      (void*)(uintptr_t)log_enable_prefix );
	hashmap_insert( map, HASH_SYM_LOG_SUPPRESS,           (void*)(uintptr_t)log_suppress );

	hashmap_insert( map, HASH_SYM_ERROR,                  (void*)(uintptr_t)error );
	hashmap_insert( map, HASH_SYM_ERROR_REPORT,           (void*)(uintptr_t)error_report );
#if BUILD_ENABLE_ERROR_CONTEXT
	hashmap_insert( map, HASH_SYM_ERROR_CONTEXT_PUSH,     (void*)(uintptr_t)_error_context_push );
	hashmap_insert( map, HASH_SYM_ERROR_CONTEXT_POP,      (void*)(uintptr_t)_error_context_pop );
#else
	hashmap_insert( map, HASH_SYM_ERROR_CONTEXT_PUSH,     (void*)(uintptr_t)_error_context_push_disabled );
	hashmap_insert( map, HASH_SYM_ERROR_CONTEXT_POP,      (void*)(uintptr_t)_error_context_pop_disabled );
#endif

	hashmap_insert( map, HASH_SYM_ENVIRONMENT_COMMAND_LINE,                   (void*)(uintptr_t)environment_command_line );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_EXECUTABLE_NAME,                (void*)(uintptr_t)environment_executable_name );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_EXECUTABLE_DIRECTORY,           (void*)(uintptr_t)environment_executable_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_INITIAL_WORKING_DIRECTORY,      (void*)(uintptr_t)environment_initial_working_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_CURRENT_WORKING_DIRECTORY,      (void*)(uintptr_t)environment_current_working_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_SET_CURRENT_WORKING_DIRECTORY,  (void*)(uintptr_t)environment_set_current_working_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_HOME_DIRECTORY,                 (void*)(uintptr_t)environment_home_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_TEMPORARY_DIRECTORY,            (void*)(uintptr_t)environment_temporary_directory );
	hashmap_insert( map, HASH_SYM_ENVIRONMENT_ENVIRONMENT_VARIABLE,           (void*)(uintptr_t)environment_variable );
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

	log_debugf( "Loading foundation built-ins" );

	lua_load_foundation_builtins( state );
	
	if( lua_load( state, lua_read_buffer, &read_buffer, "=eval" ) != 0 )
	{
		log_errorf( ERROR_SCRIPT, "Lua load failed (foundation): %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		return 0;
	}

	if( lua_pcall( state, 0, 0, 0 ) != 0 )
	{
		log_errorf( ERROR_SCRIPT, "Lua pcall failed (foundation): %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		return 0;
	}
	
	return 0;
}

	
