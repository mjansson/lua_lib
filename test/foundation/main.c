/* main.c  -  Foundation bind test for lua library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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
#include <test/test.h>

static application_t
test_foundation_application(void) {
	application_t app;
	memset(&app, 0, sizeof(app));
	app.name = string_const(STRING_CONST("Lua foundation tests"));
	app.short_name = string_const(STRING_CONST("test_lua_foundation"));
	app.company = string_const(STRING_CONST("Rampant Pixels"));
	app.flags = APPLICATION_UTILITY;
	app.exception_handler = test_exception_handler;
	return app;
}

static memory_system_t
test_foundation_memory_system(void) {
	return memory_system_malloc();
}

static foundation_config_t
test_foundation_config(void) {
	foundation_config_t config;
	memset(&config, 0, sizeof(config));
	return config;
}

static void test_parse_config(const char* buffer, size_t size,
                              const json_token_t* tokens, size_t num_tokens) {
	resource_module_parse_config(buffer, size, tokens, num_tokens);
	lua_module_parse_config(buffer, size, tokens, num_tokens);
}

static int
test_foundation_initialize(void) {
	lua_config_t lua_config;
	resource_config_t resource_config;

	log_set_suppress(HASH_RESOURCE, ERRORLEVEL_NONE);

	memset(&lua_config, 0, sizeof(lua_config));
	memset(&resource_config, 0, sizeof(resource_config));

	resource_config.enable_local_source = true;
	resource_config.enable_local_cache = true;
	resource_config.enable_remote_cache = true;

	if (resource_module_initialize(resource_config) < 0)
		return -1;

	if (lua_module_initialize(lua_config) < 0)
		return -1;

	test_set_suitable_working_directory();
	test_load_config(test_parse_config);

	return 0;
}

static void
test_foundation_finalize(void) {
	lua_module_finalize();
	resource_module_finalize();
}

static void
test_foundation_event(event_t* event) {
	resource_event_handle(event);
}

DECLARE_TEST(foundation, log) {
	lua_t* env = lua_allocate();

	log_set_suppress(HASH_LUA, ERRORLEVEL_NONE);

	EXPECT_NE(env, 0);

	string_const_t testcode = string_const(STRING_CONST(
	    "local ffi = require(\"ffi\")\n"
	    "local foundation = require(\"foundation\")\n"
	    "local C = ffi.C\n"
	    "C.log_set_suppress(foundation.HASH_LUA, foundation.ERRORLEVEL_NONE)\n"
	    "foundation.log.debug(\"Testing log debug output\")\n"
	    "foundation.log.info(\"Testing log info output\")\n"
	    "foundation.log.warn(\"Testing log warning output\")\n"
	    "C.log_enable_prefix(false)\n"
	    "foundation.log.error(\"Testing log error output without prefix\")\n"
	    "C.log_enable_stdout(false)\n"
	    "foundation.log.debug(\"Invisible on stdout\")\n"
	    "C.log_enable_stdout(true)\n"
	    "C.log_enable_prefix(true)\n"
	    "C.log_set_suppress(foundation.HASH_LUA, foundation.ERRORLEVEL_INFO)\n"
	));

	EXPECT_EQ(lua_eval_string(env, STRING_ARGS(testcode)), LUA_OK);
	EXPECT_EQ(error(), ERROR_SCRIPT);

	lua_deallocate(env);

	return 0;
}

DECLARE_TEST(foundation, environment) {
	lua_t* env = lua_allocate();

	log_set_suppress(HASH_LUA, ERRORLEVEL_NONE);
	log_info(HASH_LUA, STRING_CONST("Running environment lua tests"));

	EXPECT_NE(env, 0);

	string_const_t testcode = string_const(STRING_CONST(
	    "local ffi = require(\"ffi\")\n"
	    "local foundation = require(\"foundation\")\n"
	    "local C = ffi.C\n"
	    "C.log_set_suppress(foundation.HASH_LUA, foundation.ERRORLEVEL_DEBUG)\n"
	    "C.log_enable_prefix(false)\n"
	    "foundation.log.info(\"Executable name: \" .. tostring(C.environment_executable_name()))\n"
	    "local cmdline = \"\"\n"
	    "local cmdline_tab = C.environment_command_line()\n"
	    "local num = C.array_size(cmdline_tab)"
	    "for i = 0, num-1 do\n"
	    "  cmdline = cmdline .. \" \" .. tostring(cmdline_tab[i])\n"
	    "end\n"
	    "foundation.log.info(\"Command line:\" .. cmdline)\n"
	    "C.log_enable_prefix(true)\n"
	    "C.log_set_suppress(foundation.HASH_LUA, foundation.ERRORLEVEL_INFO)\n"
	));

	EXPECT_EQ(lua_eval_string(env, STRING_ARGS(testcode)), LUA_OK);

	log_info(HASH_LUA, STRING_CONST("Done running environment lua tests"));
	log_set_suppress(HASH_LUA, ERRORLEVEL_INFO);

	lua_deallocate(env);

	return 0;
}

static void
test_foundation_declare(void) {
	ADD_TEST(foundation, log);
	ADD_TEST(foundation, environment);
}

test_suite_t test_foundation_suite = {
	test_foundation_application,
	test_foundation_memory_system,
	test_foundation_config,
	test_foundation_declare,
	test_foundation_initialize,
	test_foundation_finalize,
	test_foundation_event
};

#if BUILD_MONOLITHIC

int
test_foundation_run(void);

int
test_foundation_run(void) {
	test_suite = test_foundation_suite;
	return test_run_all();
}

#else

test_suite_t
test_suite_define(void);

test_suite_t
test_suite_define(void) {
	return test_foundation_suite;
}

#endif
