/* main.c  -  Lua interpreter for lua library  -  MIT License  -  2013 Mattias Jansson / Rampant Pixels
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

#include <foundation/foundation.h>
#include <lua/lua.h>
#include <lua/bind.h>
#include <lua/read.h>
#include <lua/hashstrings.h>
#include <lua/luajit.h>

#include "errorcodes.h"

#include <stdio.h>


typedef struct
{
	char*               input_file;

	lua_t*              env;

	error_level_t       suppress_level;
} luainstance_t;

static bool           _lua_terminate = false;
	

static luainstance_t  _lua_parse_command_line( const char* const* cmdline );
static void           _lua_print_usage( void );
static int            _lua_process_file( lua_t* lua, const char* filename );
static int            _lua_interpreter( lua_t* lua );
//static int            _lua_load_jitbc( lua_t* env );


LUA_API int lua_load( lua_State *L, lua_Reader reader, void *dt, const char *chunkname);
LUA_API int lua_pcall( lua_State *L, int nargs, int nresults, int errfunc );


static void* event_thread( object_t thread, void* arg )
{
	event_block_t* block;
	event_t* event = 0;

	while( !thread_should_terminate( thread ) )
	{
		block = event_stream_process( system_event_stream() );
		event = 0;

		while( ( event = event_next( block, event ) ) )
		{
			switch( event->id )
			{
				case FOUNDATIONEVENT_TERMINATE:
					//process_exit( LUA_RESULT_ABORTED );
					_lua_terminate = true;
					break;

				default:
					break;
			}
		}

		thread_sleep( 10 );
	}

	return 0;
}


int main_initialize( void )
{
	int ret = 0;

	log_enable_prefix( false );
	log_set_suppress( 0, ERRORLEVEL_DEBUG );
	log_set_suppress( HASH_LUA, ERRORLEVEL_DEBUG );
	
	application_t application = {0};
	application.name = "lua";
	application.short_name = "lua";
	application.config_dir = "lua";
	application.flags = APPLICATION_UTILITY;

	if( ( ret = foundation_initialize( memory_system_malloc(), application ) ) < 0 )
		return ret;

	config_set_int( HASH_FOUNDATION, HASH_TEMPORARY_MEMORY, 1024 * 1024 );

	return 0;
}


int main_run( void* main_arg )
{	
	int result = LUA_RESULT_OK;
	luainstance_t instance = _lua_parse_command_line( environment_command_line() );
	
	lua_State* state = 0;
	
	object_t eventthread = thread_create( event_thread, "event_thread", THREAD_PRIORITY_NORMAL, 0 );
	thread_start( eventthread, 0 );

	instance.env = lua_allocate();
	state = lua_state( instance.env );
	
	if( instance.input_file )
		result = _lua_process_file( instance.env, instance.input_file );
	else
		result = _lua_interpreter( instance.env );
	
exit:

	thread_terminate( eventthread );
	thread_destroy( eventthread );

	lua_deallocate( instance.env );
	string_deallocate( instance.input_file );
	
	while( thread_is_running( eventthread ) )
		thread_sleep( 10 );

	return result;
}


void main_shutdown( void )
{
	foundation_shutdown();
}


static int _lua_interpreter( lua_t* lua )
{
	stream_t* in = stream_open_stdin();
	stream_t* out = stream_open_stdout();

	char* entry = 0;
	char* collated = 0;
	lua_readstring_t read_string = {0};
	lua_State* state = lua_state( lua );

	int status;

	log_enable_prefix( false );

	do
	{
		stream_write_string( out, collated ? "  >> " : "lua> " );
		stream_flush( out );

		entry = stream_read_line( in, '\n' );
		if( entry && string_length( entry ) )
		{
			if( collated )
			{
				collated = string_append( string_append( collated, "\n" ), entry );
				read_string.string = collated;
				read_string.size = string_length( collated );
			}
			else
			{
				read_string.string = entry;
				read_string.size = string_length( entry );
			}

			if( ( status = lua_load( state, lua_read_string, &read_string, "=eval" ) ) != 0 )
			{
				bool continued = false;
				const char* msg = lua_tostring( state, -1 );
				if( status == LUA_ERRSYNTAX )
				{
					if( string_equal( msg + strlen( msg ) - 7, "'<eof>'" ) )
						continued = true;
				}
				if( continued )
				{
					if( !collated )
					{
						collated = entry;
						entry = 0;
					}
				}
				else
				{
					if( string_equal_substr( msg, "eval:", 5 ) )
						msg += 5;
					log_errorf( HASH_LUA, ERROR_SCRIPT, "%s", msg );
				}
				lua_pop( state, 1 );
			}
			else
			{
				if( collated )
					string_deallocate( collated );
				collated = 0;

				if( lua_pcall( state, 0, 0, 0 ) != 0 )
				{
					const char* msg = lua_tostring( state, -1 );
					if( string_equal_substr( msg, "eval:", 5 ) )
						msg += 5;
					log_errorf( HASH_LUA, ERROR_SCRIPT, "%s", msg );
					lua_pop( state, 1 );
				}
			}
		}

		string_deallocate( entry );

	} while( !_lua_terminate && !stream_eos( in ) );

	stream_deallocate( in );
	stream_deallocate( out );

	return LUA_RESULT_OK;
}


static int _lua_process_file( lua_t* lua, const char* filename )
{
	int result = LUA_RESULT_OK;
	stream_t* stream = stream_open( filename, STREAM_IN );
	if( !stream || stream_eos( stream ) )
	{
		result = LUA_RESULT_UNABLE_TO_OPEN_INPUT_FILE;
		goto exit;
	}
	
	lua_readstream_t read_stream = {
		.stream = stream
	};

	lua_State* state = lua_state( lua );

	if( lua_load( state, lua_read_stream, &read_stream, "=eval" ) != 0 )
	{
		log_errorf( HASH_LUA, ERROR_INTERNAL_FAILURE, "Lua load failed: %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		result = LUA_RESULT_FAILED_EVAL;
		goto exit;
	}

	if( lua_pcall( state, 0, 0, 0 ) != 0 )
	{
		log_errorf( HASH_LUA, ERROR_INTERNAL_FAILURE, "Lua pcall failed: %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
	}
	else
	{
		log_info( HASH_LUA, "Lua interpretation successful" );
	}

exit:

	stream_deallocate( stream );

	return result;
}


static luainstance_t _lua_parse_command_line( const char* const* cmdline )
{
	int arg, asize;
	bool display_help = false;

	luainstance_t instance = {0};

	instance.suppress_level = ERRORLEVEL_INFO;
	
	error_context_push( "parsing command line", "" );
	for( arg = 1, asize = array_size( cmdline ); arg < asize; ++arg )
	{
		if( string_equal( cmdline[arg], "--help" ) )
			display_help = true;
		else if( string_equal( cmdline[arg], "--debug" ) )
			instance.suppress_level = ERRORLEVEL_NONE;
		else if( string_equal( cmdline[arg], "--" ) )
			break; //Stop parsing cmdline options
		else
		{
			string_deallocate( instance.input_file );
			instance.input_file = string_clone( cmdline[arg] );
		}
	}
	error_context_pop();

	if( display_help )
		_lua_print_usage();

	return instance;
}


static void _lua_print_usage( void )
{
	log_set_suppress( HASH_LUA, ERRORLEVEL_DEBUG );
	log_info( HASH_LUA,
		"lua usage:\n"
		"  lua [--help] [file]\n"
		"    Optional arguments:\n"
		"      --help           Show this message\n"
		"      <file>           Read <file> instead of stdin\n"
	);
	log_set_suppress( HASH_LUA, ERRORLEVEL_INFO );
}

