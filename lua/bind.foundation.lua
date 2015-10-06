
local ffi = require( "ffi" )
local C = ffi.C

-- Basic system, build and architecture types
ffi.cdef[[
typedef float  float32_t;
typedef double float64_t;
int system_size_size_t(void);
int system_size_real(void);
int system_size_pointer(void);
int system_size_wchar(void);
]]

if C.system_size_size_t() > 4 then
      ffi.cdef( "typedef int64_t ssize_t;" );
else
      ffi.cdef( "typedef int32_t ssize_t;" );
end

if C.system_size_real() > 4 then
      ffi.cdef( "typedef float64_t real;" );
else
      ffi.cdef( "typedef float32_t real;" );
end

if C.system_size_pointer() > 4 then
      ffi.cdef( "typedef uint64_t hashtable_int_t;" );
else
      ffi.cdef( "typedef uint32_t hashtable_int_t;" );
end

if C.system_size_wchar() > 2 then
      ffi.cdef( "typedef uint32_t wchar_t;" );
else
      ffi.cdef( "typedef uint16_t wchar_t;" );
end

-- Foundation types and functions
ffi.cdef[[

typedef struct application_t          application_t;
typedef struct bitbuffer_t            bitbuffer_t;
typedef struct blowfish_t             blowfish_t;
typedef struct error_frame_t          error_frame_t;
typedef struct error_context_t        error_context_t;
typedef struct event_t                event_t;
typedef struct event_block_t          event_block_t;
typedef struct event_stream_t         event_stream_t;
typedef struct fs_event_payload_t     fs_event_payload_t;
typedef struct hashmap_node_t         hashmap_node_t;
typedef struct hashmap_t              hashmap_t;
typedef struct hashtable32_entry_t    hashtable32_entry_t;
typedef struct hashtable64_entry_t    hashtable64_entry_t;
typedef struct hashtable_t            hashtable_t;
typedef struct hashtable32_t          hashtable32_t;
typedef struct hashtable64_t          hashtable64_t;
typedef struct md5_t                  md5_t;
typedef struct memory_context_t       memory_context_t;
typedef struct memory_system_t        memory_system_t;
typedef struct memory_tracker_t       memory_tracker_t;
typedef struct mutex_t                mutex_t;
typedef struct object_base_t          object_base_t;
typedef struct objectmap_t            objectmap_t;
typedef struct process_t              process_t;
typedef struct radixsort_t            radixsort_t;
typedef struct regex_t                regex_t;
typedef struct ringbuffer_t           ringbuffer_t;
typedef struct semaphore_t            semaphore_t;
typedef struct stream_t               stream_t;
typedef struct stream_buffer_t        stream_buffer_t;
typedef struct stream_pipe_t          stream_pipe_t;
typedef struct stream_ringbuffer_t    stream_ringbuffer_t;
typedef struct stream_vtable_t        stream_vtable_t;
typedef struct foundation_config_t    foundation_config_t;

typedef uint64_t       object_t;
typedef uint64_t       hash_t;
typedef uint64_t       tick_t;
typedef real           deltatime_t;
typedef uint16_t       radixsort_index_t;

typedef struct { uint64_t word[2]; } uint128_t;
typedef struct { uint64_t word[4]; } uint256_t;

typedef uint128_t      uuid_t;
typedef uint128_t      version_t;

typedef struct { char* str; size_t length; } string_t;
typedef struct { const char* str; size_t length; } string_const_t;

typedef int            (* error_callback_fn)(int, int);
typedef int            (* assert_handler_fn)(hash_t, const char*, size_t, const char*, size_t, int, const char*, size_t);
typedef void           (* log_callback_fn)(uint64_t, int, const char*, size_t);
typedef void           (* object_deallocate_fn )(object_t, void*);
typedef void           (* profile_write_fn )(void*, uint64_t);
typedef void           (* profile_read_fn )(void*, uint64_t);
typedef stream_t*      (* stream_open_fn )(const char*, size_t, unsigned int);
typedef void*          (* thread_fn )(object_t, void*);
typedef string_const_t (* string_map_fn)(hash_t);

void*       array_allocate_pointer(unsigned int);
int         array_size(const void*);
const void* array_element_pointer(const void*, int);
void        array_set_element_pointer(const void*, int, void*);
void        array_deallocate(void*);

assert_handler_fn assert_handler(void);
void              assert_set_handler(assert_handler_fn);
int               assert_report(uint64_t, const char*, size_t, const char*, size_t, int, const char*, size_t);

stream_t* asset_stream_open(const char*, size_t, unsigned int);

size_t base64_encode(const void*, size_t, char*, size_t);
size_t base64_decode(const char*, size_t, void*, size_t);

bitbuffer_t* bitbuffer_allocate_buffer(void*, size_t, bool);
bitbuffer_t* bitbuffer_allocate_stream(stream_t*);
void         bitbuffer_deallocate(bitbuffer_t*);
void         bitbuffer_initialize_buffer(bitbuffer_t*, void*, size_t, bool);
void         bitbuffer_initialize_stream(bitbuffer_t*, stream_t*);
void                   bitbuffer_finalize(bitbuffer_t*);
uint32_t               bitbuffer_read32(bitbuffer_t*, unsigned int);
uint64_t               bitbuffer_read64(bitbuffer_t*, unsigned int);
uint128_t              bitbuffer_read128(bitbuffer_t*, unsigned int);
float32_t              bitbuffer_read_float32(bitbuffer_t*);
float64_t              bitbuffer_read_float64(bitbuffer_t*);
void                   bitbuffer_write32(bitbuffer_t*, uint32_t, unsigned int);
void                   bitbuffer_write64(bitbuffer_t*, uint64_t, unsigned int);
void                   bitbuffer_write128(bitbuffer_t*, uint128_t, unsigned int);
void                   bitbuffer_write_float32(bitbuffer_t*, float32_t);
void                   bitbuffer_write_float64(bitbuffer_t*, float64_t);
void                   bitbuffer_align_read(bitbuffer_t*, bool);
void                   bitbuffer_align_write(bitbuffer_t*, bool);
void                   bitbuffer_discard_read(bitbuffer_t*);
void                   bitbuffer_discard_write(bitbuffer_t*);

uint16_t               byteorder_swap16(uint16_t);
uint32_t               byteorder_swap32(uint32_t);
uint64_t               byteorder_swap64(uint64_t);
void                   byteorder_swap(void*, const size_t);
uint16_t               byteorder_bigendian16(uint16_t);
uint32_t               byteorder_bigendian32(uint32_t);
uint64_t               byteorder_bigendian64(uint64_t);
void                   byteorder_bigendian(void*, const size_t);
uint16_t               byteorder_littleendian16(uint16_t);
uint32_t               byteorder_littleendian32(uint32_t);
uint64_t               byteorder_littleendian64(uint64_t);
void                   byteorder_littleendian(void*, const size_t);

blowfish_t*            blowfish_allocate(void);
void                   blowfish_deallocate(blowfish_t*);
void                   blowfish_initialize(blowfish_t*, const void*, size_t);
void                   blowfish_finalize(blowfish_t*);
void                   blowfish_encrypt(const blowfish_t*, void*, size_t, int, uint64_t);
void                   blowfish_decrypt(const blowfish_t*, void*, size_t, int, uint64_t);

stream_t*              buffer_stream_allocate(void*, unsigned int, size_t, size_t, bool, bool);
void                   buffer_stream_initialize(stream_buffer_t*, void*, unsigned int, size_t, size_t, bool, bool);

bool                   config_bool(hash_t, hash_t);
int64_t                config_int(hash_t, hash_t);
real                   config_real(hash_t, hash_t);
const char*            config_string(hash_t, hash_t);
hash_t                 config_string_hash(hash_t, hash_t);
void                   config_set_bool(hash_t, hash_t, bool);
void                   config_set_int(hash_t, hash_t, int64_t);
void                   config_set_real(hash_t, hash_t, real);
void                   config_set_string(hash_t, hash_t key, const char*, size_t);
void                   config_load(const char*, size_t length, hash_t, bool, bool);
void                   config_parse(stream_t*, hash_t, bool);
void                   config_write(stream_t*, hash_t, string_map_fn);
void                   config_parse_commandline(const string_const_t*, size_t);

const string_const_t*  environment_command_line(void);
string_const_t         environment_executable_name(void);
string_const_t         environment_executable_directory(void);
string_const_t         environment_executable_path(void);
string_const_t         environment_initial_working_directory(void);
string_const_t         environment_current_working_directory(void);
void                   environment_set_current_working_directory(const char*, size_t);
string_const_t         environment_home_directory(void);
string_const_t         environment_temporary_directory(void);
string_const_t         environment_variable(const char*, size_t);

int                    error(void);
int                    error_report(int, int);
void                   error_context_push(const char*, size_t, const char*, size_t);
void                   error_context_pop(void);
void                   error_context_clear(void);
string_t               error_context_buffer(char*, size_t);
error_context_t*       error_context(void);
error_callback_fn      error_callback(void);
void                   error_set_callback(error_callback_fn);

void                   event_post(event_stream_t*, uint16_t, object_t, tick_t, const void*, size_t, ...);
event_t*               event_next(const event_block_t*, event_t*);
size_t                 event_payload_size(const event_t*);
event_stream_t*        event_stream_allocate(size_t);
void                   event_stream_deallocate(event_stream_t*);
void                   event_stream_initialize(event_stream_t*, size_t);
void                   event_stream_finalize(event_stream_t*);
event_block_t*         event_stream_process(event_stream_t*);

stream_t*              fs_open_file(const char*, size_t, unsigned int);
void                   fs_copy_file(const char*, size_t, const char*, size_t);
bool                   fs_remove_file(const char*, size_t);
bool                   fs_is_file(const char*, size_t);
bool                   fs_make_directory(const char*, size_t);
bool                   fs_remove_directory(const char*, size_t);
bool                   fs_is_directory(const char*, size_t);
tick_t                 fs_last_modified(const char*, size_t);
void                   fs_touch(const char*, size_t);
size_t                 fs_size(const char*, size_t);
uint128_t              fs_md5(const char*, size_t);
string_t*              fs_matching_files(const char*, size_t, const char*, size_t, bool);
string_t*              fs_matching_files_regex(const char*, size_t, regex_t*, bool);
string_t*              fs_files(const char*, size_t);
string_t*              fs_subdirs(const char*, size_t);
void                   fs_monitor(const char*, size_t);
void                   fs_unmonitor(const char*, size_t);
stream_t*              fs_temporary_file(void);
void                   fs_post_event(int, const char*, size_t);
event_stream_t*        fs_event_stream(void);

version_t              foundation_version(void);

hash_t                 hash(const void*, size_t);
string_const_t         hash_to_string(hash_t);

hashmap_t*             hashmap_allocate(size_t, size_t);
void                   hashmap_deallocate(hashmap_t*);
void                   hashmap_initialize(hashmap_t*, size_t, size_t);
void                   hashmap_finalize(hashmap_t*);
void*                  hashmap_insert(hashmap_t*, hash_t, void*);
void*                  hashmap_erase(hashmap_t*, hash_t);
void*                  hashmap_lookup(hashmap_t*, hash_t);
bool                   hashmap_has_key(hashmap_t*, hash_t);
size_t                 hashmap_size(hashmap_t* );
void                   hashmap_clear(hashmap_t*);

hashtable_t*           hashtable_allocate(size_t);
void                   hashtable_deallocate(hashtable_t* );
void                   hashtable_initialize(hashtable_t*, size_t);
void                   hashtable_finalize(hashtable_t*);
void                   hashtable_set(hashtable_t*, hashtable_int_t, hashtable_int_t);
void                   hashtable_erase(hashtable_t*, hashtable_int_t);
hashtable_int_t        hashtable_get(hashtable_t*, hashtable_int_t );
size_t                 hashtable_size(hashtable_t*);
void                   hashtable_clear(hashtable_t*);
hashtable32_t*         hashtable32_allocate(size_t);
void                   hashtable32_deallocate(hashtable32_t*);
void                   hashtable32_initialize(hashtable32_t*, size_t);
void                   hashtable32_finalize(hashtable32_t*);
void                   hashtable32_set(hashtable32_t*, uint32_t, uint32_t);
void                   hashtable32_erase(hashtable32_t*, uint32_t);
uint32_t               hashtable32_get(hashtable32_t*, uint32_t);
size_t                 hashtable32_size(hashtable32_t*);
void                   hashtable32_clear(hashtable32_t*);
hashtable64_t*         hashtable64_allocate(size_t);
void                   hashtable64_deallocate(hashtable64_t*);
void                   hashtable64_initialize(hashtable64_t*, size_t);
void                   hashtable64_finalize(hashtable64_t*);
void                   hashtable64_set(hashtable64_t*, uint64_t, uint64_t);
void                   hashtable64_erase(hashtable64_t*, uint64_t);
uint64_t               hashtable64_get(hashtable64_t*, uint64_t);
size_t                 hashtable64_size(hashtable64_t*);
void                   hashtable64_clear(hashtable64_t*);

object_t               library_load(const char*, size_t);
object_t               library_ref(object_t);
void                   library_unload(object_t);
void*                  library_symbol(object_t, const char*, size_t);
string_const_t         library_name(object_t);
bool                   library_valid(object_t);

void                   log_debugf(hash_t, const char*, size_t, ...);
void                   log_infof(hash_t, const char*, size_t, ...);
void                   log_warnf(hash_t, int, const char*, size_t, ...);
void                   log_errorf(hash_t, int, const char*, size_t, ...);
void                   log_panicf(hash_t, int, const char*, size_t, ...);
void                   log_enable_stdout(bool);
void                   log_enable_prefix(bool);
void                   log_set_suppress(hash_t, int);
int                    log_suppress(hash_t);
void                   log_suppress_clear(void);
void                   log_error_context(hash_t, int);
log_callback_fn        log_callback(void);
void                   log_set_callback(log_callback_fn);

md5_t*                 md5_allocate(void);
void                   md5_deallocate(md5_t*);
void                   md5_initialize(md5_t*);
void                   md5_finalize(md5_t*);
md5_t*                 md5_digest(md5_t*, const void*, size_t);
void                   md5_digest_finalize(md5_t*);
string_t               md5_get_digest(const md5_t*, char*, size_t);
uint128_t              md5_get_digest_raw(const md5_t*);

void*                  memory_allocate(uint64_t, size_t, unsigned int, unsigned int);
void*                  memory_reallocate(void*, size_t, unsigned int, size_t);
void                   memory_deallocate(void*);
void                   memory_context_push(hash_t);
void                   memory_context_pop(void);
hash_t                 memory_context(void);

mutex_t*               mutex_allocate(const char*, size_t);
void                   mutex_deallocate(mutex_t*);
string_const_t         mutex_name(mutex_t*);
bool                   mutex_try_lock(mutex_t*);
bool                   mutex_lock(mutex_t*);
bool                   mutex_unlock(mutex_t*);
bool                   mutex_wait(mutex_t*, unsigned int);
void                   mutex_signal(mutex_t*);

objectmap_t*           objectmap_allocate(size_t);
void                   objectmap_deallocate(objectmap_t*);
void                   objectmap_initialize(objectmap_t*, size_t);
void                   objectmap_finalize(objectmap_t*);
size_t                 objectmap_size(const objectmap_t*);
object_t               objectmap_reserve(objectmap_t*);
void                   objectmap_free(objectmap_t*, object_t);
void                   objectmap_set(objectmap_t*, object_t, void*);
void*                  objectmap_raw_lookup(const objectmap_t*, size_t);
void*                  objectmap_lookup_ref(const objectmap_t*, object_t);
bool                   objectmap_lookup_unref(const objectmap_t*, object_t, object_deallocate_fn);
void*                  objectmap_lookup(const objectmap_t*, object_t);

string_const_t         path_base_file_name(const char*, size_t);
string_const_t         path_base_file_name_with_directory(const char*, size_t);
string_const_t         path_file_extension(const char*, size_t);
string_const_t         path_file_name(const char*, size_t);
string_const_t         path_directory_name(const char*, size_t);
string_const_t         path_subdirectory_name(const char*, size_t, const char*, size_t);
string_const_t         path_protocol(const char*, size_t);
string_t               path_allocate_concat(const char*, size_t, ...);
string_t               path_concat(char*, size_t, const char*, size_t, ...);
string_t               path_append(char*, size_t, size_t, const char*, size_t, ...);
string_t               path_prepend(char*, size_t, size_t, const char*, size_t, ...);
string_t               path_absolute(char*, size_t, size_t);
string_t               path_clean(char*, size_t, size_t);
bool                   path_is_absolute(const char*, size_t);
string_t               path_make_temporary(char*, size_t);

stream_t*              pipe_allocate(void);
void                   pipe_initialize(stream_pipe_t*);
void                   pipe_close_read(stream_t*);
void                   pipe_close_write(stream_t*);

process_t*             process_allocate(void);
void                   process_deallocate(process_t*);
void                   process_initialize(process_t*);
void                   process_finalize(process_t*);
void                   process_set_working_directory(process_t*, const char*, size_t);
void                   process_set_executable_path(process_t*, const char*, size_t);
void                   process_set_arguments(process_t*, const string_const_t*, size_t);
void                   process_set_flags(process_t*, unsigned int);
void                   process_set_verb(process_t*, const char*, size_t);
void                   process_set_exit_code(int);
int                    process_spawn(process_t*);
stream_t*              process_stdout(process_t*);
stream_t*              process_stdin(process_t*);
int                    process_wait(process_t*);
int                    process_exit_code(void);
void                   process_exit(int);

void                   profile_initialize(const char*, size_t, void*, size_t);
void                   profile_shutdown(void);
void                   profile_enable(bool);
void                   profile_set_output(profile_write_fn);
void                   profile_set_output_wait(unsigned int);
void                   profile_end_frame(uint64_t);
void                   profile_begin_block(const char*, size_t);
void                   profile_update_block(void);
void                   profile_end_block(void);
void                   profile_log(const char*, size_t);
void                   profile_trylock(const char*, size_t);
void                   profile_lock(const char*, size_t);
void                   profile_unlock(const char*, size_t);
void                   profile_wait(const char*, size_t);
void                   profile_signal(const char*, size_t);

radixsort_t*           radixsort_allocate(int, radixsort_index_t);
void                   radixsort_deallocate(radixsort_t*);
void                   radixsort_initialize(radixsort_t*, int, radixsort_index_t);
void                   radixsort_finalize(radixsort_t*);
const radixsort_index_t* radixsort(radixsort_t*, const void*, radixsort_index_t);

uint32_t               random32(void);
uint32_t               random32_range(uint32_t, uint32_t);
uint64_t               random64(void);
uint64_t               random64_range(uint64_t, uint64_t);
real                   random_normalized(void);
real                   random_range(real, real);
int32_t                random32_gaussian_range(int32_t, int32_t);
real                   random_gaussian_range(real, real);
int32_t                random32_triangle_range(int32_t, int32_t);
real                   random_triangle_range(real, real);
uint32_t               random32_weighted(uint32_t, const real*);

regex_t*               regex_compile(const char*, size_t);
bool                   regex_match(regex_t*, const char*, size_t, string_const_t*, size_t);
void                   regex_deallocate(regex_t*);

ringbuffer_t*          ringbuffer_allocate(size_t);
void                   ringbuffer_deallocate(ringbuffer_t*);
void                   ringbuffer_initialize(ringbuffer_t*, size_t);
void                   ringbuffer_finalize(ringbuffer_t*);
size_t                 ringbuffer_size(ringbuffer_t*);
void                   ringbuffer_reset(ringbuffer_t*);
size_t                 ringbuffer_read(ringbuffer_t*, void*, size_t);
size_t                 ringbuffer_write(ringbuffer_t*, const void*, size_t);
uint64_t               ringbuffer_total_read( ringbuffer_t*);
uint64_t               ringbuffer_total_written( ringbuffer_t* );
stream_t*              ringbuffer_stream_allocate(size_t, size_t);
void                   ringbuffer_stream_initialize(stream_ringbuffer_t*, size_t, size_t);

void                   semaphore_initialize(semaphore_t*, unsigned int);
void                   semaphore_initialize_named(semaphore_t*, const char*, size_t, unsigned int);
void                   semaphore_finalize(semaphore_t*);
bool                   semaphore_wait(semaphore_t*);
bool                   semaphore_try_wait(semaphore_t*, unsigned int);
void                   semaphore_post(semaphore_t*);

size_t                 stacktrace_capture(void**, size_t, size_t);
string_t               stacktrace_resolve(char*, size_t, void**, size_t, size_t);

stream_t*              stream_open(const char*, size_t, unsigned int);
stream_t*              stream_clone(stream_t*);
void                   stream_deallocate(stream_t*);
void                   stream_initialize(stream_t*, int);
void                   stream_finalize(stream_t*);
size_t                 stream_tell(stream_t*);
void                   stream_seek(stream_t*, ssize_t, int);
bool                   stream_eos(stream_t*);
size_t                 stream_size(stream_t*);
void                   stream_set_byteorder(stream_t*, int);
void                   stream_set_binary(stream_t*, bool);
void                   stream_determine_binary_mode(stream_t*, size_t);
bool                   stream_is_binary(stream_t*);
bool                   stream_is_sequential(stream_t*);
bool                   stream_is_reliable(stream_t*);
bool                   stream_is_inorder(stream_t*);
bool                   stream_is_swapped(stream_t*);
int                    stream_byteorder(stream_t*);
string_const_t         stream_path(const stream_t*);
tick_t                 stream_last_modified(stream_t*);
size_t                 stream_read(stream_t*, void*, size_t);
string_t               stream_read_line_buffer(stream_t*, char*, size_t, char);
string_t               stream_read_line(stream_t*, char);
bool                   stream_read_bool(stream_t*);
int8_t                 stream_read_int8(stream_t*);
uint8_t                stream_read_uint8(stream_t*);
int16_t                stream_read_int16(stream_t*);
uint16_t               stream_read_uint16(stream_t*);
int32_t                stream_read_int32(stream_t*);
uint32_t               stream_read_uint32(stream_t*);
int64_t                stream_read_int64(stream_t*);
uint64_t               stream_read_uint64(stream_t*);
float32_t              stream_read_float32(stream_t*);
float64_t              stream_read_float64(stream_t*);
string_t               stream_read_string(stream_t*);
string_t               stream_read_string_buffer(stream_t*, char*, size_t);
size_t                 stream_write(stream_t*, const void*, size_t);
void                   stream_write_bool(stream_t*, bool);
void                   stream_write_int8(stream_t*, int8_t);
void                   stream_write_uint8(stream_t*, uint8_t);
void                   stream_write_int16(stream_t*, int16_t);
void                   stream_write_uint16(stream_t*, uint16_t);
void                   stream_write_int32(stream_t*, int32_t);
void                   stream_write_uint32(stream_t*, uint32_t);
void                   stream_write_int64(stream_t*, int64_t);
void                   stream_write_uint64(stream_t*, uint64_t);
void                   stream_write_float32(stream_t*, float32_t);
void                   stream_write_float64(stream_t*, float64_t);
void                   stream_write_string(stream_t*, const char*, size_t);
void                   stream_write_endl(stream_t*);
void                   stream_write_format(stream_t*, const char*, size_t, ...);
void                   stream_buffer_read(stream_t*);
size_t                 stream_available_read(stream_t*);
uint128_t              stream_md5(stream_t*);
void                   stream_truncate(stream_t*, size_t);
void                   stream_flush(stream_t*);
stream_t*              stream_open_stdout(void);
stream_t*              stream_open_stderr(void);
stream_t*              stream_open_stdin(void);
void                   stream_set_protocol_handler(const char*, size_t, stream_open_fn);
stream_open_fn         stream_protocol_handler(const char*, size_t);

string_t               string_allocate(size_t, size_t);
string_t               string_clone(const char*, size_t);
string_t               string_clone_string(string_const_t);
void                   string_deallocate(char*);
string_const_t         string_null(void);
string_const_t         string_empty(void);
string_t               string(char*, size_t);
string_const_t         string_const(const char*, size_t);
string_const_t         string_to_const(string_t);
string_t               string_allocate_format(const char*, size_t, ...);
string_t               string_format(char*, size_t, const char*, size_t, ...);
size_t                 string_length(const char*);
size_t                 string_glyphs(const char*, size_t);
hash_t                 string_hash(const char*, size_t);
string_t               string_copy(char*, size_t, const char*, size_t);
string_t               string_resize(char*, size_t, size_t, size_t, char);
string_t               string_replace(char*, size_t, size_t, const char*, size_t, const char*, size_t, bool);
string_t               string_allocate_concat(const char*, size_t, const char*, size_t, ...);
string_t               string_concat(char*, size_t, const char*, size_t, const char*, size_t, ...);
string_t               string_append(char*, size_t, size_t, const char*, size_t, ...);
string_t               string_prepend(char*, size_t, size_t, const char*, size_t, ...);
string_const_t         string_substr(const char*, size_t, size_t, size_t);
string_const_t         string_strip(const char*, size_t, const char*, size_t);
size_t                 string_find(const char*, size_t, char, size_t);
size_t                 string_find_string(const char*, size_t, const char*, size_t, size_t);
size_t                 string_rfind(const char*, size_t, char, size_t);
size_t                 string_rfind_string(const char*, size_t, const char*, size_t, size_t);
size_t                 string_find_first_of(const char*, size_t, const char*, size_t, size_t);
size_t                 string_find_last_of(const char*, size_t, const char*, size_t, size_t);
size_t                 string_find_first_not_of(const char*, size_t, const char*, size_t, size_t);
size_t                 string_find_last_not_of(const char*, size_t, const char*, size_t, size_t);
bool                   string_ends_with(const char*, size_t, const char*, size_t);
bool                   string_equal(const char*, size_t, const char*, size_t);
bool                   string_equal_nocase(const char*, size_t, const char*, size_t);
bool                   string_equal_substr(const char*, size_t, size_t, const char*, size_t, size_t);
bool                   string_equal_substr_nocase(const char*, size_t, size_t, const char*, size_t, size_t);
bool                   string_match_pattern(const char*, size_t, const char*, size_t);
void                   string_split(const char*, size_t, const char*, size_t, string_const_t*, string_const_t, bool);
size_t                 string_explode(const char*, size_t, const char*, size_t, string_const_t*, size_t, bool);
string_t               string_merge(char*, size_t, const string_const_t*, size_t, const char*, size_t);
string_t               string_merge_varg(char*, size_t, const char*, size_t, const char*, size_t, ...);
uint32_t               string_glyph(const char*, size_t, size_t, size_t*);
ssize_t                string_array_find(const string_const_t*, size_t, const char*, size_t);
void                   string_array_deallocate_elements(string_t*);
void                   string_array_deallocate(string_t*);
wchar_t*               wstring_allocate_from_string(const char*, size_t);
void                   wstring_from_string(wchar_t*, size_t, const char*, size_t);
void                   wstring_deallocate(wchar_t*);
size_t                 wstring_length(const wchar_t*);
bool                   wstring_equal(const wchar_t*, const wchar_t*);
string_t               string_allocate_from_wstring(const wchar_t*, size_t);
string_t               string_allocate_from_utf16( const uint16_t*, size_t);
string_t               string_allocate_from_utf32( const uint32_t*, size_t);
string_t               string_convert_utf16(char*, size_t, const uint16_t*, size_t);
string_t               string_convert_utf32(char*, size_t, const uint32_t*, size_t);
string_t               string_from_int(char*, size_t, int64_t, unsigned int, char);
string_t               string_from_uint(char*, size_t, uint64_t, bool, unsigned int, char);
string_t               string_from_uint128(char*, size_t, const uint128_t);
string_t               string_from_real(char*, size_t, real, unsigned int, unsigned int, char);
string_t               string_from_time(char*, size_t, uint64_t);
string_t               string_from_uuid(char*, size_t, const uuid_t);
string_t               string_from_version(char*, size_t, const version_t);
string_const_t         string_from_int_static(int64_t, unsigned int, char);
string_const_t         string_from_uint_static(uint64_t, bool, unsigned int, char);
string_const_t         string_from_uint128_static(const uint128_t);
string_const_t         string_from_real_static(real, unsigned int precision, unsigned int, char);
string_const_t         string_from_time_static(uint64_t);
string_const_t         string_from_uuid_static(const uuid_t);
string_const_t         string_from_version_static(const version_t);
int                    string_to_int(const char*, size_t);
unsigned int           string_to_uint(const char*, size_t, bool);
int64_t                string_to_int64(const char*, size_t);
uint64_t               string_to_uint64(const char*, size_t, bool);
uint128_t              string_to_uint128(const char*, size_t);
float32_t              string_to_float32(const char*, size_t);
float64_t              string_to_float64(const char*, size_t);
real                   string_to_real(const char*, size_t);
uuid_t                 string_to_uuid(const char*, size_t);
version_t              string_to_version(const char*, size_t);

int                    system_error(void);
void                   system_error_reset(void);
const char*            system_error_message(int);
int                    system_platform(void);
int                    system_architecture(void);
int                    system_byteorder(void);
unsigned int           system_hardware_threads(void);
string_t               system_hostname(char*, size_t);
uint64_t               system_hostid(void);
string_t               system_username(char*, size_t);
bool                   system_debugger_attached(void);
void                   system_pause(void);
uint16_t               system_language(void);
uint16_t               system_country(void);
uint32_t               system_locale(void);
string_t               system_locale_string(char* size_t);
void                   system_set_device_orientation(int);
int                    system_device_orientation(void);
void                   system_process_events(void);
bool                   system_message_box(const char*, size_t, const char*, size_t, bool);
event_stream_t*        system_event_stream(void);
void                   system_post_event(int);

object_t               thread_create(thread_fn, const char*, size_t, int, unsigned int);
object_t               thread_ref(object_t);
void                   thread_destroy(object_t);
bool                   thread_start(object_t, void*);
void                   thread_terminate(object_t);
bool                   thread_is_started(object_t);
bool                   thread_is_running(object_t);
bool                   thread_is_thread(object_t);
bool                   thread_is_main(void);
bool                   thread_should_terminate(object_t);
void                   thread_set_main(void);
void                   thread_set_name(const char*, size_t);
void                   thread_set_hardware(uint64_t);
void*                  thread_result(object_t);
object_t               thread_self(void);
string_const_t         thread_name(void);
uint64_t               thread_id(void);
unsigned int           thread_hardware(void);
void                   thread_sleep(unsigned int);
void                   thread_yield(void);
void                   thread_finalize(void);
void*                  thread_attach_jvm(void);
void                   thread_detach_jvm(void);

tick_t                 time_current(void);
tick_t                 time_diff(const tick_t, const tick_t);
deltatime_t            time_elapsed(const tick_t);
tick_t                 time_elapsed_ticks(const tick_t);
tick_t                 time_ticks_per_second(void);
deltatime_t            time_ticks_to_seconds(const tick_t);
tick_t                 time_startup(void);
tick_t                 time_system(void);

uuid_t                 uuid_generate_time(void);
uuid_t                 uuid_generate_name(const uuid_t, const char*, size_t);
uuid_t                 uuid_generate_random(void);
bool                   uuid_equal(const uuid_t, const uuid_t);
uuid_t                 uuid_null(void);
bool                   uuid_is_null(const uuid_t);
uuid_t                 uuid_dns(void);

size_t                 type_size_t(unsigned int);
ssize_t                type_ssize_t(int);

]]

-- Metatypes
local string_meta_t
local string_meta_table = {
	__concat = function(lhs, rhs) return C.string_allocate_concat(lhs.str, lhs.length, rhs.str, rhs.length) end,
	__len = function(str) return str.length end,
	__gc = function(str) C.string_deallocate(str.str) end,
	__index = {}
}
local string_meta_t = ffi.metatype("string_t", string_meta_table)

local string_const_meta_t
local string_const_meta_table = {
	__concat = function(lhs, rhs) return C.string_allocate_concat(lhs.str, lhs.length, rhs.str, rhs.length) end,
	__len = function(str) return str.length end,
	__index = {}
}
local string_const_meta_t = ffi.metatype("string_const_t", string_const_meta_table)

-- Helper functions
function hash(str)
	return C.hash(str, #str)
end

function string_array_to_table(arr)
      local tab = {}
      local arr_size = C.array_size(arr)
      for i = 1, arr_size do
            tab[i] = ffi.string( C.array_element_pointer( arr, i-1 ) )
      end
      return tab
end

function string_table_to_array(tab)
      local num = 0
      while tab[num+1] ~= nil do num = num + 1 end
      local arr = C.array_allocate_pointer( num )
      for i = 0, num do
      	  C.array_set_element_pointer( arr, i, tab[i+1] )
      end
      return arr, num
end

-- Constants
PLATFORM_WINDOWS = 0
PLATFORM_LINUX = 1
PLATFORM_MACOSX = 2
PLATFORM_IOS = 3
PLATFORM_ANDROID = 4
PLATFORM_RASPBERRYPI = 5
PLATFORM_PNACL = 6
PLATFORM_BSD = 7
PLATFORM_TIZEN = 8

ARCHITECTURE_X86 = 0
ARCHITECTURE_X86_64 = 1
ARCHITECTURE_PPC = 2
ARCHITECTURE_PPC_64 = 3
ARCHITECTURE_ARM5 = 4
ARCHITECTURE_ARM6 = 5
ARCHITECTURE_ARM7 = 6
ARCHITECTURE_ARM8 = 7
ARCHITECTURE_ARM8_64 = 8
ARCHITECTURE_MIPS = 9
ARCHITECTURE_MIPS_64 = 10
ARCHITECTURE_GENERIC = 11

BYTEORDER_LITTLEENDIAN = 0
BYTEORDER_BIGENDIAN = 1

FOUNDATIONEVENT_START = 1
FOUNDATIONEVENT_TERMINATE = 2
FOUNDATIONEVENT_PAUSE = 3
FOUNDATIONEVENT_RESUME = 4
FOUNDATIONEVENT_FOCUS_GAIN = 5
FOUNDATIONEVENT_FOCUS_LOST = 6
FOUNDATIONEVENT_FILE_CREATED = 7
FOUNDATIONEVENT_FILE_DELETED = 8
FOUNDATIONEVENT_FILE_MODIFIED = 9
FOUNDATIONEVENT_LOW_MEMORY_WARNING = 10
FOUNDATIONEVENT_DEVICE_ORIENTATION = 11

EVENTFLAG_DELAY = 1

ERRORLEVEL_NONE = 0
ERRORLEVEL_DEBUG = 1
ERRORLEVEL_INFO = 2
ERRORLEVEL_WARNING = 3
ERRORLEVEL_ERROR = 4
ERRORLEVEL_PANIC = 5

ERROR_NONE = 0
ERROR_INVALID_VALUE = 1
ERROR_UNSUPPORTED = 2
ERROR_NOT_IMPLEMENTED = 3
ERROR_OUT_OF_MEMORY = 4
ERROR_MEMORY_LEAK = 5
ERROR_MEMORY_ALIGNMENT = 6
ERROR_INTERNAL_FAILURE = 7
ERROR_ACCESS_DENIED = 8
ERROR_EXCEPTION = 9
ERROR_SYSTEM_CALL_FAIL = 10
ERROR_UNKNOWN_TYPE = 11
ERROR_UNKNOWN_RESOURCE = 12
ERROR_DEPRECATED = 13
ERROR_ASSERT = 14
ERROR_SCRIPT = 15

WARNING_PERFORMANCE = 0
WARNING_DEPRECATED = 1
WARNING_INVALID_VALUE = 2
WARNING_MEMORY = 3
WARNING_UNSUPPORTED = 4
WARNING_SUSPICIOUS = 5
WARNING_SYSTEM_CALL_FAIL = 6
WARNING_DEADLOCK = 7
WARNING_SCRIPT = 8
WARNING_RESOURCE = 9

STREAM_IN = 0x0001
STREAM_OUT = 0x0002
STREAM_TRUNCATE = 0x0010
STREAM_CREATE = 0x0020
STREAM_ATEND = 0x0040
STREAM_BINARY = 0x0100
STREAM_SYNC = 0x0200

STREAM_SEEK_BEGIN = 0
STREAM_SEEK_CURRENT = 1
STREAM_SEEK_END = 2

BLOCKCIPHER_ECB = 0
BLOCKCIPHER_CBC = 1
BLOCKCIPHER_CFB = 2
BLOCKCIPHER_OFB = 3

RADIXSORT_INT32 = 0
RADIXSORT_UINT32 = 1
RADIXSORT_INT64 = 2
RADIXSORT_UINT64 = 3
RADIXSORT_FLOAT32 = 4
RADIXSORT_FLOAT64 = 5

DEVICEORIENTATION_UNKNOWN = 0
DEVICEORIENTATION_PORTRAIT = 1
DEVICEORIENTATION_PORTRAIT_FLIPPED = 2
DEVICEORIENTATION_LANDSCAPE_CCW = 3
DEVICEORIENTATION_LANDSCAPE_CW = 4
DEVICEORIENTATION_FACEUP = 5
DEVICEORIENTATION_FACEDOWN = 6

PROCESS_ATTACHED = 0
PROCESS_DETACHED = 0x01
PROCESS_CONSOLE = 0x02
PROCESS_STDSTREAMS = 0x04
PROCESS_WINDOWS_USE_SHELLEXECUTE = 0x08
PROCESS_MACOSX_USE_OPENAPPLICATION = 0x10
PROCESS_INVALID_ARGS = 0x7FFFFFF0
PROCESS_TERMINATED_SIGNAL = 0x7FFFFFF1
PROCESS_WAIT_INTERRUPTED = 0x7FFFFFF2
PROCESS_WAIT_FAILED = 0x7FFFFFF3
PROCESS_STILL_ACTIVE = 0x7FFFFFFF

STREAMTYPE_INVALID = 0
STREAMTYPE_MEMORY = 1
STREAMTYPE_FILE = 2
STREAMTYPE_SOCKET = 3
STREAMTYPE_RINGBUFFER = 4
STREAMTYPE_ASSET = 5
STREAMTYPE_PIPE = 6
STREAMTYPE_STDSTREAM = 7

THREAD_PRIORITY_LOW = 0
THREAD_PRIORITY_BELOWNORMAL = 1
THREAD_PRIORITY_NORMAL = 2
THREAD_PRIORITY_ABOVENORMAL = 3
THREAD_PRIORITY_HIGHEST = 4
THREAD_PRIORITY_TIMECRITICAL = 5

MEMORY_PERSISTENT          = 0x0000
MEMORY_TEMPORARY           = 0x0001
MEMORY_THREAD              = 0x0002
MEMORY_32BIT_ADDRESS       = 0x0004
MEMORY_ZERO_INITIALIZED    = 0x0008

-- Context hash value is hash of "lua", since luajit parser does not handle
-- full 64bit hex numbers (numbers > 0xc000000000000000 are truncated)
HASH_LUA = hash("lua")

-- Convenience wrappers
function log_debug(message) C.log_debugf(HASH_LUA, "%s", 2, message) end
function log_info(message) C.log_infof(HASH_LUA, "%s", 2, message) end
function log_warn(message) C.log_warnf(HASH_LUA, 8, "%s", 2, message) end -- 8 = WARNING_SCRIPT
function log_error(message) C.log_errorf(HASH_LUA, 15, "%s", 2, message) end -- 15 = ERROR_SCRIPT
function log_panic(message) C.log_panicf(HASH_LUA, 15, "%s", 2, message) end -- 15 = ERROR_SCRIPT

function size_t(val) return C.type_size_t(val) end
function ssize_t(val) return C.type_ssize_t(val) end
