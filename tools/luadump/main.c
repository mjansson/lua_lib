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
#include <lua/read.h>
#include <lua/hashstrings.h>

#include "errorcodes.h"

#include <stdio.h>

typedef struct lua_dump_t lua_dump_t;

struct lua_dump_t {
	string_t       input_file;
	string_t       output_file;

	stream_t*      input_stream;
	stream_t*      output_stream;

	lua_t*         env;

	bool           hex;

	char*          bytecode;
	size_t         bytecode_size;

	error_level_t  suppress_level;
};

static lua_dump_t
luadump_parse_command_line(const string_const_t* cmdline);

static void
luadump_print_usage(void);

//static int
//luadump_load_jitbc(lua_t* env);

static FOUNDATION_NOINLINE int
lua_dump_writer(lua_State* state, const void* buffer, size_t size, void* user_data) {
	lua_dump_t* dump = user_data;

	FOUNDATION_UNUSED(state);

	if (size <= 0)
		return 0;

	dump->bytecode = (dump->bytecode ?
	                  memory_reallocate(dump->bytecode, dump->bytecode_size + size, 0, dump->bytecode_size) :
	                  memory_allocate(HASH_LUA, size, 0, MEMORY_PERSISTENT));

	memcpy(dump->bytecode + dump->bytecode_size, buffer, size);
	dump->bytecode_size += size;

	if (dump->hex) {
		char* line = memory_allocate(HASH_LUA, size * 6 + 2, 0, MEMORY_TEMPORARY);
		for (size_t i = 0; i < size; ++i)
			string_format(line + (i * 6), 7, STRING_CONST("0x%02x, "), ((const unsigned char*)buffer)[i]);
		if (dump->output_stream) {
			stream_write_string(dump->output_stream, line, size * 6);
			stream_write_endl(dump->output_stream);
		}
		else {
			log_set_suppress(HASH_LUA, ERRORLEVEL_DEBUG);
			log_info(HASH_LUA, line, size * 6);
			log_set_suppress(HASH_LUA, dump->suppress_level);
		}
		memory_deallocate(line);
	}
	else {
		if (dump->output_stream)
			stream_write(dump->output_stream, buffer, size);
	}

	return 0;
}

LUA_API int
lua_load(lua_State* L, lua_Reader reader, void* dt, const char* chunkname);

LUA_API int
lua_dump(lua_State* L, lua_Writer writer, void* data);

LUA_API int
lua_pcall(lua_State* L, int nargs, int nresults, int errfunc);

int
main_initialize(void) {
	int ret = 0;

	log_enable_prefix(false);
	log_set_suppress(0, ERRORLEVEL_INFO);
	log_set_suppress(HASH_LUA, ERRORLEVEL_INFO);

	foundation_config_t config;
	memset(&config, 0, sizeof(config));

	application_t application;
	memset(&application, 0, sizeof(application));
	application.name = string_const(STRING_CONST("luadump"));
	application.short_name = application.name;
	application.config_dir = application.name;
	application.flags = APPLICATION_UTILITY;

	if ((ret = foundation_initialize(memory_system_malloc(), application, config)) < 0)
		return ret;

	return lua_module_initialize();
}

int
main_run(void* main_arg) {
	int result = LUADUMP_RESULT_OK;
	lua_dump_t dump = luadump_parse_command_line(environment_command_line());

	lua_State* state = 0;

	FOUNDATION_UNUSED(main_arg);

	if (!dump.input_file.length)
		return LUADUMP_RESULT_OK;

	dump.env = lua_allocate();
	state = lua_state(dump.env);

	if (lua_load_foundation(state) < 0) {
		result = LUADUMP_RESULT_UNABLE_TO_LOAD_FOUNDATION;
		goto exit;
	}

	/*if( ( result = luadump_load_jitbc( dump.env ) ) != LUADUMP_RESULT_OK )
		goto exit;

	lua_eval( dump.env, "jit.bc.dump(log.debug, nil, true)" );*/

	dump.input_stream = stream_open(STRING_ARGS(dump.input_file), STREAM_IN);
	if (!dump.input_stream || stream_eos(dump.input_stream)) {
		result = LUADUMP_RESULT_UNABLE_TO_OPEN_INPUT_FILE;
		goto exit;
	}

	if (dump.output_file.length) {
		dump.output_stream = stream_open(STRING_ARGS(dump.output_file),
		                                 STREAM_OUT | STREAM_CREATE | STREAM_TRUNCATE | (!dump.hex ? STREAM_BINARY : 0));
		if (!dump.output_stream) {
			result = LUADUMP_RESULT_UNABLE_TO_OPEN_OUTPUT_FILE;
			goto exit;
		}
	}

	lua_readstream_t read_stream = {
		.stream = dump.input_stream,
	};

	if (lua_load(state, lua_read_stream, &read_stream, "=eval") != 0) {
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua load failed: %s"), lua_tostring(state, -1));
		lua_pop(state, 1);
		result = LUADUMP_RESULT_FAILED_EVAL;
		goto exit;
	}

	lua_dump(state, lua_dump_writer, &dump);

	if (lua_pcall(state, 0, 0, 0) != 0) {
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua pcall failed: %s"), lua_tostring(state, -1));
		lua_pop(state, 1);
	}
	else {
		log_info(HASH_LUA, STRING_CONST("Lua bytecode dump successful"));
	}

exit:

	memory_deallocate(dump.bytecode);

	stream_deallocate(dump.input_stream);
	stream_deallocate(dump.output_stream);

	lua_deallocate(dump.env);

	string_deallocate(dump.input_file.str);
	string_deallocate(dump.output_file.str);

	return result;
}

void
main_finalize(void) {
	lua_module_finalize();
	foundation_finalize();
}

static lua_dump_t
luadump_parse_command_line(const string_const_t* cmdline) {
	int arg, asize;
	bool display_help = false;

	lua_dump_t dump;
	memset(&dump, 0, sizeof(dump));
	dump.suppress_level = ERRORLEVEL_INFO;

	error_context_push(STRING_CONST("parsing command line"), STRING_CONST(""));
	for (arg = 1, asize = array_size(cmdline); arg < asize; ++arg) {
		if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--help")))
			display_help = true;
		else if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--hex")))
			dump.hex = true;
		else if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--output"))) {
			if (arg < asize - 1) {
				++arg;
				string_deallocate(dump.output_file.str);
				dump.output_file = string_clone(STRING_ARGS(cmdline[arg]));
			}
		}
		else if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--debug")))
			dump.suppress_level = ERRORLEVEL_NONE;
		else if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--")))
			break; //Stop parsing cmdline options
		else {
			string_deallocate(dump.input_file.str);
			dump.input_file = string_clone(STRING_ARGS(cmdline[arg]));
		}
	}
	error_context_pop();

	if (!dump.input_file.length)
		display_help = true;

	if (display_help)
		luadump_print_usage();

	return dump;
}


#if 0

#include "lua/luajit/src/luajit.h"

LUA_API int luaopen_io(struct lua_State* L);
#define LUA_IOLIBNAME  "io"

static const char jit_vmdef[] = {
#include "vmdef.lua.hex"
	0
};

static const char jit_bc[] = {
#include "bc.lua.hex"
	0
};

LUA_API lua_t*      lua_env_from_state(struct lua_State* state);
LUA_API struct lua_State*       lua_state_from_env(lua_t* env);


static int luadump_load_jitbc(lua_t* env) {
	//Load IO lib which is disabled in init
	struct lua_State* state = lua_state_from_env(env);

	lua_pushcfunction(state, luaopen_io);
	lua_pushstring(state, LUA_IOLIBNAME);
	lua_call(state, 1, 0);

	log_debugf(HASH_LUA, "Define jit.vmdef module");
	if (lua_eval(env, jit_vmdef) != LUA_OK)
		return LUADUMP_RESULT_UNABLE_TO_LOAD_JITLIB_VMDEF;

	log_debugf(HASH_LUA, "Define jit.bc module");
	if (lua_eval(env, jit_bc) != LUA_OK)
		return LUADUMP_RESULT_UNABLE_TO_LOAD_JITLIB_BC;

	return LUADUMP_RESULT_OK;
}

#else

/*static int luadump_load_jitbc( lua_t* env )
{
	FOUNDATION_UNUSED( env );
	return LUADUMP_RESULT_OK;
}*/

#endif

static void
luadump_print_usage(void) {
	error_level_t level = log_suppress(HASH_LUA);
	log_set_suppress(HASH_LUA, ERRORLEVEL_DEBUG);
	log_info(HASH_LUA, STRING_CONST(
	             "luadump usage:\n"
	             "  luadump [--output <filename>] [--hex] [--help] file\n"
	             "    Optional arguments:\n"
	             "      --output <filename>          Output to <filename> instead of stdout\n"
	             "      --hex                        Output in hex format\n"
	             "      --help                       Show this message"
	         ));
	log_set_suppress(HASH_LUA, level);
}

