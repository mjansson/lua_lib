/* main.c  -  Lua interpreter for lua library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <foundation/foundation.h>
#include <lua/lua.h>
#include <lua/bind.h>
#include <lua/read.h>
#include <lua/hashstrings.h>
#include <lua/luajit.h>

#include "errorcodes.h"

#include <stdio.h>

typedef struct lua_instance_t lua_instance_t;

struct lua_instance_t {
	string_t       input_file;
	lua_t*         env;
	error_level_t  suppress_level;
};

static bool _lua_terminate = false;

static lua_instance_t
_lua_parse_command_line(const string_const_t* cmdline);

static void
_lua_print_usage(void);

static int
_lua_process_file(lua_t* lua, const char* filename, size_t length);

static int
_lua_interpreter(lua_t* lua);

//static int
//_lua_load_jitbc( lua_t* env );

LUA_API int
lua_load(lua_State* L, lua_Reader reader, void* dt, const char* chunkname);

LUA_API int
lua_pcall(lua_State* L, int nargs, int nresults, int errfunc);

static void*
event_thread(void* arg) {
	event_block_t* block;
	event_t* event = 0;

	FOUNDATION_UNUSED(arg);

	event_stream_set_beacon(system_event_stream(), &thread_self()->beacon);

	while (!_lua_terminate) {
		block = event_stream_process(system_event_stream());
		event = 0;

		while ((event = event_next(block, event))) {
			switch (event->id) {
			case FOUNDATIONEVENT_TERMINATE:
				//process_exit( LUA_RESULT_ABORTED );
				_lua_terminate = true;
				break;

			default:
				break;
			}
		}

		thread_wait();
	}

	return 0;
}

int
main_initialize(void) {
	int ret = 0;

	log_enable_prefix(false);
	log_set_suppress(0, ERRORLEVEL_DEBUG);
	log_set_suppress(HASH_LUA, ERRORLEVEL_DEBUG);

	foundation_config_t config;
	memset(&config, 0, sizeof(config));

	application_t application;
	memset(&application, 0, sizeof(application));
	application.name = string_const(STRING_CONST("lua"));
	application.short_name = application.name;
	application.config_dir = application.name;
	application.flags = APPLICATION_UTILITY;

	if ((ret = foundation_initialize(memory_system_malloc(), application, config)) < 0)
		return ret;

	lua_config_t lua_config;
	memset(&lua_config, 0, sizeof(lua_config_t));
	if ((ret = lua_module_initialize(lua_config)) < 0)
		return ret;

	return 0;
}

int
main_run(void* main_arg) {
	int result = LUA_RESULT_OK;
	thread_t eventthread;
	lua_State* state = 0;
	lua_instance_t instance = _lua_parse_command_line(environment_command_line());

	thread_initialize(&eventthread, event_thread, 0, STRING_CONST("event_thread"), THREAD_PRIORITY_NORMAL, 0);
	thread_start(&eventthread);

	instance.env = lua_allocate();
	state = lua_state(instance.env);

	if (instance.input_file.length)
		result = _lua_process_file(instance.env, STRING_ARGS(instance.input_file));
	else
		result = _lua_interpreter(instance.env);

	thread_signal(&eventthread);

	lua_deallocate(instance.env);
	string_deallocate(instance.input_file.str);

	thread_finalize(&eventthread);

	FOUNDATION_UNUSED(main_arg);

	return result;
}

void
main_finalize(void) {
	lua_module_finalize();
	foundation_finalize();
}

static int
_lua_interpreter(lua_t* lua) {
	stream_t* in = stream_open_stdin();
	stream_t* out = stream_open_stdout();

	string_t entry = {0, 0};
	string_t collated = {0, 0};
	lua_readstring_t read_string;
	lua_State* state = lua_state(lua);

	int status;

	log_enable_prefix(false);

	memset(&read_string, 0, sizeof(read_string));

	do {
		stream_write_string(out, collated.length ? "  >> " : "lua> ", 5);
		stream_flush(out);

		entry = stream_read_line(in, '\n');
		if (entry.length) {
			if (collated.length) {
				string_t last = collated;
				collated = string_allocate_concat_varg(STRING_ARGS(last), STRING_CONST("\n"), STRING_ARGS(entry), nullptr);
				read_string.string = collated.str;
				read_string.size = collated.length;
				string_deallocate(last.str);
			}
			else {
				read_string.string = entry.str;
				read_string.size = entry.length;
			}

			if ((status = lua_load(state, lua_read_string, &read_string, "=eval")) != 0) {
				bool continued = false;
				string_const_t msg = {0, 0};
				msg.str = lua_tolstring(state, -1, &msg.length);
				if (status == LUA_ERRSYNTAX) {
					if (string_ends_with(STRING_ARGS(msg), STRING_CONST("'<eof>'")))
						continued = true;
				}
				if (continued) {
					if (!collated.length) {
						collated = entry;
						entry = string(0, 0);
					}
				}
				else {
					if ((msg.length > 4) && (string_equal(msg.str, 5, STRING_CONST("eval:"))))
						msg = string_substr(STRING_ARGS(msg), 5, STRING_NPOS);
					log_errorf(HASH_LUA, ERROR_SCRIPT, STRING_CONST("%.*s"), STRING_FORMAT(msg));
				}
				lua_pop(state, 1);
			}
			else {
				if (collated.length)
					string_deallocate(collated.str);
				collated = string(0, 0);

				if (lua_pcall(state, 0, 0, 0) != 0) {
					string_const_t msg = {0, 0};
					msg.str = lua_tolstring(state, -1, &msg.length);
					if ((msg.length > 4) && (string_equal(msg.str, 5, STRING_CONST("eval:"))))
						msg = string_substr(STRING_ARGS(msg), 5, STRING_NPOS);
					log_errorf(HASH_LUA, ERROR_SCRIPT, STRING_CONST("%.*s"), STRING_FORMAT(msg));
					lua_pop(state, 1);
				}
			}
		}

		string_deallocate(entry.str);

	}
	while (!_lua_terminate && !stream_eos(in));

	stream_deallocate(in);
	stream_deallocate(out);

	return LUA_RESULT_OK;
}

static int
_lua_process_file(lua_t* lua, const char* filename, size_t length) {
	int result = LUA_RESULT_OK;
	stream_t* stream = stream_open(filename, length, STREAM_IN);
	if (!stream || stream_eos(stream)) {
		result = LUA_RESULT_UNABLE_TO_OPEN_INPUT_FILE;
		goto exit;
	}

	lua_readstream_t read_stream = {
		.stream = stream
	};

	lua_State* state = lua_state(lua);

	if (lua_load(state, lua_read_stream, &read_stream, "=eval") != 0) {
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua load failed: %s"), lua_tostring(state, -1));
		lua_pop(state, 1);
		result = LUA_RESULT_FAILED_EVAL;
		goto exit;
	}

	if (lua_pcall(state, 0, 0, 0) != 0) {
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua pcall failed: %s"), lua_tostring(state, -1));
		lua_pop(state, 1);
	}
	else {
		log_info(HASH_LUA, STRING_CONST("Lua interpretation successful"));
	}

exit:

	stream_deallocate(stream);

	return result;
}

static lua_instance_t
_lua_parse_command_line(const string_const_t* cmdline) {
	int arg, asize;
	bool display_help = false;

	lua_instance_t instance;
	memset(&instance, 0, sizeof(instance));
	instance.suppress_level = ERRORLEVEL_INFO;

	error_context_push(STRING_CONST("parsing command line"), STRING_CONST(""));
	for (arg = 1, asize = array_size(cmdline); arg < asize; ++arg) {
		if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--help")))
			display_help = true;
		else if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--debug")))
			instance.suppress_level = ERRORLEVEL_NONE;
		else if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--")))
			break; //Stop parsing cmdline options
		else {
			string_deallocate(instance.input_file.str);
			instance.input_file = string_clone(STRING_ARGS(cmdline[arg]));
		}
	}
	error_context_pop();

	if (display_help)
		_lua_print_usage();

	return instance;
}

static void
_lua_print_usage(void) {
	log_set_suppress(HASH_LUA, ERRORLEVEL_DEBUG);
	log_info(HASH_LUA, STRING_CONST(
	         "lua usage:\n"
	         "  lua [--help] [file]\n"
	         "    Optional arguments:\n"
	         "      --help           Show this message\n"
	         "      <file>           Read <file> instead of stdin\n"
	        ));
	log_set_suppress(HASH_LUA, ERRORLEVEL_INFO);
}

