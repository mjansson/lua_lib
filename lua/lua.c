/* lua.c  -  Lua library  -  MIT License  -  2013 Mattias Jansson / Rampant Pixels
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
#include "hashstrings.h"

#include <foundation/foundation.h>

#include <setjmp.h>

#undef LUA_API
#define LUA_HAS_LUA_STATE_TYPE

#include "luajit/src/lua.h"
#include "luajit/src/lauxlib.h"
#include "luajit/src/lualib.h"
LUA_EXTERN void lj_err_throw( lua_State* L, int errcode );
LUA_EXTERN void lj_clib_set_getsym_builtin( void* (*fn)(lua_State*, const char*) );


typedef enum _lua_command
{
	LUACMD_WAIT = 0,
	LUACMD_LOAD,
	LUACMD_EVAL,
	LUACMD_CALL,
	LUACMD_BIND,
	LUACMD_BIND_INT,
	LUACMD_BIND_VAL
} lua_command_t;

static lua_result_t lua_do_call_custom( lua_environment_t* env, const char* method, lua_arg_t* arg );
static lua_result_t lua_do_bind( lua_environment_t* env, const char* property, lua_command_t cmd, lua_value_t val );
static lua_result_t lua_do_eval( lua_environment_t* env, const char* code );
static lua_result_t lua_do_get( lua_environment_t* env, const char* property );

static void*        lua_lookup_builtin( lua_State* state, const char* sym );

lua_environment_t* lua_env_from_state( lua_State* state );
lua_State* lua_state_from_env( lua_environment_t* env );

#define LUA_CALL_QUEUE_SIZE  1024


typedef struct _lua_op
{
	lua_command_t         cmd;
	const char*           name;
	lua_arg_t             arg;
} lua_op_t;


typedef struct _lua_readstream
{
	stream_t*             stream;
	char                  chunk[128];
} lua_readstream_t;


typedef struct _lua_readbuffer
{
	const void*           buffer;
	unsigned int          size;
	unsigned int          offset;
} lua_readbuffer_t;


typedef struct _lua_readstring
{
	const char*           string;
	unsigned int          size;
} lua_readstring_t;


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


static NOINLINE const char* lua_read_chunked_buffer( lua_State* state, void* user_data, size_t* size )
{	
	lua_readbuffer_t* read = (lua_readbuffer_t*)user_data;

	if( read->offset >= read->size )
	{
		if( *size )
			*size = 0;
		return 0;
	}

	const void* current_chunk = pointer_offset_const( read->buffer, read->offset );
	unsigned int chunk_size = *(const unsigned int*)current_chunk;
	
	read->offset += chunk_size + sizeof( unsigned int );
	if( size )
		*size = (size_t)chunk_size;

	//log_debugf( "Read Lua chunk:\n%s", pointer_offset_const( current_chunk, sizeof( unsigned int ) ) );

	return pointer_offset_const( current_chunk, sizeof( unsigned int ) );
}


static NOINLINE const char* lua_read_buffer( lua_State* state, void* user_data, size_t* size )
{	
	lua_readbuffer_t* read = (lua_readbuffer_t*)user_data;

	if( read->offset >= read->size )
	{
		if( *size )
			*size = 0;
		return 0;
	}

	const void* current_chunk = pointer_offset_const( read->buffer, read->offset );
	unsigned int chunk_size = read->size - read->offset;
	
	read->offset += chunk_size;
	if( size )
		*size = (size_t)chunk_size;

	return current_chunk;
}


static NOINLINE const char* lua_read_string( lua_State* state, void* user_data, size_t* size )
{
	lua_readstring_t* read = (lua_readstring_t*)user_data;

	if( !read->size )
	{
		if( size )
			*size = 0;
		return 0;
	}

	if( size )
		*size = read->size;
	read->size = 0;

	return read->string;
}


static NOINLINE void lua_initialize_builtin_lookup( hashmap_t* map )
{
#if BUILD_ENABLE_DEBUG_LOG
	hashmap_insert( map, HASH_SYM_LOG_DEBUGF,         (void*)(uintptr_t)log_debugf );
#endif
#if BUILD_ENABLE_LOG
	hashmap_insert( map, HASH_SYM_LOG_INFOF,          (void*)(uintptr_t)log_infof );
#endif
	hashmap_insert( map, HASH_SYM_LOG_WARNF,          (void*)(uintptr_t)log_warnf );
	hashmap_insert( map, HASH_SYM_LOG_ERRORF,         (void*)(uintptr_t)log_errorf );
}


static NOINLINE int lua_bind_foundation( lua_State* state )
{
	static unsigned char bytecode[] = {
		#include "bind.foundation.hex"
	};

	lua_readbuffer_t read_buffer = {
		.buffer = bytecode,
		.size   = sizeof( bytecode ),
		.offset = 0
	};

	if( lua_load( state, lua_read_buffer, &read_buffer, "=eval" ) != 0 )
	{
		log_errorf( ERRORLEVEL_ERROR, ERROR_SCRIPT, "Lua load failed (foundation): %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		return 0;
	}

	if( lua_pcall( state, 0, 0, 0 ) != 0 )
	{
		log_errorf( ERRORLEVEL_ERROR, ERROR_SCRIPT, "Lua pcall failed (foundation): %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		return 0;
	}
	
	return 0;
}


static NOINLINE void lua_push_integer( lua_State* state, const char* name, int value )
{
	lua_pushstring( state, name );
	lua_pushinteger( state, value );
	lua_settable( state, -3 );
}


static NOINLINE void lua_push_integer_global( lua_State* state, const char* name, int value )
{
	lua_pushinteger( state, value );
	lua_setglobal( state, name );
}


static NOINLINE void lua_push_number( lua_State* state, const char* name, real value )
{
	lua_pushstring( state, name );
	lua_pushnumber( state, value );
	lua_settable( state, -3 );
}


static NOINLINE void lua_push_number_global( lua_State* state, const char* name, real value )
{
	lua_pushnumber( state, value );
	lua_setglobal( state, name );
}


static NOINLINE void lua_push_method( lua_State* state, const char* name, lua_CFunction fn )
{
	lua_pushstring( state, name );
	lua_pushcclosure( state, fn, 0 );
	lua_settable( state, -3 );
}


static NOINLINE void lua_push_method_global( lua_State* state, const char* name, lua_CFunction fn )
{
	lua_pushcclosure( state, fn, 0 );
	lua_setglobal( state, name );
}



struct _lua_environment
{
	//! Lua state
	lua_State*                        state;

	//! Call depth
	int32_t                           calldepth;

	//! Call queue
	lua_op_t                          queue[LUA_CALL_QUEUE_SIZE];
	
	//! Queue head (protected by execute semaphore)
	uint32_t                          queue_head;

	//! Queue tail
	volatile ALIGN(16) uint32_t       queue_tail;

	//! Execution right
	semaphore_t                       execution_right;
	
	//! Currently executing thread
	uint64_t                          executing_thread;

	//! Execution count
	unsigned int                      executing_count;

	//! Lookup hashmap
	hashmap_t*                        lookup_map;
};


static FORCEINLINE bool lua_has_execution_right( lua_environment_t* env )
{
	return ( env->executing_thread == thread_id() );
}


static NOINLINE bool lua_acquire_execution_right( lua_environment_t* env, bool force )
{
	uint64_t self = thread_id();
	if( env->executing_thread == self )
	{
		++env->executing_count;
		return true;
	}
	if( force )
	{
		semaphore_wait( &env->execution_right );
		env->executing_thread = self;
		FOUNDATION_ASSERT( env->executing_count == 0 );
		++env->executing_count;
		return true;
	}
	if( semaphore_try_wait( &env->execution_right, 0 ) )
	{
		env->executing_thread = self;
		FOUNDATION_ASSERT( env->executing_count == 0 );
		++env->executing_count;
		return true;
	}
	return false;
}


static void lua_release_execution_right( lua_environment_t* env )
{
	FOUNDATION_ASSERT( env->executing_thread == thread_id() );
	FOUNDATION_ASSERT( env->executing_count > 0 );
	if( !--env->executing_count )
	{
		env->executing_thread = 0;
		semaphore_post( &env->execution_right );
	}
}


static void lua_push_op( lua_environment_t* env, lua_op_t* op )
{
	unsigned int ofs, old;
	do
	{
		old = env->queue_tail;
		ofs = old+1;
		if( ofs >= LUA_CALL_QUEUE_SIZE )
			ofs = 0;
	} while( !atomic_cas32( (volatile int*)&env->queue_tail, ofs, old ) );

	//Got slot, copy except command
	memcpy( &env->queue[ofs].name, &op->name, sizeof( char* ) + sizeof( lua_arg_t ) );
	//Now set command, completing insert
	env->queue[ofs].cmd = op->cmd;
}


static void lua_execute_pending( lua_environment_t* env )
{
	profile_begin_block( "lua exec" );

	unsigned int head = env->queue_head;
	while( env->queue[head].cmd != LUACMD_WAIT )
	{
		//Execute
		switch( env->queue[head].cmd )
		{
			case LUACMD_LOAD:
			{
				/*if( !env->queue[head].arg.arg[2].flag )
					_script_do_load_once( env, env->queue[head].arg.arg[0].id, &env->queue[head].arg.arg[1].obj );
				else
				_script_do_load( env, env->queue[head].arg.arg[0].id, &env->queue[head].arg.arg[1].obj );*/
				break;
			}

			case LUACMD_EVAL:
			{
				lua_do_eval( env, env->queue[head].name );
				break;
			}
			
			case LUACMD_CALL:
			{
				lua_do_call_custom( env, env->queue[head].name, &env->queue[head].arg );
				break;				
			}
			
			case LUACMD_BIND:
			case LUACMD_BIND_INT:
			case LUACMD_BIND_VAL:
			{
				lua_do_bind( env, env->queue[head].name, env->queue[head].cmd, env->queue[head].arg.value[0] );
				break;
			}

			default:
				break;
		}

		//Mark as executed
		env->queue[head].cmd = LUACMD_WAIT;
		
		if( ++head == LUA_CALL_QUEUE_SIZE )
			head = 0;
	}
	env->queue_head = head;

	profile_end_block();
}


static FORCEINLINE void lua_run_gc( lua_environment_t* env, int milliseconds )
{
	lua_gc( env->state, LUA_GCSTEP, milliseconds );
}


void* lua_allocator( lua_State* state, void* block, size_t osize, size_t nsize )
{
	//Based on the generic Lua allocation routine
	void* oldblock;
	//global_State* global = state ? G(state) : 0;
	FOUNDATION_ASSERT( (osize == 0) == (block == 0) );
	oldblock = block;
	if( oldblock && nsize ) //Reallocation
	{
		block = memory_reallocate( oldblock, nsize, 0 );
	}
	else if( nsize ) //Allocation
	{
		block = memory_allocate( nsize, 0, MEMORY_PERSISTENT );
	}
	else //Deallocation
	{
		memory_deallocate( oldblock );
		block = 0;
	}
	if( state && block == 0 && nsize > 0 )
		lj_err_throw( state, LUA_ERRMEM );
	FOUNDATION_ASSERT( (nsize == 0) == (block == 0) );
	//if( global )
	//	global->totalbytes = (global->totalbytes - osize) + nsize;
	return block;
}


int lua_panic( lua_State* state )
{
	FOUNDATION_ASSERT_FAILFORMAT( "unprotected error in call to Lua API (%s)", lua_tostring( state, -1 ) );
	log_errorf( ERRORLEVEL_PANIC, ERROR_SCRIPT, "unprotected error in call to Lua API (%s)", lua_tostring( state, -1 ) );
	return 0;
}


lua_environment_t* lua_environment_allocate( void )
{
	lua_environment_t* env = 0;

	//TODO: Add functionality to foundation allocator to be able to meet luajit demands (low 47-bit (?) addresses)
	lua_State* state = lua_newstate( 0, 0 );
	if( !state )
	{
		log_errorf( ERRORLEVEL_ERROR, ERROR_SCRIPT, "Unable to allocate Lua state" );
		return 0;
	}

	lua_atpanic( state, lua_panic );

	lj_clib_set_getsym_builtin( lua_lookup_builtin );
	
	//Disable automagic gc
	lua_gc( state, LUA_GCCOLLECT, 0 );

	env = memory_allocate_zero( sizeof( lua_environment_t ), 0, MEMORY_PERSISTENT );
	
	lua_pushlightuserdata( state, env );
	lua_setglobal( state, "__environment" );

	env->state = state;
	env->calldepth = 0;
	env->queue_head = 0;
	env->queue_tail = 0;
	env->lookup_map = hashmap_allocate( 0, 0 );

	semaphore_initialize( &env->execution_right, 1 );

	unsigned int stacksize = lua_gettop( state );

	lua_initialize_builtin_lookup( env->lookup_map );
	
	//Libraries
	log_debugf( "Loading Lua built-ins" );
	luaL_openlibs( state );

	//Foundation bindings
	lua_bind_foundation( state );

	lua_pop( state, lua_gettop( state ) - stacksize );

	return env;
}


void lua_environment_deallocate( lua_environment_t* env )
{
	if( !env )
		return;

	FOUNDATION_ASSERT( env->calldepth == 0 );
	FOUNDATION_ASSERT( env->state );

	lua_gc( env->state, LUA_GCCOLLECT, 0 );
	lua_close( env->state );

	semaphore_destroy( &env->execution_right );

	hashmap_deallocate( env->lookup_map );
	
	memory_deallocate( env );
}


lua_result_t lua_call_custom( lua_environment_t* env, const char* method, lua_arg_t* arg )
{
	if( !lua_acquire_execution_right( env, false ) )
	{
		lua_op_t op;
		op.cmd = LUACMD_CALL;
		op.name = method;
		if( arg )
			op.arg = *arg;
		else
			op.arg.num = 0;
		lua_push_op( env, &op );
		return LUA_QUEUED;
	}
	lua_result_t res = lua_do_call_custom( env, method, arg );
	lua_execute_pending( env );
	lua_release_execution_right( env );
	return res;
}


static lua_result_t lua_do_call_custom( lua_environment_t* env, const char* method, lua_arg_t* arg )
{
	lua_State* state;
	lua_result_t result;
	int numargs, i;
	int stacksize;

	state = env->state;
	stacksize = lua_gettop( state );
	result = LUA_OK;
	
	++env->calldepth;

	char* next = strpbrk( method, ".:" );
	if( next )
	{
		char* buffer = string_clone( method );
		char* cstr = buffer;
		
		next = cstr + (unsigned int)( next - method );
		*next = 0;
		
		lua_getglobal( state, cstr );
		if( lua_isnil( state, -1 ) )
		{
			log_warnf( WARNING_SCRIPT, "Invalid script call, '%s' is not set (%s)", cstr, method );
			string_deallocate( buffer );
			--env->calldepth;
			lua_pop( state, lua_gettop( state ) - stacksize );
			return LUA_ERROR;
		}
		else if( !lua_istable( state, -1 ) )
		{
			log_warnf( WARNING_SCRIPT, "Invalid script call, existing data '%s' in '%s' is not a table", cstr, method );
			string_deallocate( buffer );
			--env->calldepth;
			lua_pop( state, lua_gettop( state ) - stacksize );
			return LUA_ERROR;
		}
		//Top of stack is now table
		FOUNDATION_ASSERT( lua_istable( state, -1 ) );
		cstr = next + 1;

		*next = '.';
		next = strpbrk( cstr, ".:" );
		while( next )
		{
			*next = 0;
			lua_pushstring( state, cstr );
			lua_gettable( state, -2 );
			if( lua_isnil( state, -1 ) )
			{
				log_warnf( WARNING_SCRIPT, "Invalid script call, '%s' is not set (%s)", cstr, method );
				string_deallocate( buffer );
				--env->calldepth;
				lua_pop( state, lua_gettop( state ) - stacksize );
				return LUA_ERROR;
			}
			else if( !lua_istable( state, -1 ) )
			{
				log_warnf( WARNING_SCRIPT, "Invalid script call, existing data '%s' in '%s' is not a table", cstr, method );
				string_deallocate( buffer );
				--env->calldepth;
				lua_pop( state, lua_gettop( state ) - stacksize );
				return LUA_ERROR;
			}
			//Top of stack is now table
			FOUNDATION_ASSERT( lua_istable( state, -1 ) );

			*next = '.';
			cstr = next + 1;

			next = strpbrk( cstr, ".:" );
		}

		lua_pushstring( state, cstr );
		lua_gettable( state, -2 );

		string_deallocate( buffer );
	}
	else
	{
		lua_getglobal( state, method );
	}

	if( lua_isnil( state, -1 ) )
	{
		--env->calldepth;
		lua_pop( state, lua_gettop( state ) - stacksize );

		//Method does not exist in Lua context
		log_debugf( "Invalid script call, '%s' is not a function", method );

		return LUA_ERROR;
	}

	numargs = 0;
	if( arg )
	{
		numargs = ( arg->num < LUA_MAX_ARGS ) ? arg->num : LUA_MAX_ARGS;
		for( i = 0; i < numargs; ++i )
		{
			switch( arg->type[i] )
			{
				case LUADATA_PTR:
					lua_pushlightuserdata( state, arg->value[i].ptr );
					break;

				case LUADATA_OBJ:
					lua_pushobject( state, arg->value[i].obj );
					break;

				case LUADATA_INT:
					lua_pushinteger( state, arg->value[i].ival );
					break;

				case LUADATA_REAL:
					lua_pushnumber( state, arg->value[i].val );
					break;

				case LUADATA_STR:
					lua_pushstring( state, arg->value[i].str );
					break;

				case LUADATA_BOOL:
					lua_pushboolean( state, arg->value[i].flag );
					break;
					
				case LUADATA_INTARR:
				{
					const int* values = arg->value[i].ptr;
					lua_newtable( state );
					for( uint16_t ia = 0; ia < arg->size[i]; ++ia )
					{
						lua_pushinteger( state, ia+1 );
						lua_pushinteger( state, values[ia] );
						lua_settable( state, -3 );
					}
					break;
				}
					
				case LUADATA_REALARR:
				{
					const real* values = arg->value[i].ptr;
					lua_newtable( state );
					for( uint16_t ia = 0; ia < arg->size[i]; ++ia )
					{
						lua_pushinteger( state, ia+1 );
						lua_pushnumber( state, values[ia] );
						lua_settable( state, -3 );
					}
					break;
				}

				default:
					--numargs;
					break;
			}
		}
	}

	//TODO: Parse return value from call
	if( lua_pcall( state, numargs, 0, 0 ) != 0 )
	{
		log_debugf( "Script error: Calling %s : %s", method, lua_tostring( state, -1 ) );
		result = LUA_ERROR;
	}

	--env->calldepth;

	lua_pop( state, lua_gettop( state ) - stacksize );

	return result;
}


lua_result_t lua_call_void( lua_environment_t* env, const char* method )
{
	return lua_call_custom( env, method, 0 );
}


lua_result_t lua_call_val( lua_environment_t* env, const char* method, real val )
{
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_REAL, .value[0].val = val };
	return lua_call_custom( env, method, &arg );
}


lua_result_t lua_call_int( lua_environment_t* env, const char* method, int val )
{
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_INT, .value[0].ival = val };
	return lua_call_custom( env, method, &arg );
}


lua_result_t lua_call_bool( lua_environment_t* env, const char* method, bool val )
{
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_BOOL, .value[0].flag = val };
	return lua_call_custom( env, method, &arg );
}


lua_result_t lua_call_string( lua_environment_t* env, const char* method, const char* str )
{
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_STR, .value[0].str = str };
	return lua_call_custom( env, method, &arg );
}


lua_result_t lua_call_object( lua_environment_t* env, const char* method, object_t obj )
{
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_OBJ, .value[0].obj = obj };
	return lua_call_custom( env, method, &arg );
}


lua_result_t lua_call_ptr( lua_environment_t* env, const char* method, void* ptr )
{
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_PTR, .value[0].ptr = ptr };
	return lua_call_custom( env, method, &arg );
}


lua_result_t lua_eval( lua_environment_t* env, const char* code )
{
	if( !lua_acquire_execution_right( env, false ) )
	{
		lua_op_t op;
		op.cmd = LUACMD_EVAL;
		op.name = code;
		lua_push_op( env, &op );
		return LUA_QUEUED;
	}
	lua_result_t res = lua_do_eval( env, code );
	lua_execute_pending( env );
	lua_release_execution_right( env );
	return res;
}


static lua_result_t lua_do_eval( lua_environment_t* env, const char* code )
{
	lua_State* state;

	if( !env || !code )
		return LUA_ERROR;

	state = env->state;

	lua_readstring_t read_string = {
		.string = code,
		.size   = string_length( code )
	};

	if( lua_load( state, lua_read_string, &read_string, "=eval" ) != 0 )
	{
		log_debugf( "Script error: Lua eval() failed on load: %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		return LUA_ERROR;
	}

	if( lua_pcall( state, 0, 0, 0 ) != 0 )
	{
		log_debugf( "Script error: Lua eval() failed on pcall: %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		return LUA_ERROR;
	}

	return LUA_OK;
}


lua_result_t lua_bind( lua_environment_t* env, const char* method, lua_fn fn )
{
	if( !lua_acquire_execution_right( env, false ) )
	{
		lua_op_t op;
		op.cmd = LUACMD_BIND;
		op.name = method;
		op.arg.value[0].fn = fn;
		lua_push_op( env, &op );
		return LUA_QUEUED;
	}
	lua_value_t val = { .fn = fn };
	lua_result_t res = lua_do_bind( env, method, LUACMD_BIND, val );
	lua_execute_pending( env );
	lua_release_execution_right( env );
	return res;
}


lua_result_t lua_bind_native( lua_environment_t* env, const char* symbol, void* value )
{
	hash_t symhash = hash( symbol, string_length( symbol ) );
	hashmap_insert( env->lookup_map, symhash, value );
	return LUA_OK;
}


lua_result_t lua_bind_int( lua_environment_t* env, const char* property, int value )
{
	if( !lua_acquire_execution_right( env, false ) )
	{
		lua_op_t op;
		op.cmd = LUACMD_BIND_INT;
		op.name = property;
		op.arg.value[0].ival = value;
		lua_push_op( env, &op );
		return LUA_QUEUED;
	}
	lua_value_t val = { .ival = value };
	lua_result_t res = lua_do_bind( env, property, LUACMD_BIND_INT, val );
	lua_execute_pending( env );
	lua_release_execution_right( env );
	return res;
}


lua_result_t lua_bind_value( lua_environment_t* env, const char* property, real value )
{
	if( !lua_acquire_execution_right( env, false ) )
	{
		lua_op_t op;
		op.cmd = LUACMD_BIND_VAL;
		op.name = property;
		op.arg.value[0].val = value;
		lua_push_op( env, &op );
		return LUA_QUEUED;
	}
	lua_value_t val = { .val = value };
	lua_result_t res = lua_do_bind( env, property, LUACMD_BIND_VAL, val );
	lua_execute_pending( env );
	lua_release_execution_right( env );
	return res;
}


static lua_result_t lua_do_bind( lua_environment_t* env, const char* property, lua_command_t cmd, lua_value_t val )
{
	lua_State* state;
	int stacksize;
	char* next;

	if( !env || !property )
		return LUA_ERROR;

	state = env->state;
	stacksize = lua_gettop( state );
	next = strpbrk( property, ".:" );

	if( next )
	{
		int tables;
		unsigned int numtables = 0;
		char* buffer = string_clone( property );
		char* cstr = buffer;
		
		next = cstr + (unsigned int)( next - property );
		*next = 0;
		
		lua_getglobal( state, cstr );
		if( lua_isnil( state, -1 ) )
		{
			//Create global table
			lua_pop( state, 1 );
			lua_newtable( state );
			lua_pushvalue( state, -1 );
			lua_setglobal( state, cstr );
			log_debugf( "Created global table: %s", cstr );
		}
		else if( !lua_istable( state, -1 ) )
		{
			log_warnf( WARNING_SCRIPT, "Script error: Invalid script bind call, existing data '%s' in '%s' is not a table", cstr, property );
			string_deallocate( buffer );
			lua_pop( state, lua_gettop( state ) - stacksize );
			return LUA_ERROR;
		}
		//Top of stack is now table
		FOUNDATION_ASSERT( lua_istable( state, -1 ) );
		cstr = next + 1;
		++numtables;

		*next = '.';
		next = strpbrk( cstr, ".:" );
		while( next )
		{
			*next = 0;
			lua_pushstring( state, cstr );
			lua_gettable( state, -2 );
			if( lua_isnil( state, -1 ) )
			{
				//Create sub-table
				lua_pop( state, 1 );
				lua_newtable( state );
				lua_pushstring( state, cstr );
				lua_pushvalue( state, -2 );
				lua_settable( state, -4 );
				log_debugf( "Created table: %s", buffer );
			}
			else if( !lua_istable( state, -1 ) )
			{
				log_warnf( WARNING_SCRIPT, "Script error: Invalid script bind call, existing data '%s' in '%s' is not a table", cstr, property );
				string_deallocate( buffer );
				lua_pop( state, lua_gettop( state ) - stacksize );
				return LUA_ERROR;
			}
			//Top of stack is now table
			FOUNDATION_ASSERT( lua_istable( state, -1 ) );

			*next = '.';
			cstr = next + 1;
			++numtables;

			next = strpbrk( cstr, ".:" );
		}

		switch( cmd )
		{
			case LUACMD_BIND:
				lua_push_method( state, cstr, val.fn );
				break;
			case LUACMD_BIND_INT:
				lua_push_integer( state, cstr, val.ival );
				break;
			case LUACMD_BIND_VAL:
				lua_push_number( state, cstr, val.val );
				break;
            default:
                break;
		}

		tables = lua_gettop( state ) - stacksize;
		lua_pop( state, tables );

		string_deallocate( buffer );

		FOUNDATION_ASSERT( tables == (int)numtables );
	}
	else switch( cmd )
	{
		case LUACMD_BIND:
			lua_push_method_global( state, property, val.fn );
			break;
		case LUACMD_BIND_INT:
			lua_push_integer_global( state, property, val.ival );
			break;
		case LUACMD_BIND_VAL:
			lua_push_number_global( state, property, val.val );
			break;
        default:
            break;
	}

	return LUA_OK;
}


const char* lua_get_string( lua_environment_t* env, const char* property )
{
	const char* value = "";
	lua_State* state = env->state;

	if( !lua_acquire_execution_right( env, true ) )
		return value;

	int stacksize = lua_gettop( env->state );
	
	if( lua_do_get( env, property ) == LUA_OK )
		value = lua_tostring( state, -1 );

	lua_pop( state, lua_gettop( state ) - stacksize );

	lua_release_execution_right( env );

	return value;
}


int lua_get_int( lua_environment_t* env, const char* property )
{
	int value = 0;
	lua_State* state = env->state;

	if( !lua_acquire_execution_right( env, true ) )
		return value;

	int stacksize = lua_gettop( env->state );
	
	if( lua_do_get( env, property ) == LUA_OK )
		value = (int)lua_tointeger( state, -1 );

	lua_pop( state, lua_gettop( state ) - stacksize );

	lua_release_execution_right( env );

	return value;
}


static lua_result_t lua_do_get( lua_environment_t* env, const char* property )
{
	lua_State* state;
	lua_result_t result;

	state = env->state;
	result = LUA_OK;
	
	char* next = strpbrk( property, ".:" );
	if( next )
	{
		char* buffer = string_clone( property );
		char* cstr = buffer;
		
		next = cstr + (unsigned int)( next - property );
		*next = 0;
		
		lua_getglobal( state, cstr );
		if( lua_isnil( state, -1 ) )
		{
			log_warnf( WARNING_SCRIPT, "Script error: Invalid script get, '%s' is not set (%s)", cstr, property );
			string_deallocate( buffer );
			return LUA_ERROR;
		}
		else if( !lua_istable( state, -1 ) )
		{
			log_warnf( WARNING_SCRIPT, "Script error: Invalid script get, existing data '%s' in '%s' is not a table", cstr, property );
			string_deallocate( buffer );
			return LUA_ERROR;
		}
		//Top of stack is now table
		FOUNDATION_ASSERT( lua_istable( state, -1 ) );
		cstr = next + 1;

		*next = '.';
		next = strpbrk( cstr, ".:" );
		while( next )
		{
			*next = 0;
			lua_pushstring( state, cstr );
			lua_gettable( state, -2 );
			if( lua_isnil( state, -1 ) )
			{
				log_warnf( WARNING_SCRIPT, "Script error: Invalid script call, '%s' is not set (%s)", cstr, property );
				string_deallocate( buffer );
				return LUA_ERROR;
			}
			else if( !lua_istable( state, -1 ) )
			{
				log_warnf( WARNING_SCRIPT, "Script error: Invalid script call, existing data '%s' in '%s' is not a table", cstr, property );
				string_deallocate( buffer );
				return LUA_ERROR;
			}
			//Top of stack is now table
			FOUNDATION_ASSERT( lua_istable( state, -1 ) );

			*next = '.';
			cstr = next + 1;

			next = strpbrk( cstr, ".:" );
		}

		lua_pushstring( state, cstr );
		lua_gettable( state, -2 );

		string_deallocate( buffer );
	}
	else
	{
		lua_getglobal( state, property );
	}

	if( lua_isnil( state, -1 ) )
	{
		//Property does not exist in Lua context
		log_debugf( "Script error: Invalid script get, '%s' is not a property", property );
		return LUA_ERROR;
	}

	return result;
}


void lua_execute( lua_environment_t* env, int gc_time, bool force )
{
	if( ( env->queue[ env->queue_head ].cmd == LUACMD_WAIT ) && !gc_time )
		return; //Nothing executable pending
	if( lua_acquire_execution_right( env, force ) )
	{
		lua_execute_pending( env );
		if( gc_time )
			lua_run_gc( env, gc_time );
		lua_release_execution_right( env );
	}
}

void lua_timed_gc( lua_environment_t* env, int milliseconds )
{
	if( lua_acquire_execution_right( env, false ) )
	{
		lua_run_gc( env, milliseconds > 0 ? milliseconds : 0 );
		lua_release_execution_right( env );
	}
}


static void* lua_lookup_builtin( lua_State* state, const char* sym )
{
	hash_t symhash = hash( sym, string_length( sym ) );
	lua_environment_t* env = lua_env_from_state( state );
	void* fn = hashmap_lookup( env->lookup_map, symhash );
	//log_debugf( "Built-in lookup: %s -> %p", sym, fn );
	return fn;
}


lua_environment_t* lua_env_from_state( lua_State* state )
{
	void* env;
	lua_getglobal( state, "__environment" );
	env = lua_touserdata( state, -1 );
	lua_pop( state, 1 );
	return env;
}


lua_State* lua_state_from_env( lua_environment_t* env )
{
	return env->state;
}

