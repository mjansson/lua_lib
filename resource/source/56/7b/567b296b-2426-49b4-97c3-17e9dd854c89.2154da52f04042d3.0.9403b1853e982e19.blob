
local foundation = require("foundation")

local ffi = require("ffi")
local C = ffi.C

-- Network types and functions
ffi.cdef[[

typedef struct network_config_t      network_config_t;
typedef struct network_address_t     network_address_t;
typedef struct network_poll_slot_t   network_poll_slot_t;
typedef struct network_poll_event_t  network_poll_event_t;
typedef struct network_poll_t        network_poll_t;
typedef struct socket_t              socket_t;
typedef struct socket_stream_t       socket_stream_t;
typedef struct socket_header_t       socket_header_t;
typedef union  socket_data_t         socket_data_t;

typedef void (*socket_open_fn)(socket_t*, unsigned int);
typedef void (*socket_stream_initialize_fn)(socket_t*, stream_t*);

network_address_t* network_address_clone(const network_address_t*);
network_address_t** network_address_resolve(const char*, size_t);
string_t network_address_to_string(char*, size_t, const network_address_t*, bool);
network_address_t* network_address_ipv4_any(void);
network_address_t* network_address_ipv6_any(void);
void network_address_ip_set_port(network_address_t*, unsigned int);
unsigned int network_address_ip_port(const network_address_t*);
network_address_t* network_address_ipv4_from_ip(uint32_t);
void network_address_ipv4_set_ip(network_address_t*, uint32_t);
uint32_t network_address_ipv4_ip(network_address_t*);
int network_address_type(const network_address_t*);
int network_address_family(const network_address_t*);
network_address_t** network_address_local(void);
bool network_address_equal(const network_address_t*, const network_address_t*);
void network_address_array_deallocate(network_address_t**);

int network_module_initialize(const network_config_t);
void network_module_finalize(void);
bool network_module_is_initialized(void);
network_config_t network_module_config(void);
version_t network_module_version(void);
bool network_supports_ipv4(void);
bool network_supports_ipv6(void);

network_poll_t* network_poll_allocate(unsigned int);
void network_poll_initialize(network_poll_t*, unsigned int);
void network_poll_finalize(network_poll_t*);
void network_poll_deallocate(network_poll_t*);
bool network_poll_add_socket(network_poll_t*, socket_t*);
void network_poll_update_socket(network_poll_t*, socket_t*);
void network_poll_remove_socket(network_poll_t*, socket_t*);
bool network_poll_has_socket(network_poll_t*, socket_t*);
size_t network_poll_num_sockets(network_poll_t*);
void network_poll_sockets(network_poll_t*, socket_t**, size_t);
size_t network_poll(network_poll_t*, network_poll_event_t*, size_t, unsigned int);

void socket_finalize(socket_t*);
void socket_deallocate(socket_t*);
bool socket_bind(socket_t*, const network_address_t*);
bool socket_connect(socket_t*, const network_address_t*, unsigned int);
void socket_close(socket_t*);
bool socket_blocking(const socket_t*);
void socket_set_blocking(socket_t*, bool);
bool socket_reuse_address(const socket_t*);
void socket_set_reuse_address(socket_t*, bool);
bool socket_reuse_port(const socket_t*);
void socket_set_reuse_port(socket_t*, bool);
bool socket_set_multicast_group(socket_t*, network_address_t*, bool);
const network_address_t* socket_address_local(const socket_t*);
const network_address_t* socket_address_remote(const socket_t*);
int socket_state(const socket_t*);
int socket_poll_state(socket_t*);
int socket_fd(socket_t*);
size_t socket_available_read(const socket_t*);
size_t socket_read(socket_t*, void*, size_t);
size_t socket_write(socket_t*, const void*, size_t);
void socket_set_beacon(socket_t*, beacon_t*);

stream_t* socket_stream_allocate(socket_t*, size_t, size_t);
void socket_stream_initialize(socket_stream_t*, socket_t*);
void socket_stream_finalize(socket_stream_t*);

socket_t* tcp_socket_allocate(void);
void tcp_socket_initialize(socket_t*);
socket_t* tcp_socket_accept(socket_t*, unsigned int);
bool tcp_socket_listen(socket_t*);
bool tcp_socket_delay(socket_t*);
void tcp_socket_set_delay(socket_t*, bool);

socket_t* udp_socket_allocate(void);
void udp_socket_initialize(socket_t*);
size_t udp_socket_recvfrom(socket_t*, void*, size_t, network_address_t const**);
size_t udp_socket_sendto(socket_t*, const void*, size_t, const network_address_t*);

]]

return {

-- Constants
NETWORK_ADDRESS_NUMERIC_MAX_LENGTH = 46,

NETWORK_TIMEOUT_INFINITE = -1,

NETWORK_SOCKET_INVALID = -1,

NETWORK_ADDRESSFAMILY_IPV4 = 0,
NETWORK_ADDRESSFAMILY_IPV6 = 1,

SOCKETSTATE_NOTCONNECTED = 0,
SOCKETSTATE_CONNECTING = 1,
SOCKETSTATE_CONNECTED = 2,
SOCKETSTATE_LISTENING = 3,
SOCKETSTATE_DISCONNECTED = 4,

NETWORKEVENT_CONNECTION = 1,
NETWORKEVENT_CONNECTED = 2,
NETWORKEVENT_DATAIN = 3,
NETWORKEVENT_ERROR = 4,
NETWORKEVENT_HANGUP = 5

}
