
local ffi = require( "ffi" )

ffi.cdef[[

typedef struct _foundation_stream stream_t;
typedef struct _foundation_event_stream event_stream_t;

typedef float float32_t;
typedef double float64_t;

uint64_t hash( const char*, unsigned int );

void  log_debugf( uint64_t, const char*, ... );
void  log_infof( uint64_t, const char*, ... );
void  log_warnf( uint64_t, int, const char*, ... );
void  log_errorf( uint64_t, int, const char*, ... );
void  log_panicf( uint64_t, int, const char*, ... );
void  log_enable_stdout( bool );
void  log_enable_prefix( bool );
void  log_set_suppress( uint64_t, int );
int   log_suppress( uint64_t );

int   error( void );
int   error_report( int, int );
void  error_context_push( const char*, const char* );
void  error_context_pop( void );

unsigned int           string_length( const char* );
unsigned int           string_glyphs( const char* );

int                    array_size( const void* );
const void*            array_element_pointer( const void*, int );

const char* const*     environment_command_line( void );
const char*            environment_executable_name( void );
const char*            environment_executable_directory( void );
const char*            environment_initial_working_directory( void );
const char*            environment_current_working_directory( void );
void                   environment_set_current_working_directory( const char* );
const char*            environment_home_directory( void );
const char*            environment_temporary_directory( void );
const char*            environment_variable( const char* );

stream_t*              stream_open( const char*, unsigned int );
stream_t*              stream_clone( stream_t* );
void                   stream_deallocate( stream_t* );
int64_t                stream_tell( stream_t* );
void                   stream_seek( stream_t*, int64_t, int );
bool                   stream_eos( stream_t* );
void                   stream_set_byteorder( stream_t*, int );
void                   stream_set_binary( stream_t*, bool );
bool                   stream_is_sequential( stream_t* );
bool                   stream_is_reliable( stream_t* );
bool                   stream_is_inorder( stream_t* );
bool                   stream_is_swapped( stream_t* );
int                    stream_byteorder( stream_t* );
uint64_t               stream_last_modified( stream_t* );
uint64_t               stream_read( stream_t*, void*, uint64_t );
char*                  stream_read_line( stream_t*, char );
uint64_t               stream_size( stream_t* );
void                   stream_determine_binary_mode( stream_t*, unsigned int );
bool                   stream_read_bool( stream_t* );
int8_t                 stream_read_int8( stream_t* );
uint8_t                stream_read_uint8( stream_t* );
int16_t                stream_read_int16( stream_t* );
uint16_t               stream_read_uint16( stream_t* );
int32_t                stream_read_int32( stream_t* );
uint32_t               stream_read_uint32( stream_t* );
int64_t                stream_read_int64( stream_t* );
uint64_t               stream_read_uint64( stream_t* );
float32_t              stream_read_float32( stream_t* );
float64_t              stream_read_float64( stream_t* );
char*                  stream_read_string( stream_t* );
void                   stream_buffer_read( stream_t* );
unsigned int           stream_available_read( stream_t* );
void                   stream_write_bool( stream_t*, bool );
void                   stream_write_int8( stream_t*, int8_t );
void                   stream_write_uint8( stream_t*, uint8_t );
void                   stream_write_int16( stream_t*, int16_t );
void                   stream_write_uint16( stream_t*, uint16_t );
void                   stream_write_int32( stream_t*, int32_t );
void                   stream_write_uint32( stream_t*, uint32_t );
void                   stream_write_int64( stream_t*, int64_t );
void                   stream_write_uint64( stream_t*, uint64_t );
void                   stream_write_float32( stream_t*, float32_t );
void                   stream_write_float64( stream_t*, float64_t );
void                   stream_write_string( stream_t*, const char* );
void                   stream_write_endl( stream_t* );
void                   stream_truncate( stream_t*, uint64_t );
void                   stream_flush( stream_t* );
stream_t*              stream_open_stdout( void );
stream_t*              stream_open_stderr( void );
stream_t*              stream_open_stdin( void );

const char*            system_error_message( int code );
int                    system_platform( void );
int                    system_architecture( void );
int                    system_byteorder( void );
unsigned int           system_hardware_threads( void );
const char*            system_hostname( void );
uint64_t               system_hostid( void );
const char*            system_username( void );
bool                   system_debugger_attached( void );
void                   system_pause( void );
uint16_t               system_language( void );
uint16_t               system_country( void );
uint32_t               system_locale( void );
const char*            system_locale_string( void );
void                   system_process_events( void );
bool                   system_message_box( const char* title, const char* message, bool cancel_button );
event_stream_t*        system_event_stream( void );
void                   system_post_event( int event );

]]

local function hash( str ) return ffi.C.hash( str, ffi.C.string_length( str ) ) end

local function log_debug( message ) ffi.C.log_debugf( 0x7da58d38bd2b23e6, "%s", message ) end
local function log_info( message ) ffi.C.log_infof( 0x7da58d38bd2b23e6, "%s", message ) end
local function log_warn( message ) ffi.C.log_warnf( 0x7da58d38bd2b23e6, 8, "%s", message ) end -- 6 = WARNING_SCRIPT
local function log_error( message ) ffi.C.log_errorf( 0x7da58d38bd2b23e6, 15, "%s", message ) end -- 15 = ERROR_SCRIPT
local function log_panic( message ) ffi.C.log_panicf( 0x7da58d38bd2b23e6, 15, "%s", message ) end -- 15 = ERROR_SCRIPT
local function log_set_suppress( context, level ) ffi.C.log_set_suppress( hash( context ), level ) end
local function log_suppress( context ) return ffi.C.log_suppress( hash( context ) ) end

local function string_array_to_table( arr )
      local tab = {}
      local arr_size = ffi.C.array_size( arr )
      for i=1,arr_size do
            tab[i] = ffi.string( ffi.C.array_element_pointer( arr, i-1 ) )
      end
      return tab
end

local function environment_command_line()
      local arr = ffi.C.environment_command_line()
      return string_array_to_table( arr )
end


module("foundation")

PLATFORM_WINDOWS = 1
PLATFORM_LINUX = 2
PLATFORM_MACOSX = 3
PLATFORM_IOS = 4
PLATFORM_ANDROID = 5
PLATFORM_RASPBERRYPI = 6

ARCHITECTURE_X86 = 0
ARCHITECTURE_X86_64 = 0
ARCHITECTURE_PPC = 0
ARCHITECTURE_PPC_64 = 0
ARCHITECTURE_ARM6 = 0
ARCHITECTURE_ARM7 = 0

BYTEORDER_LITTLEENDIAN = 0
BYTEORDER_BIGENDIAN = 1

FOUNDATIONEVENT_TERMINATE = 1
FOUNDATIONEVENT_FILE_CREATED = 2
FOUNDATIONEVENT_FILE_DELETED = 3
FOUNDATIONEVENT_FILE_MODIFIED = 4

log = {}
log.debug = log_debug
log.info = log_info
log.warn = log_warn
log.error = log_error
log.panic = log_panic
log.enable_stdout = ffi.C.log_enable_stdout
log.enable_prefix = ffi.C.log_enable_prefix
log.set_suppress = log_set_suppress
log.suppress = log_suppress

error = {}
error.get = ffi.C.error
error.report = ffi.C.error_report
error.context_push = ffi.C.error_context_push
error.context_pop = ffi.C.error_context_pop
error.LEVEL_NONE = 0
error.LEVEL_DEBUG = 1
error.LEVEL_INFO = 2
error.LEVEL_WARNING = 3
error.LEVEL_ERROR = 4
error.LEVEL_PANIC = 5
error.NONE = 0
error.INVALID_VALUE = 1
error.UNSUPPORTED = 2
error.NOT_IMPLEMENTED = 3
error.OUT_OF_MEMORY = 4
error.MEMORY_LEAK = 5
error.MEMORY_ALIGNMENT = 6
error.INTERNAL_FAILURE = 7
error.ACCESS_DENIED = 8
error.EXCEPTION = 9
error.SYSTEM_CALL_FAIL = 10
error.UNKNOWN_TYPE = 11
error.UNKNOWN_RESOURCE = 12
error.DEPRECATED = 13
error.ASSERT = 14
error.SCRIPT = 15
error.WARNING_PERFORMANCE = 0
error.WARNING_DEPRECATED = 1
error.WARNING_BAD_DATA = 2
error.WARNING_MEMORY = 3
error.WARNING_UNSUPPORTED = 4
error.WARNING_SUSPICIOUS = 5
error.WARNING_SYSTEM_CALL_FAIL = 6
error.WARNING_DEADLOCK = 7
error.WARNING_SCRIPT = 8

string = {}
string.array_to_table = string_array_to_table

environment = {}
environment.command_line = environment_command_line
environment.executable_name = ffi.C.environment_executable_name
environment.executable_directory = ffi.C.environment_executable_directory
environment.initial_working_directory = ffi.C.environment_initial_working_directory
environment.current_working_directory = ffi.C.environment_current_working_directory
environment.set_current_working_directory = ffi.C.environment_set_current_working_directory
environment.home_directory = ffi.C.environment_home_directory
environment.temporary_directory = ffi.C.environment_temporary_directory
environment.variable = ffi.C.environment_variable

stream = {}
stream.IN = 1
stream.OUT = 2
stream.INOUT = 3
stream.TRUNCATE = 4
stream.ATEND = 8
stream.BINARY = 16
stream.SYNC = 32
stream.SEEK_BEGIN = 0
stream.SEEK_CURRENT = 1
stream.SEEK_END = 2
stream.open = ffi.C.stream_open
stream.clone = ffi.C.stream_clone
stream.deallocate = ffi.C.stream_deallocate
stream.tell = ffi.C.stream_tell
stream.seek = ffi.C.stream_seek
stream.eos = ffi.C.stream_eos
stream.byteorder = ffi.C.stream_set_byteorder
stream.binary = ffi.C.stream_set_binary
stream.is_sequential = ffi.C.stream_is_sequential
stream.is_reliable = ffi.C.stream_is_reliable
stream.is_inorder = ffi.C.stream_is_inorder
stream.is_swapped = ffi.C.stream_is_swapped
stream.byteorder = ffi.C.stream_byteorder
stream.last_modified = ffi.C.stream_last_modified
stream.read = ffi.C.stream_read
stream.read_line = ffi.C.stream_read_line
stream.size = ffi.C.stream_size
stream.determine_binary_mode = ffi.C.stream_determine_binary_mode
stream.read_bool = ffi.C.stream_read_bool
stream.read_int8 = ffi.C.stream_read_int8
stream.read_uint8 = ffi.C.stream_read_uint8
stream.read_int16 = ffi.C.stream_read_int16
stream.read_uint16 = ffi.C.stream_read_uint16
stream.read_int32 = ffi.C.stream_read_int32
stream.read_uint32 = ffi.C.stream_read_uint32
stream.read_int64 = ffi.C.stream_read_int64
stream.read_uint64 = ffi.C.stream_read_uint64
stream.read_float32 = ffi.C.stream_read_float32
stream.read_float64 = ffi.C.stream_read_float64
stream.read_string = ffi.C.stream_read_string
stream.buffer_read = ffi.C.stream_buffer_read
stream.available_read = ffi.C.stream_available_read
stream.write_bool = ffi.C.stream_write_bool
stream.write_int8 = ffi.C.stream_write_int8
stream.write_uint8 = ffi.C.stream_write_uint8
stream.write_int16 = ffi.C.stream_write_int16
stream.write_uint16 = ffi.C.stream_write_uint16
stream.write_int32 = ffi.C.stream_write_int32
stream.write_uint32 = ffi.C.stream_write_uint32
stream.write_in64 = ffi.C.stream_write_int64
stream.write_uint64 = ffi.C.stream_write_uint64
stream.write_float32 = ffi.C.stream_write_float32
stream.write_float64 = ffi.C.stream_write_float64
stream.write_string = ffi.C.stream_write_string
stream.write_endl = ffi.C.stream_write_endl
stream.truncate = ffi.C.stream_truncate
stream.flush = ffi.C.stream_flush
stream.open_stdout = ffi.C.stream_open_stdout
stream.open_stderr = ffi.C.stream_open_stderr
stream.open_stdin = ffi.C.stream_open_stdin

system = {}
system.error_message = ffi.C.system_error_message
system.platform = ffi.C.system_platform
system.architecture = ffi.C.system_architecture
system.byteorder = ffi.C.system_byteorder
system.hardware_threads = ffi.C.system_hardware_threads
system.hostname = ffi.C.system_hostname
system.hostid = ffi.C.system_hostid
system.username = ffi.C.system_username
system.debugger_attached = ffi.C.system_debugger_attached
system.pause = ffi.C.system_pause
system.language = ffi.C.system_language
system.country = ffi.C.system_country
system.locale = ffi.C.system_locale
system.locale_string = ffi.C.system_locale_string
system.process_events = ffi.C.system_process_events
system.message_box = ffi.C.system_message_box
system.event_stream = ffi.C.system_event_stream
system.post_event = ffi.C.system_post_event
