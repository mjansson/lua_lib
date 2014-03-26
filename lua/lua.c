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

#include <lua/lua.h>
#include <lua/hashstrings.h>
#include <lua/foundation.h>
#include <lua/read.h>

#include <foundation/foundation.h>

#include <setjmp.h>

#undef LUA_API
#define LUA_HAS_LUA_STATE_TYPE

#include "luajit/src/lua.h"
#include "luajit/src/lauxlib.h"
#include "luajit/src/lualib.h"
LUA_EXTERN void* (*lj_clib_getsym_builtin)(lua_State*, const char*);

#undef LUA_API


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


#define LUA_CALL_QUEUE_SIZE  1024


typedef struct _lua_op
{
	lua_command_t         cmd;
	union 
	{
		const char*       name;
		void*             ptr;
	} data;
	lua_arg_t             arg;
} lua_op_t;


struct _lua
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
	atomic32_t                        queue_tail;

	//! Execution right
	semaphore_t                       execution_right;
	
	//! Currently executing thread
	uint64_t                          executing_thread;

	//! Execution count
	unsigned int                      executing_count;

	//! Lookup hashmap
	hashmap_t*                        lookup_map;
};


static lua_result_t lua_do_call_custom( lua_t* env, const char* method, lua_arg_t* arg );
static lua_result_t lua_do_bind( lua_t* env, const char* property, lua_command_t cmd, lua_value_t val );
static lua_result_t lua_do_eval_string( lua_t* env, const char* code );
static lua_result_t lua_do_eval_stream( lua_t* env, stream_t* stream );
static lua_result_t lua_do_get( lua_t* env, const char* property );

static void*        lua_lookup_builtin( lua_State* state, const char* sym );


static FORCEINLINE bool lua_has_execution_right( lua_t* env )
{
	return ( env->executing_thread == thread_id() );
}


static NOINLINE bool lua_acquire_execution_right( lua_t* env, bool force )
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


static void lua_release_execution_right( lua_t* env )
{
	FOUNDATION_ASSERT( env->executing_thread == thread_id() );
	FOUNDATION_ASSERT( env->executing_count > 0 );
	if( !--env->executing_count )
	{
		env->executing_thread = 0;
		semaphore_post( &env->execution_right );
	}
}


static void lua_push_op( lua_t* env, lua_op_t* op )
{
	unsigned int ofs, old;
	do
	{
		old = atomic_load32( &env->queue_tail );
		ofs = old+1;
		if( ofs >= LUA_CALL_QUEUE_SIZE )
			ofs = 0;
	} while( !atomic_cas32( &env->queue_tail, ofs, old ) );

	//Got slot, copy except command
	memcpy( &env->queue[ofs].data, &op->data, sizeof( op->data ) + sizeof( lua_arg_t ) );
	//Now set command, completing insert
	env->queue[ofs].cmd = op->cmd;
}


static void lua_execute_pending( lua_t* env )
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
				lua_do_eval_stream( env, env->queue[head].data.ptr );
				break;
			}

			case LUACMD_EVAL:
			{
				lua_do_eval_string( env, env->queue[head].data.name );
				break;
			}
			
			case LUACMD_CALL:
			{
				lua_do_call_custom( env, env->queue[head].data.name, &env->queue[head].arg );
				break;				
			}
			
			case LUACMD_BIND:
			case LUACMD_BIND_INT:
			case LUACMD_BIND_VAL:
			{
				lua_do_bind( env, env->queue[head].data.name, env->queue[head].cmd, env->queue[head].arg.value[0] );
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


static FORCEINLINE void lua_run_gc( lua_t* env, int milliseconds )
{
	lua_gc( env->state, LUA_GCSTEP, milliseconds );
}


static NOINLINE void* lua_allocator( void* env, void* block, size_t osize, size_t nsize )
{
	if( !nsize && osize )
		memory_deallocate( block );
	else if( !block )
		block = memory_allocate_context( HASH_LUA, nsize, 0, MEMORY_32BIT_ADDRESS );
	else if( nsize )
		block = memory_reallocate( block, nsize, 0, osize );
	if( block == 0 && nsize > 0 && env && ((lua_t*)env)->state )
		log_panicf( HASH_LUA, ERROR_OUT_OF_MEMORY, "Unable to allocate Lua memory (%u bytes)", nsize );
	return block;
}


static NOINLINE int lua_panic( lua_State* state )
{
	FOUNDATION_ASSERT_FAILFORMAT( "unprotected error in call to Lua API (%s)", lua_tostring( state, -1 ) );
	log_errorf( HASH_LUA, ERROR_EXCEPTION, "unprotected error in call to Lua API (%s)", lua_tostring( state, -1 ) );
	return 0;
}


lua_t* lua_allocate( void )
{
	lua_t* env = memory_allocate_zero_context( HASH_LUA, sizeof( lua_t ), 0, MEMORY_32BIT_ADDRESS );

	//Foundation allocators can meet demands of luajit on both 32 and 64 bit platforms
	lua_State* state = lua_newstate( lua_allocator, env );
	if( !state )
	{
		log_errorf( HASH_LUA, ERROR_INTERNAL_FAILURE, "Unable to allocate Lua state" );
		memory_deallocate( env );
		return 0;
	}

	lua_atpanic( state, lua_panic );

	lj_clib_getsym_builtin = lua_lookup_builtin;
	
	//Disable automagic gc
	lua_gc( state, LUA_GCCOLLECT, 0 );

	lua_pushlightuserdata( state, env );
	lua_setglobal( state, "__environment" );

	env->state = state;
	env->calldepth = 0;
	env->queue_head = 0;
	atomic_store32( &env->queue_tail, 0 );
	env->lookup_map = hashmap_allocate( 0, 0 );

	semaphore_initialize( &env->execution_right, 1 );

	unsigned int stacksize = lua_gettop( state );

	//Libraries
	log_debug( HASH_LUA, "Loading Lua built-ins" );
	luaL_openlibs( state );

	//Foundation bindings
	lua_load_foundation( state );

	lua_pop( state, lua_gettop( state ) - stacksize );

	return env;
}


void lua_deallocate( lua_t* env )
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


lua_result_t lua_call_custom( lua_t* env, const char* method, lua_arg_t* arg )
{
	if( !lua_acquire_execution_right( env, false ) )
	{
		lua_op_t op;
		op.cmd = LUACMD_CALL;
		op.data.name = method;
		if( arg )
			op.arg = *arg;
		else
			op.arg.num = 0;
		lua_push_op( env, &op );
		return LUA_QUEUED;
	}
	lua_execute_pending( env );
	lua_result_t res = lua_do_call_custom( env, method, arg );
	lua_release_execution_right( env );
	return res;
}


static lua_result_t lua_do_call_custom( lua_t* env, const char* method, lua_arg_t* arg )
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
			log_errorf( HASH_LUA, ERROR_INVALID_VALUE, "Invalid script call, '%s' is not set (%s)", cstr, method );
			string_deallocate( buffer );
			--env->calldepth;
			lua_pop( state, lua_gettop( state ) - stacksize );
			return LUA_ERROR;
		}
		else if( !lua_istable( state, -1 ) )
		{
			log_errorf( HASH_LUA, ERROR_INVALID_VALUE, "Invalid script call, existing data '%s' in '%s' is not a table", cstr, method );
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
				log_errorf( HASH_LUA, ERROR_INVALID_VALUE, "Invalid script call, '%s' is not set (%s)", cstr, method );
				string_deallocate( buffer );
				--env->calldepth;
				lua_pop( state, lua_gettop( state ) - stacksize );
				return LUA_ERROR;
			}
			else if( !lua_istable( state, -1 ) )
			{
				log_errorf( HASH_LUA, ERROR_INVALID_VALUE, "Invalid script call, existing data '%s' in '%s' is not a table", cstr, method );
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
		log_errorf( HASH_LUA, ERROR_INVALID_VALUE, "Invalid script call, '%s' is not a function", method );

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
		log_errorf( HASH_LUA, ERROR_INTERNAL_FAILURE, "Calling %s : %s", method, lua_tostring( state, -1 ) );
		result = LUA_ERROR;
	}

	--env->calldepth;

	lua_pop( state, lua_gettop( state ) - stacksize );

	return result;
}


lua_result_t lua_call_void( lua_t* env, const char* method )
{
	return lua_call_custom( env, method, 0 );
}


lua_result_t lua_call_val( lua_t* env, const char* method, real val )
{
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_REAL, .value[0].val = val };
	return lua_call_custom( env, method, &arg );
}


lua_result_t lua_call_int( lua_t* env, const char* method, int val )
{
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_INT, .value[0].ival = val };
	return lua_call_custom( env, method, &arg );
}


lua_result_t lua_call_bool( lua_t* env, const char* method, bool val )
{
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_BOOL, .value[0].flag = val };
	return lua_call_custom( env, method, &arg );
}


lua_result_t lua_call_string( lua_t* env, const char* method, const char* str )
{
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_STR, .value[0].str = str };
	return lua_call_custom( env, method, &arg );
}


lua_result_t lua_call_object( lua_t* env, const char* method, object_t obj )
{
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_OBJ, .value[0].obj = obj };
	return lua_call_custom( env, method, &arg );
}


lua_result_t lua_call_ptr( lua_t* env, const char* method, void* ptr )
{
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_PTR, .value[0].ptr = ptr };
	return lua_call_custom( env, method, &arg );
}


lua_result_t lua_eval_string( lua_t* env, const char* code )
{
	if( !lua_acquire_execution_right( env, true ) )
	{
		lua_op_t op;
		op.cmd = LUACMD_EVAL;
		op.data.name = code;
		lua_push_op( env, &op );
		return LUA_QUEUED;
	}
	lua_execute_pending( env );
	lua_result_t res = lua_do_eval_string( env, code );
	lua_release_execution_right( env );
	return res;
}


lua_result_t lua_eval_stream( lua_t* env, stream_t* stream )
{
	if( !lua_acquire_execution_right( env, true ) )
	{
		lua_op_t op;
		op.cmd = LUACMD_LOAD;
		op.data.ptr = stream;
		lua_push_op( env, &op );
		return LUA_QUEUED;
	}
	lua_execute_pending( env );
	lua_result_t res = lua_do_eval_stream( env, stream );
	lua_release_execution_right( env );
	return res;
}


static lua_result_t lua_do_eval_string( lua_t* env, const char* code )
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
		log_errorf( HASH_LUA, ERROR_INVALID_VALUE, "Lua eval string failed on load: %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		return LUA_ERROR;
	}

	if( lua_pcall( state, 0, 0, 0 ) != 0 )
	{
		log_errorf( HASH_LUA, ERROR_INTERNAL_FAILURE, "Lua eval string failed on pcall: %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		return LUA_ERROR;
	}

	return LUA_OK;
}


static lua_result_t lua_do_eval_stream( lua_t* env, stream_t* stream )
{
	lua_State* state;

	if( !env || !stream )
		return LUA_ERROR;

	state = env->state;

	lua_readstream_t read_stream = {
		.stream = stream
	};

	if( lua_load( state, lua_read_stream, &read_stream, "=eval" ) != 0 )
	{
		log_errorf( HASH_LUA, ERROR_INVALID_VALUE, "Lua eval stream failed on load: %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		return LUA_ERROR;
	}

	if( lua_pcall( state, 0, 0, 0 ) != 0 )
	{
		log_errorf( HASH_LUA, ERROR_INTERNAL_FAILURE, "Lua eval stream failed on pcall: %s", lua_tostring( state, -1 ) );
		lua_pop( state, 1 );
		return LUA_ERROR;
	}

	return LUA_OK;
}


lua_result_t lua_bind( lua_t* env, const char* method, lua_fn fn )
{
	if( !lua_acquire_execution_right( env, false ) )
	{
		lua_op_t op;
		op.cmd = LUACMD_BIND;
		op.data.name = method;
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


lua_result_t lua_bind_native( lua_t* env, const char* symbol, void* value )
{
	hash_t symhash = hash( symbol, string_length( symbol ) );
	hashmap_insert( env->lookup_map, symhash, value );
	return LUA_OK;
}


lua_result_t lua_bind_int( lua_t* env, const char* property, int value )
{
	if( !lua_acquire_execution_right( env, false ) )
	{
		lua_op_t op;
		op.cmd = LUACMD_BIND_INT;
		op.data.name = property;
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


lua_result_t lua_bind_value( lua_t* env, const char* property, real value )
{
	if( !lua_acquire_execution_right( env, false ) )
	{
		lua_op_t op;
		op.cmd = LUACMD_BIND_VAL;
		op.data.name = property;
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


static lua_result_t lua_do_bind( lua_t* env, const char* property, lua_command_t cmd, lua_value_t val )
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
			log_debugf( HASH_LUA, "Created global table: %s", cstr );
		}
		else if( !lua_istable( state, -1 ) )
		{
			log_errorf( HASH_LUA, ERROR_INVALID_VALUE, "Invalid script bind call, existing data '%s' in '%s' is not a table", cstr, property );
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
				log_debugf( HASH_LUA, "Created table: %s", buffer );
			}
			else if( !lua_istable( state, -1 ) )
			{
				log_errorf( HASH_LUA, ERROR_INVALID_VALUE, "Invalid script bind call, existing data '%s' in '%s' is not a table", cstr, property );
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


const char* lua_get_string( lua_t* env, const char* property )
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


int lua_get_int( lua_t* env, const char* property )
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


static lua_result_t lua_do_get( lua_t* env, const char* property )
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
			log_errorf( HASH_LUA, ERROR_INVALID_VALUE, "Invalid script get, '%s' is not set (%s)", cstr, property );
			string_deallocate( buffer );
			return LUA_ERROR;
		}
		else if( !lua_istable( state, -1 ) )
		{
			log_errorf( HASH_LUA, ERROR_INVALID_VALUE, "Invalid script get, existing data '%s' in '%s' is not a table", cstr, property );
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
				log_errorf( HASH_LUA, ERROR_INVALID_VALUE, "Invalid script call, '%s' is not set (%s)", cstr, property );
				string_deallocate( buffer );
				return LUA_ERROR;
			}
			else if( !lua_istable( state, -1 ) )
			{
				log_errorf( HASH_LUA, ERROR_INVALID_VALUE, "Invalid script call, existing data '%s' in '%s' is not a table", cstr, property );
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
		log_errorf( HASH_LUA, ERROR_INVALID_VALUE, "Invalid script get, '%s' is not a property", property );
		return LUA_ERROR;
	}

	return result;
}


void lua_execute( lua_t* env, int gc_time, bool force )
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


void lua_timed_gc( lua_t* env, int milliseconds )
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
	lua_t* env = lua_from_state( state );
	void* fn = hashmap_lookup( env->lookup_map, symhash );
	//log_debugf( HASH_LUA, "Built-in lookup: %s -> %p", sym, fn );
	return fn;
}


lua_t* lua_from_state( lua_State* state )
{
	void* env;
	lua_getglobal( state, "__environment" );
	env = lua_touserdata( state, -1 );
	lua_pop( state, 1 );
	return env;
}


lua_State* lua_state( lua_t* env )
{
	return env->state;
}


hashmap_t* lua_lookup_map( lua_t* env )
{
	return env->lookup_map;
}

