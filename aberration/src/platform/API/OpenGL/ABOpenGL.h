#pragma once
#include "AB.h"
#include "khrplatform.h"
#include "src/utils/Log.h"

// Aberration OpenGL header
// Based on glcorearb.h

#if defined(__gl_h_)
#error gl/gl.h included before ABOpenGL.h
#endif

namespace AB::GL {
	void _ClearErrorQueue();
	bool32 _PeekError();
	void _GetLog(char* buffer, uint64 size);
}

#if defined(AB_CONFIG_DEBUG)
#define _AB_GLCALL(call) \
	do {\
		AB::GL::_ClearErrorQueue();\
		call;\
		while (AB::GL::_PeekError()) {\
			char buffer[256];\
			AB::GL::_GetLog(buffer, 256);\
			AB_CORE_ERROR(buffer, "Call: ", #call);\
		}\
	} while (false)\

#define AB_GLCALL(proc) _AB_GLCALL(proc)
#define GLCall(proc) _AB_GLCALL(proc)
#else
#define AB_GLCALL(proc) proc
#define GLCall(proc) proc
#endif

union ABGLProcs;

namespace AB::GL {
	bool32 LoadFunctions();
	bool32 LoadExtensions();
	void InitAPI();
	ABGLProcs* GetFunctions();

}

#ifdef __cplusplus
extern "C" {
#endif
	/*
	** Copyright (c) 2013-2018 The Khronos Group Inc.
	**
	** Permission is hereby granted, free of charge, to any person obtaining a
	** copy of this software and/or associated documentation files (the
	** "Materials"), to deal in the Materials without restriction, including
	** without limitation the rights to use, copy, modify, merge, publish,
	** distribute, sublicense, and/or sell copies of the Materials, and to
	** permit persons to whom the Materials are furnished to do so, subject to
	** the following conditions:
	**
	** The above copyright notice and this permission notice shall be included
	** in all copies or substantial portions of the Materials.
	**
	** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
	** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
	** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
	*/
	/*
	** This header is generated from the Khronos OpenGL / OpenGL ES XML
	** API Registry. The current version of the Registry, generator scripts
	** used to make the header, and the header can be found at
	**   https://github.com/KhronosGroup/OpenGL-Registry
	*/

#if defined(AB_PLATFORM_WINDOWS) && !defined(APIENTRY)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
	// TODO: THIS Should not be here
#include <windows.h>
#endif
}

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
#define GLAPI extern
#endif

typedef void                 GLvoid;
typedef unsigned int         GLenum;
typedef khronos_float_t      GLfloat;
typedef int                  GLint;
typedef int                  GLsizei;
typedef unsigned int         GLbitfield;
typedef double               GLdouble;
typedef unsigned int         GLuint;
typedef unsigned char        GLboolean;
typedef khronos_uint8_t      GLubyte;
typedef khronos_float_t      GLclampf;
typedef double               GLclampd;
typedef char                 GLchar;
typedef khronos_int16_t      GLshort;
typedef khronos_int8_t       GLbyte;
typedef khronos_uint16_t     GLushort;
typedef khronos_ssize_t      GLsizeiptr;
typedef khronos_intptr_t     GLintptr;
typedef khronos_uint16_t     GLhalf;
typedef struct __GLsync*     GLsync;
typedef khronos_uint64_t     GLuint64;
typedef khronos_int64_t      GLint64;


#if defined(AB_PLATFORM_LINUX)
typedef void(*PROC)(void);
#endif

#define AB_GLFUNCPTR PROC

#define GL_DEPTH_BUFFER_BIT                              0x00000100
#define GL_STENCIL_BUFFER_BIT                            0x00000400
#define GL_COLOR_BUFFER_BIT                              0x00004000
#define GL_COLOR_CLEAR_VALUE                             0x0C22
#define GL_INVALID_VALUE                                 0x0501
#define GL_NO_ERROR                                      0
#define GL_INVALID_ENUM                                  0x0500
#define GL_INVALID_VALUE                                 0x0501
#define GL_INVALID_OPERATION                             0x0502
#define GL_OUT_OF_MEMORY                                 0x0505
#define GL_DEPTH_CLEAR_VALUE                             0x0B73
#define GL_COLOR_CLEAR_VALUE                             0x0C22
#define GL_STENCIL_CLEAR_VALUE                           0x0B91
#define GL_MAX_VIEWPORT_DIMS                             0x0D3A
#define GL_VIEWPORT                                      0x0BA2
#define GL_MAX_VIEWPORT_DIMS                             0x0D3A
#define GL_COMPUTE_SHADER                                0x91B9
#define GL_VERTEX_SHADER                                 0x8B31
#define GL_GEOMETRY_SHADER                               0x8DD9
#define GL_FRAGMENT_SHADER                               0x8B30
#define GL_SHADER_SOURCE_LENGTH                          0x8B88
#define GL_FALSE                                         0
#define GL_TRUE                                          1
#define GL_POINTS                                        0x0000
#define GL_LINES                                         0x0001
#define GL_LINE_LOOP                                     0x0002
#define GL_LINE_STRIP                                    0x0003
#define GL_TRIANGLES                                     0x0004
#define GL_TRIANGLE_STRIP                                0x0005
#define GL_TRIANGLE_FAN                                  0x0006
#define GL_QUADS                                         0x0007
#define GL_NEVER                                         0x0200
#define GL_LESS                                          0x0201
#define GL_EQUAL                                         0x0202
#define GL_LEQUAL                                        0x0203
#define GL_GREATER                                       0x0204
#define GL_NOTEQUAL                                      0x0205
#define GL_GEQUAL                                        0x0206
#define GL_ALWAYS                                        0x0207
#define GL_ZERO                                          0
#define GL_ONE                                           1
#define GL_SRC_COLOR                                     0x0300
#define GL_ONE_MINUS_SRC_COLOR                           0x0301
#define GL_SRC_ALPHA                                     0x0302
#define GL_ONE_MINUS_SRC_ALPHA                           0x0303
#define GL_DST_ALPHA                                     0x0304
#define GL_ONE_MINUS_DST_ALPHA                           0x0305
#define GL_DST_COLOR                                     0x0306
#define GL_ONE_MINUS_DST_COLOR                           0x0307
#define GL_SRC_ALPHA_SATURATE                            0x0308
#define GL_NONE                                          0
#define GL_FRONT_LEFT                                    0x0400
#define GL_FRONT_RIGHT                                   0x0401
#define GL_BACK_LEFT                                     0x0402
#define GL_BACK_RIGHT                                    0x0403
#define GL_FRONT                                         0x0404
#define GL_BACK                                          0x0405
#define GL_LEFT                                          0x0406
#define GL_RIGHT                                         0x0407
#define GL_FRONT_AND_BACK                                0x0408
#define GL_CW                                            0x0900
#define GL_CCW                                           0x0901
#define GL_POINT_SIZE                                    0x0B11
#define GL_POINT_SIZE_RANGE                              0x0B12
#define GL_POINT_SIZE_GRANULARITY                        0x0B13
#define GL_LINE_SMOOTH                                   0x0B20
#define GL_LINE_WIDTH                                    0x0B21
#define GL_LINE_WIDTH_RANGE                              0x0B22
#define GL_LINE_WIDTH_GRANULARITY                        0x0B23
#define GL_POLYGON_MODE                                  0x0B40
#define GL_POLYGON_SMOOTH                                0x0B41
#define GL_CULL_FACE                                     0x0B44
#define GL_CULL_FACE_MODE                                0x0B45
#define GL_FRONT_FACE                                    0x0B46
#define GL_DEPTH_RANGE                                   0x0B70
#define GL_DEPTH_TEST                                    0x0B71
#define GL_DEPTH_WRITEMASK                               0x0B72
#define GL_DEPTH_CLEAR_VALUE                             0x0B73
#define GL_DEPTH_FUNC                                    0x0B74
#define GL_STENCIL_TEST                                  0x0B90
#define GL_STENCIL_CLEAR_VALUE                           0x0B91
#define GL_STENCIL_FUNC                                  0x0B92
#define GL_STENCIL_VALUE_MASK                            0x0B93
#define GL_STENCIL_FAIL                                  0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL                       0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS                       0x0B96
#define GL_STENCIL_REF                                   0x0B97
#define GL_STENCIL_WRITEMASK                             0x0B98
#define GL_VIEWPORT                                      0x0BA2
#define GL_DITHER                                        0x0BD0
#define GL_BLEND_DST                                     0x0BE0
#define GL_BLEND_SRC                                     0x0BE1
#define GL_BLEND                                         0x0BE2
#define GL_LOGIC_OP_MODE                                 0x0BF0
#define GL_DRAW_BUFFER                                   0x0C01
#define GL_READ_BUFFER                                   0x0C02
#define GL_SCISSOR_BOX                                   0x0C10
#define GL_SCISSOR_TEST                                  0x0C11
#define GL_COLOR_CLEAR_VALUE                             0x0C22
#define GL_COLOR_WRITEMASK                               0x0C23
#define GL_DOUBLEBUFFER                                  0x0C32
#define GL_STEREO                                        0x0C33
#define GL_LINE_SMOOTH_HINT                              0x0C52
#define GL_POLYGON_SMOOTH_HINT                           0x0C53
#define GL_UNPACK_SWAP_BYTES                             0x0CF0
#define GL_UNPACK_LSB_FIRST                              0x0CF1
#define GL_UNPACK_ROW_LENGTH                             0x0CF2
#define GL_UNPACK_SKIP_ROWS                              0x0CF3
#define GL_UNPACK_SKIP_PIXELS                            0x0CF4
#define GL_UNPACK_ALIGNMENT                              0x0CF5
#define GL_PACK_SWAP_BYTES                               0x0D00
#define GL_PACK_LSB_FIRST                                0x0D01
#define GL_PACK_ROW_LENGTH                               0x0D02
#define GL_PACK_SKIP_ROWS                                0x0D03
#define GL_PACK_SKIP_PIXELS                              0x0D04
#define GL_PACK_ALIGNMENT                                0x0D05
#define GL_MAX_TEXTURE_SIZE                              0x0D33
#define GL_MAX_VIEWPORT_DIMS                             0x0D3A
#define GL_SUBPIXEL_BITS                                 0x0D50
#define GL_TEXTURE_1D                                    0x0DE0
#define GL_TEXTURE_2D                                    0x0DE1
#define GL_TEXTURE_WIDTH                                 0x1000
#define GL_TEXTURE_HEIGHT                                0x1001
#define GL_TEXTURE_BORDER_COLOR                          0x1004
#define GL_DONT_CARE                                     0x1100
#define GL_FASTEST                                       0x1101
#define GL_NICEST                                        0x1102
#define GL_BYTE                                          0x1400
#define GL_UNSIGNED_BYTE                                 0x1401
#define GL_SHORT                                         0x1402
#define GL_UNSIGNED_SHORT                                0x1403
#define GL_INT                                           0x1404
#define GL_UNSIGNED_INT                                  0x1405
#define GL_FLOAT                                         0x1406
#define GL_STACK_OVERFLOW                                0x0503
#define GL_STACK_UNDERFLOW                               0x0504
#define GL_CLEAR                                         0x1500
#define GL_AND                                           0x1501
#define GL_AND_REVERSE                                   0x1502
#define GL_COPY                                          0x1503
#define GL_AND_INVERTED                                  0x1504
#define GL_NOOP                                          0x1505
#define GL_XOR                                           0x1506
#define GL_OR                                            0x1507
#define GL_NOR                                           0x1508
#define GL_EQUIV                                         0x1509
#define GL_INVERT                                        0x150A
#define GL_OR_REVERSE                                    0x150B
#define GL_COPY_INVERTED                                 0x150C
#define GL_OR_INVERTED                                   0x150D
#define GL_NAND                                          0x150E
#define GL_SET                                           0x150F
#define GL_TEXTURE                                       0x1702
#define GL_COLOR                                         0x1800
#define GL_DEPTH                                         0x1801
#define GL_STENCIL                                       0x1802
#define GL_STENCIL_INDEX                                 0x1901
#define GL_DEPTH_COMPONENT                               0x1902
#define GL_RED                                           0x1903
#define GL_GREEN                                         0x1904
#define GL_BLUE                                          0x1905
#define GL_ALPHA                                         0x1906
#define GL_RGB                                           0x1907
#define GL_RGBA                                          0x1908
#define GL_POINT                                         0x1B00
#define GL_LINE                                          0x1B01
#define GL_FILL                                          0x1B02
#define GL_KEEP                                          0x1E00
#define GL_REPLACE                                       0x1E01
#define GL_INCR                                          0x1E02
#define GL_DECR                                          0x1E03
#define GL_VENDOR                                        0x1F00
#define GL_RENDERER                                      0x1F01
#define GL_VERSION                                       0x1F02
#define GL_EXTENSIONS                                    0x1F03
#define GL_NEAREST                                       0x2600
#define GL_LINEAR                                        0x2601
#define GL_NEAREST_MIPMAP_NEAREST                        0x2700
#define GL_LINEAR_MIPMAP_NEAREST                         0x2701
#define GL_NEAREST_MIPMAP_LINEAR                         0x2702
#define GL_LINEAR_MIPMAP_LINEAR                          0x2703
#define GL_TEXTURE_MAG_FILTER                            0x2800
#define GL_TEXTURE_MIN_FILTER                            0x2801
#define GL_TEXTURE_WRAP_S                                0x2802
#define GL_TEXTURE_WRAP_T                                0x2803
#define GL_REPEAT                                        0x2901
#define GL_COLOR_LOGIC_OP                                0x0BF2
#define GL_POLYGON_OFFSET_UNITS                          0x2A00
#define GL_POLYGON_OFFSET_POINT                          0x2A01
#define GL_POLYGON_OFFSET_LINE                           0x2A02
#define GL_POLYGON_OFFSET_FILL                           0x8037
#define GL_POLYGON_OFFSET_FACTOR                         0x8038
#define GL_TEXTURE_BINDING_1D                            0x8068
#define GL_TEXTURE_BINDING_2D                            0x8069
#define GL_TEXTURE_INTERNAL_FORMAT                       0x1003
#define GL_TEXTURE_RED_SIZE                              0x805C
#define GL_TEXTURE_GREEN_SIZE                            0x805D
#define GL_TEXTURE_BLUE_SIZE                             0x805E
#define GL_TEXTURE_ALPHA_SIZE                            0x805F
#define GL_DOUBLE                                        0x140A
#define GL_PROXY_TEXTURE_1D                              0x8063
#define GL_PROXY_TEXTURE_2D                              0x8064
#define GL_R3_G3_B2                                      0x2A10
#define GL_RGB4                                          0x804F
#define GL_RGB5                                          0x8050
#define GL_RGB8                                          0x8051
#define GL_RGB10                                         0x8052
#define GL_RGB12                                         0x8053
#define GL_RGB16                                         0x8054
#define GL_RGBA2                                         0x8055
#define GL_RGBA4                                         0x8056
#define GL_RGB5_A1                                       0x8057
#define GL_RGBA8                                         0x8058
#define GL_RGB10_A2                                      0x8059
#define GL_RGBA12                                        0x805A
#define GL_RGBA16                                        0x805B
#define GL_VERTEX_ARRAY                                  0x8074
#define GL_UNSIGNED_BYTE_3_3_2                           0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4                        0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1                        0x8034
#define GL_UNSIGNED_INT_8_8_8_8                          0x8035
#define GL_UNSIGNED_INT_10_10_10_2                       0x8036
#define GL_TEXTURE_BINDING_3D                            0x806A
#define GL_PACK_SKIP_IMAGES                              0x806B
#define GL_PACK_IMAGE_HEIGHT                             0x806C
#define GL_UNPACK_SKIP_IMAGES                            0x806D
#define GL_UNPACK_IMAGE_HEIGHT                           0x806E
#define GL_TEXTURE_3D                                    0x806F
#define GL_PROXY_TEXTURE_3D                              0x8070
#define GL_TEXTURE_DEPTH                                 0x8071
#define GL_TEXTURE_WRAP_R                                0x8072
#define GL_MAX_3D_TEXTURE_SIZE                           0x8073
#define GL_UNSIGNED_BYTE_2_3_3_REV                       0x8362
#define GL_UNSIGNED_SHORT_5_6_5                          0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV                      0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV                    0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV                    0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV                      0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV                   0x8368
#define GL_BGR                                           0x80E0
#define GL_BGRA                                          0x80E1
#define GL_MAX_ELEMENTS_VERTICES                         0x80E8
#define GL_MAX_ELEMENTS_INDICES                          0x80E9
#define GL_CLAMP_TO_EDGE                                 0x812F
#define GL_TEXTURE_MIN_LOD                               0x813A
#define GL_TEXTURE_MAX_LOD                               0x813B
#define GL_TEXTURE_BASE_LEVEL                            0x813C
#define GL_TEXTURE_MAX_LEVEL                             0x813D
#define GL_SMOOTH_POINT_SIZE_RANGE                       0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY                 0x0B13
#define GL_SMOOTH_LINE_WIDTH_RANGE                       0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY                 0x0B23
#define GL_ALIASED_LINE_WIDTH_RANGE                      0x846E
#define GL_VERSION_1_3 1
#define GL_TEXTURE0                                      0x84C0
#define GL_TEXTURE1                                      0x84C1
#define GL_TEXTURE2                                      0x84C2
#define GL_TEXTURE3                                      0x84C3
#define GL_TEXTURE4                                      0x84C4
#define GL_TEXTURE5                                      0x84C5
#define GL_TEXTURE6                                      0x84C6
#define GL_TEXTURE7                                      0x84C7
#define GL_TEXTURE8                                      0x84C8
#define GL_TEXTURE9                                      0x84C9
#define GL_TEXTURE10                                     0x84CA
#define GL_TEXTURE11                                     0x84CB
#define GL_TEXTURE12                                     0x84CC
#define GL_TEXTURE13                                     0x84CD
#define GL_TEXTURE14                                     0x84CE
#define GL_TEXTURE15                                     0x84CF
#define GL_TEXTURE16                                     0x84D0
#define GL_TEXTURE17                                     0x84D1
#define GL_TEXTURE18                                     0x84D2
#define GL_TEXTURE19                                     0x84D3
#define GL_TEXTURE20                                     0x84D4
#define GL_TEXTURE21                                     0x84D5
#define GL_TEXTURE22                                     0x84D6
#define GL_TEXTURE23                                     0x84D7
#define GL_TEXTURE24                                     0x84D8
#define GL_TEXTURE25                                     0x84D9
#define GL_TEXTURE26                                     0x84DA
#define GL_TEXTURE27                                     0x84DB
#define GL_TEXTURE28                                     0x84DC
#define GL_TEXTURE29                                     0x84DD
#define GL_TEXTURE30                                     0x84DE
#define GL_TEXTURE31                                     0x84DF
#define GL_ACTIVE_TEXTURE                                0x84E0
#define GL_MULTISAMPLE                                   0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE                      0x809E
#define GL_SAMPLE_ALPHA_TO_ONE                           0x809F
#define GL_SAMPLE_COVERAGE                               0x80A0
#define GL_SAMPLE_BUFFERS                                0x80A8
#define GL_SAMPLES                                       0x80A9
#define GL_SAMPLE_COVERAGE_VALUE                         0x80AA
#define GL_SAMPLE_COVERAGE_INVERT                        0x80AB
#define GL_TEXTURE_CUBE_MAP                              0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP                      0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X                   0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X                   0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y                   0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y                   0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z                   0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z                   0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP                        0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE                     0x851C
#define GL_COMPRESSED_RGB                                0x84ED
#define GL_COMPRESSED_RGBA                               0x84EE
#define GL_TEXTURE_COMPRESSION_HINT                      0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE                 0x86A0
#define GL_TEXTURE_COMPRESSED                            0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS                0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS                    0x86A3
#define GL_CLAMP_TO_BORDER                               0x812D
#define GL_BLEND_DST_RGB                                 0x80C8
#define GL_BLEND_SRC_RGB                                 0x80C9
#define GL_BLEND_DST_ALPHA                               0x80CA
#define GL_BLEND_SRC_ALPHA                               0x80CB
#define GL_POINT_FADE_THRESHOLD_SIZE                     0x8128
#define GL_DEPTH_COMPONENT16                             0x81A5
#define GL_DEPTH_COMPONENT24                             0x81A6
#define GL_DEPTH_COMPONENT32                             0x81A7
#define GL_MIRRORED_REPEAT                               0x8370
#define GL_MAX_TEXTURE_LOD_BIAS                          0x84FD
#define GL_TEXTURE_LOD_BIAS                              0x8501
#define GL_INCR_WRAP                                     0x8507
#define GL_DECR_WRAP                                     0x8508
#define GL_TEXTURE_DEPTH_SIZE                            0x884A
#define GL_TEXTURE_COMPARE_MODE                          0x884C
#define GL_TEXTURE_COMPARE_FUNC                          0x884D
#define GL_BLEND_COLOR                                   0x8005
#define GL_BLEND_EQUATION                                0x8009
#define GL_CONSTANT_COLOR                                0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR                      0x8002
#define GL_CONSTANT_ALPHA                                0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA                      0x8004
#define GL_FUNC_ADD                                      0x8006
#define GL_FUNC_REVERSE_SUBTRACT                         0x800B
#define GL_FUNC_SUBTRACT                                 0x800A
#define GL_MIN                                           0x8007
#define GL_MAX                                           0x8008
#define GL_BUFFER_SIZE                                   0x8764
#define GL_BUFFER_USAGE                                  0x8765
#define GL_QUERY_COUNTER_BITS                            0x8864
#define GL_CURRENT_QUERY                                 0x8865
#define GL_QUERY_RESULT                                  0x8866
#define GL_QUERY_RESULT_AVAILABLE                        0x8867
#define GL_ARRAY_BUFFER                                  0x8892
#define GL_ELEMENT_ARRAY_BUFFER                          0x8893
#define GL_ARRAY_BUFFER_BINDING                          0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING                  0x8895
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING            0x889F
#define GL_READ_ONLY                                     0x88B8
#define GL_WRITE_ONLY                                    0x88B9
#define GL_READ_WRITE                                    0x88BA
#define GL_BUFFER_ACCESS                                 0x88BB
#define GL_BUFFER_MAPPED                                 0x88BC
#define GL_BUFFER_MAP_POINTER                            0x88BD
#define GL_STREAM_DRAW                                   0x88E0
#define GL_STREAM_READ                                   0x88E1
#define GL_STREAM_COPY                                   0x88E2
#define GL_STATIC_DRAW                                   0x88E4
#define GL_STATIC_READ                                   0x88E5
#define GL_STATIC_COPY                                   0x88E6
#define GL_DYNAMIC_DRAW                                  0x88E8
#define GL_DYNAMIC_READ                                  0x88E9
#define GL_DYNAMIC_COPY                                  0x88EA
#define GL_SAMPLES_PASSED                                0x8914
#define GL_SRC1_ALPHA                                    0x8589
#define GL_BLEND_EQUATION_RGB                            0x8009
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED                   0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE                      0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE                    0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE                      0x8625
#define GL_CURRENT_VERTEX_ATTRIB                         0x8626
#define GL_VERTEX_PROGRAM_POINT_SIZE                     0x8642
#define GL_VERTEX_ATTRIB_ARRAY_POINTER                   0x8645
#define GL_STENCIL_BACK_FUNC                             0x8800
#define GL_STENCIL_BACK_FAIL                             0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL                  0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS                  0x8803
#define GL_MAX_DRAW_BUFFERS                              0x8824
#define GL_DRAW_BUFFER0                                  0x8825
#define GL_DRAW_BUFFER1                                  0x8826
#define GL_DRAW_BUFFER2                                  0x8827
#define GL_DRAW_BUFFER3                                  0x8828
#define GL_DRAW_BUFFER4                                  0x8829
#define GL_DRAW_BUFFER5                                  0x882A
#define GL_DRAW_BUFFER6                                  0x882B
#define GL_DRAW_BUFFER7                                  0x882C
#define GL_DRAW_BUFFER8                                  0x882D
#define GL_DRAW_BUFFER9                                  0x882E
#define GL_DRAW_BUFFER10                                 0x882F
#define GL_DRAW_BUFFER11                                 0x8830
#define GL_DRAW_BUFFER12                                 0x8831
#define GL_DRAW_BUFFER13                                 0x8832
#define GL_DRAW_BUFFER14                                 0x8833
#define GL_DRAW_BUFFER15                                 0x8834
#define GL_BLEND_EQUATION_ALPHA                          0x883D
#define GL_MAX_VERTEX_ATTRIBS                            0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED                0x886A
#define GL_MAX_TEXTURE_IMAGE_UNITS                       0x8872
#define GL_FRAGMENT_SHADER                               0x8B30
#define GL_VERTEX_SHADER                                 0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS               0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS                 0x8B4A
#define GL_MAX_VARYING_FLOATS                            0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS                0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS              0x8B4D
#define GL_SHADER_TYPE                                   0x8B4F
#define GL_FLOAT_VEC2                                    0x8B50
#define GL_FLOAT_VEC3                                    0x8B51
#define GL_FLOAT_VEC4                                    0x8B52
#define GL_INT_VEC2                                      0x8B53
#define GL_INT_VEC3                                      0x8B54
#define GL_INT_VEC4                                      0x8B55
#define GL_BOOL                                          0x8B56
#define GL_BOOL_VEC2                                     0x8B57
#define GL_BOOL_VEC3                                     0x8B58
#define GL_BOOL_VEC4                                     0x8B59
#define GL_FLOAT_MAT2                                    0x8B5A
#define GL_FLOAT_MAT3                                    0x8B5B
#define GL_FLOAT_MAT4                                    0x8B5C
#define GL_SAMPLER_1D                                    0x8B5D
#define GL_SAMPLER_2D                                    0x8B5E
#define GL_SAMPLER_3D                                    0x8B5F
#define GL_SAMPLER_CUBE                                  0x8B60
#define GL_SAMPLER_1D_SHADOW                             0x8B61
#define GL_SAMPLER_2D_SHADOW                             0x8B62
#define GL_DELETE_STATUS                                 0x8B80
#define GL_COMPILE_STATUS                                0x8B81
#define GL_LINK_STATUS                                   0x8B82
#define GL_VALIDATE_STATUS                               0x8B83
#define GL_INFO_LOG_LENGTH                               0x8B84
#define GL_ATTACHED_SHADERS                              0x8B85
#define GL_ACTIVE_UNIFORMS                               0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH                     0x8B87
#define GL_SHADER_SOURCE_LENGTH                          0x8B88
#define GL_ACTIVE_ATTRIBUTES                             0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH                   0x8B8A
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT               0x8B8B
#define GL_SHADING_LANGUAGE_VERSION                      0x8B8C
#define GL_CURRENT_PROGRAM                               0x8B8D
#define GL_POINT_SPRITE_COORD_ORIGIN                     0x8CA0
#define GL_LOWER_LEFT                                    0x8CA1
#define GL_UPPER_LEFT                                    0x8CA2
#define GL_STENCIL_BACK_REF                              0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK                       0x8CA4
#define GL_STENCIL_BACK_WRITEMASK                        0x8CA5
#define GL_PIXEL_PACK_BUFFER                             0x88EB
#define GL_PIXEL_UNPACK_BUFFER                           0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING                     0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING                   0x88EF
#define GL_FLOAT_MAT2x3                                  0x8B65
#define GL_FLOAT_MAT2x4                                  0x8B66
#define GL_FLOAT_MAT3x2                                  0x8B67
#define GL_FLOAT_MAT3x4                                  0x8B68
#define GL_FLOAT_MAT4x2                                  0x8B69
#define GL_FLOAT_MAT4x3                                  0x8B6A
#define GL_SRGB                                          0x8C40
#define GL_SRGB8                                         0x8C41
#define GL_SRGB_ALPHA                                    0x8C42
#define GL_SRGB8_ALPHA8                                  0x8C43
#define GL_COMPRESSED_SRGB                               0x8C48
#define GL_COMPRESSED_SRGB_ALPHA                         0x8C49
#define GL_COMPARE_REF_TO_TEXTURE                        0x884E
#define GL_CLIP_DISTANCE0                                0x3000
#define GL_CLIP_DISTANCE1                                0x3001
#define GL_CLIP_DISTANCE2                                0x3002
#define GL_CLIP_DISTANCE3                                0x3003
#define GL_CLIP_DISTANCE4                                0x3004
#define GL_CLIP_DISTANCE5                                0x3005
#define GL_CLIP_DISTANCE6                                0x3006
#define GL_CLIP_DISTANCE7                                0x3007
#define GL_MAX_CLIP_DISTANCES                            0x0D32
#define GL_MAJOR_VERSION                                 0x821B
#define GL_MINOR_VERSION                                 0x821C
#define GL_NUM_EXTENSIONS                                0x821D
#define GL_CONTEXT_FLAGS                                 0x821E
#define GL_COMPRESSED_RED                                0x8225
#define GL_COMPRESSED_RG                                 0x8226
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT           0x00000001
#define GL_RGBA32F                                       0x8814
#define GL_RGB32F                                        0x8815
#define GL_RGBA16F                                       0x881A
#define GL_RGB16F                                        0x881B
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER                   0x88FD
#define GL_MAX_ARRAY_TEXTURE_LAYERS                      0x88FF
#define GL_MIN_PROGRAM_TEXEL_OFFSET                      0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET                      0x8905
#define GL_CLAMP_READ_COLOR                              0x891C
#define GL_FIXED_ONLY                                    0x891D
#define GL_MAX_VARYING_COMPONENTS                        0x8B4B
#define GL_TEXTURE_1D_ARRAY                              0x8C18
#define GL_PROXY_TEXTURE_1D_ARRAY                        0x8C19
#define GL_TEXTURE_2D_ARRAY                              0x8C1A
#define GL_PROXY_TEXTURE_2D_ARRAY                        0x8C1B
#define GL_TEXTURE_BINDING_1D_ARRAY                      0x8C1C
#define GL_TEXTURE_BINDING_2D_ARRAY                      0x8C1D
#define GL_R11F_G11F_B10F                                0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV                  0x8C3B
#define GL_RGB9_E5                                       0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV                      0x8C3E
#define GL_TEXTURE_SHARED_SIZE                           0x8C3F
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH         0x8C76
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE                0x8C7F
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS    0x8C80
#define GL_TRANSFORM_FEEDBACK_VARYINGS                   0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_START               0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE                0x8C85
#define GL_PRIMITIVES_GENERATED                          0x8C87
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN         0x8C88
#define GL_RASTERIZER_DISCARD                            0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS       0x8C8B
#define GL_INTERLEAVED_ATTRIBS                           0x8C8C
#define GL_SEPARATE_ATTRIBS                              0x8C8D
#define GL_TRANSFORM_FEEDBACK_BUFFER                     0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING             0x8C8F
#define GL_RGBA32UI                                      0x8D70
#define GL_RGB32UI                                       0x8D71
#define GL_RGBA16UI                                      0x8D76
#define GL_RGB16UI                                       0x8D77
#define GL_RGBA8UI                                       0x8D7C
#define GL_RGB8UI                                        0x8D7D
#define GL_RGBA32I                                       0x8D82
#define GL_RGB32I                                        0x8D83
#define GL_RGBA16I                                       0x8D88
#define GL_RGB16I                                        0x8D89
#define GL_RGBA8I                                        0x8D8E
#define GL_RGB8I                                         0x8D8F
#define GL_RED_INTEGER                                   0x8D94
#define GL_GREEN_INTEGER                                 0x8D95
#define GL_BLUE_INTEGER                                  0x8D96
#define GL_RGB_INTEGER                                   0x8D98
#define GL_RGBA_INTEGER                                  0x8D99
#define GL_BGR_INTEGER                                   0x8D9A
#define GL_BGRA_INTEGER                                  0x8D9B
#define GL_SAMPLER_1D_ARRAY                              0x8DC0
#define GL_SAMPLER_2D_ARRAY                              0x8DC1
#define GL_SAMPLER_1D_ARRAY_SHADOW                       0x8DC3
#define GL_SAMPLER_2D_ARRAY_SHADOW                       0x8DC4
#define GL_SAMPLER_CUBE_SHADOW                           0x8DC5
#define GL_UNSIGNED_INT_VEC2                             0x8DC6
#define GL_UNSIGNED_INT_VEC3                             0x8DC7
#define GL_UNSIGNED_INT_VEC4                             0x8DC8
#define GL_INT_SAMPLER_1D                                0x8DC9
#define GL_INT_SAMPLER_2D                                0x8DCA
#define GL_INT_SAMPLER_3D                                0x8DCB
#define GL_INT_SAMPLER_CUBE                              0x8DCC
#define GL_INT_SAMPLER_1D_ARRAY                          0x8DCE
#define GL_INT_SAMPLER_2D_ARRAY                          0x8DCF
#define GL_UNSIGNED_INT_SAMPLER_1D                       0x8DD1
#define GL_UNSIGNED_INT_SAMPLER_2D                       0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D                       0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE                     0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY                 0x8DD6
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY                 0x8DD7
#define GL_QUERY_WAIT                                    0x8E13
#define GL_QUERY_NO_WAIT                                 0x8E14
#define GL_QUERY_BY_REGION_WAIT                          0x8E15
#define GL_QUERY_BY_REGION_NO_WAIT                       0x8E16
#define GL_BUFFER_ACCESS_FLAGS                           0x911F
#define GL_BUFFER_MAP_LENGTH                             0x9120
#define GL_BUFFER_MAP_OFFSET                             0x9121
#define GL_DEPTH_COMPONENT32F                            0x8CAC
#define GL_DEPTH32F_STENCIL8                             0x8CAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV                0x8DAD
#define GL_INVALID_FRAMEBUFFER_OPERATION                 0x0506
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING         0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE         0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE               0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE             0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE              0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE             0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE             0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE           0x8217
#define GL_FRAMEBUFFER_DEFAULT                           0x8218
#define GL_FRAMEBUFFER_UNDEFINED                         0x8219
#define GL_DEPTH_STENCIL_ATTACHMENT                      0x821A
#define GL_MAX_RENDERBUFFER_SIZE                         0x84E8
#define GL_DEPTH_STENCIL                                 0x84F9
#define GL_UNSIGNED_INT_24_8                             0x84FA
#define GL_DEPTH24_STENCIL8                              0x88F0
#define GL_TEXTURE_STENCIL_SIZE                          0x88F1
#define GL_TEXTURE_RED_TYPE                              0x8C10
#define GL_TEXTURE_GREEN_TYPE                            0x8C11
#define GL_TEXTURE_BLUE_TYPE                             0x8C12
#define GL_TEXTURE_ALPHA_TYPE                            0x8C13
#define GL_TEXTURE_DEPTH_TYPE                            0x8C16
#define GL_UNSIGNED_NORMALIZED                           0x8C17
#define GL_FRAMEBUFFER_BINDING                           0x8CA6
#define GL_DRAW_FRAMEBUFFER_BINDING                      0x8CA6
#define GL_RENDERBUFFER_BINDING                          0x8CA7
#define GL_READ_FRAMEBUFFER                              0x8CA8
#define GL_DRAW_FRAMEBUFFER                              0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING                      0x8CAA
#define GL_RENDERBUFFER_SAMPLES                          0x8CAB
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE            0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME            0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL          0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE  0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER          0x8CD4
#define GL_FRAMEBUFFER_COMPLETE                          0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT             0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT     0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER            0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER            0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED                       0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS                         0x8CDF
#define GL_COLOR_ATTACHMENT0                             0x8CE0
#define GL_COLOR_ATTACHMENT1                             0x8CE1
#define GL_COLOR_ATTACHMENT2                             0x8CE2
#define GL_COLOR_ATTACHMENT3                             0x8CE3
#define GL_COLOR_ATTACHMENT4                             0x8CE4
#define GL_COLOR_ATTACHMENT5                             0x8CE5
#define GL_COLOR_ATTACHMENT6                             0x8CE6
#define GL_COLOR_ATTACHMENT7                             0x8CE7
#define GL_COLOR_ATTACHMENT8                             0x8CE8
#define GL_COLOR_ATTACHMENT9                             0x8CE9
#define GL_COLOR_ATTACHMENT10                            0x8CEA
#define GL_COLOR_ATTACHMENT11                            0x8CEB
#define GL_COLOR_ATTACHMENT12                            0x8CEC
#define GL_COLOR_ATTACHMENT13                            0x8CED
#define GL_COLOR_ATTACHMENT14                            0x8CEE
#define GL_COLOR_ATTACHMENT15                            0x8CEF
#define GL_COLOR_ATTACHMENT16                            0x8CF0
#define GL_COLOR_ATTACHMENT17                            0x8CF1
#define GL_COLOR_ATTACHMENT18                            0x8CF2
#define GL_COLOR_ATTACHMENT19                            0x8CF3
#define GL_COLOR_ATTACHMENT20                            0x8CF4
#define GL_COLOR_ATTACHMENT21                            0x8CF5
#define GL_COLOR_ATTACHMENT22                            0x8CF6
#define GL_COLOR_ATTACHMENT23                            0x8CF7
#define GL_COLOR_ATTACHMENT24                            0x8CF8
#define GL_COLOR_ATTACHMENT25                            0x8CF9
#define GL_COLOR_ATTACHMENT26                            0x8CFA
#define GL_COLOR_ATTACHMENT27                            0x8CFB
#define GL_COLOR_ATTACHMENT28                            0x8CFC
#define GL_COLOR_ATTACHMENT29                            0x8CFD
#define GL_COLOR_ATTACHMENT30                            0x8CFE
#define GL_COLOR_ATTACHMENT31                            0x8CFF
#define GL_DEPTH_ATTACHMENT                              0x8D00
#define GL_STENCIL_ATTACHMENT                            0x8D20
#define GL_FRAMEBUFFER                                   0x8D40
#define GL_RENDERBUFFER                                  0x8D41
#define GL_RENDERBUFFER_WIDTH                            0x8D42
#define GL_RENDERBUFFER_HEIGHT                           0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT                  0x8D44
#define GL_STENCIL_INDEX1                                0x8D46
#define GL_STENCIL_INDEX4                                0x8D47
#define GL_STENCIL_INDEX8                                0x8D48
#define GL_STENCIL_INDEX16                               0x8D49
#define GL_RENDERBUFFER_RED_SIZE                         0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE                       0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE                        0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE                       0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE                       0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE                     0x8D55
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE            0x8D56
#define GL_MAX_SAMPLES                                   0x8D57
#define GL_FRAMEBUFFER_SRGB                              0x8DB9
#define GL_HALF_FLOAT                                    0x140B
#define GL_MAP_READ_BIT                                  0x0001
#define GL_MAP_WRITE_BIT                                 0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT                      0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT                     0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT                        0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT                        0x0020
#define GL_COMPRESSED_RED_RGTC1                          0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1                   0x8DBC
#define GL_COMPRESSED_RG_RGTC2                           0x8DBD
#define GL_COMPRESSED_SIGNED_RG_RGTC2                    0x8DBE
#define GL_RG                                            0x8227
#define GL_RG_INTEGER                                    0x8228
#define GL_R8                                            0x8229
#define GL_R16                                           0x822A
#define GL_RG8                                           0x822B
#define GL_RG16                                          0x822C
#define GL_R16F                                          0x822D
#define GL_R32F                                          0x822E
#define GL_RG16F                                         0x822F
#define GL_RG32F                                         0x8230
#define GL_R8I                                           0x8231
#define GL_R8UI                                          0x8232
#define GL_R16I                                          0x8233
#define GL_R16UI                                         0x8234
#define GL_R32I                                          0x8235
#define GL_R32UI                                         0x8236
#define GL_RG8I                                          0x8237
#define GL_RG8UI                                         0x8238
#define GL_RG16I                                         0x8239
#define GL_RG16UI                                        0x823A
#define GL_RG32I                                         0x823B
#define GL_RG32UI                                        0x823C
#define GL_VERTEX_ARRAY_BINDING                          0x85B5
#define GL_SAMPLER_2D_RECT                               0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW                        0x8B64
#define GL_SAMPLER_BUFFER                                0x8DC2
#define GL_INT_SAMPLER_2D_RECT                           0x8DCD
#define GL_INT_SAMPLER_BUFFER                            0x8DD0
#define GL_UNSIGNED_INT_SAMPLER_2D_RECT                  0x8DD5
#define GL_UNSIGNED_INT_SAMPLER_BUFFER                   0x8DD8
#define GL_TEXTURE_BUFFER                                0x8C2A
#define GL_MAX_TEXTURE_BUFFER_SIZE                       0x8C2B
#define GL_TEXTURE_BINDING_BUFFER                        0x8C2C
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING             0x8C2D
#define GL_TEXTURE_RECTANGLE                             0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE                     0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE                       0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE                    0x84F8
#define GL_R8_SNORM                                      0x8F94
#define GL_RG8_SNORM                                     0x8F95
#define GL_RGB8_SNORM                                    0x8F96
#define GL_RGBA8_SNORM                                   0x8F97
#define GL_R16_SNORM                                     0x8F98
#define GL_RG16_SNORM                                    0x8F99
#define GL_RGB16_SNORM                                   0x8F9A
#define GL_RGBA16_SNORM                                  0x8F9B
#define GL_SIGNED_NORMALIZED                             0x8F9C
#define GL_PRIMITIVE_RESTART                             0x8F9D
#define GL_PRIMITIVE_RESTART_INDEX                       0x8F9E
#define GL_COPY_READ_BUFFER                              0x8F36
#define GL_COPY_WRITE_BUFFER                             0x8F37
#define GL_UNIFORM_BUFFER                                0x8A11
#define GL_UNIFORM_BUFFER_BINDING                        0x8A28
#define GL_UNIFORM_BUFFER_START                          0x8A29
#define GL_UNIFORM_BUFFER_SIZE                           0x8A2A
#define GL_MAX_VERTEX_UNIFORM_BLOCKS                     0x8A2B
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS                   0x8A2C
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS                   0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS                   0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS                   0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE                        0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS        0x8A31
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS      0x8A32
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS      0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT               0x8A34
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH          0x8A35
#define GL_ACTIVE_UNIFORM_BLOCKS                         0x8A36
#define GL_UNIFORM_TYPE                                  0x8A37
#define GL_UNIFORM_SIZE                                  0x8A38
#define GL_UNIFORM_NAME_LENGTH                           0x8A39
#define GL_UNIFORM_BLOCK_INDEX                           0x8A3A
#define GL_UNIFORM_OFFSET                                0x8A3B
#define GL_UNIFORM_ARRAY_STRIDE                          0x8A3C
#define GL_UNIFORM_MATRIX_STRIDE                         0x8A3D
#define GL_UNIFORM_IS_ROW_MAJOR                          0x8A3E
#define GL_UNIFORM_BLOCK_BINDING                         0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE                       0x8A40
#define GL_UNIFORM_BLOCK_NAME_LENGTH                     0x8A41
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS                 0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES          0x8A43
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER     0x8A44
#define GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER   0x8A45
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER   0x8A46
#define GL_INVALID_INDEX                                 0xFFFFFFFFu
#define GL_CONTEXT_CORE_PROFILE_BIT                      0x00000001
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT             0x00000002
#define GL_LINES_ADJACENCY                               0x000A
#define GL_LINE_STRIP_ADJACENCY                          0x000B
#define GL_TRIANGLES_ADJACENCY                           0x000C
#define GL_TRIANGLE_STRIP_ADJACENCY                      0x000D
#define GL_PROGRAM_POINT_SIZE                            0x8642
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS              0x8C29
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED                0x8DA7
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS          0x8DA8
#define GL_GEOMETRY_SHADER                               0x8DD9
#define GL_GEOMETRY_VERTICES_OUT                         0x8916
#define GL_GEOMETRY_INPUT_TYPE                           0x8917
#define GL_GEOMETRY_OUTPUT_TYPE                          0x8918
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS               0x8DDF
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES                  0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS          0x8DE1
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS                  0x9122
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS                 0x9123
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS                0x9124
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS                 0x9125
#define GL_CONTEXT_PROFILE_MASK                          0x9126
#define GL_DEPTH_CLAMP                                   0x864F
#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION      0x8E4C
#define GL_FIRST_VERTEX_CONVENTION                       0x8E4D
#define GL_LAST_VERTEX_CONVENTION                        0x8E4E
#define GL_PROVOKING_VERTEX                              0x8E4F
#define GL_TEXTURE_CUBE_MAP_SEAMLESS                     0x884F
#define GL_MAX_SERVER_WAIT_TIMEOUT                       0x9111
#define GL_OBJECT_TYPE                                   0x9112
#define GL_SYNC_CONDITION                                0x9113
#define GL_SYNC_STATUS                                   0x9114
#define GL_SYNC_FLAGS                                    0x9115
#define GL_SYNC_FENCE                                    0x9116
#define GL_SYNC_GPU_COMMANDS_COMPLETE                    0x9117
#define GL_UNSIGNALED                                    0x9118
#define GL_SIGNALED                                      0x9119
#define GL_ALREADY_SIGNALED                              0x911A
#define GL_TIMEOUT_EXPIRED                               0x911B
#define GL_CONDITION_SATISFIED                           0x911C
#define GL_WAIT_FAILED                                   0x911D
#define GL_TIMEOUT_IGNORED                               0xFFFFFFFFFFFFFFFFull
#define GL_SYNC_FLUSH_COMMANDS_BIT                       0x00000001
#define GL_SAMPLE_POSITION                               0x8E50
#define GL_SAMPLE_MASK                                   0x8E51
#define GL_SAMPLE_MASK_VALUE                             0x8E52
#define GL_MAX_SAMPLE_MASK_WORDS                         0x8E59
#define GL_TEXTURE_2D_MULTISAMPLE                        0x9100
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE                  0x9101
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY                  0x9102
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY            0x9103
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE                0x9104
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY          0x9105
#define GL_TEXTURE_SAMPLES                               0x9106
#define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS                0x9107
#define GL_SAMPLER_2D_MULTISAMPLE                        0x9108
#define GL_INT_SAMPLER_2D_MULTISAMPLE                    0x9109
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE           0x910A
#define GL_SAMPLER_2D_MULTISAMPLE_ARRAY                  0x910B
#define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY              0x910C
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY     0x910D
#define GL_MAX_COLOR_TEXTURE_SAMPLES                     0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES                     0x910F
#define GL_MAX_INTEGER_SAMPLES                           0x9110
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR                   0x88FE
#define GL_SRC1_COLOR                                    0x88F9
#define GL_ONE_MINUS_SRC1_COLOR                          0x88FA
#define GL_ONE_MINUS_SRC1_ALPHA                          0x88FB
#define GL_MAX_DUAL_SOURCE_DRAW_BUFFERS                  0x88FC
#define GL_ANY_SAMPLES_PASSED                            0x8C2F
#define GL_SAMPLER_BINDING                               0x8919
#define GL_RGB10_A2UI                                    0x906F
#define GL_TEXTURE_SWIZZLE_R                             0x8E42
#define GL_TEXTURE_SWIZZLE_G                             0x8E43
#define GL_TEXTURE_SWIZZLE_B                             0x8E44
#define GL_TEXTURE_SWIZZLE_A                             0x8E45
#define GL_TEXTURE_SWIZZLE_RGBA                          0x8E46
#define GL_TIME_ELAPSED                                  0x88BF
#define GL_TIMESTAMP                                     0x8E28
#define GL_INT_2_10_10_10_REV                            0x8D9F

// OpenGL 1.0
typedef void           (APIENTRYP PFNGLCULLFACEPROC) (GLenum mode);
typedef void           (APIENTRYP PFNGLFRONTFACEPROC) (GLenum mode);
typedef void           (APIENTRYP PFNGLHINTPROC) (GLenum target, GLenum mode);
typedef void           (APIENTRYP PFNGLLINEWIDTHPROC) (GLfloat width);
typedef void           (APIENTRYP PFNGLPOINTSIZEPROC) (GLfloat size);
typedef void           (APIENTRYP PFNGLPOLYGONMODEPROC) (GLenum face, GLenum mode);
typedef void           (APIENTRYP PFNGLSCISSORPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void           (APIENTRYP PFNGLTEXPARAMETERFPROC) (GLenum target, GLenum pname, GLfloat param);
typedef void           (APIENTRYP PFNGLTEXPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void           (APIENTRYP PFNGLTEXPARAMETERIPROC) (GLenum target, GLenum pname, GLint param);
typedef void           (APIENTRYP PFNGLTEXPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void           (APIENTRYP PFNGLTEXIMAGE1DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void           (APIENTRYP PFNGLTEXIMAGE2DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void           (APIENTRYP PFNGLDRAWBUFFERPROC) (GLenum buf);
typedef void           (APIENTRYP PFNGLCLEARPROC) (GLbitfield mask);
typedef void           (APIENTRYP PFNGLCLEARCOLORPROC) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void           (APIENTRYP PFNGLCLEARSTENCILPROC) (GLint s);
typedef void           (APIENTRYP PFNGLCLEARDEPTHPROC) (GLdouble depth);
typedef void           (APIENTRYP PFNGLSTENCILMASKPROC) (GLuint mask);
typedef void           (APIENTRYP PFNGLCOLORMASKPROC) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void           (APIENTRYP PFNGLDEPTHMASKPROC) (GLboolean flag);
typedef void           (APIENTRYP PFNGLDISABLEPROC) (GLenum cap);
typedef void           (APIENTRYP PFNGLENABLEPROC) (GLenum cap);
typedef void           (APIENTRYP PFNGLFINISHPROC) (void);
typedef void           (APIENTRYP PFNGLFLUSHPROC) (void);
typedef void           (APIENTRYP PFNGLBLENDFUNCPROC) (GLenum sfactor, GLenum dfactor);
typedef void           (APIENTRYP PFNGLLOGICOPPROC) (GLenum opcode);
typedef void           (APIENTRYP PFNGLSTENCILFUNCPROC) (GLenum func, GLint ref, GLuint mask);
typedef void           (APIENTRYP PFNGLSTENCILOPPROC) (GLenum fail, GLenum zfail, GLenum zpass);
typedef void           (APIENTRYP PFNGLDEPTHFUNCPROC) (GLenum func);
typedef void           (APIENTRYP PFNGLPIXELSTOREFPROC) (GLenum pname, GLfloat param);
typedef void           (APIENTRYP PFNGLPIXELSTOREIPROC) (GLenum pname, GLint param);
typedef void           (APIENTRYP PFNGLREADBUFFERPROC) (GLenum src);
typedef void           (APIENTRYP PFNGLREADPIXELSPROC) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
typedef void           (APIENTRYP PFNGLGETBOOLEANVPROC) (GLenum pname, GLboolean *data);
typedef void           (APIENTRYP PFNGLGETDOUBLEVPROC) (GLenum pname, GLdouble *data);
typedef GLenum         (APIENTRYP PFNGLGETERRORPROC) (void);
typedef void           (APIENTRYP PFNGLGETFLOATVPROC) (GLenum pname, GLfloat *data);
typedef void           (APIENTRYP PFNGLGETINTEGERVPROC) (GLenum pname, GLint *data);
typedef const GLubyte* (APIENTRYP PFNGLGETSTRINGPROC) (GLenum name);
typedef void           (APIENTRYP PFNGLGETTEXIMAGEPROC) (GLenum target, GLint level, GLenum format, GLenum type, void *pixels);
typedef void           (APIENTRYP PFNGLGETTEXPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void           (APIENTRYP PFNGLGETTEXPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void           (APIENTRYP PFNGLGETTEXLEVELPARAMETERFVPROC) (GLenum target, GLint level, GLenum pname, GLfloat *params);
typedef void           (APIENTRYP PFNGLGETTEXLEVELPARAMETERIVPROC) (GLenum target, GLint level, GLenum pname, GLint *params);
typedef GLboolean      (APIENTRYP PFNGLISENABLEDPROC) (GLenum cap);
typedef void           (APIENTRYP PFNGLDEPTHRANGEPROC) (GLdouble n, GLdouble f);
typedef void           (APIENTRYP PFNGLVIEWPORTPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
//OpenGL 1.1
typedef void           (APIENTRYP PFNGLDRAWARRAYSPROC) (GLenum mode, GLint first, GLsizei count);
typedef void           (APIENTRYP PFNGLDRAWELEMENTSPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef void           (APIENTRYP PFNGLGETPOINTERVPROC) (GLenum pname, void **params);
typedef void           (APIENTRYP PFNGLPOLYGONOFFSETPROC) (GLfloat factor, GLfloat units);
typedef void           (APIENTRYP PFNGLCOPYTEXIMAGE1DPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef void           (APIENTRYP PFNGLCOPYTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void           (APIENTRYP PFNGLCOPYTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void           (APIENTRYP PFNGLCOPYTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void           (APIENTRYP PFNGLTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
typedef void           (APIENTRYP PFNGLTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void           (APIENTRYP PFNGLBINDTEXTUREPROC) (GLenum target, GLuint texture);
typedef void           (APIENTRYP PFNGLDELETETEXTURESPROC) (GLsizei n, const GLuint *textures);
typedef void           (APIENTRYP PFNGLGENTEXTURESPROC) (GLsizei n, GLuint *textures);
typedef GLboolean      (APIENTRYP PFNGLISTEXTUREPROC) (GLuint texture);
// OpenGL 1.2
typedef void           (APIENTRYP PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
typedef void           (APIENTRYP PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void           (APIENTRYP PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
typedef void           (APIENTRYP PFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
// OpenGL 1.3
typedef void           (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void           (APIENTRYP PFNGLSAMPLECOVERAGEPROC) (GLfloat value, GLboolean invert);
typedef void           (APIENTRYP PFNGLCOMPRESSEDTEXIMAGE3DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
typedef void           (APIENTRYP PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
typedef void           (APIENTRYP PFNGLCOMPRESSEDTEXIMAGE1DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data);
typedef void           (APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
typedef void           (APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
typedef void           (APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
typedef void           (APIENTRYP PFNGLGETCOMPRESSEDTEXIMAGEPROC) (GLenum target, GLint level, void *img);
// OpenGL 1.4
typedef void           (APIENTRYP PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void           (APIENTRYP PFNGLMULTIDRAWARRAYSPROC) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
typedef void           (APIENTRYP PFNGLMULTIDRAWELEMENTSPROC) (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount);
typedef void           (APIENTRYP PFNGLPOINTPARAMETERFPROC) (GLenum pname, GLfloat param);
typedef void           (APIENTRYP PFNGLPOINTPARAMETERFVPROC) (GLenum pname, const GLfloat *params);
typedef void           (APIENTRYP PFNGLPOINTPARAMETERIPROC) (GLenum pname, GLint param);
typedef void           (APIENTRYP PFNGLPOINTPARAMETERIVPROC) (GLenum pname, const GLint *params);
typedef void           (APIENTRYP PFNGLBLENDCOLORPROC) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void           (APIENTRYP PFNGLBLENDEQUATIONPROC) (GLenum mode);
// OpenGL 1.5
typedef void           (APIENTRYP PFNGLGENQUERIESPROC) (GLsizei n, GLuint *ids);
typedef void           (APIENTRYP PFNGLDELETEQUERIESPROC) (GLsizei n, const GLuint *ids);
typedef GLboolean      (APIENTRYP PFNGLISQUERYPROC) (GLuint id);
typedef void           (APIENTRYP PFNGLBEGINQUERYPROC) (GLenum target, GLuint id);
typedef void           (APIENTRYP PFNGLENDQUERYPROC) (GLenum target);
typedef void           (APIENTRYP PFNGLGETQUERYIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void           (APIENTRYP PFNGLGETQUERYOBJECTIVPROC) (GLuint id, GLenum pname, GLint *params);
typedef void           (APIENTRYP PFNGLGETQUERYOBJECTUIVPROC) (GLuint id, GLenum pname, GLuint *params);
typedef void           (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void           (APIENTRYP PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
typedef void           (APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef GLboolean      (APIENTRYP PFNGLISBUFFERPROC) (GLuint buffer);
typedef void           (APIENTRYP PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void           (APIENTRYP PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
typedef void           (APIENTRYP PFNGLGETBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, void *data);
typedef void*          (APIENTRYP PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
typedef GLboolean      (APIENTRYP PFNGLUNMAPBUFFERPROC) (GLenum target);
typedef void           (APIENTRYP PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void           (APIENTRYP PFNGLGETBUFFERPOINTERVPROC) (GLenum target, GLenum pname, void **params);
// OpenGL 2.0
typedef void           (APIENTRYP PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum modeRGB, GLenum modeAlpha);
typedef void           (APIENTRYP PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum *bufs);
typedef void           (APIENTRYP PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void           (APIENTRYP PFNGLSTENCILFUNCSEPARATEPROC) (GLenum face, GLenum func, GLint ref, GLuint mask);
typedef void           (APIENTRYP PFNGLSTENCILMASKSEPARATEPROC) (GLenum face, GLuint mask);
typedef void           (APIENTRYP PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void           (APIENTRYP PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar *name);
typedef void           (APIENTRYP PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef GLuint         (APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint         (APIENTRYP PFNGLCREATESHADERPROC) (GLenum type);
typedef void           (APIENTRYP PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void           (APIENTRYP PFNGLDELETESHADERPROC) (GLuint shader);
typedef void           (APIENTRYP PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef void           (APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void           (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void           (APIENTRYP PFNGLGETACTIVEATTRIBPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void           (APIENTRYP PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void           (APIENTRYP PFNGLGETATTACHEDSHADERSPROC) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
typedef GLint          (APIENTRYP PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void           (APIENTRYP PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void           (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void           (APIENTRYP PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void           (APIENTRYP PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void           (APIENTRYP PFNGLGETSHADERSOURCEPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
typedef GLint          (APIENTRYP PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void           (APIENTRYP PFNGLGETUNIFORMFVPROC) (GLuint program, GLint location, GLfloat *params);
typedef void           (APIENTRYP PFNGLGETUNIFORMIVPROC) (GLuint program, GLint location, GLint *params);
typedef void           (APIENTRYP PFNGLGETVERTEXATTRIBDVPROC) (GLuint index, GLenum pname, GLdouble *params);
typedef void           (APIENTRYP PFNGLGETVERTEXATTRIBFVPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef void           (APIENTRYP PFNGLGETVERTEXATTRIBIVPROC) (GLuint index, GLenum pname, GLint *params);
typedef void           (APIENTRYP PFNGLGETVERTEXATTRIBPOINTERVPROC) (GLuint index, GLenum pname, void **pointer);
typedef GLboolean      (APIENTRYP PFNGLISPROGRAMPROC) (GLuint program);
typedef GLboolean      (APIENTRYP PFNGLISSHADERPROC) (GLuint shader);
typedef void           (APIENTRYP PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void           (APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void           (APIENTRYP PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void           (APIENTRYP PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
typedef void           (APIENTRYP PFNGLUNIFORM2FPROC) (GLint location, GLfloat v0, GLfloat v1);
typedef void           (APIENTRYP PFNGLUNIFORM3FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void           (APIENTRYP PFNGLUNIFORM4FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void           (APIENTRYP PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void           (APIENTRYP PFNGLUNIFORM2IPROC) (GLint location, GLint v0, GLint v1);
typedef void           (APIENTRYP PFNGLUNIFORM3IPROC) (GLint location, GLint v0, GLint v1, GLint v2);
typedef void           (APIENTRYP PFNGLUNIFORM4IPROC) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void           (APIENTRYP PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void           (APIENTRYP PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void           (APIENTRYP PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void           (APIENTRYP PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void           (APIENTRYP PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void           (APIENTRYP PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void           (APIENTRYP PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void           (APIENTRYP PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void           (APIENTRYP PFNGLUNIFORMMATRIX2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void           (APIENTRYP PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void           (APIENTRYP PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void           (APIENTRYP PFNGLVALIDATEPROGRAMPROC) (GLuint program);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB1DPROC) (GLuint index, GLdouble x);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB1DVPROC) (GLuint index, const GLdouble *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB1FPROC) (GLuint index, GLfloat x);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB1FVPROC) (GLuint index, const GLfloat *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB1SPROC) (GLuint index, GLshort x);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB1SVPROC) (GLuint index, const GLshort *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB2DPROC) (GLuint index, GLdouble x, GLdouble y);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB2DVPROC) (GLuint index, const GLdouble *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB2FPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB2FVPROC) (GLuint index, const GLfloat *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB2SPROC) (GLuint index, GLshort x, GLshort y);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB2SVPROC) (GLuint index, const GLshort *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB3DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB3DVPROC) (GLuint index, const GLdouble *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB3FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB3FVPROC) (GLuint index, const GLfloat *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB3SPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB3SVPROC) (GLuint index, const GLshort *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4NBVPROC) (GLuint index, const GLbyte *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4NIVPROC) (GLuint index, const GLint *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4NSVPROC) (GLuint index, const GLshort *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4NUBPROC) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4NUBVPROC) (GLuint index, const GLubyte *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4NUIVPROC) (GLuint index, const GLuint *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4NUSVPROC) (GLuint index, const GLushort *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4BVPROC) (GLuint index, const GLbyte *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4DVPROC) (GLuint index, const GLdouble *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4FVPROC) (GLuint index, const GLfloat *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4IVPROC) (GLuint index, const GLint *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4SPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4SVPROC) (GLuint index, const GLshort *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4UBVPROC) (GLuint index, const GLubyte *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4UIVPROC) (GLuint index, const GLuint *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIB4USVPROC) (GLuint index, const GLushort *v);
typedef void           (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
// OpenGL 2.1
typedef void           (APIENTRYP PFNGLUNIFORMMATRIX2X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void           (APIENTRYP PFNGLUNIFORMMATRIX3X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void           (APIENTRYP PFNGLUNIFORMMATRIX2X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void           (APIENTRYP PFNGLUNIFORMMATRIX4X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void           (APIENTRYP PFNGLUNIFORMMATRIX3X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void           (APIENTRYP PFNGLUNIFORMMATRIX4X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
// OpenGL 3.0
typedef void            (APIENTRYP PFNGLCOLORMASKIPROC) (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
typedef void            (APIENTRYP PFNGLGETBOOLEANI_VPROC) (GLenum target, GLuint index, GLboolean *data);
typedef void            (APIENTRYP PFNGLGETINTEGERI_VPROC) (GLenum target, GLuint index, GLint *data);
typedef void            (APIENTRYP PFNGLENABLEIPROC) (GLenum target, GLuint index);
typedef void            (APIENTRYP PFNGLDISABLEIPROC) (GLenum target, GLuint index);
typedef GLboolean       (APIENTRYP PFNGLISENABLEDIPROC) (GLenum target, GLuint index);
typedef void            (APIENTRYP PFNGLBEGINTRANSFORMFEEDBACKPROC) (GLenum primitiveMode);
typedef void            (APIENTRYP PFNGLENDTRANSFORMFEEDBACKPROC) (void);
typedef void            (APIENTRYP PFNGLBINDBUFFERRANGEPROC) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void            (APIENTRYP PFNGLBINDBUFFERBASEPROC) (GLenum target, GLuint index, GLuint buffer);
typedef void            (APIENTRYP PFNGLTRANSFORMFEEDBACKVARYINGSPROC) (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
typedef void            (APIENTRYP PFNGLGETTRANSFORMFEEDBACKVARYINGPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
typedef void            (APIENTRYP PFNGLCLAMPCOLORPROC) (GLenum target, GLenum clamp);
typedef void            (APIENTRYP PFNGLBEGINCONDITIONALRENDERPROC) (GLuint id, GLenum mode);
typedef void            (APIENTRYP PFNGLENDCONDITIONALRENDERPROC) (void);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBIPOINTERPROC) (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void            (APIENTRYP PFNGLGETVERTEXATTRIBIIVPROC) (GLuint index, GLenum pname, GLint *params);
typedef void            (APIENTRYP PFNGLGETVERTEXATTRIBIUIVPROC) (GLuint index, GLenum pname, GLuint *params);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI1IPROC) (GLuint index, GLint x);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI2IPROC) (GLuint index, GLint x, GLint y);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI3IPROC) (GLuint index, GLint x, GLint y, GLint z);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI4IPROC) (GLuint index, GLint x, GLint y, GLint z, GLint w);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI1UIPROC) (GLuint index, GLuint x);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI2UIPROC) (GLuint index, GLuint x, GLuint y);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI3UIPROC) (GLuint index, GLuint x, GLuint y, GLuint z);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI4UIPROC) (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI1IVPROC) (GLuint index, const GLint *v);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI2IVPROC) (GLuint index, const GLint *v);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI3IVPROC) (GLuint index, const GLint *v);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI4IVPROC) (GLuint index, const GLint *v);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI1UIVPROC) (GLuint index, const GLuint *v);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI2UIVPROC) (GLuint index, const GLuint *v);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI3UIVPROC) (GLuint index, const GLuint *v);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI4UIVPROC) (GLuint index, const GLuint *v);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI4BVPROC) (GLuint index, const GLbyte *v);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI4SVPROC) (GLuint index, const GLshort *v);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI4UBVPROC) (GLuint index, const GLubyte *v);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBI4USVPROC) (GLuint index, const GLushort *v);
typedef void            (APIENTRYP PFNGLGETUNIFORMUIVPROC) (GLuint program, GLint location, GLuint *params);
typedef void            (APIENTRYP PFNGLBINDFRAGDATALOCATIONPROC) (GLuint program, GLuint color, const GLchar *name);
typedef GLint           (APIENTRYP PFNGLGETFRAGDATALOCATIONPROC) (GLuint program, const GLchar *name);
typedef void            (APIENTRYP PFNGLUNIFORM1UIPROC) (GLint location, GLuint v0);
typedef void            (APIENTRYP PFNGLUNIFORM2UIPROC) (GLint location, GLuint v0, GLuint v1);
typedef void            (APIENTRYP PFNGLUNIFORM3UIPROC) (GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void            (APIENTRYP PFNGLUNIFORM4UIPROC) (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void            (APIENTRYP PFNGLUNIFORM1UIVPROC) (GLint location, GLsizei count, const GLuint *value);
typedef void            (APIENTRYP PFNGLUNIFORM2UIVPROC) (GLint location, GLsizei count, const GLuint *value);
typedef void            (APIENTRYP PFNGLUNIFORM3UIVPROC) (GLint location, GLsizei count, const GLuint *value);
typedef void            (APIENTRYP PFNGLUNIFORM4UIVPROC) (GLint location, GLsizei count, const GLuint *value);
typedef void            (APIENTRYP PFNGLTEXPARAMETERIIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void            (APIENTRYP PFNGLTEXPARAMETERIUIVPROC) (GLenum target, GLenum pname, const GLuint *params);
typedef void            (APIENTRYP PFNGLGETTEXPARAMETERIIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void            (APIENTRYP PFNGLGETTEXPARAMETERIUIVPROC) (GLenum target, GLenum pname, GLuint *params);
typedef void            (APIENTRYP PFNGLCLEARBUFFERIVPROC) (GLenum buffer, GLint drawbuffer, const GLint *value);
typedef void            (APIENTRYP PFNGLCLEARBUFFERUIVPROC) (GLenum buffer, GLint drawbuffer, const GLuint *value);
typedef void            (APIENTRYP PFNGLCLEARBUFFERFVPROC) (GLenum buffer, GLint drawbuffer, const GLfloat *value);
typedef void            (APIENTRYP PFNGLCLEARBUFFERFIPROC) (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
typedef const GLubyte * (APIENTRYP PFNGLGETSTRINGIPROC) (GLenum name, GLuint index);
typedef GLboolean       (APIENTRYP PFNGLISRENDERBUFFERPROC) (GLuint renderbuffer);
typedef void            (APIENTRYP PFNGLBINDRENDERBUFFERPROC) (GLenum target, GLuint renderbuffer);
typedef void            (APIENTRYP PFNGLDELETERENDERBUFFERSPROC) (GLsizei n, const GLuint *renderbuffers);
typedef void            (APIENTRYP PFNGLGENRENDERBUFFERSPROC) (GLsizei n, GLuint *renderbuffers);
typedef void            (APIENTRYP PFNGLRENDERBUFFERSTORAGEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void            (APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef GLboolean       (APIENTRYP PFNGLISFRAMEBUFFERPROC) (GLuint framebuffer);
typedef void            (APIENTRYP PFNGLBINDFRAMEBUFFERPROC) (GLenum target, GLuint framebuffer);
typedef void            (APIENTRYP PFNGLDELETEFRAMEBUFFERSPROC) (GLsizei n, const GLuint *framebuffers);
typedef void            (APIENTRYP PFNGLGENFRAMEBUFFERSPROC) (GLsizei n, GLuint *framebuffers);
typedef GLenum          (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSPROC) (GLenum target);
typedef void            (APIENTRYP PFNGLFRAMEBUFFERTEXTURE1DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void            (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void            (APIENTRYP PFNGLFRAMEBUFFERTEXTURE3DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef void            (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFERPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void            (APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void            (APIENTRYP PFNGLGENERATEMIPMAPPROC) (GLenum target);
typedef void            (APIENTRYP PFNGLBLITFRAMEBUFFERPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void            (APIENTRYP PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void            (APIENTRYP PFNGLFRAMEBUFFERTEXTURELAYERPROC) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef void*           (APIENTRYP PFNGLMAPBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void            (APIENTRYP PFNGLFLUSHMAPPEDBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length);
typedef void            (APIENTRYP PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void            (APIENTRYP PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint *arrays);
typedef void            (APIENTRYP PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef GLboolean       (APIENTRYP PFNGLISVERTEXARRAYPROC) (GLuint array);
// OpenGL 3.1
typedef void            (APIENTRYP PFNGLDRAWARRAYSINSTANCEDPROC) (GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
typedef void            (APIENTRYP PFNGLDRAWELEMENTSINSTANCEDPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
typedef void            (APIENTRYP PFNGLTEXBUFFERPROC) (GLenum target, GLenum internalformat, GLuint buffer);
typedef void            (APIENTRYP PFNGLPRIMITIVERESTARTINDEXPROC) (GLuint index);
typedef void            (APIENTRYP PFNGLCOPYBUFFERSUBDATAPROC) (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
typedef void            (APIENTRYP PFNGLGETUNIFORMINDICESPROC) (GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
typedef void            (APIENTRYP PFNGLGETACTIVEUNIFORMSIVPROC) (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
typedef void            (APIENTRYP PFNGLGETACTIVEUNIFORMNAMEPROC) (GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);
typedef GLuint          (APIENTRYP PFNGLGETUNIFORMBLOCKINDEXPROC) (GLuint program, const GLchar *uniformBlockName);
typedef void            (APIENTRYP PFNGLGETACTIVEUNIFORMBLOCKIVPROC) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
typedef void            (APIENTRYP PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC) (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
typedef void            (APIENTRYP PFNGLUNIFORMBLOCKBINDINGPROC) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
// OpenGL 3.2
typedef void            (APIENTRYP PFNGLDRAWELEMENTSBASEVERTEXPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
typedef void            (APIENTRYP PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
typedef void            (APIENTRYP PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
typedef void            (APIENTRYP PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC) (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex);
typedef void            (APIENTRYP PFNGLPROVOKINGVERTEXPROC) (GLenum mode);
typedef GLsync          (APIENTRYP PFNGLFENCESYNCPROC) (GLenum condition, GLbitfield flags);
typedef GLboolean       (APIENTRYP PFNGLISSYNCPROC) (GLsync sync);
typedef void            (APIENTRYP PFNGLDELETESYNCPROC) (GLsync sync);
typedef GLenum          (APIENTRYP PFNGLCLIENTWAITSYNCPROC) (GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void            (APIENTRYP PFNGLWAITSYNCPROC) (GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void            (APIENTRYP PFNGLGETINTEGER64VPROC) (GLenum pname, GLint64 *data);
typedef void            (APIENTRYP PFNGLGETSYNCIVPROC) (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
typedef void            (APIENTRYP PFNGLGETINTEGER64I_VPROC) (GLenum target, GLuint index, GLint64 *data);
typedef void            (APIENTRYP PFNGLGETBUFFERPARAMETERI64VPROC) (GLenum target, GLenum pname, GLint64 *params);
typedef void            (APIENTRYP PFNGLFRAMEBUFFERTEXTUREPROC) (GLenum target, GLenum attachment, GLuint texture, GLint level);
typedef void            (APIENTRYP PFNGLTEXIMAGE2DMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void            (APIENTRYP PFNGLTEXIMAGE3DMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
typedef void            (APIENTRYP PFNGLGETMULTISAMPLEFVPROC) (GLenum pname, GLuint index, GLfloat *val);
typedef void            (APIENTRYP PFNGLSAMPLEMASKIPROC) (GLuint maskNumber, GLbitfield mask);
// OpenGL 3.3
typedef void            (APIENTRYP PFNGLBINDFRAGDATALOCATIONINDEXEDPROC) (GLuint program, GLuint colorNumber, GLuint index, const GLchar *name);
typedef GLint           (APIENTRYP PFNGLGETFRAGDATAINDEXPROC) (GLuint program, const GLchar *name);
typedef void            (APIENTRYP PFNGLGENSAMPLERSPROC) (GLsizei count, GLuint *samplers);
typedef void            (APIENTRYP PFNGLDELETESAMPLERSPROC) (GLsizei count, const GLuint *samplers);
typedef GLboolean       (APIENTRYP PFNGLISSAMPLERPROC) (GLuint sampler);
typedef void            (APIENTRYP PFNGLBINDSAMPLERPROC) (GLuint unit, GLuint sampler);
typedef void            (APIENTRYP PFNGLSAMPLERPARAMETERIPROC) (GLuint sampler, GLenum pname, GLint param);
typedef void            (APIENTRYP PFNGLSAMPLERPARAMETERIVPROC) (GLuint sampler, GLenum pname, const GLint *param);
typedef void            (APIENTRYP PFNGLSAMPLERPARAMETERFPROC) (GLuint sampler, GLenum pname, GLfloat param);
typedef void            (APIENTRYP PFNGLSAMPLERPARAMETERFVPROC) (GLuint sampler, GLenum pname, const GLfloat *param);
typedef void            (APIENTRYP PFNGLSAMPLERPARAMETERIIVPROC) (GLuint sampler, GLenum pname, const GLint *param);
typedef void            (APIENTRYP PFNGLSAMPLERPARAMETERIUIVPROC) (GLuint sampler, GLenum pname, const GLuint *param);
typedef void            (APIENTRYP PFNGLGETSAMPLERPARAMETERIVPROC) (GLuint sampler, GLenum pname, GLint *params);
typedef void            (APIENTRYP PFNGLGETSAMPLERPARAMETERIIVPROC) (GLuint sampler, GLenum pname, GLint *params);
typedef void            (APIENTRYP PFNGLGETSAMPLERPARAMETERFVPROC) (GLuint sampler, GLenum pname, GLfloat *params);
typedef void            (APIENTRYP PFNGLGETSAMPLERPARAMETERIUIVPROC) (GLuint sampler, GLenum pname, GLuint *params);
typedef void            (APIENTRYP PFNGLQUERYCOUNTERPROC) (GLuint id, GLenum target);
typedef void            (APIENTRYP PFNGLGETQUERYOBJECTI64VPROC) (GLuint id, GLenum pname, GLint64 *params);
typedef void            (APIENTRYP PFNGLGETQUERYOBJECTUI64VPROC) (GLuint id, GLenum pname, GLuint64 *params);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBDIVISORPROC) (GLuint index, GLuint divisor);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBP1UIPROC) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBP1UIVPROC) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBP2UIPROC) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBP2UIVPROC) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBP3UIPROC) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBP3UIVPROC) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBP4UIPROC) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
typedef void            (APIENTRYP PFNGLVERTEXATTRIBP4UIVPROC) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);

// GL_ARB_gpu_shader5 extension
#define GEOMETRY_SHADER_INVOCATIONS                      0x887F
#define MAX_GEOMETRY_SHADER_INVOCATIONS                  0x8E5A
#define MIN_FRAGMENT_INTERPOLATION_OFFSET                0x8E5B
#define MAX_FRAGMENT_INTERPOLATION_OFFSET                0x8E5C
#define FRAGMENT_INTERPOLATION_OFFSET_BITS               0x8E5D
#define MAX_VERTEX_STREAMS                               0x8E71

// GL_ARB_shader_subroutine
#define ACTIVE_SUBROUTINES                               0x8DE5
#define ACTIVE_SUBROUTINE_UNIFORMS                       0x8DE6
#define ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS              0x8E47
#define ACTIVE_SUBROUTINE_MAX_LENGTH                     0x8E48
#define ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH             0x8E49
#define MAX_SUBROUTINES                                  0x8DE7
#define MAX_SUBROUTINE_UNIFORM_LOCATIONS                 0x8DE8
#define NUM_COMPATIBLE_SUBROUTINES                       0x8E4A
#define COMPATIBLE_SUBROUTINES                           0x8E4B

typedef GLint  (APIENTRYP PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC) (GLuint program, GLenum shadertype, const GLchar *name);
typedef GLuint (APIENTRYP PFNGLGETSUBROUTINEINDEXPROC) (GLuint program, GLenum shadertype, const GLchar *name);
typedef GLvoid (APIENTRYP PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC) (GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values);
typedef GLvoid (APIENTRYP PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC) (GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name);
typedef GLvoid (APIENTRYP PFNGLGETACTIVESUBROUTINENAMEPROC) (GLuint program, GLenum shadertype, GLuint index,	GLsizei bufsize, GLsizei *length, GLchar *name);
typedef GLvoid (APIENTRYP PFNGLUNIFORMSUBROUTINESUIVPROC) (GLenum shadertype, GLsizei count,	const GLuint *indices);
typedef GLvoid (APIENTRYP PFNGLGETUNIFORMSUBROUTINEUIVPROC) (GLenum shadertype, GLint location,	GLuint *params);
typedef GLvoid (APIENTRYP PFNGLGETPROGRAMSTAGEIVPROC) (GLuint program, GLenum shadertype, GLenum pname, GLint *values);


#define AB_OPENGL_FUNCTIONS_COUNT 345
#define AB_OPENGL_EXTENSIONS_FUNCTIONS_COUNT 8

union ABGLExtensionsProcs {
	AB_GLFUNCPTR procs[AB_OPENGL_EXTENSIONS_FUNCTIONS_COUNT];
	struct {
		PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC   _glGetSubroutineUniformLocationARB;
		PFNGLGETSUBROUTINEINDEXPROC             _glGetSubroutineIndexARB;
		PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC   _glGetActiveSubroutineUniformivARB;
		PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC _glGetActiveSubroutineUniformNameARB;
		PFNGLGETACTIVESUBROUTINENAMEPROC        _glGetActiveSubroutineNameARB;
		PFNGLUNIFORMSUBROUTINESUIVPROC          _glUniformSubroutinesuivARB;
		PFNGLGETUNIFORMSUBROUTINEUIVPROC        _glGetUniformSubroutineuivARB;
		PFNGLGETPROGRAMSTAGEIVPROC              _glGetProgramStageivARB;
	};
};

union ABGLProcs {
	AB_GLFUNCPTR procs[AB_OPENGL_FUNCTIONS_COUNT];
	struct {
		// 1.0
		PFNGLCULLFACEPROC								_glCullFace;
		PFNGLFRONTFACEPROC								_glFrontFace;
		PFNGLHINTPROC									_glHint;
		PFNGLLINEWIDTHPROC								_glLineWidth;
		PFNGLPOINTSIZEPROC								_glPointSize;
		PFNGLPOLYGONMODEPROC							_glPolygonMode;
		PFNGLSCISSORPROC								_glScissor;
		PFNGLTEXPARAMETERFPROC							_glTexParameterf;
		PFNGLTEXPARAMETERFVPROC							_glTexParameterfv;
		PFNGLTEXPARAMETERIPROC							_glTexParameteri;
		PFNGLTEXPARAMETERIVPROC							_glTexParameteriv;
		PFNGLTEXIMAGE1DPROC								_glTexImage1D;
		PFNGLTEXIMAGE2DPROC								_glTexImage2D;
		PFNGLDRAWBUFFERPROC								_glDrawBuffer;
		PFNGLCLEARPROC									_glClear;
		PFNGLCLEARCOLORPROC								_glClearColor;
		PFNGLCLEARSTENCILPROC							_glClearStencil;
		PFNGLCLEARDEPTHPROC								_glClearDepth;
		PFNGLSTENCILMASKPROC							_glStencilMask;
		PFNGLCOLORMASKPROC								_glColorMask;
		PFNGLDEPTHMASKPROC								_glDepthMask;
		PFNGLDISABLEPROC								_glDisable;
		PFNGLENABLEPROC									_glEnable;
		PFNGLFINISHPROC									_glFinish;
		PFNGLFLUSHPROC									_glFlush;
		PFNGLBLENDFUNCPROC								_glBlendFunc;
		PFNGLLOGICOPPROC								_glLogicOp;
		PFNGLSTENCILFUNCPROC							_glStencilFunc;
		PFNGLSTENCILOPPROC								_glStencilOp;
		PFNGLDEPTHFUNCPROC								_glDepthFunc;
		PFNGLPIXELSTOREFPROC							_glPixelStoref;
		PFNGLPIXELSTOREIPROC							_glPixelStorei;
		PFNGLREADBUFFERPROC								_glReadBuffer;
		PFNGLREADPIXELSPROC								_glReadPixels;
		PFNGLGETBOOLEANVPROC							_glGetBooleanv;
		PFNGLGETDOUBLEVPROC								_glGetDoublev;
		PFNGLGETERRORPROC								_glGetError;
		PFNGLGETFLOATVPROC								_glGetFloatv;
		PFNGLGETINTEGERVPROC							_glGetIntegerv;
		PFNGLGETSTRINGPROC								_glGetString;
		PFNGLGETTEXIMAGEPROC							_glGetTexImage;
		PFNGLGETTEXPARAMETERFVPROC						_glGetTexParameterfv;
		PFNGLGETTEXPARAMETERIVPROC						_glGetTexParameteriv;
		PFNGLGETTEXLEVELPARAMETERFVPROC					_glGetTexLevelParameterfv;
		PFNGLGETTEXLEVELPARAMETERIVPROC					_glGetTexLevelParameteriv;
		PFNGLISENABLEDPROC								_glIsEnabled;
		PFNGLDEPTHRANGEPROC								_glDepthRange;
		PFNGLVIEWPORTPROC								_glViewport;
		// 1.1
		PFNGLDRAWARRAYSPROC								_glDrawArrays;
		PFNGLDRAWELEMENTSPROC							_glDrawElements;
		PFNGLGETPOINTERVPROC							_glGetPointerv;
		PFNGLPOLYGONOFFSETPROC							_glPolygonOffset;
		PFNGLCOPYTEXIMAGE1DPROC							_glCopyTexImage1D;
		PFNGLCOPYTEXIMAGE2DPROC							_glCopyTexImage2D;
		PFNGLCOPYTEXSUBIMAGE1DPROC						_glCopyTexSubImage1D;
		PFNGLCOPYTEXSUBIMAGE2DPROC						_glCopyTexSubImage2D;
		PFNGLTEXSUBIMAGE1DPROC							_glTexSubImage1D;
		PFNGLTEXSUBIMAGE2DPROC							_glTexSubImage2D;
		PFNGLBINDTEXTUREPROC							_glBindTexture;
		PFNGLDELETETEXTURESPROC							_glDeleteTextures;
		PFNGLGENTEXTURESPROC							_glGenTextures;
		PFNGLISTEXTUREPROC								_glIsTexture;
		// 1.2
		PFNGLDRAWRANGEELEMENTSPROC						_glDrawRangeElements;
		PFNGLTEXIMAGE3DPROC								_glTexImage3D;
		PFNGLTEXSUBIMAGE3DPROC							_glTexSubImage3D;
		PFNGLCOPYTEXSUBIMAGE3DPROC						_glCopyTexSubImage3D;
		// 1.3
		PFNGLACTIVETEXTUREPROC							_glActiveTexture;
		PFNGLSAMPLECOVERAGEPROC							_glSampleCoverage;
		PFNGLCOMPRESSEDTEXIMAGE3DPROC					_glCompressedTexImage3D;
		PFNGLCOMPRESSEDTEXIMAGE2DPROC					_glCompressedTexImage2D;
		PFNGLCOMPRESSEDTEXIMAGE1DPROC					_glCompressedTexImage1D;
		PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC				_glCompressedTexSubImage3D;
		PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC				_glCompressedTexSubImage2D;
		PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC				_glCompressedTexSubImage1D;
		PFNGLGETCOMPRESSEDTEXIMAGEPROC					_glGetCompressedTexImage;
		// 1.4
		PFNGLBLENDFUNCSEPARATEPROC						_glBlendFuncSeparate;
		PFNGLMULTIDRAWARRAYSPROC						_glMultiDrawArrays;
		PFNGLMULTIDRAWELEMENTSPROC						_glMultiDrawElements;
		PFNGLPOINTPARAMETERFPROC						_glPointParameterf;
		PFNGLPOINTPARAMETERFVPROC						_glPointParameterfv;
		PFNGLPOINTPARAMETERIPROC						_glPointParameteri;
		PFNGLPOINTPARAMETERIVPROC						_glPointParameteriv;
		PFNGLBLENDCOLORPROC								_glBlendColor;
		PFNGLBLENDEQUATIONPROC							_glBlendEquation;
		// 1.5
		PFNGLGENQUERIESPROC								_glGenQueries;
		PFNGLDELETEQUERIESPROC							_glDeleteQueries;
		PFNGLISQUERYPROC								_glIsQuery;
		PFNGLBEGINQUERYPROC								_glBeginQuery;
		PFNGLENDQUERYPROC								_glEndQuery;
		PFNGLGETQUERYIVPROC								_glGetQueryiv;
		PFNGLGETQUERYOBJECTIVPROC						_glGetQueryObjectiv;
		PFNGLGETQUERYOBJECTUIVPROC						_glGetQueryObjectuiv;
		PFNGLBINDBUFFERPROC								_glBindBuffer;
		PFNGLDELETEBUFFERSPROC							_glDeleteBuffers;
		PFNGLGENBUFFERSPROC								_glGenBuffers;
		PFNGLISBUFFERPROC								_glIsBuffer;
		PFNGLBUFFERDATAPROC								_glBufferData;
		PFNGLBUFFERSUBDATAPROC							_glBufferSubData;
		PFNGLGETBUFFERSUBDATAPROC						_glGetBufferSubData;
		PFNGLMAPBUFFERPROC								_glMapBuffer;
		PFNGLUNMAPBUFFERPROC							_glUnmapBuffer;
		PFNGLGETBUFFERPARAMETERIVPROC					_glGetBufferParameteriv;
		PFNGLGETBUFFERPOINTERVPROC						_glGetBufferPointerv;
		// 2.0
		PFNGLBLENDEQUATIONSEPARATEPROC					_glBlendEquationSeparate;
		PFNGLDRAWBUFFERSPROC							_glDrawBuffers;
		PFNGLSTENCILOPSEPARATEPROC						_glStencilOpSeparate;
		PFNGLSTENCILFUNCSEPARATEPROC					_glStencilFuncSeparate;
		PFNGLSTENCILMASKSEPARATEPROC					_glStencilMaskSeparate;
		PFNGLATTACHSHADERPROC							_glAttachShader;
		PFNGLBINDATTRIBLOCATIONPROC						_glBindAttribLocation;
		PFNGLCOMPILESHADERPROC							_glCompileShader;
		PFNGLCREATEPROGRAMPROC							_glCreateProgram;
		PFNGLCREATESHADERPROC							_glCreateShader;
		PFNGLDELETEPROGRAMPROC							_glDeleteProgram;
		PFNGLDELETESHADERPROC							_glDeleteShader;
		PFNGLDETACHSHADERPROC							_glDetachShader;
		PFNGLDISABLEVERTEXATTRIBARRAYPROC				_glDisableVertexAttribArray;
		PFNGLENABLEVERTEXATTRIBARRAYPROC				_glEnableVertexAttribArray;
		PFNGLGETACTIVEATTRIBPROC						_glGetActiveAttrib;
		PFNGLGETACTIVEUNIFORMPROC						_glGetActiveUniform;
		PFNGLGETATTACHEDSHADERSPROC						_glGetAttachedShaders;
		PFNGLGETATTRIBLOCATIONPROC						_glGetAttribLocation;
		PFNGLGETPROGRAMIVPROC							_glGetProgramiv;
		PFNGLGETPROGRAMINFOLOGPROC						_glGetProgramInfoLog;
		PFNGLGETSHADERIVPROC							_glGetShaderiv;
		PFNGLGETSHADERINFOLOGPROC						_glGetShaderInfoLog;
		PFNGLGETSHADERSOURCEPROC						_glGetShaderSource;
		PFNGLGETUNIFORMLOCATIONPROC						_glGetUniformLocation;
		PFNGLGETUNIFORMFVPROC							_glGetUniformfv;
		PFNGLGETUNIFORMIVPROC							_glGetUniformiv;
		PFNGLGETVERTEXATTRIBDVPROC						_glGetVertexAttribdv;
		PFNGLGETVERTEXATTRIBFVPROC						_glGetVertexAttribfv;
		PFNGLGETVERTEXATTRIBIVPROC						_glGetVertexAttribiv;
		PFNGLGETVERTEXATTRIBPOINTERVPROC				_glGetVertexAttribPointerv;
		PFNGLISPROGRAMPROC								_glIsProgram;
		PFNGLISSHADERPROC								_glIsShader;
		PFNGLLINKPROGRAMPROC							_glLinkProgram;
		PFNGLSHADERSOURCEPROC							_glShaderSource;
		PFNGLUSEPROGRAMPROC								_glUseProgram;
		PFNGLUNIFORM1FPROC								_glUniform1f;
		PFNGLUNIFORM2FPROC								_glUniform2f;
		PFNGLUNIFORM3FPROC								_glUniform3f;
		PFNGLUNIFORM4FPROC								_glUniform4f;
		PFNGLUNIFORM1IPROC								_glUniform1i;
		PFNGLUNIFORM2IPROC								_glUniform2i;
		PFNGLUNIFORM3IPROC								_glUniform3i;
		PFNGLUNIFORM4IPROC								_glUniform4i;
		PFNGLUNIFORM1FVPROC								_glUniform1fv;
		PFNGLUNIFORM2FVPROC								_glUniform2fv;
		PFNGLUNIFORM3FVPROC								_glUniform3fv;
		PFNGLUNIFORM4FVPROC								_glUniform4fv;
		PFNGLUNIFORM1IVPROC								_glUniform1iv;
		PFNGLUNIFORM2IVPROC								_glUniform2iv;
		PFNGLUNIFORM3IVPROC								_glUniform3iv;
		PFNGLUNIFORM4IVPROC								_glUniform4iv;
		PFNGLUNIFORMMATRIX2FVPROC						_glUniformMatrix2fv;
		PFNGLUNIFORMMATRIX3FVPROC						_glUniformMatrix3fv;
		PFNGLUNIFORMMATRIX4FVPROC						_glUniformMatrix4fv;
		PFNGLVALIDATEPROGRAMPROC						_glValidateProgram;
		PFNGLVERTEXATTRIB1DPROC							_glVertexAttrib1d;
		PFNGLVERTEXATTRIB1DVPROC						_glVertexAttrib1dv;
		PFNGLVERTEXATTRIB1FPROC							_glVertexAttrib1f;
		PFNGLVERTEXATTRIB1FVPROC						_glVertexAttrib1fv;
		PFNGLVERTEXATTRIB1SPROC							_glVertexAttrib1s;
		PFNGLVERTEXATTRIB1SVPROC						_glVertexAttrib1sv;
		PFNGLVERTEXATTRIB2DPROC							_glVertexAttrib2d;
		PFNGLVERTEXATTRIB2DVPROC						_glVertexAttrib2dv;
		PFNGLVERTEXATTRIB2FPROC							_glVertexAttrib2f;
		PFNGLVERTEXATTRIB2FVPROC						_glVertexAttrib2fv;
		PFNGLVERTEXATTRIB2SPROC							_glVertexAttrib2s;
		PFNGLVERTEXATTRIB2SVPROC						_glVertexAttrib2sv;
		PFNGLVERTEXATTRIB3DPROC							_glVertexAttrib3d;
		PFNGLVERTEXATTRIB3DVPROC						_glVertexAttrib3dv;
		PFNGLVERTEXATTRIB3FPROC							_glVertexAttrib3f;
		PFNGLVERTEXATTRIB3FVPROC						_glVertexAttrib3fv;
		PFNGLVERTEXATTRIB3SPROC							_glVertexAttrib3s;
		PFNGLVERTEXATTRIB3SVPROC						_glVertexAttrib3sv;
		PFNGLVERTEXATTRIB4NBVPROC						_glVertexAttrib4Nbv;
		PFNGLVERTEXATTRIB4NIVPROC						_glVertexAttrib4Niv;
		PFNGLVERTEXATTRIB4NSVPROC						_glVertexAttrib4Nsv;
		PFNGLVERTEXATTRIB4NUBPROC						_glVertexAttrib4Nub;
		PFNGLVERTEXATTRIB4NUBVPROC						_glVertexAttrib4Nubv;
		PFNGLVERTEXATTRIB4NUIVPROC						_glVertexAttrib4Nuiv;
		PFNGLVERTEXATTRIB4NUSVPROC						_glVertexAttrib4Nusv;
		PFNGLVERTEXATTRIB4BVPROC						_glVertexAttrib4bv;
		PFNGLVERTEXATTRIB4DPROC							_glVertexAttrib4d;
		PFNGLVERTEXATTRIB4DVPROC						_glVertexAttrib4dv;
		PFNGLVERTEXATTRIB4FPROC							_glVertexAttrib4f;
		PFNGLVERTEXATTRIB4FVPROC						_glVertexAttrib4fv;
		PFNGLVERTEXATTRIB4IVPROC						_glVertexAttrib4iv;
		PFNGLVERTEXATTRIB4SPROC							_glVertexAttrib4s;
		PFNGLVERTEXATTRIB4SVPROC						_glVertexAttrib4sv;
		PFNGLVERTEXATTRIB4UBVPROC						_glVertexAttrib4ubv;
		PFNGLVERTEXATTRIB4UIVPROC						_glVertexAttrib4uiv;
		PFNGLVERTEXATTRIB4USVPROC						_glVertexAttrib4usv;
		PFNGLVERTEXATTRIBPOINTERPROC					_glVertexAttribPointer;
		// 2.1
		PFNGLUNIFORMMATRIX2X3FVPROC						_glUniformMatrix2x3fv;
		PFNGLUNIFORMMATRIX3X2FVPROC						_glUniformMatrix3x2fv;
		PFNGLUNIFORMMATRIX2X4FVPROC						_glUniformMatrix2x4fv;
		PFNGLUNIFORMMATRIX4X2FVPROC						_glUniformMatrix4x2fv;
		PFNGLUNIFORMMATRIX3X4FVPROC						_glUniformMatrix3x4fv;
		PFNGLUNIFORMMATRIX4X3FVPROC						_glUniformMatrix4x3fv;
		// 3.0
		PFNGLCOLORMASKIPROC								_glColorMaski;
		PFNGLGETBOOLEANI_VPROC							_glGetBooleani_v;
		PFNGLGETINTEGERI_VPROC							_glGetIntegeri_v;
		PFNGLENABLEIPROC								_glEnablei;
		PFNGLDISABLEIPROC								_glDisablei;
		PFNGLISENABLEDIPROC								_glIsEnabledi;
		PFNGLBEGINTRANSFORMFEEDBACKPROC					_glBeginTransformFeedback;
		PFNGLENDTRANSFORMFEEDBACKPROC					_glEndTransformFeedback;
		PFNGLBINDBUFFERRANGEPROC						_glBindBufferRange;
		PFNGLBINDBUFFERBASEPROC							_glBindBufferBase;
		PFNGLTRANSFORMFEEDBACKVARYINGSPROC				_glTransformFeedbackVaryings;
		PFNGLGETTRANSFORMFEEDBACKVARYINGPROC			_glGetTransformFeedbackVarying;
		PFNGLCLAMPCOLORPROC								_glClampColor;
		PFNGLBEGINCONDITIONALRENDERPROC					_glBeginConditionalRender;
		PFNGLENDCONDITIONALRENDERPROC					_glEndConditionalRender;
		PFNGLVERTEXATTRIBIPOINTERPROC					_glVertexAttribIPointer;
		PFNGLGETVERTEXATTRIBIIVPROC						_glGetVertexAttribIiv;
		PFNGLGETVERTEXATTRIBIUIVPROC					_glGetVertexAttribIuiv;
		PFNGLVERTEXATTRIBI1IPROC						_glVertexAttribI1i;
		PFNGLVERTEXATTRIBI2IPROC						_glVertexAttribI2i;
		PFNGLVERTEXATTRIBI3IPROC						_glVertexAttribI3i;
		PFNGLVERTEXATTRIBI4IPROC						_glVertexAttribI4i;
		PFNGLVERTEXATTRIBI1UIPROC						_glVertexAttribI1ui;
		PFNGLVERTEXATTRIBI2UIPROC						_glVertexAttribI2ui;
		PFNGLVERTEXATTRIBI3UIPROC						_glVertexAttribI3ui;
		PFNGLVERTEXATTRIBI4UIPROC						_glVertexAttribI4ui;
		PFNGLVERTEXATTRIBI1IVPROC						_glVertexAttribI1iv;
		PFNGLVERTEXATTRIBI2IVPROC						_glVertexAttribI2iv;
		PFNGLVERTEXATTRIBI3IVPROC						_glVertexAttribI3iv;
		PFNGLVERTEXATTRIBI4IVPROC						_glVertexAttribI4iv;
		PFNGLVERTEXATTRIBI1UIVPROC						_glVertexAttribI1uiv;
		PFNGLVERTEXATTRIBI2UIVPROC						_glVertexAttribI2uiv;
		PFNGLVERTEXATTRIBI3UIVPROC						_glVertexAttribI3uiv;
		PFNGLVERTEXATTRIBI4UIVPROC						_glVertexAttribI4uiv;
		PFNGLVERTEXATTRIBI4BVPROC						_glVertexAttribI4bv;
		PFNGLVERTEXATTRIBI4SVPROC						_glVertexAttribI4sv;
		PFNGLVERTEXATTRIBI4UBVPROC						_glVertexAttribI4ubv;
		PFNGLVERTEXATTRIBI4USVPROC						_glVertexAttribI4usv;
		PFNGLGETUNIFORMUIVPROC							_glGetUniformuiv;
		PFNGLBINDFRAGDATALOCATIONPROC					_glBindFragDataLocation;
		PFNGLGETFRAGDATALOCATIONPROC					_glGetFragDataLocation;
		PFNGLUNIFORM1UIPROC								_glUniform1ui;
		PFNGLUNIFORM2UIPROC								_glUniform2ui;
		PFNGLUNIFORM3UIPROC								_glUniform3ui;
		PFNGLUNIFORM4UIPROC								_glUniform4ui;
		PFNGLUNIFORM1UIVPROC							_glUniform1uiv;
		PFNGLUNIFORM2UIVPROC							_glUniform2uiv;
		PFNGLUNIFORM3UIVPROC							_glUniform3uiv;
		PFNGLUNIFORM4UIVPROC							_glUniform4uiv;
		PFNGLTEXPARAMETERIIVPROC						_glTexParameterIiv;
		PFNGLTEXPARAMETERIUIVPROC						_glTexParameterIuiv;
		PFNGLGETTEXPARAMETERIIVPROC						_glGetTexParameterIiv;
		PFNGLGETTEXPARAMETERIUIVPROC					_glGetTexParameterIuiv;
		PFNGLCLEARBUFFERIVPROC							_glClearBufferiv;
		PFNGLCLEARBUFFERUIVPROC							_glClearBufferuiv;
		PFNGLCLEARBUFFERFVPROC							_glClearBufferfv;
		PFNGLCLEARBUFFERFIPROC							_glClearBufferfi;
		PFNGLGETSTRINGIPROC								_glGetStringi;
		PFNGLISRENDERBUFFERPROC							_glIsRenderbuffer;
		PFNGLBINDRENDERBUFFERPROC						_glBindRenderbuffer;
		PFNGLDELETERENDERBUFFERSPROC					_glDeleteRenderbuffers;
		PFNGLGENRENDERBUFFERSPROC						_glGenRenderbuffers;
		PFNGLRENDERBUFFERSTORAGEPROC					_glRenderbufferStorage;
		PFNGLGETRENDERBUFFERPARAMETERIVPROC				_glGetRenderbufferParameteriv;
		PFNGLISFRAMEBUFFERPROC							_glIsFramebuffer;
		PFNGLBINDFRAMEBUFFERPROC						_glBindFramebuffer;
		PFNGLDELETEFRAMEBUFFERSPROC						_glDeleteFramebuffers;
		PFNGLGENFRAMEBUFFERSPROC						_glGenFramebuffers;
		PFNGLCHECKFRAMEBUFFERSTATUSPROC					_glCheckFramebufferStatus;
		PFNGLFRAMEBUFFERTEXTURE1DPROC					_glFramebufferTexture1D;
		PFNGLFRAMEBUFFERTEXTURE2DPROC					_glFramebufferTexture2D;
		PFNGLFRAMEBUFFERTEXTURE3DPROC					_glFramebufferTexture3D;
		PFNGLFRAMEBUFFERRENDERBUFFERPROC				_glFramebufferRenderbuffer;
		PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC	_glGetFramebufferAttachmentParameteriv;
		PFNGLGENERATEMIPMAPPROC							_glGenerateMipmap;
		PFNGLBLITFRAMEBUFFERPROC						_glBlitFramebuffer;
		PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC			_glRenderbufferStorageMultisample;
		PFNGLFRAMEBUFFERTEXTURELAYERPROC				_glFramebufferTextureLayer;
		PFNGLMAPBUFFERRANGEPROC							_glMapBufferRange;
		PFNGLFLUSHMAPPEDBUFFERRANGEPROC					_glFlushMappedBufferRange;
		PFNGLBINDVERTEXARRAYPROC						_glBindVertexArray;
		PFNGLDELETEVERTEXARRAYSPROC						_glDeleteVertexArrays;
		PFNGLGENVERTEXARRAYSPROC						_glGenVertexArrays;
		PFNGLISVERTEXARRAYPROC							_glIsVertexArray;
		// 3.1
		PFNGLDRAWARRAYSINSTANCEDPROC					_glDrawArraysInstanced;
		PFNGLDRAWELEMENTSINSTANCEDPROC					_glDrawElementsInstanced;
		PFNGLTEXBUFFERPROC								_glTexBuffer;
		PFNGLPRIMITIVERESTARTINDEXPROC					_glPrimitiveRestartIndex;
		PFNGLCOPYBUFFERSUBDATAPROC						_glCopyBufferSubData;
		PFNGLGETUNIFORMINDICESPROC						_glGetUniformIndices;
		PFNGLGETACTIVEUNIFORMSIVPROC					_glGetActiveUniformsiv;
		PFNGLGETACTIVEUNIFORMNAMEPROC					_glGetActiveUniformName;
		PFNGLGETUNIFORMBLOCKINDEXPROC					_glGetUniformBlockIndex;
		PFNGLGETACTIVEUNIFORMBLOCKIVPROC				_glGetActiveUniformBlockiv;
		PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC				_glGetActiveUniformBlockName;
		PFNGLUNIFORMBLOCKBINDINGPROC					_glUniformBlockBinding;
		// 3.2
		PFNGLDRAWELEMENTSBASEVERTEXPROC					_glDrawElementsBaseVertex;
		PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC			_glDrawRangeElementsBaseVertex;
		PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC		_glDrawElementsInstancedBaseVertex;
		PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC			_glMultiDrawElementsBaseVertex;
		PFNGLPROVOKINGVERTEXPROC						_glProvokingVertex;
		PFNGLFENCESYNCPROC								_glFenceSync;
		PFNGLISSYNCPROC									_glIsSync;
		PFNGLDELETESYNCPROC								_glDeleteSync;
		PFNGLCLIENTWAITSYNCPROC							_glClientWaitSync;
		PFNGLWAITSYNCPROC								_glWaitSync;
		PFNGLGETINTEGER64VPROC							_glGetInteger64v;
		PFNGLGETSYNCIVPROC								_glGetSynciv;
		PFNGLGETINTEGER64I_VPROC						_glGetInteger64i_v;
		PFNGLGETBUFFERPARAMETERI64VPROC					_glGetBufferParameteri64v;
		PFNGLFRAMEBUFFERTEXTUREPROC						_glFramebufferTexture;
		PFNGLTEXIMAGE2DMULTISAMPLEPROC					_glTexImage2DMultisample;
		PFNGLTEXIMAGE3DMULTISAMPLEPROC					_glTexImage3DMultisample;
		PFNGLGETMULTISAMPLEFVPROC						_glGetMultisamplefv;
		PFNGLSAMPLEMASKIPROC							_glSampleMaski;
		// 3.3
		PFNGLBINDFRAGDATALOCATIONINDEXEDPROC			_glBindFragDataLocationIndexed;
		PFNGLGETFRAGDATAINDEXPROC						_glGetFragDataIndex;
		PFNGLGENSAMPLERSPROC							_glGenSamplers;
		PFNGLDELETESAMPLERSPROC							_glDeleteSamplers;
		PFNGLISSAMPLERPROC								_glIsSampler;
		PFNGLBINDSAMPLERPROC							_glBindSampler;
		PFNGLSAMPLERPARAMETERIPROC						_glSamplerParameteri;
		PFNGLSAMPLERPARAMETERIVPROC						_glSamplerParameteriv;
		PFNGLSAMPLERPARAMETERFPROC						_glSamplerParameterf;
		PFNGLSAMPLERPARAMETERFVPROC						_glSamplerParameterfv;
		PFNGLSAMPLERPARAMETERIIVPROC					_glSamplerParameterIiv;
		PFNGLSAMPLERPARAMETERIUIVPROC					_glSamplerParameterIuiv;
		PFNGLGETSAMPLERPARAMETERIVPROC					_glGetSamplerParameteriv;
		PFNGLGETSAMPLERPARAMETERIIVPROC					_glGetSamplerParameterIiv;
		PFNGLGETSAMPLERPARAMETERFVPROC					_glGetSamplerParameterfv;
		PFNGLGETSAMPLERPARAMETERIUIVPROC				_glGetSamplerParameterIuiv;
		PFNGLQUERYCOUNTERPROC							_glQueryCounter;
		PFNGLGETQUERYOBJECTI64VPROC						_glGetQueryObjecti64v;
		PFNGLGETQUERYOBJECTUI64VPROC					_glGetQueryObjectui64v;
		PFNGLVERTEXATTRIBDIVISORPROC					_glVertexAttribDivisor;
		PFNGLVERTEXATTRIBP1UIPROC						_glVertexAttribP1ui;
		PFNGLVERTEXATTRIBP1UIVPROC						_glVertexAttribP1uiv;
		PFNGLVERTEXATTRIBP2UIPROC						_glVertexAttribP2ui;
		PFNGLVERTEXATTRIBP2UIVPROC						_glVertexAttribP2uiv;
		PFNGLVERTEXATTRIBP3UIPROC						_glVertexAttribP3ui;
		PFNGLVERTEXATTRIBP3UIVPROC						_glVertexAttribP3uiv;
		PFNGLVERTEXATTRIBP4UIPROC						_glVertexAttribP4ui;
		PFNGLVERTEXATTRIBP4UIVPROC						_glVertexAttribP4uiv;
	};
};

extern AB_API ABGLProcs _ABOpenGLProcs;
extern AB_API ABGLExtensionsProcs _ABOpenGLExtProcs;

// GL_ARB_shader_subroutine
#define glGetSubroutineUniformLocationARB          _ABOpenGLExtProcs._glGetSubroutineUniformLocationARB
#define glGetSubroutineIndexARB					   _ABOpenGLExtProcs._glGetSubroutineIndexARB
#define glGetActiveSubroutineUniformivARB		   _ABOpenGLExtProcs._glGetActiveSubroutineUniformivARB
#define glGetActiveSubroutineUniformNameARB		   _ABOpenGLExtProcs._glGetActiveSubroutineUniformNameARB
#define glGetActiveSubroutineNameARB			   _ABOpenGLExtProcs._glGetActiveSubroutineNameARB
#define glUniformSubroutinesuivARB				   _ABOpenGLExtProcs._glUniformSubroutinesuivARB
#define glGetUniformSubroutineuivARB			   _ABOpenGLExtProcs._glGetUniformSubroutineuivARB
#define glGetProgramStageivARB					   _ABOpenGLExtProcs._glGetProgramStageivARB

// OpenGL 1.0
#define glCullFace					_ABOpenGLProcs._glCullFace
#define glFrontFace					_ABOpenGLProcs._glFrontFace
#define glHint						_ABOpenGLProcs._glHint
#define glLineWidth					_ABOpenGLProcs._glLineWidth
#define glPointSize					_ABOpenGLProcs._glPointSize
#define glPolygonMode				_ABOpenGLProcs._glPolygonMode
#define glScissor					_ABOpenGLProcs._glScissor
#define glTexParameterf				_ABOpenGLProcs._glTexParameterf
#define glTexParameterfv			_ABOpenGLProcs._glTexParameterfv
#define glTexParameteri				_ABOpenGLProcs._glTexParameteri
#define glTexParameteriv			_ABOpenGLProcs._glTexParameteriv
#define glTexImage1D				_ABOpenGLProcs._glTexImage1D
#define glTexImage2D				_ABOpenGLProcs._glTexImage2D
#define glDrawBuffer				_ABOpenGLProcs._glDrawBuffer
#define glClear						_ABOpenGLProcs._glClear
#define glClearColor				_ABOpenGLProcs._glClearColor
#define glClearStencil				_ABOpenGLProcs._glClearStencil
#define glClearDepth				_ABOpenGLProcs._glClearDepth
#define glStencilMask				_ABOpenGLProcs._glStencilMask
#define glColorMask					_ABOpenGLProcs._glColorMask
#define glDepthMask					_ABOpenGLProcs._glDepthMask
#define glDisable					_ABOpenGLProcs._glDisable
#define glEnable					_ABOpenGLProcs._glEnable
#define glFinish					_ABOpenGLProcs._glFinish
#define glFlush						_ABOpenGLProcs._glFlush
#define glBlendFunc					_ABOpenGLProcs._glBlendFunc
#define glLogicOp					_ABOpenGLProcs._glLogicOp
#define glStencilFunc				_ABOpenGLProcs._glStencilFunc
#define glStencilOp					_ABOpenGLProcs._glStencilOp
#define glDepthFunc					_ABOpenGLProcs._glDepthFunc
#define glPixelStoref				_ABOpenGLProcs._glPixelStoref
#define glPixelStorei				_ABOpenGLProcs._glPixelStorei
#define glReadBuffer				_ABOpenGLProcs._glReadBuffer
#define glReadPixels				_ABOpenGLProcs._glReadPixels
#define glGetBooleanv				_ABOpenGLProcs._glGetBooleanv
#define glGetDoublev				_ABOpenGLProcs._glGetDoublev
#define glGetError					_ABOpenGLProcs._glGetError
#define glGetFloatv					_ABOpenGLProcs._glGetFloatv
#define glGetIntegerv				_ABOpenGLProcs._glGetIntegerv
#define glGetString					_ABOpenGLProcs._glGetString
#define glGetTexImage				_ABOpenGLProcs._glGetTexImage
#define glGetTexParameterfv			_ABOpenGLProcs._glGetTexParameterfv
#define glGetTexParameteriv			_ABOpenGLProcs._glGetTexParameteriv
#define glGetTexLevelParameterfv	_ABOpenGLProcs._glGetTexLevelParameterfv
#define glGetTexLevelParameteriv	_ABOpenGLProcs._glGetTexLevelParameteriv
#define glIsEnabled					_ABOpenGLProcs._glIsEnabled
#define glDepthRange				_ABOpenGLProcs._glDepthRange
#define glViewport					_ABOpenGLProcs._glViewport
// OpenGL 1.1
#define glDrawArrays				_ABOpenGLProcs._glDrawArrays
#define glDrawElements				_ABOpenGLProcs._glDrawElements
#define glGetPointerv				_ABOpenGLProcs._glGetPointerv
#define glPolygonOffset				_ABOpenGLProcs._glPolygonOffset
#define glCopyTexImage1D			_ABOpenGLProcs._glCopyTexImage1D
#define glCopyTexImage2D			_ABOpenGLProcs._glCopyTexImage2D
#define glCopyTexSubImage1D			_ABOpenGLProcs._glCopyTexSubImage1D
#define glCopyTexSubImage2D			_ABOpenGLProcs._glCopyTexSubImage2D
#define glTexSubImage1D				_ABOpenGLProcs._glTexSubImage1D
#define glTexSubImage2D				_ABOpenGLProcs._glTexSubImage2D
#define glBindTexture				_ABOpenGLProcs._glBindTexture
#define glDeleteTextures			_ABOpenGLProcs._glDeleteTextures
#define glGenTextures				_ABOpenGLProcs._glGenTextures
#define glIsTexture					_ABOpenGLProcs._glIsTexture
// OpenGL 1.2
#define glDrawRangeElements			_ABOpenGLProcs._glDrawRangeElements
#define glTexImage3D				_ABOpenGLProcs._glTexImage3D
#define glTexSubImage3D				_ABOpenGLProcs._glTexSubImage3D
#define glCopyTexSubImage3D			_ABOpenGLProcs._glCopyTexSubImage3D
//OpenGL 1.3
#define glActiveTexture				_ABOpenGLProcs._glActiveTexture
#define glSampleCoverage			_ABOpenGLProcs._glSampleCoverage
#define glCompressedTexImage3D		_ABOpenGLProcs._glCompressedTexImage3D
#define glCompressedTexImage2D		_ABOpenGLProcs._glCompressedTexImage2D
#define glCompressedTexImage1D		_ABOpenGLProcs._glCompressedTexImage1D
#define glCompressedTexSubImage3D	_ABOpenGLProcs._glCompressedTexSubImage3D
#define glCompressedTexSubImage2D	_ABOpenGLProcs._glCompressedTexSubImage2D
#define glCompressedTexSubImage1D	_ABOpenGLProcs._glCompressedTexSubImage1D
#define glGetCompressedTexImage		_ABOpenGLProcs._glGetCompressedTexImage
// OpenGL 1.4
#define glBlendFuncSeparate			_ABOpenGLProcs._glBlendFuncSeparate
#define glMultiDrawArrays			_ABOpenGLProcs._glMultiDrawArrays
#define glMultiDrawElements			_ABOpenGLProcs._glMultiDrawElements
#define glPointParameterf			_ABOpenGLProcs._glPointParameterf
#define glPointParameterfv			_ABOpenGLProcs._glPointParameterfv
#define glPointParameteri			_ABOpenGLProcs._glPointParameteri
#define glPointParameteriv			_ABOpenGLProcs._glPointParameteriv
#define glBlendColor				_ABOpenGLProcs._glBlendColor
#define glBlendEquation				_ABOpenGLProcs._glBlendEquation
// OpenGL 1.5
#define glGenQueries				_ABOpenGLProcs._glGenQueries
#define glDeleteQueries				_ABOpenGLProcs._glDeleteQueries
#define glIsQuery					_ABOpenGLProcs._glIsQuery
#define glBeginQuery				_ABOpenGLProcs._glBeginQuery
#define glEndQuery					_ABOpenGLProcs._glEndQuery
#define glGetQueryiv				_ABOpenGLProcs._glGetQueryiv
#define glGetQueryObjectiv			_ABOpenGLProcs._glGetQueryObjectiv
#define glGetQueryObjectuiv			_ABOpenGLProcs._glGetQueryObjectuiv
#define glBindBuffer				_ABOpenGLProcs._glBindBuffer
#define glDeleteBuffers				_ABOpenGLProcs._glDeleteBuffers
#define glGenBuffers				_ABOpenGLProcs._glGenBuffers
#define glIsBuffer					_ABOpenGLProcs._glIsBuffer
#define glBufferData				_ABOpenGLProcs._glBufferData
#define glBufferSubData				_ABOpenGLProcs._glBufferSubData
#define glGetBufferSubData			_ABOpenGLProcs._glGetBufferSubData
#define glMapBuffer					_ABOpenGLProcs._glMapBuffer
#define glUnmapBuffer				_ABOpenGLProcs._glUnmapBuffer
#define glGetBufferParameteriv		_ABOpenGLProcs._glGetBufferParameteriv
#define glGetBufferPointerv			_ABOpenGLProcs._glGetBufferPointerv
// OpenGL 2.0
#define glBlendEquationSeparate		_ABOpenGLProcs._glBlendEquationSeparate
#define glDrawBuffers				_ABOpenGLProcs._glDrawBuffers
#define glStencilOpSeparate			_ABOpenGLProcs._glStencilOpSeparate
#define glStencilFuncSeparate		_ABOpenGLProcs._glStencilFuncSeparate
#define glStencilMaskSeparate		_ABOpenGLProcs._glStencilMaskSeparate
#define glAttachShader				_ABOpenGLProcs._glAttachShader
#define glBindAttribLocation		_ABOpenGLProcs._glBindAttribLocation
#define glCompileShader				_ABOpenGLProcs._glCompileShader
#define glCreateProgram				_ABOpenGLProcs._glCreateProgram
#define glCreateShader				_ABOpenGLProcs._glCreateShader
#define glDeleteProgram				_ABOpenGLProcs._glDeleteProgram
#define glDeleteShader				_ABOpenGLProcs._glDeleteShader
#define glDetachShader				_ABOpenGLProcs._glDetachShader
#define glDisableVertexAttribArray	_ABOpenGLProcs._glDisableVertexAttribArray
#define glEnableVertexAttribArray	_ABOpenGLProcs._glEnableVertexAttribArray
#define glGetActiveAttrib			_ABOpenGLProcs._glGetActiveAttrib
#define glGetActiveUniform			_ABOpenGLProcs._glGetActiveUniform
#define glGetAttachedShaders		_ABOpenGLProcs._glGetAttachedShaders
#define glGetAttribLocation			_ABOpenGLProcs._glGetAttribLocation
#define glGetProgramiv				_ABOpenGLProcs._glGetProgramiv
#define glGetProgramInfoLog			_ABOpenGLProcs._glGetProgramInfoLog
#define glGetShaderiv				_ABOpenGLProcs._glGetShaderiv
#define glGetShaderInfoLog			_ABOpenGLProcs._glGetShaderInfoLog
#define glGetShaderSource			_ABOpenGLProcs._glGetShaderSource
#define glGetUniformLocation		_ABOpenGLProcs._glGetUniformLocation
#define glGetUniformfv				_ABOpenGLProcs._glGetUniformfv
#define glGetUniformiv				_ABOpenGLProcs._glGetUniformiv
#define glGetVertexAttribdv			_ABOpenGLProcs._glGetVertexAttribdv
#define glGetVertexAttribfv			_ABOpenGLProcs._glGetVertexAttribfv
#define glGetVertexAttribiv			_ABOpenGLProcs._glGetVertexAttribiv
#define glGetVertexAttribPointerv	_ABOpenGLProcs._glGetVertexAttribPointerv
#define glIsProgram					_ABOpenGLProcs._glIsProgram
#define glIsShader					_ABOpenGLProcs._glIsShader
#define glLinkProgram				_ABOpenGLProcs._glLinkProgram
#define glShaderSource				_ABOpenGLProcs._glShaderSource
#define glUseProgram				_ABOpenGLProcs._glUseProgram
#define glUniform1f					_ABOpenGLProcs._glUniform1f
#define glUniform2f					_ABOpenGLProcs._glUniform2f
#define glUniform3f					_ABOpenGLProcs._glUniform3f
#define glUniform4f					_ABOpenGLProcs._glUniform4f
#define glUniform1i					_ABOpenGLProcs._glUniform1i
#define glUniform2i					_ABOpenGLProcs._glUniform2i
#define glUniform3i					_ABOpenGLProcs._glUniform3i
#define glUniform4i					_ABOpenGLProcs._glUniform4i
#define glUniform1fv				_ABOpenGLProcs._glUniform1fv
#define glUniform2fv				_ABOpenGLProcs._glUniform2fv
#define glUniform3fv				_ABOpenGLProcs._glUniform3fv
#define glUniform4fv				_ABOpenGLProcs._glUniform4fv
#define glUniform1iv				_ABOpenGLProcs._glUniform1iv
#define glUniform2iv				_ABOpenGLProcs._glUniform2iv
#define glUniform3iv				_ABOpenGLProcs._glUniform3iv
#define glUniform4iv				_ABOpenGLProcs._glUniform4iv
#define glUniformMatrix2fv			_ABOpenGLProcs._glUniformMatrix2fv
#define glUniformMatrix3fv			_ABOpenGLProcs._glUniformMatrix3fv
#define glUniformMatrix4fv			_ABOpenGLProcs._glUniformMatrix4fv
#define glValidateProgram			_ABOpenGLProcs._glValidateProgram
#define glVertexAttrib1d			_ABOpenGLProcs._glVertexAttrib1d
#define glVertexAttrib1dv			_ABOpenGLProcs._glVertexAttrib1dv
#define glVertexAttrib1f			_ABOpenGLProcs._glVertexAttrib1f
#define glVertexAttrib1fv			_ABOpenGLProcs._glVertexAttrib1fv
#define glVertexAttrib1s			_ABOpenGLProcs._glVertexAttrib1s
#define glVertexAttrib1sv			_ABOpenGLProcs._glVertexAttrib1sv
#define glVertexAttrib2d			_ABOpenGLProcs._glVertexAttrib2d
#define glVertexAttrib2dv			_ABOpenGLProcs._glVertexAttrib2dv
#define glVertexAttrib2f			_ABOpenGLProcs._glVertexAttrib2f
#define glVertexAttrib2fv			_ABOpenGLProcs._glVertexAttrib2fv
#define glVertexAttrib2s			_ABOpenGLProcs._glVertexAttrib2s
#define glVertexAttrib2sv			_ABOpenGLProcs._glVertexAttrib2sv
#define glVertexAttrib3d			_ABOpenGLProcs._glVertexAttrib3d
#define glVertexAttrib3dv			_ABOpenGLProcs._glVertexAttrib3dv
#define glVertexAttrib3f			_ABOpenGLProcs._glVertexAttrib3f
#define glVertexAttrib3fv			_ABOpenGLProcs._glVertexAttrib3fv
#define glVertexAttrib3s			_ABOpenGLProcs._glVertexAttrib3s
#define glVertexAttrib3sv			_ABOpenGLProcs._glVertexAttrib3sv
#define glVertexAttrib4Nbv			_ABOpenGLProcs._glVertexAttrib4Nbv
#define glVertexAttrib4Niv			_ABOpenGLProcs._glVertexAttrib4Niv
#define glVertexAttrib4Nsv			_ABOpenGLProcs._glVertexAttrib4Nsv
#define glVertexAttrib4Nub			_ABOpenGLProcs._glVertexAttrib4Nub
#define glVertexAttrib4Nubv			_ABOpenGLProcs._glVertexAttrib4Nubv
#define glVertexAttrib4Nuiv			_ABOpenGLProcs._glVertexAttrib4Nuiv
#define glVertexAttrib4Nusv			_ABOpenGLProcs._glVertexAttrib4Nusv
#define glVertexAttrib4bv			_ABOpenGLProcs._glVertexAttrib4bv
#define glVertexAttrib4d			_ABOpenGLProcs._glVertexAttrib4d
#define glVertexAttrib4dv			_ABOpenGLProcs._glVertexAttrib4dv
#define glVertexAttrib4f			_ABOpenGLProcs._glVertexAttrib4f
#define glVertexAttrib4fv			_ABOpenGLProcs._glVertexAttrib4fv
#define glVertexAttrib4iv			_ABOpenGLProcs._glVertexAttrib4iv
#define glVertexAttrib4s			_ABOpenGLProcs._glVertexAttrib4s
#define glVertexAttrib4sv			_ABOpenGLProcs._glVertexAttrib4sv
#define glVertexAttrib4ubv			_ABOpenGLProcs._glVertexAttrib4ubv
#define glVertexAttrib4uiv			_ABOpenGLProcs._glVertexAttrib4uiv
#define glVertexAttrib4usv			_ABOpenGLProcs._glVertexAttrib4usv
#define glVertexAttribPointer		_ABOpenGLProcs._glVertexAttribPointer
// OpenGL 2.1
#define glUniformMatrix2x3fv		_ABOpenGLProcs._glUniformMatrix2x3fv
#define glUniformMatrix3x2fv		_ABOpenGLProcs._glUniformMatrix3x2fv
#define glUniformMatrix2x4fv		_ABOpenGLProcs._glUniformMatrix2x4fv
#define glUniformMatrix4x2fv		_ABOpenGLProcs._glUniformMatrix4x2fv
#define glUniformMatrix3x4fv		_ABOpenGLProcs._glUniformMatrix3x4fv
#define glUniformMatrix4x3fv		_ABOpenGLProcs._glUniformMatrix4x3fv
// OpenGL 3.0
#define glColorMaski							_ABOpenGLProcs._glColorMaski
#define glGetBooleani_v							_ABOpenGLProcs._glGetBooleani_v
#define glGetIntegeri_v							_ABOpenGLProcs._glGetIntegeri_v
#define glEnablei								_ABOpenGLProcs._glEnablei
#define glDisablei								_ABOpenGLProcs._glDisablei
#define glIsEnabledi							_ABOpenGLProcs._glIsEnabledi
#define glBeginTransformFeedback				_ABOpenGLProcs._glBeginTransformFeedback
#define glEndTransformFeedback					_ABOpenGLProcs._glEndTransformFeedback
#define glBindBufferRange						_ABOpenGLProcs._glBindBufferRange
#define glBindBufferBase						_ABOpenGLProcs._glBindBufferBase
#define glTransformFeedbackVaryings				_ABOpenGLProcs._glTransformFeedbackVaryings
#define glGetTransformFeedbackVarying			_ABOpenGLProcs._glGetTransformFeedbackVarying
#define glClampColor							_ABOpenGLProcs._glClampColor
#define glBeginConditionalRender				_ABOpenGLProcs._glBeginConditionalRender
#define glEndConditionalRender					_ABOpenGLProcs._glEndConditionalRender
#define glVertexAttribIPointer					_ABOpenGLProcs._glVertexAttribIPointer
#define glGetVertexAttribIiv					_ABOpenGLProcs._glGetVertexAttribIiv
#define glGetVertexAttribIuiv					_ABOpenGLProcs._glGetVertexAttribIuiv
#define glVertexAttribI1i						_ABOpenGLProcs._glVertexAttribI1i
#define glVertexAttribI2i						_ABOpenGLProcs._glVertexAttribI2i
#define glVertexAttribI3i						_ABOpenGLProcs._glVertexAttribI3i
#define glVertexAttribI4i						_ABOpenGLProcs._glVertexAttribI4i
#define glVertexAttribI1ui						_ABOpenGLProcs._glVertexAttribI1ui
#define glVertexAttribI2ui						_ABOpenGLProcs._glVertexAttribI2ui
#define glVertexAttribI3ui						_ABOpenGLProcs._glVertexAttribI3ui
#define glVertexAttribI4ui						_ABOpenGLProcs._glVertexAttribI4ui
#define glVertexAttribI1iv						_ABOpenGLProcs._glVertexAttribI1iv
#define glVertexAttribI2iv						_ABOpenGLProcs._glVertexAttribI2iv
#define glVertexAttribI3iv						_ABOpenGLProcs._glVertexAttribI3iv
#define glVertexAttribI4iv						_ABOpenGLProcs._glVertexAttribI4iv
#define glVertexAttribI1uiv						_ABOpenGLProcs._glVertexAttribI1uiv
#define glVertexAttribI2uiv						_ABOpenGLProcs._glVertexAttribI2uiv
#define glVertexAttribI3uiv						_ABOpenGLProcs._glVertexAttribI3uiv
#define glVertexAttribI4uiv						_ABOpenGLProcs._glVertexAttribI4uiv
#define glVertexAttribI4bv						_ABOpenGLProcs._glVertexAttribI4bv
#define glVertexAttribI4sv						_ABOpenGLProcs._glVertexAttribI4sv
#define glVertexAttribI4ubv						_ABOpenGLProcs._glVertexAttribI4ubv
#define glVertexAttribI4usv						_ABOpenGLProcs._glVertexAttribI4usv
#define glGetUniformuiv							_ABOpenGLProcs._glGetUniformuiv
#define glBindFragDataLocation					_ABOpenGLProcs._glBindFragDataLocation
#define glGetFragDataLocation					_ABOpenGLProcs._glGetFragDataLocation
#define glUniform1ui							_ABOpenGLProcs._glUniform1ui
#define glUniform2ui							_ABOpenGLProcs._glUniform2ui
#define glUniform3ui							_ABOpenGLProcs._glUniform3ui
#define glUniform4ui							_ABOpenGLProcs._glUniform4ui
#define glUniform1uiv							_ABOpenGLProcs._glUniform1uiv
#define glUniform2uiv							_ABOpenGLProcs._glUniform2uiv
#define glUniform3uiv							_ABOpenGLProcs._glUniform3uiv
#define glUniform4uiv							_ABOpenGLProcs._glUniform4uiv
#define glTexParameterIiv						_ABOpenGLProcs._glTexParameterIiv
#define glTexParameterIuiv						_ABOpenGLProcs._glTexParameterIuiv
#define glGetTexParameterIiv					_ABOpenGLProcs._glGetTexParameterIiv
#define glGetTexParameterIuiv					_ABOpenGLProcs._glGetTexParameterIuiv
#define glClearBufferiv							_ABOpenGLProcs._glClearBufferiv
#define glClearBufferuiv						_ABOpenGLProcs._glClearBufferuiv
#define glClearBufferfv							_ABOpenGLProcs._glClearBufferfv
#define glClearBufferfi							_ABOpenGLProcs._glClearBufferfi
#define glGetStringi							_ABOpenGLProcs._glGetStringi
#define glIsRenderbuffer						_ABOpenGLProcs._glIsRenderbuffer
#define glBindRenderbuffer						_ABOpenGLProcs._glBindRenderbuffer
#define glDeleteRenderbuffers					_ABOpenGLProcs._glDeleteRenderbuffers
#define glGenRenderbuffers						_ABOpenGLProcs._glGenRenderbuffers
#define glRenderbufferStorage					_ABOpenGLProcs._glRenderbufferStorage
#define glGetRenderbufferParameteriv			_ABOpenGLProcs._glGetRenderbufferParameteriv
#define glIsFramebuffer							_ABOpenGLProcs._glIsFramebuffer
#define glBindFramebuffer						_ABOpenGLProcs._glBindFramebuffer
#define glDeleteFramebuffers					_ABOpenGLProcs._glDeleteFramebuffers
#define glGenFramebuffers						_ABOpenGLProcs._glGenFramebuffers
#define glCheckFramebufferStatus				_ABOpenGLProcs._glCheckFramebufferStatus
#define glFramebufferTexture1D					_ABOpenGLProcs._glFramebufferTexture1D
#define glFramebufferTexture2D					_ABOpenGLProcs._glFramebufferTexture2D
#define glFramebufferTexture3D					_ABOpenGLProcs._glFramebufferTexture3D
#define glFramebufferRenderbuffer				_ABOpenGLProcs._glFramebufferRenderbuffer
#define glGetFramebufferAttachmentParameteriv	_ABOpenGLProcs._glGetFramebufferAttachmentParameteriv
#define glGenerateMipmap						_ABOpenGLProcs._glGenerateMipmap
#define glBlitFramebuffer						_ABOpenGLProcs._glBlitFramebuffer
#define glRenderbufferStorageMultisample		_ABOpenGLProcs._glRenderbufferStorageMultisample
#define glFramebufferTextureLayer				_ABOpenGLProcs._glFramebufferTextureLayer
#define glMapBufferRange						_ABOpenGLProcs._glMapBufferRange
#define glFlushMappedBufferRange				_ABOpenGLProcs._glFlushMappedBufferRange
#define glBindVertexArray						_ABOpenGLProcs._glBindVertexArray
#define glDeleteVertexArrays					_ABOpenGLProcs._glDeleteVertexArrays
#define glGenVertexArrays						_ABOpenGLProcs._glGenVertexArrays
#define glIsVertexArray							_ABOpenGLProcs._glIsVertexArray
// OpenGL 3.1
#define glDrawArraysInstanced					_ABOpenGLProcs._glDrawArraysInstanced
#define glDrawElementsInstanced					_ABOpenGLProcs._glDrawElementsInstanced
#define glTexBuffer								_ABOpenGLProcs._glTexBuffer
#define glPrimitiveRestartIndex					_ABOpenGLProcs._glPrimitiveRestartIndex
#define glCopyBufferSubData						_ABOpenGLProcs._glCopyBufferSubData
#define glGetUniformIndices						_ABOpenGLProcs._glGetUniformIndices
#define glGetActiveUniformsiv					_ABOpenGLProcs._glGetActiveUniformsiv
#define glGetActiveUniformName					_ABOpenGLProcs._glGetActiveUniformName
#define glGetUniformBlockIndex					_ABOpenGLProcs._glGetUniformBlockIndex
#define glGetActiveUniformBlockiv				_ABOpenGLProcs._glGetActiveUniformBlockiv
#define glGetActiveUniformBlockName				_ABOpenGLProcs._glGetActiveUniformBlockName
#define glUniformBlockBinding					_ABOpenGLProcs._glUniformBlockBinding
// OpenGL 3.2
#define glDrawElementsBaseVertex				_ABOpenGLProcs._glDrawElementsBaseVertex
#define glDrawRangeElementsBaseVertex			_ABOpenGLProcs._glDrawRangeElementsBaseVertex
#define glDrawElementsInstancedBaseVertex		_ABOpenGLProcs._glDrawElementsInstancedBaseVertex
#define glMultiDrawElementsBaseVertex			_ABOpenGLProcs._glMultiDrawElementsBaseVertex
#define glProvokingVertex						_ABOpenGLProcs._glProvokingVertex
#define glFenceSync								_ABOpenGLProcs._glFenceSync
#define glIsSync								_ABOpenGLProcs._glIsSync
#define glDeleteSync							_ABOpenGLProcs._glDeleteSync
#define glClientWaitSync						_ABOpenGLProcs._glClientWaitSync
#define glWaitSync								_ABOpenGLProcs._glWaitSync
#define glGetInteger64v							_ABOpenGLProcs._glGetInteger64v
#define glGetSynciv								_ABOpenGLProcs._glGetSynciv
#define glGetInteger64i_v						_ABOpenGLProcs._glGetInteger64i_v
#define glGetBufferParameteri64v				_ABOpenGLProcs._glGetBufferParameteri64v
#define glFramebufferTexture					_ABOpenGLProcs._glFramebufferTexture
#define glTexImage2DMultisample					_ABOpenGLProcs._glTexImage2DMultisample
#define glTexImage3DMultisample					_ABOpenGLProcs._glTexImage3DMultisample
#define glGetMultisamplefv						_ABOpenGLProcs._glGetMultisamplefv
#define glSampleMaski							_ABOpenGLProcs._glSampleMaski
// OpenGL 3.3
#define glBindFragDataLocationIndexed			_ABOpenGLProcs._glBindFragDataLocationIndexed
#define glGetFragDataIndex						_ABOpenGLProcs._glGetFragDataIndex
#define glGenSamplers							_ABOpenGLProcs._glGenSamplers
#define glDeleteSamplers						_ABOpenGLProcs._glDeleteSamplers
#define glIsSampler								_ABOpenGLProcs._glIsSampler
#define glBindSampler							_ABOpenGLProcs._glBindSampler
#define glSamplerParameteri						_ABOpenGLProcs._glSamplerParameteri
#define glSamplerParameteriv					_ABOpenGLProcs._glSamplerParameteriv
#define glSamplerParameterf						_ABOpenGLProcs._glSamplerParameterf
#define glSamplerParameterfv					_ABOpenGLProcs._glSamplerParameterfv
#define glSamplerParameterIiv					_ABOpenGLProcs._glSamplerParameterIiv
#define glSamplerParameterIuiv					_ABOpenGLProcs._glSamplerParameterIuiv
#define glGetSamplerParameteriv					_ABOpenGLProcs._glGetSamplerParameteriv
#define glGetSamplerParameterIiv				_ABOpenGLProcs._glGetSamplerParameterIiv
#define glGetSamplerParameterfv					_ABOpenGLProcs._glGetSamplerParameterfv
#define glGetSamplerParameterIuiv				_ABOpenGLProcs._glGetSamplerParameterIuiv
#define glQueryCounter							_ABOpenGLProcs._glQueryCounter
#define glGetQueryObjecti64v					_ABOpenGLProcs._glGetQueryObjecti64v
#define glGetQueryObjectui64v					_ABOpenGLProcs._glGetQueryObjectui64v
#define glVertexAttribDivisor					_ABOpenGLProcs._glVertexAttribDivisor
#define glVertexAttribP1ui						_ABOpenGLProcs._glVertexAttribP1ui
#define glVertexAttribP1uiv						_ABOpenGLProcs._glVertexAttribP1uiv
#define glVertexAttribP2ui						_ABOpenGLProcs._glVertexAttribP2ui
#define glVertexAttribP2uiv						_ABOpenGLProcs._glVertexAttribP2uiv
#define glVertexAttribP3ui						_ABOpenGLProcs._glVertexAttribP3ui
#define glVertexAttribP3uiv						_ABOpenGLProcs._glVertexAttribP3uiv
#define glVertexAttribP4ui						_ABOpenGLProcs._glVertexAttribP4ui
#define glVertexAttribP4uiv						_ABOpenGLProcs._glVertexAttribP4uiv
