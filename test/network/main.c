/* main.c  -  Network bind test for lua library  -  Public Domain  -  2013 Mattias Jansson
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
#include <resource/resource.h>
#include <network/network.h>
#include <lua/lua.h>
#include <test/test.h>

static application_t
test_network_application(void) {
	application_t app;
	memset(&app, 0, sizeof(app));
	app.name = string_const(STRING_CONST("Lua network tests"));
	app.short_name = string_const(STRING_CONST("test_lua_network"));
	app.company = string_const(STRING_CONST(""));
	app.flags = APPLICATION_UTILITY;
	app.exception_handler = test_exception_handler;
	return app;
}

static memory_system_t
test_network_memory_system(void) {
	return memory_system_malloc();
}

static foundation_config_t
test_network_config(void) {
	foundation_config_t config;
	memset(&config, 0, sizeof(config));
	return config;
}

static void
test_parse_config(const char* path, size_t path_size, const char* buffer, size_t size, const json_token_t* tokens,
                  size_t num_tokens) {
	resource_module_parse_config(path, path_size, buffer, size, tokens, num_tokens);
	lua_module_parse_config(path, path_size, buffer, size, tokens, num_tokens);
}

static int
test_network_initialize(void) {
	lua_config_t lua_config;
	resource_config_t resource_config;
	network_config_t network_config;

	memset(&lua_config, 0, sizeof(lua_config));
	memset(&resource_config, 0, sizeof(resource_config));
	memset(&network_config, 0, sizeof(network_config));

	resource_config.enable_local_source = true;
	resource_config.enable_local_cache = true;
	resource_config.enable_remote_sourced = true;
	resource_config.enable_remote_compiled = true;
	resource_config.enable_local_autoimport = true;

	if (network_module_initialize(network_config) < 0)
		return -1;

	if (resource_module_initialize(resource_config) < 0)
		return -1;

	if (lua_module_initialize(lua_config) < 0)
		return -1;

	test_set_suitable_working_directory();
	test_load_config(test_parse_config);

	return 0;
}

static void
test_network_finalize(void) {
	lua_module_finalize();
	resource_module_finalize();
	network_module_finalize();
}

static void
test_network_event(event_t* event) {
	resource_event_handle(event);
}

DECLARE_TEST(network, log) {
	lua_t* env = lua_allocate();

	log_set_suppress(HASH_LUA, ERRORLEVEL_NONE);
	// log_set_suppress(HASH_RESOURCE, ERRORLEVEL_NONE);

	EXPECT_NE(env, 0);

	string_const_t testcode =
	    string_const(STRING_CONST("local ffi = require(\"ffi\")\n"
	                              "local foundation = require(\"foundation\")\n"
	                              "local network = require(\"network\")\n"
	                              "local C = ffi.C\n"
	                              "C.log_set_suppress(foundation.HASH_LUA, foundation.ERRORLEVEL_NONE)\n"
	                              "if C.network_module_is_initialized then"
	                              "	foundation.log.debug(\"Network module was initialized\")\n"
	                              "end\n"));

	EXPECT_EQ(lua_eval_string(env, STRING_ARGS(testcode)), LUA_OK);

	lua_deallocate(env);

	return 0;
}

static void
test_network_declare(void) {
	ADD_TEST(network, log);
}

static test_suite_t test_network_suite = {test_network_application, test_network_memory_system, test_network_config,
                                          test_network_declare,     test_network_initialize,    test_network_finalize,
                                          test_network_event};

#if BUILD_MONOLITHIC

int
test_network_run(void);

int
test_network_run(void) {
	test_suite = test_network_suite;
	return test_run_all();
}

#else

test_suite_t
test_suite_define(void);

test_suite_t
test_suite_define(void) {
	return test_network_suite;
}

#endif
