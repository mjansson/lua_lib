/* main.c  -  Lua library importer  -  Public Domain  -  2016 Mattias Jansson / Rampant Pixels
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
#include <resource/resource.h>
#include <lua/lua.h>

#include "main.h"
#include "errorcodes.h"

typedef struct {
	bool              display_help;
	int               binary;
	string_const_t    source_path;
	string_const_t*   config_files;
	string_const_t*   input_files;
} luaimport_input_t;

typedef struct {
	char*          bytecode;
	size_t         bytecode_size;
} luaimport_dump_t;

LUA_API int
lua_load(lua_State* L, lua_Reader reader, void* dt, const char* chunkname);

LUA_API int
lua_dump(lua_State* L, lua_Writer writer, void* data);

LUA_API int
lua_pcall(lua_State* L, int nargs, int nresults, int errfunc);

static void
luaimport_parse_config(const char* buffer, size_t size, json_token_t* tokens,
                       size_t numtokens);

static luaimport_input_t
luaimport_parse_command_line(const string_const_t* cmdline);

static void
luaimport_load_config(const char* path, size_t length);

static void
luaimport_print_usage(void);

static FOUNDATION_NOINLINE int
luaimport_dump_writer(lua_State* state, const void* buffer, size_t size, void* user_data) {
	luaimport_dump_t* dump = user_data;

	FOUNDATION_UNUSED(state);

	if (size <= 0)
		return 0;

	dump->bytecode = (dump->bytecode ?
	                  memory_reallocate(dump->bytecode, dump->bytecode_size + size, 0, dump->bytecode_size) :
	                  memory_allocate(HASH_LUA, size, 0, MEMORY_PERSISTENT));

	memcpy(dump->bytecode + dump->bytecode_size, buffer, size);
	dump->bytecode_size += size;

	return 0;
}

static int
luaimport_import_stream(stream_t* stream, const uuid_t uuid, luaimport_dump_t* dump) {
	lua_t* env;
	lua_State* state;
	int result = 0;
	lua_readstream_t read_stream = {
		.stream = stream,
	};

	env = lua_allocate();
	state = lua_state(env);

	lua_symbol_load_foundation();

	if (lua_load(state, lua_read_stream, &read_stream, "import") != 0) {
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua load failed: %s"), lua_tostring(state, -1));
		lua_pop(state, 1);
		result = LUAIMPORT_RESULT_FAILED_EVAL;
		goto exit;
	}

	lua_dump(state, luaimport_dump_writer, dump);

	if (lua_pcall(state, 0, 0, 0) != 0) {
		log_errorf(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Lua pcall failed: %s"), lua_tostring(state, -1));
		lua_pop(state, 1);
	}
	else {
		log_info(HASH_LUA, STRING_CONST("Lua bytecode dump successful"));
	}

exit:

	lua_deallocate(env);

	return result;
}

static int
luaimport_output(const uuid_t uuid, const luaimport_dump_t* dump) {
	resource_source_t source;
	hash_t checksum;
	tick_t timestamp;
	uint64_t platform;
	string_const_t type = string_const(STRING_CONST("lua"));
	int ret = 0;

	resource_source_initialize(&source);
	resource_source_read(&source, uuid);

	timestamp = time_system();
	platform = 0;

	checksum = hash(dump->bytecode, dump->bytecode_size);
	if (resource_source_write_blob(uuid, timestamp, HASH_SOURCE,
	                               platform, checksum,
	                               dump->bytecode, dump->bytecode_size)) {
		resource_source_set_blob(&source, timestamp, HASH_SOURCE,
		                         platform, checksum, dump->bytecode_size);
	}
	else {
		ret = LUAIMPORT_RESULT_UNABLE_TO_WRITE_BLOB;
		goto finalize;
	}

	resource_source_set(&source, timestamp, HASH_RESOURCE_TYPE,
	                    0, STRING_ARGS(type));

	if (!resource_source_write(&source, uuid, false)) {
		ret = LUAIMPORT_RESULT_UNABLE_TO_WRITE_SOURCE;
		goto finalize;
	}

finalize:

	resource_source_finalize(&source);

	return ret;
}

int
luaimport_import(stream_t* stream, const uuid_t uuid_given) {
	uuid_t uuid = uuid_given;
	string_const_t path;
	string_const_t extension;
	bool store_import = false;
	luaimport_dump_t dump = { 0, 0 };
	int ret;

	path = stream_path(stream);
	extension = path_file_extension(STRING_ARGS(path));
	if (!string_equal_nocase(STRING_ARGS(extension), STRING_CONST("lua")))
		return LUAIMPORT_RESULT_UNSUPPORTED_INPUT;

	if (uuid_is_null(uuid))
		uuid = resource_import_map_lookup(STRING_ARGS(path));
	
	if (uuid_is_null(uuid)) {
		uuid = uuid_generate_random();
		store_import = true;
	}
	
	if (store_import) {
		if (!resource_import_map_store(STRING_ARGS(path), &uuid)) {
			log_warn(HASH_RESOURCE, WARNING_SUSPICIOUS,
			         STRING_CONST("Unable to open import map file to store new resource"));
			return LUAIMPORT_RESULT_UNABLE_TO_OPEN_MAP_FILE;
		}
	}

	if ((ret = luaimport_import_stream(stream, uuid, &dump)) < 0)
		goto exit;

	if ((ret = luaimport_output(uuid, &dump)) < 0)
		goto exit;

exit:

	memory_deallocate(dump.bytecode);

	return ret;
}

int
main_initialize(void) {
	int ret = 0;
	application_t application;
	foundation_config_t foundation_config;
	resource_config_t resource_config;
	lua_config_t lua_config;

	memset(&foundation_config, 0, sizeof(foundation_config));
	memset(&resource_config, 0, sizeof(resource_config));
	memset(&lua_config, 0, sizeof(lua_config));

	memset(&application, 0, sizeof(application));
	application.name = string_const(STRING_CONST("luaimport"));
	application.short_name = string_const(STRING_CONST("luaimport"));
	application.company = string_const(STRING_CONST("Rampant Pixels"));
	application.flags = APPLICATION_UTILITY;

	log_enable_prefix(false);
	log_set_suppress(0, ERRORLEVEL_WARNING);

	resource_config.enable_local_source = true;
	resource_config.enable_local_cache = true;
	resource_config.enable_remote_cache = true;

	if ((ret = foundation_initialize(memory_system_malloc(), application, foundation_config)) < 0)
		return ret;
	if ((ret = resource_module_initialize(resource_config)) < 0)
		return ret;
	if ((ret = lua_module_initialize(lua_config)) < 0)
		return ret;

	log_set_suppress(HASH_RESOURCE, ERRORLEVEL_DEBUG);

	return 0;
}

int
main_run(void* main_arg) {
	int result = LUAIMPORT_RESULT_OK;
	luaimport_input_t input = luaimport_parse_command_line(environment_command_line());

	FOUNDATION_UNUSED(main_arg);

	for (size_t cfgfile = 0, fsize = array_size(input.config_files); cfgfile < fsize; ++cfgfile)
		sjson_parse_path(STRING_ARGS(input.config_files[cfgfile]), luaimport_parse_config);

	if (input.source_path.length)
		resource_source_set_path(STRING_ARGS(input.source_path));

	if (!resource_source_path().length) {
		log_errorf(HASH_RESOURCE, ERROR_INVALID_VALUE, STRING_CONST("No source path given"));
		input.display_help = true;
	}

	if (input.display_help) {
		luaimport_print_usage();
		goto exit;
	}

	resource_import_register(luaimport_import);

	size_t ifile, fsize;
	for (ifile = 0, fsize = array_size(input.input_files); ifile < fsize; ++ifile) {
		if (resource_import(STRING_ARGS(input.input_files[ifile]), uuid_null()))
			log_infof(HASH_RESOURCE, STRING_CONST("Successfully imported: %.*s"),
			          STRING_FORMAT(input.input_files[ifile]));
		else
			log_warnf(HASH_RESOURCE, WARNING_UNSUPPORTED, STRING_CONST("Failed to import: %.*s"),
			          STRING_FORMAT(input.input_files[ifile]));
	}

exit:

	array_deallocate(input.config_files);
	array_deallocate(input.input_files);

	return result;
}

void
main_finalize(void) {
	lua_module_finalize();
	resource_module_finalize();
	foundation_finalize();
}

static void
luaimport_parse_config(const char* buffer, size_t size, json_token_t* tokens,
                       size_t numtokens) {
	resource_module_parse_config(buffer, size, tokens, numtokens);
	lua_module_parse_config(buffer, size, tokens, numtokens);
}

static luaimport_input_t
luaimport_parse_command_line(const string_const_t* cmdline) {
	luaimport_input_t input;
	size_t arg, asize;

	memset(&input, 0, sizeof(input));

	for (arg = 1, asize = array_size(cmdline); arg < asize; ++arg) {
		if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--help")))
			input.display_help = true;
		else if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--source"))) {
			if (arg < asize - 1)
				input.source_path = cmdline[++arg];
		}
		else if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--config"))) {
			if (arg < asize - 1)
				array_push(input.config_files, cmdline[++arg]);
		}
		/*else if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--uuid"))) {
			if (arg < asize - 1) {
				++arg;
				input.uuid = string_to_uuid(STRING_ARGS(cmdline[arg]));
				if (uuid_is_null(input.uuid))
					log_warnf(HASH_RESOURCE, WARNING_INVALID_VALUE, STRING_CONST("Invalid UUID: %.*s"),
					          STRING_FORMAT(cmdline[arg]));
			}
		}
		else if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--set"))) {
			if (arg < asize - 2) {
				input.key = cmdline[++arg];
				input.value = cmdline[++arg];
			}
		}*/
		else if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--binary"))) {
			input.binary = 1;
		}
		else if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--ascii"))) {
			input.binary = 0;
		}
		else if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--debug"))) {
			log_set_suppress(0, ERRORLEVEL_NONE);
			log_set_suppress(HASH_RESOURCE, ERRORLEVEL_NONE);
			log_set_suppress(HASH_SCRIPT, ERRORLEVEL_NONE);
			log_set_suppress(HASH_LUA, ERRORLEVEL_NONE);
		}
		else if (string_equal(STRING_ARGS(cmdline[arg]), STRING_CONST("--")))
			break; //Stop parsing cmdline options
		else {
			array_push(input.input_files, cmdline[arg]);
		}
	}
	error_context_pop();

	if (!array_size(input.input_files)) {
		log_errorf(HASH_RESOURCE, ERROR_INVALID_VALUE, STRING_CONST("No input files given"));
		input.display_help = true;
	}

	return input;
}

static void
luaimport_print_usage(void) {
	const error_level_t saved_level = log_suppress(0);
	log_set_suppress(0, ERRORLEVEL_DEBUG);
	log_info(0, STRING_CONST(
	             "luaimport usage:\n"
	             "  luaimport [--source <path>] [--config <path> ...] [--ascii] [--binary]\n"
	             "            [--debug] [--help] <file> <file> ... [--]\n"
	             "    Arguments:\n"
	             "      <file> <file> ...            Any number of input files\n"
	             "    Optional arguments:\n"
	             "      --source <path>              Operate on resource file source structure given by <path>\n"
	             "      --config <file>              Read and parse config file given by <path>\n"
	             "                                   Loads all .json/.sjson files in <path> if it is a directory\n"
	             "      --binary                     Write binary files\n"
	             "      --ascii                      Write ASCII files (default)\n"
	             "      --debug                      Enable debug output\n"
	             "      --help                       Display this help message\n"
	             "      --                           Stop processing command line arguments"
	         ));
	log_set_suppress(0, saved_level);
}
