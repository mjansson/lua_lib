/* eval.c  -  Lua library  -  Public Domain  -  2013 Mattias Jansson
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
#include <foundation/stream.h>
#include <foundation/uuid.h>

#include <resource/compile.h>
#include <resource/stream.h>

#undef LUA_API
#define LUA_HAS_LUA_STATE_TYPE

#include "luajit/src/lua.h"

lua_result_t
lua_do_eval_string(lua_t* env, const char* code, size_t length);

lua_result_t
lua_do_eval_stream(lua_t* env, stream_t* stream, uint64_t size_limit);

lua_result_t
lua_do_eval_uuid(lua_t* env, const uuid_t uuid);

lua_result_t
lua_do_eval_string(lua_t* env, const char* code, size_t length) {
	lua_State* state;

	if (!env || !code)
		return LUA_ERROR;

	state = env->state;

	lua_readstring_t read_string = {.string = code, .size = length};

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

lua_result_t
lua_do_eval_stream(lua_t* env, stream_t* stream, uint64_t size_limit) {
	lua_State* state;

	if (!env || !stream)
		return LUA_ERROR;

	state = env->state;

	lua_readstream_t read_stream = {.stream = stream, .remain = size_limit ? size_limit : 0x7FFFFFFFFFFFFFFFULL};

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
lua_do_eval_uuid(lua_t* env, const uuid_t uuid) {
	if (!env || uuid_is_null(uuid))
		return LUA_ERROR;

	const uint32_t expected_version = LUA_RESOURCE_MODULE_VERSION;
	uint64_t platform = 0;
	stream_t* stream;
	bool success = false;
	bool recompile = false;
	bool recompiled = false;

	error_context_declare_local(char uuidbuf[40];
	                            const string_t uuidstr = string_from_uuid(uuidbuf, sizeof(uuidbuf), uuid));
	error_context_push(STRING_CONST("loading resource"), STRING_ARGS(uuidstr));

	platform = lua_resource_platform();

retry:

	stream = resource_stream_open_static(uuid, platform);
	if (stream) {
		resource_header_t header = resource_stream_read_header(stream);
		if ((header.type == HASH_LUA) && (header.version == expected_version)) {
			success = true;
		} else {
			log_warnf(HASH_LUA, WARNING_INVALID_VALUE, STRING_CONST("Got unexpected type/version: %" PRIx64 " : %u"),
			          header.type, header.version);
			recompile = true;
		}
		stream_deallocate(stream);
		stream = nullptr;
	}
	if (success) {
		success = false;
		stream = resource_stream_open_dynamic(uuid, platform);
	}
	if (stream) {
		uint32_t version = stream_read_uint32(stream);
		size_t size = (size_t)stream_read_uint64(stream);
		if (version == expected_version) {
			if (lua_do_eval_stream(env, stream, size) == LUA_OK)
				success = true;
		} else {
			log_warnf(HASH_LUA, WARNING_INVALID_VALUE, STRING_CONST("Got unexpected type/version: %u"), version);
			recompile = true;
		}
		stream_deallocate(stream);
		stream = nullptr;
	}

	if (recompile && !recompiled) {
		recompiled = resource_compile(uuid, platform);
		if (recompiled)
			goto retry;
	}

	error_context_pop();

	return success ? LUA_OK : LUA_ERROR;
}

lua_result_t
lua_eval_string(lua_t* env, const char* code, size_t length) {
#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (!lua_acquire_execution_right(env, true)) {
		lua_op_t op;
		op.cmd = LUACMD_EVAL;
		op.data.name = code;
		op.size = length;
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
	lua_result_t res = lua_do_eval_stream(env, stream, 0);
	lua_release_execution_right(env);
	return res;
#else
	return lua_do_eval_stream(env, stream, 0);
#endif
}

lua_result_t
lua_eval_resource(lua_t* env, const uuid_t uuid) {
#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (!lua_acquire_execution_right(env, true)) {
		lua_op_t op;
		op.cmd = LUACMD_LOAD_RESOURCE;
		op.data.uuid = uuid;
		lua_push_op(env, &op);
		return LUA_QUEUED;
	}
	lua_execute_pending(env);
	lua_result_t res = lua_do_eval_uuid(env, uuid);
	lua_release_execution_right(env);
	return res;
#else
	return lua_do_eval_uuid(env, uuid);
#endif
}
