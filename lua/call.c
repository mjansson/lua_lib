/* call.c  -  Lua library  -  Public Domain  -  2013 Mattias Jansson
 *
 * This library provides a cross-platform lua library in C11 for games and applications
 * based on out foundation library. The latest source code is always available at
 *
 * https://github.com/mjansson/lua_lib
 *
 * This library is put in the public domain; you can redistribute it and/or modify it without
 * any restrictions.
 *
 * The LuaJIT library is released under the MIT license. For more information about LuaJIT, see
 * http://luajit.org/
 */

#define LUA_USE_INTERNAL_HEADER

#include <lua/lua.h>

#include <foundation/log.h>

#undef LUA_API
#define LUA_HAS_LUA_STATE_TYPE

#include "luajit/src/lua.h"

lua_result_t
lua_do_call_custom(lua_t* env, const char* method, size_t length, lua_arg_t* arg);

lua_result_t
lua_do_call_custom(lua_t* env, const char* method, size_t length, lua_arg_t* arg) {
	lua_State* state;
	lua_result_t result;
	int numargs, i;
	int stacksize;
	size_t start, next;
	string_const_t part;

	state = env->state;
	stacksize = lua_gettop(state);
	result = LUA_OK;

	++env->calldepth;

	next = string_find(method, length, '.', 0);
	if (next != STRING_NPOS) {
		part = string_const(method, next);
		lua_getlglobal(state, part.str, part.length);
		if (lua_isnil(state, -1)) {
			log_errorf(HASH_LUA, ERROR_INVALID_VALUE, STRING_CONST("Invalid script call, '%.*s' is not set (%.*s)"),
			           STRING_FORMAT(part), (int)length, method);
			--env->calldepth;
			lua_pop(state, lua_gettop(state) - stacksize);
			return LUA_ERROR;
		} else if (!lua_istable(state, -1)) {
			log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
			           STRING_CONST("Invalid script call, existing data '%.*s' in '%.*s' is not a table"),
			           STRING_FORMAT(part), (int)length, method);
			--env->calldepth;
			lua_pop(state, lua_gettop(state) - stacksize);
			return LUA_ERROR;
		}
		// Top of stack is now table
		FOUNDATION_ASSERT(lua_istable(state, -1));
		++next;
		start = next;

		next = string_find(method, length, '.', next);
		while (next != STRING_NPOS) {
			part = string_const(method + start, next - start);
			lua_pushlstring(state, part.str, part.length);
			lua_gettable(state, -2);
			if (lua_isnil(state, -1)) {
				log_errorf(HASH_LUA, ERROR_INVALID_VALUE, STRING_CONST("Invalid script call, '%.*s' is not set (%.*s)"),
				           STRING_FORMAT(part), (int)next, method);
				--env->calldepth;
				lua_pop(state, lua_gettop(state) - stacksize);
				return LUA_ERROR;
			} else if (!lua_istable(state, -1)) {
				log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
				           STRING_CONST("Invalid script call, existing data '%.*s' in '%.*s' is not a table"),
				           STRING_FORMAT(part), (int)next, method);
				--env->calldepth;
				lua_pop(state, lua_gettop(state) - stacksize);
				return LUA_ERROR;
			}
			// Top of stack is now table
			FOUNDATION_ASSERT(lua_istable(state, -1));

			++next;
			start = next;
			next = string_find(method, length, '.', next);
		}

		part = string_const(method + start, length - start);
		lua_pushlstring(state, part.str, part.length);
		lua_gettable(state, -2);
	} else {
		lua_getlglobal(state, method, length);
	}

	if (lua_isnil(state, -1)) {
		--env->calldepth;
		lua_pop(state, lua_gettop(state) - stacksize);

		// Method does not exist in Lua context
		log_errorf(HASH_LUA, ERROR_INVALID_VALUE, STRING_CONST("Invalid script call, '%.*s' is not a function"),
		           (int)length, method);
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
					lua_pushinteger(state, arg->value[i].obj);
					break;

				case LUADATA_INT:
					lua_pushinteger(state, arg->value[i].ival);
					break;

				case LUADATA_REAL:
					lua_pushnumber(state, (lua_Number)arg->value[i].val);
					break;

				case LUADATA_STR:
					lua_pushlstring(state, arg->value[i].str, arg->size[i]);
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
						lua_pushnumber(state, (lua_Number)values[ia]);
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

	// TODO: Parse return value from call
	if (lua_pcall(state, numargs, 0, 0) != 0) {
		string_const_t errmsg = {0, 0};
		errmsg.str = lua_tolstring(state, -1, &errmsg.length);
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Calling %.*s : %.*s"), (int)length, method,
		           STRING_FORMAT(errmsg));
		result = LUA_ERROR;
	}

	--env->calldepth;

	lua_pop(state, lua_gettop(state) - stacksize);

	return result;
}

lua_result_t
lua_call_custom(lua_t* env, const char* method, size_t length, lua_arg_t* arg) {
#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (!lua_acquire_execution_right(env, false)) {
		lua_op_t op;
		op.cmd = LUACMD_CALL;
		op.data.name = method;
		op.size = length;
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

lua_result_t
lua_call_void(lua_t* env, const char* method, size_t length) {
	return lua_call_custom(env, method, length, 0);
}

lua_result_t
lua_call_real(lua_t* env, const char* method, size_t length, real val) {
	lua_arg_t arg = {.num = 1, .type[0] = LUADATA_REAL, .value[0].val = val};
	return lua_call_custom(env, method, length, &arg);
}

lua_result_t
lua_call_int(lua_t* env, const char* method, size_t length, int val) {
	lua_arg_t arg = {.num = 1, .type[0] = LUADATA_INT, .value[0].ival = val};
	return lua_call_custom(env, method, length, &arg);
}

lua_result_t
lua_call_bool(lua_t* env, const char* method, size_t length, bool val) {
	lua_arg_t arg = {.num = 1, .type[0] = LUADATA_BOOL, .value[0].flag = val};
	return lua_call_custom(env, method, length, &arg);
}

lua_result_t
lua_call_string(lua_t* env, const char* method, size_t length, const char* str, size_t arglength) {
	lua_arg_t arg = {.num = 1, .type[0] = LUADATA_STR, .size[0] = (uint16_t)arglength, .value[0].str = str};
	return lua_call_custom(env, method, length, &arg);
}

lua_result_t
lua_call_object(lua_t* env, const char* method, size_t length, object_t obj) {
	lua_arg_t arg = {.num = 1, .type[0] = LUADATA_OBJ, .value[0].obj = obj};
	return lua_call_custom(env, method, length, &arg);
}

lua_result_t
lua_call_ptr(lua_t* env, const char* method, size_t length, void* ptr) {
	lua_arg_t arg = {.num = 1, .type[0] = LUADATA_PTR, .value[0].ptr = ptr};
	return lua_call_custom(env, method, length, &arg);
}
