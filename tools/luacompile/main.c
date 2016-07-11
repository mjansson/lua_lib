/* main.c  -  Lua library compiler  -  Public Domain  -  2016 Mattias Jansson / Rampant Pixels
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

#include "errorcodes.h"

typedef struct {
	bool              display_help;
	int               binary;
	string_const_t    source_path;
	string_const_t*   config_files;
	string_const_t*   input_files;
} luacompile_input_t;

static void
luacompile_parse_config(const char* path, size_t path_size,
                        const char* buffer, size_t size,
                        const json_token_t* tokens, size_t numtokens);

static luacompile_input_t
luacompile_parse_command_line(const string_const_t* cmdline);

static void
luacompile_print_usage(void);

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
	application.name = string_const(STRING_CONST("luacompile"));
	application.short_name = string_const(STRING_CONST("luacompile"));
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
	log_set_suppress(HASH_SCRIPT, ERRORLEVEL_INFO);
	log_set_suppress(HASH_LUA, ERRORLEVEL_INFO);

	return 0;
}

int
main_run(void* main_arg) {
	int result = LUACOMPILE_RESULT_OK;
	luacompile_input_t input = luacompile_parse_command_line(environment_command_line());

	FOUNDATION_UNUSED(main_arg);

	for (size_t cfgfile = 0, fsize = array_size(input.config_files); cfgfile < fsize; ++cfgfile)
		sjson_parse_path(STRING_ARGS(input.config_files[cfgfile]), luacompile_parse_config);

	if (input.source_path.length)
		resource_source_set_path(STRING_ARGS(input.source_path));

	if (!resource_source_path().length) {
		log_errorf(HASH_RESOURCE, ERROR_INVALID_VALUE, STRING_CONST("No source path given"));
		input.display_help = true;
	}

	if (input.display_help) {
		luacompile_print_usage();
		goto exit;
	}

	resource_compile_register(lua_compile);

	size_t ifile, fsize;
	for (ifile = 0, fsize = array_size(input.input_files); ifile < fsize; ++ifile) {
		uuid_t uuid = string_to_uuid(STRING_ARGS(input.input_files[ifile]));
		if (uuid_is_null(uuid)) {
			char buffer[BUILD_MAX_PATHLEN];
			string_t pathstr = string_copy(buffer, sizeof(buffer), STRING_ARGS(input.input_files[ifile]));
			pathstr = path_clean(STRING_ARGS(pathstr), sizeof(buffer));
			pathstr = path_absolute(STRING_ARGS(pathstr), sizeof(buffer));
			uuid = resource_import_map_lookup(STRING_ARGS(pathstr)).uuid;
		}
		if (uuid_is_null(uuid)) {
			log_warnf(HASH_RESOURCE, WARNING_INVALID_VALUE, STRING_CONST("Failed to lookup: %.*s"),
			          STRING_FORMAT(input.input_files[ifile]));
			result = LUACOMPILE_RESULT_INVALID_INPUT;
			break;
		}

		if (resource_compile(uuid, RESOURCE_PLATFORM_ALL)) {
			string_const_t uuidstr = string_from_uuid_static(uuid);
			log_infof(HASH_RESOURCE, STRING_CONST("Successfully compiled: %.*s (%.*s)"),
			          STRING_FORMAT(uuidstr), STRING_FORMAT(input.input_files[ifile]));
		}
		else {
			string_const_t uuidstr = string_from_uuid_static(uuid);
			log_warnf(HASH_RESOURCE, WARNING_UNSUPPORTED, STRING_CONST("Failed to compile: %.*s (%.*s)"),
			          STRING_FORMAT(uuidstr), STRING_FORMAT(input.input_files[ifile]));
		}
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
luacompile_parse_config(const char* path, size_t path_size,
                        const char* buffer, size_t size,
                        const json_token_t* tokens, size_t numtokens) {
	resource_module_parse_config(path, path_size, buffer, size, tokens, numtokens);
	lua_module_parse_config(path, path_size, buffer, size, tokens, numtokens);
}

static luacompile_input_t
luacompile_parse_command_line(const string_const_t* cmdline) {
	luacompile_input_t input;
	size_t arg, asize;

	error_context_push(STRING_CONST("parse command line"), STRING_CONST(""));
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
luacompile_print_usage(void) {
	const error_level_t saved_level = log_suppress(0);
	log_set_suppress(0, ERRORLEVEL_DEBUG);
	log_info(0, STRING_CONST(
	             "luacompile usage:\n"
	             "  luacompile [--source <path>] [--config <path> ...] [--ascii] [--binary]\n"
	             "             [--debug] [--help] <file> <uuid> ... [--]\n"
	             "    Arguments:\n"
	             "      <file> <uuid> ...            Any number of input files or UUIDs\n"
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
