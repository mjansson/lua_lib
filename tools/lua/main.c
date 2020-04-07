/* main.c  -  Lua interpreter for lua library  -  Public Domain  -  2013 Mattias Jansson
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

#include <foundation/foundation.h>
#include <network/network.h>
#include <resource/resource.h>
#include <lua/lua.h>
#include <lua/bind.h>
#include <lua/read.h>
#include <lua/hashstrings.h>
#include <lua/luajit.h>

#include "errorcodes.h"

#include <stdio.h>

typedef struct lua_instance_t lua_instance_t;

struct lua_instance_t {
	string_const_t* config_files;
	string_t        input_file;
	lua_t*          env;
	mutex_t*        lock;
	error_level_t   suppress_level;
};

static atomic32_t _lua_terminate_flag;

static void
_lua_terminate(void);

static bool
_lua_should_terminate(void);

static lua_instance_t
_lua_parse_command_line(const string_const_t* cmdline);

static void
_lua_print_usage(void);

static int
_lua_process_file(lua_t* lua, const char* filename, size_t length);

static void
_lua_process_resource_event(lua_t* lua, mutex_t* lock, const event_t* event);

static int
_lua_interpreter(lua_t* lua, mutex_t* lock);

static void
_lua_parse_config(const char* path, size_t path_size,
                  const char* buffer, size_t size,
                  const json_token_t* tokens, size_t numtokens);

static void
_lua_load_default_config(void);

//static int
//_lua_load_jitbc( lua_t* env );

LUA_API int
lua_load(lua_State* L, lua_Reader reader, void* dt, const char* chunkname);

LUA_API int
lua_pcall(lua_State* L, int nargs, int nresults, int errfunc);

static void
event_process_system(void) {
	event_t* event = nullptr;
	event_block_t* const block = event_stream_process(system_event_stream());

	while ((event = event_next(block, event))) {
		switch (event->id) {
		case FOUNDATIONEVENT_TERMINATE:
			//process_exit( LUA_RESULT_ABORTED );
			_lua_terminate();
			break;

		default:
			break;
		}
	}
}

static void
event_process_fs(void) {
	event_t* event = nullptr;
	event_block_t* const block = event_stream_process(fs_event_stream());

	while ((event = event_next(block, event))) {
		resource_event_handle(event);
	}
}

static void
event_process_resource(lua_t* lua, mutex_t* lock) {
	event_t* event = nullptr;
	event_block_t* const block = event_stream_process(resource_event_stream());

	while ((event = event_next(block, event))) {
		_lua_process_resource_event(lua, lock, event);
	}
}

static void*
event_thread(void* arg) {
	lua_instance_t* instance = arg;

	event_stream_set_beacon(system_event_stream(), &thread_self()->beacon);
	event_stream_set_beacon(fs_event_stream(), &thread_self()->beacon);
	event_stream_set_beacon(resource_event_stream(), &thread_self()->beacon);

	while (!_lua_should_terminate()) {
		event_process_system();
		event_process_fs();
		event_process_resource(instance->env, instance->lock);
		thread_wait();
	}

	return 0;
}

int
main_initialize(void) {
	int ret = 0;

	log_enable_prefix(false);
	log_set_suppress(0, ERRORLEVEL_INFO);
	log_set_suppress(HASH_LUA, ERRORLEVEL_DEBUG);

	foundation_config_t config;
	memset(&config, 0, sizeof(config));

	application_t application;
	memset(&application, 0, sizeof(application));
	application.name = string_const(STRING_CONST("lua"));
	application.short_name = application.name;
	application.company = string_const(STRING_CONST(""));
	application.flags = APPLICATION_UTILITY;

	if ((ret = foundation_initialize(memory_system_malloc(), application, config)) < 0)
		return ret;

	network_config_t network_config;
	memset(&network_config, 0, sizeof(network_config_t));
	if ((ret = network_module_initialize(network_config)) < 0)
		return ret;

	resource_config_t resource_config;
	memset(&resource_config, 0, sizeof(resource_config_t));
	resource_config.enable_local_cache = true;
	resource_config.enable_local_source = true;
	resource_config.enable_local_autoimport = true;
	resource_config.enable_remote_sourced = true;
	resource_config.enable_remote_compiled = true;
	if ((ret = resource_module_initialize(resource_config)) < 0)
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
	lua_instance_t instance = _lua_parse_command_line(environment_command_line());

	log_set_suppress(HASH_LUA, instance.suppress_level);
	log_set_suppress(HASH_NETWORK, instance.suppress_level);
	log_set_suppress(HASH_RESOURCE, instance.suppress_level);
	log_set_suppress(0, instance.suppress_level);
	log_set_suppress(HASH_SCRIPT, ERRORLEVEL_NONE);

	thread_initialize(&eventthread, event_thread, &instance, STRING_CONST("event_thread"),
	                  THREAD_PRIORITY_NORMAL, 0);
	thread_start(&eventthread);

	for (size_t cfgfile = 0, fsize = array_size(instance.config_files); cfgfile < fsize; ++cfgfile)
		sjson_parse_path(STRING_ARGS(instance.config_files[cfgfile]), _lua_parse_config);

	if (array_size(instance.config_files) == 0)
		_lua_load_default_config();

	instance.env = lua_allocate();

	if (instance.input_file.length) {
		result = _lua_process_file(instance.env, STRING_ARGS(instance.input_file));
	}
	else {
		instance.lock = mutex_allocate(STRING_CONST("lua"));
		result = _lua_interpreter(instance.env, instance.lock);
	}

	_lua_terminate();
	thread_signal(&eventthread);

	lua_deallocate(instance.env);
	mutex_deallocate(instance.lock);
	string_deallocate(instance.input_file.str);
	array_deallocate(instance.config_files);

	thread_finalize(&eventthread);

	FOUNDATION_UNUSED(main_arg);

	return result;
}

void
main_finalize(void) {
	lua_module_finalize();
	resource_module_finalize();
	network_module_finalize();
	foundation_finalize();
}

static bool
_lua_should_terminate(void) {
	return atomic_load32(&_lua_terminate_flag, memory_order_acquire) > 0;
}

static void
_lua_terminate(void) {
	atomic_store32(&_lua_terminate_flag, 1, memory_order_release);
}

static int
_lua_interpreter(lua_t* lua, mutex_t* lock) {
	stream_t* in = stream_open_stdin();
	stream_t* out = stream_open_stdout();

	string_t entry = {0, 0};
	string_t collated = {0, 0};
	lua_readstring_t read_string;
	lua_State* state = lua_state(lua);

	int status;

	log_enable_prefix(true);

	memset(&read_string, 0, sizeof(read_string));

	do {
		stream_write_string(out, collated.length ? "  >> " : "lua> ", 5);
		stream_flush(out);

		entry = stream_read_line(in, '\n');
		if (entry.length) {
			if (collated.length) {
				string_t last = collated;
				collated = string_allocate_concat_varg(STRING_ARGS(last), STRING_CONST("\n"), STRING_ARGS(entry),
				                                       nullptr);
				read_string.string = collated.str;
				read_string.size = collated.length;
				string_deallocate(last.str);
			}
			else {
				read_string.string = entry.str;
				read_string.size = entry.length;
			}

			if (lock)
				mutex_lock(lock);

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

			lua_timed_gc(lua, 0);

			if (lock)
				mutex_unlock(lock);
		}

		string_deallocate(entry.str);

	}
	while (!_lua_should_terminate() && !stream_eos(in));

	stream_deallocate(in);
	stream_deallocate(out);

	return LUA_RESULT_OK;
}

static void
_lua_process_resource_event(lua_t* lua, mutex_t* lock, const event_t* event) {
	FOUNDATION_UNUSED(lua);
	if (lock)
		mutex_lock(lock);
	lua_event_handle_resource(event);
	if (lock)
		mutex_unlock(lock);
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
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua load failed: %s"),
		           lua_tostring(state, -1));
		lua_pop(state, 1);
		result = LUA_RESULT_FAILED_EVAL;
		goto exit;
	}

	if (lua_pcall(state, 0, 0, 0) != 0) {
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua pcall failed: %s"),
		           lua_tostring(state, -1));
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
	unsigned int arg, asize;
	bool display_help = false;
	lua_instance_t instance;

	error_context_push(STRING_CONST("parse command line"), STRING_CONST(""));

	memset(&instance, 0, sizeof(instance));
	instance.suppress_level = ERRORLEVEL_INFO;

	for (arg = 1, asize = array_size(cmdline); arg < asize; ++arg) {
		if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--config"))) {
			if (arg < asize - 1)
				array_push(instance.config_files, cmdline[++arg]);
		}
		else if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--help")))
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
_lua_parse_config(const char* path, size_t path_size,
                  const char* buffer, size_t size,
                  const json_token_t* tokens, size_t numtokens) {
	resource_module_parse_config(path, path_size, buffer, size, tokens, numtokens);
	lua_module_parse_config(path, path_size, buffer, size, tokens, numtokens);
}

static void
_lua_print_usage(void) {
	log_set_suppress(HASH_LUA, ERRORLEVEL_DEBUG);
	log_info(HASH_LUA, STRING_CONST(
	             "lua usage:\n"
	             "  lua [--config <path> ...] [--help] [file] [--]\n"
	             "    Optional arguments:\n"
	             "      --config <file>  Read and parse config file given by <path>\n"
	             "                       Loads all .json/.sjson files in <path> if it is a directory\n"
	             "      --help           Show this message\n"
	             "      <file>           Read <file> instead of stdin\n"
	             "      --               Stop processing command line arguments"
	         ));
	log_set_suppress(HASH_LUA, ERRORLEVEL_INFO);
}

static void
_lua_load_default_config(void) {
	char buffer[BUILD_MAX_PATHLEN];
	
	string_const_t last_dir;
	string_t config_dir;
	string_const_t current_dir = environment_current_working_directory();
	do {
		last_dir = current_dir;
		config_dir = path_concat(buffer, sizeof(buffer), STRING_ARGS(current_dir), STRING_CONST("config"));
		if (fs_is_directory(STRING_ARGS(config_dir))) {
			sjson_parse_path(STRING_ARGS(config_dir), _lua_parse_config);
			break;
		}
		
		current_dir = path_directory_name(STRING_ARGS(current_dir));
	}
	while (!string_equal(STRING_ARGS(current_dir), STRING_ARGS(last_dir)));
}

