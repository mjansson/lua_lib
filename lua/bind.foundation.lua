
local ffi = require( "ffi" )

ffi.cdef[[

void  log_debugf( const char*, ... );
void  log_infof( const char*, ... );
void  log_warnf( int, const char*, ... );
void  log_errorf( int, const char*, ... );
void  log_panicf( int, const char*, ... );
void  log_stdout( bool );
void  log_enable_prefix( bool );
void  log_suppress( int );

int   error( void );
int   error_report( int, int );
void  error_context_push( const char*, const char* );
void  error_context_pop( void );

const char* const*     environment_command_line( void );
const char*            environment_executable_name( void );
const char*            environment_executable_directory( void );
const char*            environment_initial_working_directory( void );
const char*            environment_current_working_directory( void );
void                   environment_set_current_working_directory( const char* );
const char*            environment_home_directory( void );
const char*            environment_temporary_directory( void );
const char*            environment_variable( const char* );

]]

local function log_debug( message ) ffi.C.log_debugf( "%s", message ) end
local function log_info( message ) ffi.C.log_infof( "%s", message ) end
local function log_warn( message ) ffi.C.log_warnf( 6, "%s", message ) end -- 6 = WARNING_SCRIPT
local function log_error( message ) ffi.C.log_errorf( 11, "%s", message ) end -- 11 = ERROR_SCRIPT
local function log_panic( message ) ffi.C.log_panicf( 11, "%s", message ) end -- 11 = ERROR_SCRIPT

local function string_array_to_table( arr )

end

local function environment_command_line()
      local arr = ffi.C.environment_command_line()
      return string_array_to_table( arr )
end


module("foundation")

log = {}
log.debug = log_debug
log.info = log_info
log.warn = log_warn
log.error = log_error
log.panic = log_panic
log.stdout = ffi.C.log_stdout
log.enable_prefix = ffi.C.log_enable_prefix
log.suppress = ffi.C.log_suppress

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
error.INTERNAL_FAILURE = 5
error.MALLOC_FAILURE = 6
error.EMORY_LEAK = 7
error.ACCESS_DENIED = 8
error.EXCEPTION = 9
error.SYSTEM_CALL_FAIL = 10
error.SCRIPT = 11
error.UNKNOWN_TYPE = 12
error.UNKNOWN_RESOURCE = 13
error.MEMORY_ALIGNMENT = 14
error.DEPRECATED = 15
error.WARNING_PERFORMANCE = 0
error.WARNING_DEPRECATED = 1
error.WARNING_BAD_DATA = 2
error.WARNING_MEMORY = 3
error.WARNING_UNSUPPORTED = 4
error.WARNING_SUSPICIOUS = 5
error.WARNING_SCRIPT = 6
error.WARNING_SYSTEM_CALL_FAIL = 7
error.WARNING_DEADLOCK = 8

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
