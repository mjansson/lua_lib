/* main.c  -  Lua dump tool for lua library  -  MIT License  -  2013 Mattias Jansson / Rampant Pixels
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

#include "errorcodes.h"

#include <stdio.h>


typedef struct
{
	char*               input_file;
	char*               output_file;

	stream_t*           input_stream;
	stream_t*           output_stream;
	
	lua_environment_t*  env;

	bool                hex;

	char*               bytecode;
	unsigned int        bytecode_size;
} luadump_t;
	

static luadump_t  luadump_parse_command_line( const char* const* cmdline );
static void       luadump_print_usage( void );
static int        luadump_load_jitbc( lua_environment_t* env );


typedef struct _lua_readstream
{
	stream_t*             stream;
	char                  chunk[128];
} lua_readstream_t;


static NOINLINE const char* lua_read_stream( lua_State* state, void* user_data, size_t* size )
{
	lua_readstream_t* read = (lua_readstream_t*)user_data;

	if( stream_eos( read->stream ) )
	{
		if( *size )
			*size = 0;
		return 0;
	}
	
	uint64_t num_read = stream_read( read->stream, read->chunk, 128 );

	if( size )
		*size = (size_t)num_read;

	return num_read ? read->chunk : 0;
}


static NOINLINE int lua_dump_writer( lua_State* state, const void* buffer, size_t size, void* user_data )
{
	luadump_t* dump = user_data;

	if( size <= 0 )
		return 0;
	
	if( !dump->bytecode )
		dump->bytecode = ( dump->bytecode_size ? memory_reallocate( dump->bytecode, dump->bytecode_size + size, 0 ) : memory_allocate( size, 0, MEMORY_PERSISTENT ) );

	memcpy( dump->bytecode + dump->bytecode_size, buffer, size );
	dump->bytecode_size += size;
	
	if( dump->hex )
	{
		char* line = memory_allocate( size * 6 + 2, 0, MEMORY_TEMPORARY );
		for( size_t i = 0; i < size; ++i )
			string_format_buffer( line + ( i * 6 ), 7, "0x%02x, ", ((const unsigned char*)buffer)[i] );
		if( dump->output_stream )
		{
			stream_write_string( dump->output_stream, line );
			stream_write_endl( dump->output_stream );
		}
		else
		{
			log_suppress( ERRORLEVEL_DEBUG );
			log_infof( line );
			log_suppress( ERRORLEVEL_INFO );
		}
		memory_deallocate( line );
	}
	else
	{
		if( dump->output_stream )
			stream_write( dump->output_stream, buffer, size );
	}

	return 0;
}


LUA_API int lua_load( lua_State *L, lua_Reader reader, void *dt, const char *chunkname);
LUA_API int lua_dump( lua_State *L, lua_Writer writer, void *data);
LUA_API int lua_pcall( lua_State *L, int nargs, int nresults, int errfunc );


int main_initialize( void )
{
	int ret = 0;

	log_enable_prefix( false );
	log_suppress( ERRORLEVEL_INFO );
	
	application_t application = {0};
	application.name = "luadump";
	application.short_name = "luadump";
	application.config_dir = "luadump";
	application.flags = APPLICATION_UTILITY;

	if( ( ret = foundation_initialize( memory_system_malloc(), application ) ) < 0 )
		return ret;

	config_set_int( HASH_FOUNDATION, HASH_TEMPORARY_MEMORY, 1024 * 1024 );

	return 0;
}


int main_run( void* main_arg )
{
	int result = LUADUMP_RESULT_OK;
	luadump_t dump = luadump_parse_command_line( environment_command_line() );

	lua_State* state = 0;
	
	if( !string_length( dump.input_file ) )
		return LUADUMP_RESULT_OK;
	
	dump.env = lua_environment_allocate();
	state = lua_state_from_env( dump.env );
	
	/*if( ( result = luadump_load_jitbc( dump.env ) ) != LUADUMP_RESULT_OK )
		goto exit;

	lua_eval( dump.env, "jit.bc.dump(log.debug, nil, true)" );*/

	dump.input_stream = stream_open( dump.input_file, STREAM_IN );
	if( !dump.input_stream || stream_eos( dump.input_stream ) )
	{
		result = LUADUMP_RESULT_UNABLE_TO_OPEN_INPUT_FILE;
		goto exit;
	}

	if( dump.output_file )
	{
		dump.output_stream = stream_open( dump.output_file, STREAM_OUT | ( !dump.hex ? STREAM_BINARY : 0 ) );
		if( !dump.output_stream )
		{
			result = LUADUMP_RESULT_UNABLE_TO_OPEN_OUTPUT_FILE;
			goto exit;
		}
	}
	
	lua_readstream_t read_stream = {
		.stream = dump.input_stream,
	};

	if( lua_load( state, lua_read_stream, &read_stream, "=eval" ) != 0 )
	{
		log_errorf( ERRORLEVEL_ERROR, ERROR_SCRIPT, "Lua load failed: %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		result = LUADUMP_RESULT_FAILED_EVAL;
		goto exit;
	}

	lua_dump( state, lua_dump_writer, &dump );

	//lua_pop( state, 1 );
	if( lua_pcall( state, 0, 0, 0 ) != 0 )
	{
		log_errorf( ERRORLEVEL_ERROR, ERROR_SCRIPT, "Lua pcall failed: %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
	}
	else
	{
		log_infof( "Lua bytecode dump successful" );
		//lua_call_string( dump.env, "foundation.log.info", "Testing log output" );
	}
	
exit:

	memory_deallocate( dump.bytecode );
	
	stream_deallocate( dump.input_stream );
	stream_deallocate( dump.output_stream );
	
	lua_environment_deallocate( dump.env );

	string_deallocate( dump.input_file );
	string_deallocate( dump.output_file );
	
	return result;
}


void main_shutdown( void )
{
	foundation_shutdown();
}


static luadump_t luadump_parse_command_line( const char* const* cmdline )
{
	int arg, asize;
	bool display_help = false;

	luadump_t dump = {0};
	
	error_context_push( "parsing command line", "" );
	for( arg = 1, asize = array_size( cmdline ); arg < asize; ++arg )
	{
		if( string_equal( cmdline[arg], "--help" ) )
			display_help = true;
		else if( string_equal( cmdline[arg], "--hex" ) )
			dump.hex = true;
		else if( string_equal( cmdline[arg], "--output" ) )
		{
			if( arg < asize - 1 )
			{
				++arg;
				string_deallocate( dump.output_file );
				dump.output_file = string_clone( cmdline[arg] );
			}
		}
		else if( string_equal( cmdline[arg], "--" ) )
			break; //Stop parsing cmdline options
		else
		{
			string_deallocate( dump.input_file );
			dump.input_file = string_clone( cmdline[arg] );
		}
	}
	error_context_pop();

	if( !string_length( dump.input_file ) )
		display_help = true;
	
	if( display_help )
		luadump_print_usage();

	return dump;
}


#if 0

#include "lua/luajit/src/luajit.h"

LUA_API int luaopen_io( struct lua_State *L );
#define LUA_IOLIBNAME  "io"

static const char jit_vmdef[] = {
#include "vmdef.lua.hex"
	0
};

static const char jit_bc[] = {
#include "bc.lua.hex"
	0
};

LUA_API lua_environment_t*      lua_env_from_state( struct lua_State* state );
LUA_API struct lua_State*       lua_state_from_env( lua_environment_t* env );


static int luadump_load_jitbc( lua_environment_t* env )
{
	//Load IO lib which is disabled in init
	struct lua_State* state = lua_state_from_env( env );

	lua_pushcfunction( state, luaopen_io );
	lua_pushstring( state, LUA_IOLIBNAME );
	lua_call( state, 1, 0 );

	log_debugf( "Define jit.vmdef module" );
	if( lua_eval( env, jit_vmdef ) != LUA_OK )
		return LUADUMP_RESULT_UNABLE_TO_LOAD_JITLIB_VMDEF;

	log_debugf( "Define jit.bc module" );
	if( lua_eval( env, jit_bc ) != LUA_OK )
		return LUADUMP_RESULT_UNABLE_TO_LOAD_JITLIB_BC;

	return LUADUMP_RESULT_OK;
}

#else

static int luadump_load_jitbc( lua_environment_t* env )
{
	return LUADUMP_RESULT_OK;
}

#endif


static void luadump_print_usage( void )
{
	log_suppress( ERRORLEVEL_DEBUG );
	log_infof( 
		"luadump usage:\n"
		"  luadump [--output <filename>] [--hex] [--help] file\n"
		"    Optional arguments:\n"
		"      --output <filename>          Output to <filename> instead of stdout\n"
		"      --hex                        Output in hex format\n"
		"      --help                       Show this message"
	);
	log_suppress( ERRORLEVEL_INFO );
}

