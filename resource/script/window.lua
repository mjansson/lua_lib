
local foundation = require("foundation")

local ffi = require("ffi")
local C = ffi.C

-- Network types and functions
ffi.cdef[[

typedef struct resource_config_t            resource_config_t;
typedef union  resource_change_value_t      resource_change_value_t;
typedef struct resource_change_t            resource_change_t;
typedef struct resource_change_data_t       resource_change_data_t;
typedef struct resource_change_data_fixed_t resource_change_data_fixed_t;
typedef struct resource_change_block_t      resource_change_block_t;
typedef struct resource_change_map_t        resource_change_map_t;
typedef struct resource_source_t            resource_source_t;
typedef struct resource_blob_t              resource_blob_t;
typedef struct resource_platform_t          resource_platform_t;
typedef struct resource_header_t            resource_header_t;
typedef struct resource_signature_t         resource_signature_t;

typedef int (* resource_import_fn)(stream_t*, const uuid_t);
typedef int (* resource_compile_fn)(const uuid_t, uint64_t, resource_source_t*, const uint256_t, const char*, size_t);
typedef resource_change_t* (* resource_source_map_reduce_fn)(resource_change_t*, resource_change_t*, void*);
typedef int (* resource_source_map_iterate_fn)(resource_change_t*, void*);

bool resource_bundle_load(const uuid_t);
stream_t* resource_bundle_stream(const uuid_t);

bool resource_change_is_value(resource_change_t*);
bool resource_change_is_blob(resource_change_t*);
resource_change_data_t* resource_change_data_allocate(size_t);
void resource_change_data_deallocate(resource_change_data_t*);
void resource_change_data_initialize(resource_change_data_t*, void*, size_t);
void resource_change_data_finalize(resource_change_data_t*);
resource_change_block_t* resource_change_block_allocate(void);
void resource_change_block_deallocate(resource_change_block_t*);
void resource_change_block_initialize(resource_change_block_t*);
void resource_change_block_finalize(resource_change_block_t*);

bool resource_compile_need_update(const uuid_t, uint64_t);
bool resource_compile(const uuid_t, uint64_t);
void resource_compile_register(resource_compile_fn);
void resource_compile_register_path(const char*, size_t);
void resource_compile_unregister(resource_compile_fn);
void resource_compile_unregister_path(const char*, size_t);

uuid_t resource_event_uuid(const event_t*);
hash_t resource_event_token(const event_t*);
void resource_event_post(int, uuid_t, hash_t);
event_stream_t* resource_event_stream(void);
void resource_event_handle(event_t*);

string_const_t resource_import_base_path(void);
void resource_import_set_base_path(const char*, size_t);
bool resource_import(const char*, size_t, const uuid_t);
void resource_import_register(resource_import_fn);
void resource_import_register_path(const char*, size_t);
void resource_import_unregister(resource_import_fn);
void resource_import_unregister_path(const char*, size_t);
resource_signature_t resource_import_lookup(const char*, size_t);
uuid_t resource_import_map_store(const char*, size_t, uuid_t, uint256_t);
bool resource_import_map_purge(const char*, size_t);

bool resource_autoimport(const uuid_t);
bool resource_autoimport_need_update(const uuid_t, uint64_t);
void resource_autoimport_watch(const char*, size_t);
void resource_autoimport_unwatch(const char*, size_t);
void resource_autoimport_clear(void);
void resource_autoimport_event_handle(event_t*);

const string_const_t* resource_local_paths(void);
void resource_local_set_paths(const string_const_t*, size_t);
void resource_local_add_path(const char*, size_t);
void resource_local_remove_path(const char*, size_t);
void resource_local_clear_paths(void);
stream_t* resource_local_open_static(const uuid_t, uint64_t);
stream_t* resource_local_open_dynamic(const uuid_t, uint64_t);
stream_t* resource_local_create_static(const uuid_t, uint64_t);
stream_t* resource_local_create_dynamic(const uuid_t, uint64_t);

uint64_t resource_platform(const resource_platform_t);
resource_platform_t resource_platform_decompose(uint64_t);
bool resource_platform_is_equal_or_more_specific(uint64_t, uint64_t);
uint64_t resource_platform_reduce(uint64_t, uint64_t);

string_const_t resource_remote_sourced(void);
void resource_remote_sourced_connect(const char*, size_t);
void resource_remote_sourced_disconnect(void);
bool resource_remote_sourced_is_connected(void);
resource_signature_t resource_remote_sourced_lookup(const char*, size_t);
uint256_t resource_remote_sourced_hash(uuid_t, uint64_t);
size_t resource_remote_sourced_dependencies(uuid_t, uint64_t, uuid_t*, size_t);
bool resource_remote_sourced_read(resource_source_t*, uuid_t);
bool resource_remote_sourced_read_blob(const uuid_t, hash_t, uint64_t, hash_t, void*, size_t);
string_const_t resource_remote_compiled(void);
void resource_remote_compiled_connect(const char*, size_t);
void resource_remote_compiled_disconnect(void);
stream_t* resource_remote_open_static(const uuid_t, uint64_t);
stream_t* resource_remote_open_dynamic(const uuid_t, uint64_t);

int resource_module_initialize(const resource_config_t);
void resource_module_finalize(void);
bool resource_module_is_initialized(void);
version_t resource_module_version(void);
resource_config_t resource_module_config(void);
void resource_module_parse_config(const char*, size_t, const char*, size_t, const json_token_t*, size_t);

string_const_t resource_source_path(void);
bool resource_source_set_path(const char*, size_t);
uint256_t resource_source_read_hash(const uuid_t, uint64_t);
resource_source_t* resource_source_allocate(void);
void resource_source_deallocate(resource_source_t*);
void resource_source_initialize(resource_source_t*);
void resource_source_finalize(resource_source_t*);
bool resource_source_read(resource_source_t*, const uuid_t);
bool resource_source_write(resource_source_t*, const uuid_t, bool);
void resource_source_set(resource_source_t*, tick_t, hash_t, uint64_t, const char*, size_t);
void resource_source_unset(resource_source_t*, tick_t, hash_t, uint64_t);
resource_change_t* resource_source_get(resource_source_t*, hash_t, uint64_t);
void resource_source_set_blob(resource_source_t*, tick_t, hash_t, uint64_t, hash_t, size_t);
bool resource_source_read_blob(const uuid_t, hash_t, uint64_t, hash_t, void*, size_t);
bool resource_source_write_blob(const uuid_t, tick_t, hash_t, uint64_t, hash_t, const void*, size_t);
void resource_source_collapse_history(resource_source_t*);
void resource_source_clear_blob_history(resource_source_t*, const uuid_t);
void resource_source_map(resource_source_t*, uint64_t, hashmap_t*);
void resource_source_map_all(resource_source_t*, hashmap_t*, bool);
void resource_source_map_iterate(resource_source_t*, hashmap_t*, void*, resource_source_map_iterate_fn);
void resource_source_map_reduce(resource_source_t*, hashmap_t*, void*, resource_source_map_reduce_fn);
void resource_source_map_clear(hashmap_t*);
size_t resource_source_num_dependencies(const uuid_t, uint64_t);
size_t resource_source_dependencies(const uuid_t, uint64_t, uuid_t*, size_t);
void resource_source_set_dependencies(const uuid_t, uint64_t, const uuid_t*, size_t);

stream_t* resource_stream_open_static(const uuid_t, uint64_t);
stream_t* resource_stream_open_dynamic(const uuid_t, uint64_t);
string_t resource_stream_make_path(char*, size_t, const char*, size_t, const uuid_t);
void resource_stream_write_header(stream_t*, const resource_header_t);
resource_header_t resource_stream_read_header(stream_t*);

]]

return {

-- Constants
RESOURCE_PLATFORM_ALL = -1,

RESOURCEEVENT_CREATE = 0,
RESOURCEEVENT_MODIFY = 1,
RESOURCEEVENT_DELETE = 2,
RESOURCEEVENT_LAST_RESERVED = 32,

RESOURCE_SOURCEFLAG_UNSET = 0,
RESOURCE_SOURCEFLAG_VALUE = 1,
RESOURCE_SOURCEFLAG_BLOB  = 2

}
