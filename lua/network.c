/* network.c  -  Lua library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <lua/lua.h>

#include <foundation/foundation.h>
#include <network/network.h>

static bool _symbols_loaded;

void
lua_symbol_load_network(void) {
	hashmap_t* map = lua_symbol_lookup_map();

	if (_symbols_loaded)
		return;
	_symbols_loaded = true;

#define NETWORK_SYM(fn, name) hashmap_insert(map, HASH_SYM_##name, (void*)(uintptr_t)fn)

	NETWORK_SYM(network_address_clone, NETWORK_ADDRESS_CLONE);
	NETWORK_SYM(network_address_resolve, NETWORK_ADDRESS_RESOLVE);
	NETWORK_SYM(network_address_to_string, NETWORK_ADDRESS_TO_STRING);
	NETWORK_SYM(network_address_ipv4_initialize, NETWORK_ADDRESS_IPV4_INITIALIZE);
	NETWORK_SYM(network_address_ipv6_initialize, NETWORK_ADDRESS_IPV6_INITIALIZE);
	NETWORK_SYM(network_address_ip_set_port, NETWORK_ADDRESS_IP_SET_PORT);
	NETWORK_SYM(network_address_ip_port, NETWORK_ADDRESS_IP_PORT);
	NETWORK_SYM(network_address_ipv4_set_ip, NETWORK_ADDRESS_IPV4_SET_IP);
	NETWORK_SYM(network_address_ipv4_ip, NETWORK_ADDRESS_IPV4_IP);
	NETWORK_SYM(network_address_ipv6_set_ip, NETWORK_ADDRESS_IPV6_SET_IP);
	NETWORK_SYM(network_address_ipv6_ip, NETWORK_ADDRESS_IPV6_IP);
	NETWORK_SYM(network_address_type, NETWORK_ADDRESS_TYPE);
	NETWORK_SYM(network_address_family, NETWORK_ADDRESS_FAMILY);
	NETWORK_SYM(network_address_local, NETWORK_ADDRESS_LOCAL);
	NETWORK_SYM(network_address_equal, NETWORK_ADDRESS_EQUAL);
	NETWORK_SYM(network_address_array_deallocate, NETWORK_ADDRESS_ARRAY_DEALLOCATE);

	NETWORK_SYM(network_module_initialize, NETWORK_MODULE_INITIALIZE);
	NETWORK_SYM(network_module_finalize, NETWORK_MODULE_FINALIZE);
	NETWORK_SYM(network_module_is_initialized, NETWORK_MODULE_IS_INITIALIZED);
	NETWORK_SYM(network_module_config, NETWORK_MODULE_CONFIG);
	NETWORK_SYM(network_module_version, NETWORK_MODULE_VERSION);
	NETWORK_SYM(network_supports_ipv4, NETWORK_SUPPORTS_IPV4);
	NETWORK_SYM(network_supports_ipv6, NETWORK_SUPPORTS_IPV6);

	NETWORK_SYM(network_poll_allocate, NETWORK_POLL_ALLOCATE);
	NETWORK_SYM(network_poll_initialize, NETWORK_POLL_INITIALIZE);
	NETWORK_SYM(network_poll_finalize, NETWORK_POLL_FINALIZE);
	NETWORK_SYM(network_poll_deallocate, NETWORK_POLL_DEALLOCATE);
	NETWORK_SYM(network_poll_add_socket, NETWORK_POLL_ADD_SOCKET);
	NETWORK_SYM(network_poll_update_socket, NETWORK_POLL_UPDATE_SOCKET);
	NETWORK_SYM(network_poll_remove_socket, NETWORK_POLL_REMOVE_SOCKET);
	NETWORK_SYM(network_poll_has_socket, NETWORK_POLL_HAS_SOCKET);
	NETWORK_SYM(network_poll_num_sockets, NETWORK_POLL_NUM_SOCKETS);
	NETWORK_SYM(network_poll_sockets, NETWORK_POLL_SOCKETS);
	NETWORK_SYM(network_poll, NETWORK_POLL);

	NETWORK_SYM(socket_finalize, SOCKET_FINALIZE);
	NETWORK_SYM(socket_deallocate, SOCKET_DEALLOCATE);
	NETWORK_SYM(socket_bind, SOCKET_BIND);
	NETWORK_SYM(socket_connect, SOCKET_CONNECT);
	NETWORK_SYM(socket_close, SOCKET_CLOSE);
	NETWORK_SYM(socket_blocking, SOCKET_BLOCKING);
	NETWORK_SYM(socket_set_blocking, SOCKET_SET_BLOCKING);
	NETWORK_SYM(socket_reuse_address, SOCKET_REUSE_ADDRESS);
	NETWORK_SYM(socket_set_reuse_address, SOCKET_SET_REUSE_ADDRESS);
	NETWORK_SYM(socket_reuse_port, SOCKET_REUSE_PORT);
	NETWORK_SYM(socket_set_reuse_port, SOCKET_SET_REUSE_PORT);
	NETWORK_SYM(socket_set_multicast_group, SOCKET_SET_MULTICAST_GROUP);
	NETWORK_SYM(socket_address_local, SOCKET_ADDRESS_LOCAL);
	NETWORK_SYM(socket_address_remote, SOCKET_ADDRESS_REMOTE);
	NETWORK_SYM(socket_state, SOCKET_STATE);
	NETWORK_SYM(socket_poll_state, SOCKET_POLL_STATE);
	NETWORK_SYM(socket_fd, SOCKET_FD);
	NETWORK_SYM(socket_available_read, SOCKET_AVAILABLE_READ);
	NETWORK_SYM(socket_read, SOCKET_READ);
	NETWORK_SYM(socket_write, SOCKET_WRITE);
	NETWORK_SYM(socket_set_beacon, SOCKET_SET_BEACON);

	NETWORK_SYM(socket_stream_allocate, SOCKET_STREAM_ALLOCATE);
	NETWORK_SYM(socket_stream_initialize, SOCKET_STREAM_INITIALIZE);
	NETWORK_SYM(socket_stream_finalize, SOCKET_STREAM_FINALIZE);

	NETWORK_SYM(tcp_socket_allocate, TCP_SOCKET_ALLOCATE);
	NETWORK_SYM(tcp_socket_initialize, TCP_SOCKET_INITIALIZE);
	NETWORK_SYM(tcp_socket_accept, TCP_SOCKET_ACCEPT);
	NETWORK_SYM(tcp_socket_listen, TCP_SOCKET_LISTEN);
	NETWORK_SYM(tcp_socket_delay, TCP_SOCKET_DELAY);
	NETWORK_SYM(tcp_socket_set_delay, TCP_SOCKET_SET_DELAY);

	NETWORK_SYM(udp_socket_allocate, UDP_SOCKET_ALLOCATE);
	NETWORK_SYM(udp_socket_initialize, UDP_SOCKET_INITIALIZE);
	NETWORK_SYM(udp_socket_recvfrom, UDP_SOCKET_RECVFROM);
	NETWORK_SYM(udp_socket_sendto, UDP_SOCKET_SENDTO);
}
