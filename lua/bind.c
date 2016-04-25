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

#define LUA_USE_INTERNAL_HEADER

#include <lua/lua.h>

#include <foundation/foundation.h>

#undef LUA_API
#define LUA_HAS_LUA_STATE_TYPE

#include "luajit/src/lua.h"

lua_result_t
lua_do_bind(lua_t* env, const char* property, size_t length, lua_command_t cmd, lua_value_t val);

static void
lua_push_integer(lua_State* state, const char* name, size_t length, int value) {
	lua_pushlstring(state, name, length);
	lua_pushinteger(state, value);
	lua_settable(state, -3);
}

static void
lua_push_integer_global(lua_State* state, const char* name, size_t length, int value) {
	lua_pushinteger(state, value);
	lua_setlglobal(state, name, length);
}

static void
lua_push_number(lua_State* state, const char* name, size_t length, real value) {
	lua_pushlstring(state, name, length);
	lua_pushnumber(state, value);
	lua_settable(state, -3);
}

static void
lua_push_number_global(lua_State* state, const char* name, size_t length, real value) {
	lua_pushnumber(state, value);
	lua_setlglobal(state, name, length);
}

static void
lua_push_method(lua_State* state, const char* name, size_t length, lua_CFunction fn) {
	lua_pushlstring(state, name, length);
	lua_pushcclosure(state, fn, 0);
	lua_settable(state, -3);
}

static void
lua_push_method_global(lua_State* state, const char* name, size_t length, lua_CFunction fn) {
	lua_pushcclosure(state, fn, 0);
	lua_setlglobal(state, name, length);
}

lua_result_t
lua_bind_function(lua_t* env, const char* method, size_t length, lua_fn fn) {
#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (!lua_acquire_execution_right(env, false)) {
		lua_op_t op;
		op.cmd = LUACMD_BIND;
		op.data.name = method;
		op.size = length;
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
		op.size = length;
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
lua_bind_real(lua_t* env, const char* property, size_t length, real value) {
#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (!lua_acquire_execution_right(env, false)) {
		lua_op_t op;
		op.cmd = LUACMD_BIND_VAL;
		op.data.name = property;
		op.size = length;
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

lua_result_t
lua_do_bind(lua_t* env, const char* property, size_t length, lua_command_t cmd, lua_value_t val) {
	lua_State* state;
	int stacksize;
	size_t start, next;
	string_const_t part;

	if (!env || !length)
		return LUA_ERROR;

	state = env->state;
	stacksize = lua_gettop(state);

	next = string_find(property, length, '.', 0);
	if (next != STRING_NPOS) {
		int tables;
		unsigned int numtables = 0;
		part = string_const(property, next);

		lua_getlglobal(state, part.str, part.length);
		if (lua_isnil(state, -1)) {
			//Create global table
			lua_pop(state, 1);
			lua_newtable(state);
			lua_pushvalue(state, -1);
			lua_setlglobal(state, part.str, part.length);
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
			part = string_const(property + start, next - start);
			lua_pushlstring(state, part.str, part.length);
			lua_gettable(state, -2);
			if (lua_isnil(state, -1)) {
				//Create sub-table
				lua_pop(state, 1);
				lua_newtable(state);
				lua_pushlstring(state, part.str, part.length);
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

		part = string_const(property + start, length - start);

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
		part = string_const(property, length);
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
