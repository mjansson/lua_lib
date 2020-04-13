
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
typedef struct render_vertexshader_t render_vertexshader_t;
typedef struct render_pixelshader_t render_pixelshader_t;
typedef struct render_program_t render_program_t;
typedef struct render_program_attribute_t render_program_attribute_t;
typedef struct render_texture_t render_texture_t;
typedef struct render_state_t render_state_t;
typedef struct render_resolution_t render_resolution_t;
typedef struct render_vertex_decl_element_t render_vertex_decl_element_t;
typedef struct render_parameter_t render_parameter_t;
typedef struct render_parameterbuffer_t render_parameterbuffer_t;
typedef struct render_statebuffer_t render_statebuffer_t;
typedef struct render_pipeline_t render_pipeline_t;
typedef struct render_pipeline_step_t render_pipeline_step_t;
typedef struct render_config_t render_config_t;

render_backend_t* render_backend_allocate(int, bool);
void render_backend_deallocate(render_backend_t*);
int render_backend_api(render_backend_t*);
size_t render_backend_enumerate_adapters(render_backend_t*, unsigned int*, size_t);
size_t render_backend_enumerate_modes(render_backend_t*, unsigned int, render_resolution_t*, size_t);
void render_backend_set_format(render_backend_t*, int, int);
void render_backend_set_max_concurrency(render_backend_t*, size_t);
size_t render_backend_max_concurrency(render_backend_t*);
void render_backend_set_drawable(render_backend_t*, render_drawable_t*);
render_drawable_t* render_backend_drawable(render_backend_t*);
render_target_t* render_backend_target_framebuffer(render_backend_t*);
void render_backend_dispatch(render_backend_t*, render_target_t*, render_context_t**, size_t);
void render_backend_flip(render_backend_t*);
uint64_t render_backend_frame_count(render_backend_t*);
void render_backend_enable_thread(render_backend_t*);
void render_backend_disable_thread(render_backend_t*);
render_backend_t* render_backend_thread(void);
bool render_backend_try_enter_exclusive(render_backend_t*);
void render_backend_enter_exclusive(render_backend_t*);
void render_backend_leave_exclusive(render_backend_t*);
uint64_t render_backend_resource_platform(render_backend_t*);
void render_backend_set_resource_platform(render_backend_t*, uint64_t);
bool render_backend_shader_upload(render_backend_t*, render_shader_t*, const void*, size_t);
uuidmap_t* render_backend_shader_table(render_backend_t*);
bool render_backend_program_upload(render_backend_t*, render_program_t*);
uuidmap_t* render_backend_program_table(render_backend_t*);
bool render_backend_texture_upload(render_backend_t*, render_texture_t*, const void*, size_t);
void render_backend_parameter_bind_texture(render_backend_t*, void*, render_texture_t*);
void render_backend_parameter_bind_target(render_backend_t*, void*, render_target_t*);
uuidmap_t* render_backend_texture_table(render_backend_t*);
render_backend_t** render_backends(void);

render_command_t* render_command_allocate(void);
void render_command_null(render_command_t* command);
void render_command_clear(render_command_t*, unsigned int, uint32_t, unsigned int, float, uint32_t);
void render_command_viewport(render_command_t*, int, int, int, int, real, real);
void render_command_render(render_command_t*, int, size_t, render_program_t*, render_vertexbuffer_t*, render_indexbuffer_t*, render_parameterbuffer_t*, render_statebuffer_t*);

int render_compile(const uuid_t, uint64_t, resource_source_t*, const uint256_t, const char*, size_t);

render_context_t* render_context_allocate(size_t);
void render_context_deallocate(render_context_t*);
render_command_t* render_context_reserve(render_context_t*, uint64_t);
void render_context_queue(render_context_t*, render_command_t*, uint64_t);
size_t render_context_reserved(render_context_t* context);

render_drawable_t* render_drawable_allocate(void);
void render_drawable_deallocate(render_drawable_t*);
void render_drawable_initialize_window(render_drawable_t*, window_t*, unsigned int);
void render_drawable_initialize_fullscreen(render_drawable_t*, unsigned int, unsigned int, unsigned int, unsigned int);
void render_drawable_finalize(render_drawable_t*);

void render_event_handle_resource(const event_t*);

int render_import(stream_t* stream, const uuid_t);

render_indexbuffer_t* render_indexbuffer_allocate(render_backend_t*, render_usage_t, size_t, size_t, int, const void*, size_t);
void render_indexbuffer_deallocate(render_indexbuffer_t*);
render_indexbuffer_t* render_indexbuffer_load(const uuid_t);
void render_indexbuffer_lock(render_indexbuffer_t*, unsigned int);
void render_indexbuffer_unlock(render_indexbuffer_t*);
void render_indexbuffer_upload(render_indexbuffer_t*);
void render_indexbuffer_free(render_indexbuffer_t*, bool, bool);
void render_indexbuffer_restore(render_indexbuffer_t*);

void render_parameterbuffer_initialize(render_parameterbuffer_t*, render_backend_t*, int, const render_parameter_t*, size_t, const void*, size_t);
render_parameterbuffer_t* render_parameterbuffer_allocate(render_backend_t*, int, const render_parameter_t*, size_t, const void*, size_t);
void render_parameterbuffer_deallocate(render_parameterbuffer_t*);
void render_parameterbuffer_finalize(render_parameterbuffer_t*);
void render_parameterbuffer_link(render_parameterbuffer_t*, render_program_t*);
void render_parameterbuffer_lock(render_parameterbuffer_t*, unsigned int);
void render_parameterbuffer_unlock(render_parameterbuffer_t*);
void render_parameterbuffer_upload(render_parameterbuffer_t*);
void render_parameterbuffer_free(render_parameterbuffer_t*, bool, bool);
void render_parameterbuffer_restore(render_parameterbuffer_t*);

render_pipeline_t* render_pipeline_allocate(render_backend_t*);
void render_pipeline_initialize(render_pipeline_t*, render_backend_t*);
void render_pipeline_finalize(render_pipeline_t*);
void render_pipeline_deallocate(render_pipeline_t*);
void render_pipeline_execute(render_pipeline_t*);
void render_pipeline_dispatch(render_pipeline_t*);
void render_pipeline_step_initialize(render_pipeline_step_t*, render_target_t*, render_pipeline_execute_fn);
void render_pipeline_step_finalize(render_pipeline_step_t*);
void render_pipeline_step_blit_initialize(render_pipeline_step_t*, render_target_t*, render_target_t*);

render_program_t* render_program_allocate(size_t);
void render_program_initialize(render_program_t*, size_t);
void render_program_finalize(render_program_t*);
void render_program_deallocate(render_program_t*);
render_program_t* render_program_load(render_backend_t*, const uuid_t);
render_program_t* render_program_lookup(render_backend_t*, const uuid_t);
bool render_program_reload(render_program_t*, const uuid_t);
void render_program_unload(render_program_t*);
int render_program_compile(const uuid_t, uint64_t, resource_source_t*, const uint256_t, const char*, size_t);

matrix_t render_projection_perspective(real, real, real, real);
matrix_t render_projection_orthographic(real, real, real, real, real, real);

int render_module_initialize(render_config_t);
void render_module_finalize(void);
bool render_module_is_initialized(void);
version_t render_module_version(void);
void render_api_enable(const render_api_t*, size_t);
void render_api_disable(const render_api_t*, size_t);
void render_module_parse_config(const char*, size_t, const char*, size_t, const json_token_t*, size_t);

render_shader_t* render_pixelshader_allocate(void);
render_shader_t* render_vertexshader_allocate(void);
void render_pixelshader_initialize(render_shader_t*);
void render_vertexshader_initialize(render_shader_t*);
void render_shader_finalize(render_shader_t*);
void render_shader_deallocate(render_shader_t*);
render_shader_t* render_shader_load(render_backend_t*, const uuid_t);
render_shader_t* render_shader_lookup(render_backend_t*, const uuid_t);
bool render_shader_reload(render_shader_t*, const uuid_t);
void render_shader_unload(render_shader_t*);
int render_shader_compile(const uuid_t, uint64_t, resource_source_t*, const uint256_t, const char*, size_t);

void render_sort_merge(render_context_t**, size_t);
void render_sort_reset(render_context_t*);
uint64_t render_sort_sequential_key(render_context_t*);
uint64_t render_sort_render_key(render_context_t*, render_buffer_t*, render_buffer_t*, render_state_t*);

render_state_t render_state_default(void);
void render_statebuffer_initialize(render_statebuffer_t*, render_backend_t*, int, const render_state_t);
render_statebuffer_t* render_statebuffer_allocate(render_backend_t*, int, const render_state_t);
void render_statebuffer_deallocate(render_statebuffer_t*);
void render_statebuffer_finalize(render_statebuffer_t*);
void render_statebuffer_lock(render_statebuffer_t*, unsigned int);
void render_statebuffer_unlock(render_statebuffer_t*);
render_state_t* render_statebuffer_data(render_statebuffer_t*);
void render_statebuffer_upload(render_statebuffer_t*);
void render_statebuffer_free(render_statebuffer_t*, bool, bool);
void render_statebuffer_restore(render_statebuffer_t*);

render_target_t* render_target_allocate(render_backend_t*, unsigned int, unsigned int, int, int);
void render_target_initialize(render_target_t*, render_backend_t*, unsigned int, unsigned int, int, int);
void render_target_finalize(render_target_t*);
void render_target_deallocate(render_target_t*);
bool render_target_resize(render_target_t*, unsigned int, unsigned int);

render_texture_t* render_texture_allocate(void);
void render_texture_initialize(render_texture_t*);
void render_texture_finalize(render_texture_t*);
void render_texture_deallocate(render_texture_t*);
render_texture_t* render_texture_load(render_backend_t*, const uuid_t);
render_texture_t* render_texture_lookup(render_backend_t*, const uuid_t);
bool render_texture_reload(render_texture_t*, const uuid_t);
void render_texture_unload(render_texture_t*);
int render_texture_compile(const uuid_t, uint64_t, resource_source_t*, const uint256_t, const char*, size_t);

uint16_t render_vertex_attribute_size(render_vertex_format_t);
render_vertex_decl_t* render_vertex_decl_allocate(render_vertex_decl_element_t*, size_t);
render_vertex_decl_t* render_vertex_decl_allocate_varg(int, int, ...);
void render_vertex_decl_initialize(render_vertex_decl_t*, render_vertex_decl_element_t*, size_t);
void render_vertex_decl_initialize_varg(render_vertex_decl_t*, int, int, ...);
void render_vertex_decl_finalize(render_vertex_decl_t*);
void render_vertex_decl_deallocate(render_vertex_decl_t*);
size_t render_vertex_decl_calculate_size(const render_vertex_decl_t*);
render_vertexbuffer_t* render_vertexbuffer_allocate(render_backend_t*, int, size_t, size_t, const render_vertex_decl_t*, const void*, size_t);
void render_vertexbuffer_deallocate(render_vertexbuffer_t*);
void render_vertexbuffer_lock(render_vertexbuffer_t*, unsigned int);
void render_vertexbuffer_unlock(render_vertexbuffer_t*);
void render_vertexbuffer_upload(render_vertexbuffer_t*);
void render_vertexbuffer_free(render_vertexbuffer_t*, bool, bool);
void render_vertexbuffer_restore(render_vertexbuffer_t*);

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
RENDERAPI_COUNT = 17,

RENDERAPIGROUP_NONE = 0,
RENDERAPIGROUP_OPENGL = 1,
RENDERAPIGROUP_DIRECTX = 2,
RENDERAPIGROUP_GLES = 3,
RENDERAPIGROUP_COUNT = 4,

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

RENDERBUFFER_LOCK_READ        = 0x00000010,
RENDERBUFFER_LOCK_WRITE       = 0x00000020,
RENDERBUFFER_LOCK_NOUPLOAD    = 0x00000040,
RENDERBUFFER_LOCK_FORCEUPLOAD = 0x00000080,
RENDERBUFFER_LOCK_BITS        = 0x000000F0,

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
VERTEXFORMAT_COUNT = 12,
VERTEXFORMAT_UNUSED = 255,

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
VERTEXATTRIBUTE_COUNT = 16,

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
PIXELFORMAT_R8G8B8 = 1,
PIXELFORMAT_R8G8B8A8 = 2,
PIXELFORMAT_R16G16B16 = 3,
PIXELFORMAT_R16G16B16A16 = 4,
PIXELFORMAT_R32G32B32F = 5,
PIXELFORMAT_R32G32B32A32F = 6,
PIXELFORMAT_A8 = 7,
PIXELFORMAT_PVRTC_2 = 8,
PIXELFORMAT_PVRTC_4 = 9,
PIXELFORMAT_COUNT = 10,

COLORSPACE_INVALID = 0,
COLORSPACE_LINEAR = 1,
COLORSPACE_sRGB = 2,
COLORSPACE_COUNT = 3,

RENDERTEXTURE_FLAG_AUTOGENERATE_MIPMAPS = 1,

RENDERPRIMITIVE_INVALID = 0,
RENDERPRIMITIVE_TRIANGLELIST = 1,
RENDERPRIMITIVE_LINELIST = 2,
RENDERPRIMITIVE_COUNT = 1,

RENDERCOMMAND_INVALID = 0,
RENDERCOMMAND_CLEAR = 1,
RENDERCOMMAND_VIEWPORT = 2,
RENDERCOMMAND_RENDER_TRIANGLELIST = 3,
RENDERCOMMAND_RENDER_LINELIST = 4,

}
