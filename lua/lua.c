/* lua.c  -  Lua library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#undef LUA_API

typedef enum {
	LUACMD_WAIT = 0,
	LUACMD_LOAD,
	LUACMD_EVAL,
	LUACMD_CALL,
	LUACMD_BIND,
	LUACMD_BIND_INT,
	LUACMD_BIND_VAL
} lua_command_t;

typedef struct lua_op_t {
	lua_command_t         cmd;
	union {
		const char*       name;
		void*             ptr;
	} data;
	lua_arg_t             arg;
} lua_op_t;

static lua_result_t
lua_do_call_custom(lua_t* env, const char* method, size_t length, lua_arg_t* arg);

static lua_result_t
lua_do_bind(lua_t* env, const char* property, size_t length, lua_command_t cmd, lua_value_t val);

static lua_result_t
lua_do_eval_string(lua_t* env, const char* code, size_t length);

static lua_result_t
lua_do_eval_stream(lua_t* env, stream_t* stream);

static lua_result_t
lua_do_get(lua_t* env, const char* property, size_t length);

#if BUILD_ENABLE_LUA_THREAD_SAFE

/*static FOUNDATION_FORCEINLINE bool lua_has_execution_right( lua_t* env )
{
	return ( env->executing_thread == thread_id() );
}*/

static FOUNDATION_NOINLINE bool
lua_acquire_execution_right(lua_t* env, bool force) {
	uint64_t self = thread_id();
	if (env->executing_thread == self) {
		++env->executing_count;
		return true;
	}
	if (force) {
		semaphore_wait(&env->execution_right);
		env->executing_thread = self;
		FOUNDATION_ASSERT(env->executing_count == 0);
		++env->executing_count;
		return true;
	}
	if (semaphore_try_wait(&env->execution_right, 0)) {
		env->executing_thread = self;
		FOUNDATION_ASSERT(env->executing_count == 0);
		++env->executing_count;
		return true;
	}
	return false;
}

static void
lua_release_execution_right(lua_t* env) {
	FOUNDATION_ASSERT(env->executing_thread == thread_id());
	FOUNDATION_ASSERT(env->executing_count > 0);
	if (!--env->executing_count) {
		env->executing_thread = 0;
		semaphore_post(&env->execution_right);
	}
}

static void
lua_push_op(lua_t* env, lua_op_t* op) {
	unsigned int ofs, old;
	do {
		old = atomic_load32(&env->queue_tail);
		ofs = old + 1;
		if (ofs >= LUA_CALL_QUEUE_SIZE)
			ofs = 0;
	}
	while (!atomic_cas32(&env->queue_tail, ofs, old));

	//Got slot, copy except command
	memcpy(&env->queue[ofs].data, &op->data, sizeof(op->data) + sizeof(lua_arg_t));
	//Now set command, completing insert
	env->queue[ofs].cmd = op->cmd;
}

static void
lua_execute_pending(lua_t* env) {
	profile_begin_block("lua exec");

	unsigned int head = env->queue_head;
	while (env->queue[head].cmd != LUACMD_WAIT) {
		//Execute
		switch (env->queue[head].cmd) {
		case LUACMD_LOAD: {
				lua_do_eval_stream(env, env->queue[head].data.ptr);
				break;
			}

		case LUACMD_EVAL: {
				lua_do_eval_string(env, env->queue[head].data.name);
				break;
			}

		case LUACMD_CALL: {
				lua_do_call_custom(env, env->queue[head].data.name, &env->queue[head].arg);
				break;
			}

		case LUACMD_BIND:
		case LUACMD_BIND_INT:
		case LUACMD_BIND_VAL: {
				lua_do_bind(env, env->queue[head].data.name, env->queue[head].cmd, env->queue[head].arg.value[0]);
				break;
			}

		default:
			break;
		}

		//Mark as executed
		env->queue[head].cmd = LUACMD_WAIT;

		if (++head == LUA_CALL_QUEUE_SIZE)
			head = 0;
	}
	env->queue_head = head;

	profile_end_block();
}

#endif

static FOUNDATION_FORCEINLINE void
lua_run_gc(lua_t* env, int milliseconds) {
	lua_gc(env->state, LUA_GCSTEP, milliseconds);
}

static FOUNDATION_NOINLINE void*
lua_allocator(void* env, void* block, size_t osize, size_t nsize) {
	if (!nsize && osize)
		memory_deallocate(block);
	else if (!block)
		block = memory_allocate(HASH_LUA, nsize, 0, MEMORY_PERSISTENT | MEMORY_32BIT_ADDRESS);
	else if (nsize)
		block = memory_reallocate(block, nsize, 0, osize);
	if (block == 0 && nsize > 0 && env && ((lua_t*)env)->state)
		log_panicf(HASH_LUA, ERROR_OUT_OF_MEMORY, STRING_CONST("Unable to allocate Lua memory (%u bytes)"),
		           nsize);
	return block;
}

static FOUNDATION_NOINLINE int
lua_panic(lua_State* state) {
	string_const_t errmsg = {0, 0};
	errmsg.str = lua_tolstring(state, -1, &errmsg.length);
	FOUNDATION_ASSERT_FAILFORMAT("unprotected error in call to Lua API: %.*s", errmsg.length,
	                             errmsg.str);
	log_errorf(HASH_LUA, ERROR_EXCEPTION, STRING_CONST("unprotected error in call to Lua API: %.*s"),
	           STRING_FORMAT(errmsg));
	return 0;
}

lua_t*
lua_allocate(void) {
	lua_t* env = memory_allocate(HASH_LUA,
	                             sizeof(lua_t) + sizeof(hashmap_node_t*) * BUILD_SIZE_LUA_LOOKUP_BUCKETS, 0,
	                             MEMORY_PERSISTENT | MEMORY_32BIT_ADDRESS | MEMORY_ZERO_INITIALIZED);

	//Foundation allocators can meet demands of luajit on both 32 and 64 bit platforms
	lua_State* state = lua_newstate(lua_allocator, env);
	if (!state) {
		log_error(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Unable to allocate Lua state"));
		memory_deallocate(env);
		return 0;
	}

	lua_atpanic(state, lua_panic);

	//Disable automagic gc
	lua_gc(state, LUA_GCCOLLECT, 0);

	lua_pushlightuserdata(state, env);
	lua_setglobal(state, "__environment");

	env->state = state;
	env->calldepth = 0;

#if BUILD_ENABLE_LUA_THREAD_SAFE
	semaphore_initialize(&env->execution_right, 1);
	env->queue_head = 0;
	atomic_store32(&env->queue_tail, 0);
#endif

	int stacksize = lua_gettop(state);

	luaL_openlibs(state);

	lua_pop(state, lua_gettop(state) - stacksize);

	return env;
}

void
lua_deallocate(lua_t* env) {
	if (!env)
		return;

	FOUNDATION_ASSERT(env->calldepth == 0);
	FOUNDATION_ASSERT(env->state);

	lua_gc(env->state, LUA_GCCOLLECT, 0);
	lua_close(env->state);

#if BUILD_ENABLE_LUA_THREAD_SAFE
	semaphore_finalize(&env->execution_right);
#endif

	memory_deallocate(env);
}

lua_result_t
lua_call_custom(lua_t* env, const char* method, size_t length, lua_arg_t* arg) {
#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (!lua_acquire_execution_right(env, false)) {
		lua_op_t op;
		op.cmd = LUACMD_CALL;
		op.data.name = method;
		if (arg)
			op.arg = *arg;
		else
			op.arg.num = 0;
		lua_push_op(env, &op);
		return LUA_QUEUED;
	}
	lua_execute_pending(env);
	lua_result_t res = lua_do_call_custom(env, method, length, arg);
	lua_release_execution_right(env);
	return res;
#else
	return lua_do_call_custom(env, method, length, arg);
#endif
}

static lua_result_t
lua_do_call_custom(lua_t* env, const char* method, size_t length, lua_arg_t* arg) {
	lua_State* state;
	lua_result_t result;
	int numargs, i;
	int stacksize;
	size_t start, next;
	char buffer[BUILD_SIZE_LUA_NAME_MAXLENGTH];
	string_t part;

	state = env->state;
	stacksize = lua_gettop(state);
	result = LUA_OK;

	++env->calldepth;

	next = string_find(method, length, '.', 0);
	if (next != STRING_NPOS) {
		part = string_copy(buffer, sizeof(buffer), method, next);
		lua_getglobal(state, part.str);
		if (lua_isnil(state, -1)) {
			log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
			           STRING_CONST("Invalid script call, '%.*s' is not set (%.*s)"),
			           STRING_FORMAT(part), (int)length, method);
			--env->calldepth;
			lua_pop(state, lua_gettop(state) - stacksize);
			return LUA_ERROR;
		}
		else if (!lua_istable(state, -1)) {
			log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
			           STRING_CONST("Invalid script call, existing data '%.*s' in '%.*s' is not a table"),
			           STRING_FORMAT(part), (int)length, method);
			--env->calldepth;
			lua_pop(state, lua_gettop(state) - stacksize);
			return LUA_ERROR;
		}
		//Top of stack is now table
		FOUNDATION_ASSERT(lua_istable(state, -1));
		++next;
		start = next;

		next = string_find(method, length, '.', next);
		while (next != STRING_NPOS) {
			part = string_copy(buffer, sizeof(buffer), method + start, next - start);
			lua_pushstring(state, part.str);
			lua_gettable(state, -2);
			if (lua_isnil(state, -1)) {
				log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
				           STRING_CONST("Invalid script call, '%.*s' is not set (%.*s)"),
				           STRING_FORMAT(part), (int)next, method);
				--env->calldepth;
				lua_pop(state, lua_gettop(state) - stacksize);
				return LUA_ERROR;
			}
			else if (!lua_istable(state, -1)) {
				log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
				           STRING_CONST("Invalid script call, existing data '%.*s' in '%.*s' is not a table"),
				           STRING_FORMAT(part), (int)next, method);
				--env->calldepth;
				lua_pop(state, lua_gettop(state) - stacksize);
				return LUA_ERROR;
			}
			//Top of stack is now table
			FOUNDATION_ASSERT(lua_istable(state, -1));

			++next;
			start = next;
			next = string_find(method, length, '.', next);
		}

		part = string_copy(buffer, sizeof(buffer), method + start, length - start);
		lua_pushstring(state, part.str);
		lua_gettable(state, -2);
	}
	else {
		part = string_copy(buffer, sizeof(buffer), method, length);
		lua_getglobal(state, part.str);
	}

	if (lua_isnil(state, -1)) {
		--env->calldepth;
		lua_pop(state, lua_gettop(state) - stacksize);

		//Method does not exist in Lua context
		log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
		           STRING_CONST("Invalid script call, '%.*s' is not a function"), (int)length, method);
		return LUA_ERROR;
	}

	numargs = 0;
	if (arg) {
		numargs = (arg->num < LUA_MAX_ARGS) ? arg->num : LUA_MAX_ARGS;
		for (i = 0; i < numargs; ++i) {
			switch (arg->type[i]) {
			case LUADATA_PTR:
				lua_pushlightuserdata(state, arg->value[i].ptr);
				break;

			case LUADATA_OBJ:
				lua_pushobject(state, arg->value[i].obj);
				break;

			case LUADATA_INT:
				lua_pushinteger(state, arg->value[i].ival);
				break;

			case LUADATA_REAL:
				lua_pushnumber(state, arg->value[i].val);
				break;

			case LUADATA_STR:
				lua_pushstring(state, arg->value[i].str);
				break;

			case LUADATA_BOOL:
				lua_pushboolean(state, arg->value[i].flag);
				break;

			case LUADATA_INTARR: {
					const int* values = arg->value[i].ptr;
					lua_newtable(state);
					for (uint16_t ia = 0; ia < arg->size[i]; ++ia) {
						lua_pushinteger(state, ia + 1);
						lua_pushinteger(state, values[ia]);
						lua_settable(state, -3);
					}
					break;
				}

			case LUADATA_REALARR: {
					const real* values = arg->value[i].ptr;
					lua_newtable(state);
					for (uint16_t ia = 0; ia < arg->size[i]; ++ia) {
						lua_pushinteger(state, ia + 1);
						lua_pushnumber(state, values[ia]);
						lua_settable(state, -3);
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
	if (lua_pcall(state, numargs, 0, 0) != 0) {
		string_const_t errmsg = {0, 0};
		errmsg.str = lua_tolstring(state, -1, &errmsg.length);
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Calling %.*s : %.*s"),
		           (int)length, method, STRING_FORMAT(errmsg));
		result = LUA_ERROR;
	}

	--env->calldepth;

	lua_pop(state, lua_gettop(state) - stacksize);

	return result;
}

lua_result_t
lua_call_void(lua_t* env, const char* method, size_t length) {
	return lua_call_custom(env, method, length, 0);
}

lua_result_t
lua_call_val(lua_t* env, const char* method, size_t length, real val) {
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_REAL, .value[0].val = val };
	return lua_call_custom(env, method, length, &arg);
}

lua_result_t
lua_call_int(lua_t* env, const char* method, size_t length, int val) {
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_INT, .value[0].ival = val };
	return lua_call_custom(env, method, length, &arg);
}

lua_result_t
lua_call_bool(lua_t* env, const char* method, size_t length, bool val) {
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_BOOL, .value[0].flag = val };
	return lua_call_custom(env, method, length, &arg);
}

lua_result_t
lua_call_string(lua_t* env, const char* method, size_t length, const char* str) {
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_STR, .value[0].str = str };
	return lua_call_custom(env, method, length, &arg);
}

lua_result_t
lua_call_object(lua_t* env, const char* method, size_t length, object_t obj) {
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_OBJ, .value[0].obj = obj };
	return lua_call_custom(env, method, length, &arg);
}

lua_result_t
lua_call_ptr(lua_t* env, const char* method, size_t length, void* ptr) {
	lua_arg_t arg = { .num = 1, .type[0] = LUADATA_PTR, .value[0].ptr = ptr };
	return lua_call_custom(env, method, length, &arg);
}

lua_result_t
lua_eval_string(lua_t* env, const char* code, size_t length) {
#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (!lua_acquire_execution_right(env, true)) {
		lua_op_t op;
		op.cmd = LUACMD_EVAL;
		op.data.name = code;
		lua_push_op(env, &op);
		return LUA_QUEUED;
	}
	lua_execute_pending(env);
	lua_result_t res = lua_do_eval_string(env, code, length);
	lua_release_execution_right(env);
	return res;
#else
	return lua_do_eval_string(env, code, length);
#endif
}

lua_result_t
lua_eval_stream(lua_t* env, stream_t* stream) {
#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (!lua_acquire_execution_right(env, true)) {
		lua_op_t op;
		op.cmd = LUACMD_LOAD;
		op.data.ptr = stream;
		lua_push_op(env, &op);
		return LUA_QUEUED;
	}
	lua_execute_pending(env);
	lua_result_t res = lua_do_eval_stream(env, stream);
	lua_release_execution_right(env);
	return res;
#else
	return lua_do_eval_stream(env, stream);
#endif
}

static lua_result_t
lua_do_eval_string(lua_t* env, const char* code, size_t length) {
	lua_State* state;

	if (!env || !code)
		return LUA_ERROR;

	state = env->state;

	lua_readstring_t read_string = {
		.string = code,
		.size   = length
	};

	if (lua_load(state, lua_read_string, &read_string, "=eval") != 0) {
		string_const_t errmsg = {0, 0};
		errmsg.str = lua_tolstring(state, -1, &errmsg.length);
		log_errorf(HASH_LUA, ERROR_INVALID_VALUE, STRING_CONST("Lua eval string failed on load: %.*s"),
		           STRING_FORMAT(errmsg));
		lua_pop(state, 1);
		return LUA_ERROR;
	}

	if (lua_pcall(state, 0, 0, 0) != 0) {
		string_const_t errmsg = {0, 0};
		errmsg.str = lua_tolstring(state, -1, &errmsg.length);
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua eval string failed on pcall: %.*s"),
		           STRING_FORMAT(errmsg));
		lua_pop(state, 1);
		return LUA_ERROR;
	}

	return LUA_OK;
}

static lua_result_t
lua_do_eval_stream(lua_t* env, stream_t* stream) {
	lua_State* state;

	if (!env || !stream)
		return LUA_ERROR;

	state = env->state;

	lua_readstream_t read_stream = {
		.stream = stream
	};

	if (lua_load(state, lua_read_stream, &read_stream, "=eval") != 0) {
		string_const_t errmsg = {0, 0};
		errmsg.str = lua_tolstring(state, -1, &errmsg.length);
		log_errorf(HASH_LUA, ERROR_INVALID_VALUE, STRING_CONST("Lua eval stream failed on load: %.*s"),
		           STRING_FORMAT(errmsg));
		lua_pop(state, 1);
		return LUA_ERROR;
	}

	if (lua_pcall(state, 0, 0, 0) != 0) {
		string_const_t errmsg = {0, 0};
		errmsg.str = lua_tolstring(state, -1, &errmsg.length);
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua eval stream failed on pcall: %.*s"),
		           STRING_FORMAT(errmsg));
		lua_pop(state, 1);
		return LUA_ERROR;
	}

	return LUA_OK;
}

lua_result_t
lua_bind(lua_t* env, const char* method, size_t length, lua_fn fn) {
#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (!lua_acquire_execution_right(env, false)) {
		lua_op_t op;
		op.cmd = LUACMD_BIND;
		op.data.name = method;
		op.arg.value[0].fn = fn;
		lua_push_op(env, &op);
		return LUA_QUEUED;
	}
	lua_value_t val = { .fn = fn };
	lua_result_t res = lua_do_bind(env, method, length, LUACMD_BIND, val);
	lua_execute_pending(env);
	lua_release_execution_right(env);
	return res;
#else
	lua_value_t val = { .fn = fn };
	return lua_do_bind(env, method, length, LUACMD_BIND, val);
#endif
}

lua_result_t
lua_bind_int(lua_t* env, const char* property, size_t length, int value) {
#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (!lua_acquire_execution_right(env, false)) {
		lua_op_t op;
		op.cmd = LUACMD_BIND_INT;
		op.data.name = property;
		op.arg.value[0].ival = value;
		lua_push_op(env, &op);
		return LUA_QUEUED;
	}
	lua_value_t val = { .ival = value };
	lua_result_t res = lua_do_bind(env, property, length, LUACMD_BIND_INT, val);
	lua_execute_pending(env);
	lua_release_execution_right(env);
	return res;
#else
	lua_value_t val = { .ival = value };
	return lua_do_bind(env, property, length, LUACMD_BIND_INT, val);
#endif
}

lua_result_t
lua_bind_value(lua_t* env, const char* property, size_t length, real value) {
#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (!lua_acquire_execution_right(env, false)) {
		lua_op_t op;
		op.cmd = LUACMD_BIND_VAL;
		op.data.name = property;
		op.arg.value[0].val = value;
		lua_push_op(env, &op);
		return LUA_QUEUED;
	}
	lua_value_t val = { .val = value };
	lua_result_t res = lua_do_bind(env, property, length, LUACMD_BIND_VAL, val);
	lua_execute_pending(env);
	lua_release_execution_right(env);
	return res;
#else
	lua_value_t val = { .val = value };
	return lua_do_bind(env, property, length, LUACMD_BIND_VAL, val);
#endif
}

static FOUNDATION_NOINLINE void
lua_push_integer(lua_State* state, const char* name, size_t length, int value) {
	lua_pushlstring(state, name, length);
	lua_pushinteger(state, value);
	lua_settable(state, -3);
}

static FOUNDATION_NOINLINE void
lua_push_integer_global(lua_State* state, const char* name, size_t length, int value) {
	FOUNDATION_UNUSED(length);
	lua_pushinteger(state, value);
	lua_setglobal(state, name);
}

static FOUNDATION_NOINLINE void
lua_push_number(lua_State* state, const char* name, size_t length, real value) {
	lua_pushlstring(state, name, length);
	lua_pushnumber(state, value);
	lua_settable(state, -3);
}

static FOUNDATION_NOINLINE void
lua_push_number_global(lua_State* state, const char* name, size_t length, real value) {
	FOUNDATION_UNUSED(length);
	lua_pushnumber(state, value);
	lua_setglobal(state, name);
}

static FOUNDATION_NOINLINE void
lua_push_method(lua_State* state, const char* name, size_t length, lua_CFunction fn) {
	lua_pushlstring(state, name, length);
	lua_pushcclosure(state, fn, 0);
	lua_settable(state, -3);
}

static FOUNDATION_NOINLINE void
lua_push_method_global(lua_State* state, const char* name, size_t length, lua_CFunction fn) {
	FOUNDATION_UNUSED(length);
	lua_pushcclosure(state, fn, 0);
	lua_setglobal(state, name);
}

static lua_result_t
lua_do_bind(lua_t* env, const char* property, size_t length, lua_command_t cmd, lua_value_t val) {
	lua_State* state;
	int stacksize;
	size_t start, next;
	char buffer[BUILD_SIZE_LUA_NAME_MAXLENGTH];
	string_t part;

	if (!env || !length)
		return LUA_ERROR;

	state = env->state;
	stacksize = lua_gettop(state);

	next = string_find(property, length, '.', 0);
	if (next != STRING_NPOS) {
		int tables;
		unsigned int numtables = 0;
		part = string_copy(buffer, sizeof(buffer), property, next);

		lua_getglobal(state, part.str);
		if (lua_isnil(state, -1)) {
			//Create global table
			lua_pop(state, 1);
			lua_newtable(state);
			lua_pushvalue(state, -1);
			lua_setglobal(state, part.str);
			log_debugf(HASH_LUA, STRING_CONST("Created global table: %.*s"), STRING_FORMAT(part));
		}
		else if (!lua_istable(state, -1)) {
			log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
			           STRING_CONST("Invalid script bind call, existing data '%.*s' in '%.*s' is not a table"),
			           STRING_FORMAT(part), (int)length, property);
			lua_pop(state, lua_gettop(state) - stacksize);
			return LUA_ERROR;
		}
		//Top of stack is now table
		FOUNDATION_ASSERT(lua_istable(state, -1));

		++next;
		start = next;
		++numtables;

		next = string_find(property, length, '.', next);
		while (next != STRING_NPOS) {
			part = string_copy(buffer, sizeof(buffer), property + start, next - start);
			lua_pushstring(state, part.str);
			lua_gettable(state, -2);
			if (lua_isnil(state, -1)) {
				//Create sub-table
				lua_pop(state, 1);
				lua_newtable(state);
				lua_pushstring(state, part.str);
				lua_pushvalue(state, -2);
				lua_settable(state, -4);
				log_debugf(HASH_LUA, STRING_CONST("Created table: %.*s"), next, property);
			}
			else if (!lua_istable(state, -1)) {
				log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
				           STRING_CONST("Invalid script bind call, existing data '%.*s' in '%.*s' is not a table"),
				           STRING_FORMAT(part), (int)next, property);
				lua_pop(state, lua_gettop(state) - stacksize);
				return LUA_ERROR;
			}
			//Top of stack is now table
			FOUNDATION_ASSERT(lua_istable(state, -1));

			++next;
			start = next;
			next = string_find(property, length, '.', next);
			++numtables;
		}

		part = string_copy(buffer, sizeof(buffer), property + start, length - start);

		switch (cmd) {
		case LUACMD_BIND:
			lua_push_method(state, STRING_ARGS(part), val.fn);
			break;
		case LUACMD_BIND_INT:
			lua_push_integer(state, STRING_ARGS(part), val.ival);
			break;
		case LUACMD_BIND_VAL:
			lua_push_number(state, STRING_ARGS(part), val.val);
			break;
		default:
			break;
		}

		tables = lua_gettop(state) - stacksize;
		lua_pop(state, tables);

		FOUNDATION_ASSERT(tables == (int)numtables);
	}
	else {
		part = string_copy(buffer, sizeof(buffer), property, length);
		switch (cmd) {
		case LUACMD_BIND:
			lua_push_method_global(state, STRING_ARGS(part), val.fn);
			break;
		case LUACMD_BIND_INT:
			lua_push_integer_global(state, STRING_ARGS(part), val.ival);
			break;
		case LUACMD_BIND_VAL:
			lua_push_number_global(state, STRING_ARGS(part), val.val);
			break;
		default:
			break;
		}
	}
	return LUA_OK;
}

string_const_t
lua_get_string(lua_t* env, const char* property, size_t length) {
	string_const_t value = {0, 0};
	lua_State* state = env->state;

#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (!lua_acquire_execution_right(env, true))
		return value;
#endif

	int stacksize = lua_gettop(env->state);
	if (lua_do_get(env, property, length) == LUA_OK)
		value.str = lua_tolstring(state, -1, &value.length);

	lua_pop(state, lua_gettop(state) - stacksize);

#if BUILD_ENABLE_LUA_THREAD_SAFE
	lua_release_execution_right(env);
#endif

	return value;
}

int
lua_get_int(lua_t* env, const char* property, size_t length) {
	int value = 0;
	lua_State* state = env->state;

#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (!lua_acquire_execution_right(env, true))
		return value;
#endif

	int stacksize = lua_gettop(env->state);

	if (lua_do_get(env, property, length) == LUA_OK)
		value = (int)lua_tointeger(state, -1);

	lua_pop(state, lua_gettop(state) - stacksize);

#if BUILD_ENABLE_LUA_THREAD_SAFE
	lua_release_execution_right(env);
#endif

	return value;
}

static lua_result_t
lua_do_get(lua_t* env, const char* property, size_t length) {
	lua_State* state;
	lua_result_t result;
	size_t start, next;
	char buffer[BUILD_SIZE_LUA_NAME_MAXLENGTH];
	string_t part;

	state = env->state;
	result = LUA_OK;

	next = string_find(property, length, '.', 0);
	if (next != STRING_NPOS) {
		part = string_copy(buffer, sizeof(buffer), property, next);
		lua_getglobal(state, part.str);
		if (lua_isnil(state, -1)) {
			log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
			           STRING_CONST("Invalid script get, '%.*s' is not set (%.*s)"),
			           STRING_FORMAT(part), (int)length, property);
			return LUA_ERROR;
		}
		else if (!lua_istable(state, -1)) {
			log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
			           STRING_CONST("Invalid script get, existing data '%.*s' in '%.*s' is not a table"),
			           STRING_FORMAT(part), (int)length, property);
			return LUA_ERROR;
		}
		//Top of stack is now table
		FOUNDATION_ASSERT(lua_istable(state, -1));
		++next;
		start = next;

		next = string_find(property, length, '.', next);
		while (next != STRING_NPOS) {
			part = string_copy(buffer, sizeof(buffer), property + start, next - start);
			lua_pushstring(state, part.str);
			lua_gettable(state, -2);
			if (lua_isnil(state, -1)) {
				log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
				           STRING_CONST("Invalid script call, '%.*s' is not set (%.*s)"),
				           STRING_FORMAT(part), (int)next, property);
				return LUA_ERROR;
			}
			else if (!lua_istable(state, -1)) {
				log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
				           STRING_CONST("Invalid script call, existing data '%.*s' in '%.*s' is not a table"),
				           STRING_FORMAT(part), (int)next, property);
				return LUA_ERROR;
			}
			//Top of stack is now table
			FOUNDATION_ASSERT(lua_istable(state, -1));

			++next;
			start = next;
			next = string_find(property, length, '.', next);
		}

		part = string_copy(buffer, sizeof(buffer), property + start, length - start);
		lua_pushstring(state, part.str);
		lua_gettable(state, -2);
	}
	else {
		part = string_copy(buffer, sizeof(buffer), property, length);
		lua_getglobal(state, part.str);
	}

	if (lua_isnil(state, -1)) {
		//Property does not exist in Lua context
		log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
		           STRING_CONST("Invalid script get, '%.*s' is not a property"),
		           (int)length, property);
		return LUA_ERROR;
	}

	return result;
}

void
lua_execute(lua_t* env, int gc_time, bool force) {
#if BUILD_ENABLE_LUA_THREAD_SAFE
	if ((env->queue[ env->queue_head ].cmd == LUACMD_WAIT) && !gc_time)
		return; //Nothing executable pending

	if (!lua_acquire_execution_right(env, force))
		return;

	lua_execute_pending(env);
#else
	FOUNDATION_UNUSED(force);
#endif

	if (gc_time)
		lua_run_gc(env, gc_time);

#if BUILD_ENABLE_LUA_THREAD_SAFE
	lua_release_execution_right(env);
#endif
}

void
lua_timed_gc(lua_t* env, int milliseconds) {
#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (lua_acquire_execution_right(env, false)) {
		lua_run_gc(env, milliseconds > 0 ? milliseconds : 0);
		lua_release_execution_right(env);
	}
#else
	lua_run_gc(env, milliseconds > 0 ? milliseconds : 0);
#endif
}

lua_t*
lua_from_state(lua_State* state) {
	void* env;
	lua_getglobal(state, "__environment");
	env = lua_touserdata(state, -1);
	lua_pop(state, 1);
	return env;
}

lua_State*
lua_state(lua_t* env) {
	return env->state;
}

extern int
lua_registry_initialize(void);

extern void
lua_registry_finalize(void);

extern int
lua_symbol_initialize(void);

extern void
lua_symbol_finalize(void);

int
lua_module_initialize(const lua_config_t config) {
	FOUNDATION_UNUSED(config);

	if (lua_symbol_initialize() < 0)
		return -1;

	if (lua_registry_initialize() < 0)
		return -1;

	lua_module_register(STRING_CONST("foundation"), lua_load_foundation_module);

	return 0;
}

void
lua_module_finalize(void) {
	lua_registry_finalize();
	lua_symbol_finalize();
}
