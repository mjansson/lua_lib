
local ffi = require("ffi")
local C = ffi.C

local foundation = require("foundation")
local window = require("window")

-- Render types and functions
ffi.cdef[[

typedef struct render_backend_vtable_t render_backend_vtable_t;
typedef struct render_backend_t render_backend_t;
typedef struct render_drawable_t render_drawable_t;
typedef struct render_target_t render_target_t;
typedef struct render_context_t render_context_t;
typedef struct render_command_clear_t render_command_clear_t;
typedef struct render_command_viewport_t render_command_viewport_t;
typedef struct render_command_render_t render_command_render_t;
typedef struct render_command_t render_command_t;
typedef struct render_vertex_attribute_t render_vertex_attribute_t;
typedef struct render_vertex_decl_t render_vertex_decl_t;
typedef struct render_buffer_t render_buffer_t;
typedef struct render_vertexbuffer_t render_vertexbuffer_t;
typedef struct render_indexbuffer_t render_indexbuffer_t;
typedef struct render_shader_t render_shader_t;
typedef struct render_shader_ref_t render_shader_ref_t;
typedef struct render_vertexshader_t render_vertexshader_t;
typedef struct render_pixelshader_t render_pixelshader_t;
typedef struct render_program_t render_program_t;
typedef struct render_program_ref_t render_program_ref_t;
typedef struct render_state_t render_state_t;
typedef struct render_resolution_t render_resolution_t;
typedef struct render_vertex_decl_element_t render_vertex_decl_element_t;
typedef struct render_parameter_t render_parameter_t;
typedef struct render_parameterbuffer_t render_parameterbuffer_t;
typedef struct render_statebuffer_t render_statebuffer_t;
typedef struct render_config_t render_config_t;

typedef bool (* render_backend_construct_fn)(render_backend_t*);
typedef void (* render_backend_destruct_fn)(render_backend_t*);
typedef size_t (* render_backend_enumerate_adapters_fn)(render_backend_t*, unsigned int*, size_t);
typedef size_t (* render_backend_enumerate_modes_fn)(render_backend_t*, unsigned int, render_resolution_t*, size_t);
typedef void (* render_backend_enable_thread_fn)(render_backend_t*);
typedef void (* render_backend_disable_thread_fn)(render_backend_t*);
typedef bool (* render_backend_set_drawable_fn)(render_backend_t*, render_drawable_t*);
typedef void (* render_backend_dispatch_fn)(render_backend_t*, render_context_t**, size_t);
typedef void (* render_backend_flip_fn)(render_backend_t*);
typedef void* (* render_backend_allocate_buffer_fn)(render_backend_t*, render_buffer_t*);
typedef void (* render_backend_deallocate_buffer_fn)(render_backend_t*, render_buffer_t*, bool,
                                                     bool);
typedef bool (* render_backend_upload_buffer_fn)(render_backend_t*, render_buffer_t*);
typedef bool (* render_backend_upload_shader_fn)(render_backend_t*, render_shader_t*, const void*,
                                                 size_t);
typedef bool (* render_backend_upload_program_fn)(render_backend_t*, render_program_t*);
typedef void (* render_backend_deallocate_shader_fn)(render_backend_t*, render_shader_t*);
typedef void (* render_backend_deallocate_program_fn)(render_backend_t*, render_program_t*);
typedef void (* render_backend_link_buffer_fn)(render_backend_t*, render_buffer_t*,
                                               render_program_t* program);

render_backend_t* render_backend_allocate(int, bool);
void render_backend_deallocate(render_backend_t*);
int render_backend_api(render_backend_t*);
size_t render_backend_enumerate_adapters(render_backend_t*, unsigned int*, size_t);
size_t render_backend_enumerate_modes(render_backend_t*, unsigned int, render_resolution_t*, size_t);
void render_backend_set_format(render_backend_t*, int, int);
void render_backend_set_drawable(render_backend_t*, render_drawable_t*);
render_drawable_t* render_backend_drawable(render_backend_t*);
object_t render_backend_target_framebuffer(render_backend_t*);
void render_backend_dispatch(render_backend_t*, render_context_t**, size_t);
void render_backend_flip(render_backend_t*);
uint64_t render_backend_frame_count(render_backend_t*);
void render_backend_enable_thread(render_backend_t*);
void render_backend_disable_thread(render_backend_t*);
render_backend_t* render_backend_thread(void);
uint64_t render_backend_resource_platform(render_backend_t*);
void render_backend_set_resource_platform(render_backend_t*, uint64_t);
object_t render_backend_shader_ref(render_backend_t*, const uuid_t);
render_shader_t* render_backend_shader_ptr(render_backend_t*, object_t);
object_t render_backend_shader_bind(render_backend_t*, const uuid_t, render_shader_t*);
void render_backend_shader_unref(render_backend_t*, object_t);
bool render_backend_shader_upload(render_backend_t*, render_shader_t*, const void*, size_t);
object_t render_backend_program_ref(render_backend_t*, const uuid_t);
render_program_t* render_backend_program_ptr(render_backend_t*, object_t);
object_t render_backend_program_bind(render_backend_t*, const uuid_t, render_program_t*); 
void render_backend_program_unref(render_backend_t*, object_t);
bool render_backend_program_upload(render_backend_t*, render_program_t*);

render_command_t* render_command_allocate(void);
void render_command_null(render_command_t* command);
void render_command_clear(render_command_t*, unsigned int, uint32_t, unsigned int, float, uint32_t);
void render_command_viewport(render_command_t*, int, int, int, int, real, real);
void render_command_render(render_command_t*, int, uint16_t, render_program_t*, object_t, object_t, object_t, object_t);

int render_compile(const uuid_t, uint64_t, resource_source_t*, const uint256_t, const char*, size_t);

render_context_t* render_context_allocate(size_t);
void render_context_deallocate(render_context_t*);
object_t render_context_target(render_context_t*);
void render_context_set_target(render_context_t*, object_t);
render_command_t* render_context_reserve(render_context_t*, uint64_t);
void render_context_queue(render_context_t*, render_command_t*, uint64_t);
size_t render_context_reserved(render_context_t* context);
uint8_t render_context_group(render_context_t* context);
void render_context_set_group(render_context_t* context, uint8_t);

render_drawable_t* render_drawable_allocate(void);
void render_drawable_deallocate(render_drawable_t*);
void render_drawable_set_window(render_drawable_t*, window_t*, unsigned int);
void render_drawable_set_offscreen(render_drawable_t*, object_t);
void render_drawable_set_fullscreen(render_drawable_t*, unsigned int, int, int, int);
int render_drawable_type(render_drawable_t*); 
int render_drawable_width(render_drawable_t*);
int render_drawable_height(render_drawable_t*);

void render_event_handle_resource(render_backend_t*, const event_t*);

int render_import(stream_t* stream, const uuid_t);

object_t render_indexbuffer_create(render_backend_t*, int, size_t, const uint16_t*);
object_t render_indexbuffer_load(const uuid_t);
object_t render_indexbuffer_ref(object_t);
void render_indexbuffer_unref(object_t);
int render_indexbuffer_usage(object_t);
size_t render_indexbuffer_num_allocated(object_t);
size_t render_indexbuffer_num_elements(object_t);
void render_indexbuffer_set_num_elements(object_t, size_t);
void render_indexbuffer_lock(object_t, unsigned int);
void render_indexbuffer_unlock(object_t);
int render_indexbuffer_upload_policy(object_t);
void render_indexbuffer_set_upload_policy(object_t, int);
void render_indexbuffer_upload(object_t);
uint16_t* render_indexbuffer_element(object_t, size_t);
size_t render_indexbuffer_element_size(object_t);
void render_indexbuffer_release(object_t, bool, bool);
void render_indexbuffer_restore(object_t);

object_t render_parameterbuffer_create(render_backend_t*, int, const render_parameter_t*, size_t, const void*, size_t);
object_t render_parameterbuffer_ref(object_t);
void render_parameterbuffer_unref(object_t);
void render_parameterbuffer_link(object_t, render_program_t*);
unsigned int render_parameterbuffer_num_parameters(object_t);
const render_parameter_t* render_parameterbuffer_parameters(object_t);
void render_parameterbuffer_lock(object_t, unsigned int);
void render_parameterbuffer_unlock(object_t);
int render_parameterbuffer_upload_policy(object_t);
void render_parameterbuffer_set_upload_policy(object_t, int);
void* render_parameterbuffer_data(object_t);
void render_parameterbuffer_upload(object_t);
void render_parameterbuffer_release(object_t, bool, bool);
void render_parameterbuffer_restore(object_t buffer);

]]

return {

-- Constants
RENDERAPI_UNKNOWN = 0,
RENDERAPI_DEFAULT = 1,
RENDERAPI_NULL = 2,
RENDERAPI_OPENGL = 3,
RENDERAPI_OPENGL2 = 4,
RENDERAPI_OPENGL3 = 5,
RENDERAPI_OPENGL4 = 6,
RENDERAPI_DIRECTX = 7,
RENDERAPI_DIRECTX10 = 8,
RENDERAPI_DIRECTX11 = 9,
RENDERAPI_PS3 = 10,
RENDERAPI_PS4 = 11,
RENDERAPI_XBOX360 = 12,
RENDERAPI_XBOXONE = 13,
RENDERAPI_GLES = 14,
RENDERAPI_GLES2 = 15,
RENDERAPI_GLES3 = 16,
RENDERAPI_NUM = 17,

RENDERAPIGROUP_NONE = 0,
RENDERAPIGROUP_OPENGL = 1,
RENDERAPIGROUP_DIRECTX = 2,
RENDERAPIGROUP_GLES = 3,
RENDERAPIGROUP_NUM = 4,

RENDERDRAWABLE_INVALID = 0,
RENDERDRAWABLE_WINDOW = 1,
RENDERDRAWABLE_OFFSCREEN = 2,
RENDERDRAWABLE_FULLSCREEN = 3,

RENDERUSAGE_INVALID = 0,
RENDERUSAGE_NORENDER = 1,
RENDERUSAGE_DYNAMIC = 2,
RENDERUSAGE_STATIC = 3,
RENDERUSAGE_TARGET = 4,

RENDERBUFFER_COLOR     = 0x01,
RENDERBUFFER_DEPTH     = 0x02,
RENDERBUFFER_STENCIL   = 0x04,

RENDERBUFFER_VERTEX    = 0x10,
RENDERBUFFER_INDEX     = 0x20,
RENDERBUFFER_PARAMETER = 0x40,
RENDERBUFFER_STATE     = 0x80,

RENDERBUFFER_UPLOAD_ONUNLOCK = 0,
RENDERBUFFER_UPLOAD_ONRENDER = 1,
RENDERBUFFER_UPLOAD_ONDISPATCH = 2,

RENDERBUFFER_DIRTY            = 0x00000001,
RENDERBUFFER_LOST             = 0x00000002,

RENDERBUFFER_LOCK_READ        = 0x00010000,
RENDERBUFFER_LOCK_WRITE       = 0x00020000,
RENDERBUFFER_LOCK_NOUPLOAD    = 0x00040000,
RENDERBUFFER_LOCK_FORCEUPLOAD = 0x00080000,
RENDERBUFFER_LOCK_BITS        = 0x000F0000,

VERTEXFORMAT_FLOAT = 0,
VERTEXFORMAT_FLOAT2 = 1,
VERTEXFORMAT_FLOAT3 = 2,
VERTEXFORMAT_FLOAT4 = 3,
VERTEXFORMAT_UBYTE4_UNORM = 4,
VERTEXFORMAT_UBYTE4_SNORM = 5,
VERTEXFORMAT_SHORT = 6,
VERTEXFORMAT_SHORT2 = 7,
VERTEXFORMAT_SHORT4 = 8,
VERTEXFORMAT_INT = 9,
VERTEXFORMAT_INT2 = 10,
VERTEXFORMAT_INT4 = 11,
VERTEXFORMAT_NUMTYPES = 12,
VERTEXFORMAT_UNKNOWN = 13,

VERTEXATTRIBUTE_POSITION = 0,
VERTEXATTRIBUTE_WEIGHT = 1,
VERTEXATTRIBUTE_NORMAL = 2,
VERTEXATTRIBUTE_PRIMARYCOLOR = 3,
VERTEXATTRIBUTE_SECONDARYCOLOR = 4,
VERTEXATTRIBUTE_UNNAMED_05 = 5,
VERTEXATTRIBUTE_UNNAMED_06 = 6,
VERTEXATTRIBUTE_INDEX = 7,
VERTEXATTRIBUTE_TEXCOORD0 = 8,
VERTEXATTRIBUTE_TEXCOORD1 = 9,
VERTEXATTRIBUTE_TEXCOORD2 = 10,
VERTEXATTRIBUTE_TEXCOORD3 = 11,
VERTEXATTRIBUTE_TEXCOORD4 = 12,
VERTEXATTRIBUTE_TEXCOORD5 = 13,
VERTEXATTRIBUTE_TANGENT = 14,
VERTEXATTRIBUTE_BINORMAL = 15,
VERTEXATTRIBUTE_NUMATTRIBUTES = 16,

SHADER_COMPUTE  = 1,
SHADER_GEOMETRY = 2,
SHADER_VERTEX   = 4,
SHADER_PIXEL    = 8,

RENDERPARAMETER_FLOAT4 = 0,
RENDERPARAMETER_INT4 = 1,
RENDERPARAMETER_MATRIX = 2,
RENDERPARAMETER_TEXTURE = 3,
RENDERPARAMETER_ATTRIBUTE = 4,

RENDERTEXTURE_1D = 0,
RENDERTEXTURE_2D = 1,
RENDERTEXTURE_3D = 2,
RENDERTEXTURE_CUBE = 3,

BLEND_ZERO = 0,
BLEND_ONE = 1,
BLEND_SRCCOLOR = 2,
BLEND_INVSRCCOLOR = 3,
BLEND_DESTCOLOR = 4,
BLEND_INVDESTCOLOR = 5,
BLEND_SRCALPHA = 6,
BLEND_INVSRCALPHA = 7,
BLEND_DESTALPHA = 8,
BLEND_INVDESTALPHA = 9,
BLEND_FACTOR = 10,
BLEND_INVFACTOR = 11,
BLEND_SRCALPHASAT = 12,

BLEND_OP_ADD = 0,
BLEND_OP_SUBTRACT = 1,
BLEND_OP_REV_SUBTRACT = 2,
BLEND_OP_MIN = 3,
BLEND_OP_MAX = 4,

RENDER_CMP_NEVER = 0,
RENDER_CMP_LESS = 1,
RENDER_CMP_LESSEQUAL = 2,
RENDER_CMP_EQUAL = 3,
RENDER_CMP_NOTEQUAL = 4,
RENDER_CMP_GREATEREQUAL = 5,
RENDER_CMP_GREATER = 6,
RENDER_CMP_ALWAYS = 7,

PIXELFORMAT_INVALID = 0,
PIXELFORMAT_R8G8B8X8 = 1,
PIXELFORMAT_R8G8B8A8 = 2,
PIXELFORMAT_R8G8B8 = 3,
PIXELFORMAT_R32G32B32A32F = 4,
PIXELFORMAT_A8 = 5,
PIXELFORMAT_PVRTC_2 = 6,
PIXELFORMAT_PVRTC_4 = 7,
PIXELFORMAT_NUMFORMATS = 8,

COLORSPACE_INVALID = 0,
COLORSPACE_LINEAR = 1,
COLORSPACE_sRGB = 2,
COLORSPACE_NUMSPACES = 3,

RENDERPRIMITIVE_TRIANGLELIST = 0,
RENDERPRIMITIVE_NUMTYPES = 1,

RENDERCOMMAND_INVALID = 0,
RENDERCOMMAND_CLEAR = 1,
RENDERCOMMAND_VIEWPORT = 2,
RENDERCOMMAND_RENDER_TRIANGLELIST = 3,

RENDERTARGET_FRAMEBUFFER = 0,
RENDERTARGET_NUMTYPES = 1

}
