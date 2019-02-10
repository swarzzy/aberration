#include "ABOpenGL.h"

ABGLProcs _ABOpenGLProcs = {};

static const char* procNames[AB_OPENGL_FUNCTIONS_COUNT] = {
	// 1.0
	"glCullFace",
	"glFrontFace",
	"glHint",
	"glLineWidth",
	"glPointSize",
	"glPolygonMode",
	"glScissor",
	"glTexParameterf",
	"glTexParameterfv",
	"glTexParameteri",
	"glTexParameteriv",
	"glTexImage1D",
	"glTexImage2D",
	"glDrawBuffer",
	"glClear",
	"glClearColor",
	"glClearStencil",
	"glClearDepth",
	"glStencilMask",
	"glColorMask",
	"glDepthMask",
	"glDisable",
	"glEnable",
	"glFinish",
	"glFlush",
	"glBlendFunc",
	"glLogicOp",
	"glStencilFunc",
	"glStencilOp",
	"glDepthFunc",
	"glPixelStoref",
	"glPixelStorei",
	"glReadBuffer",
	"glReadPixels",
	"glGetBooleanv",
	"glGetDoublev",
	"glGetError",
	"glGetFloatv",
	"glGetIntegerv",
	"glGetString",
	"glGetTexImage",
	"glGetTexParameterfv",
	"glGetTexParameteriv",
	"glGetTexLevelParameterfv",
	"glGetTexLevelParameteriv",
	"glIsEnabled",
	"glDepthRange",
	"glViewport",
	// 1.1
	"glDrawArrays",
	"glDrawElements",
	"glGetPointerv",
	"glPolygonOffset",
	"glCopyTexImage1D",
	"glCopyTexImage2D",
	"glCopyTexSubImage1D",
	"glCopyTexSubImage2D",
	"glTexSubImage1D",
	"glTexSubImage2D",
	"glBindTexture",
	"glDeleteTextures",
	"glGenTextures",
	"glIsTexture",
	// 1.2
	"glDrawRangeElements",
	"glTexImage3D",
	"glTexSubImage3D",
	"glCopyTexSubImage3D",
	//1.3
	"glActiveTexture",
	"glSampleCoverage",
	"glCompressedTexImage3D",
	"glCompressedTexImage2D",
	"glCompressedTexImage1D",
	"glCompressedTexSubImage3D",
	"glCompressedTexSubImage2D",
	"glCompressedTexSubImage1D",
	"glGetCompressedTexImage",
	// 1.4
	"glBlendFuncSeparate",
	"glMultiDrawArrays",
	"glMultiDrawElements",
	"glPointParameterf",
	"glPointParameterfv",
	"glPointParameteri",
	"glPointParameteriv",
	"glBlendColor",
	"glBlendEquation",
	// 1.5
	"glGenQueries",
	"glDeleteQueries",
	"glIsQuery",
	"glBeginQuery",
	"glEndQuery",
	"glGetQueryiv",
	"glGetQueryObjectiv",
	"glGetQueryObjectuiv",
	"glBindBuffer",
	"glDeleteBuffers",
	"glGenBuffers",
	"glIsBuffer",
	"glBufferData",
	"glBufferSubData",
	"glGetBufferSubData",
	"glMapBuffer",
	"glUnmapBuffer",
	"glGetBufferParameteriv",
	"glGetBufferPointerv",
	// 2.0
	"glBlendEquationSeparate",
	"glDrawBuffers",
	"glStencilOpSeparate",
	"glStencilFuncSeparate",
	"glStencilMaskSeparate",
	"glAttachShader",
	"glBindAttribLocation",
	"glCompileShader",
	"glCreateProgram",
	"glCreateShader",
	"glDeleteProgram",
	"glDeleteShader",
	"glDetachShader",
	"glDisableVertexAttribArray",
	"glEnableVertexAttribArray",
	"glGetActiveAttrib",
	"glGetActiveUniform",
	"glGetAttachedShaders",
	"glGetAttribLocation",
	"glGetProgramiv",
	"glGetProgramInfoLog",
	"glGetShaderiv",
	"glGetShaderInfoLog",
	"glGetShaderSource",
	"glGetUniformLocation",
	"glGetUniformfv",
	"glGetUniformiv",
	"glGetVertexAttribdv",
	"glGetVertexAttribfv",
	"glGetVertexAttribiv",
	"glGetVertexAttribPointerv",
	"glIsProgram",
	"glIsShader",
	"glLinkProgram",
	"glShaderSource",
	"glUseProgram",
	"glUniform1f",
	"glUniform2f",
	"glUniform3f",
	"glUniform4f",
	"glUniform1i",
	"glUniform2i",
	"glUniform3i",
	"glUniform4i",
	"glUniform1fv",
	"glUniform2fv",
	"glUniform3fv",
	"glUniform4fv",
	"glUniform1iv",
	"glUniform2iv",
	"glUniform3iv",
	"glUniform4iv",
	"glUniformMatrix2fv",
	"glUniformMatrix3fv",
	"glUniformMatrix4fv",
	"glValidateProgram",
	"glVertexAttrib1d",
	"glVertexAttrib1dv",
	"glVertexAttrib1f",
	"glVertexAttrib1fv",
	"glVertexAttrib1s",
	"glVertexAttrib1sv",
	"glVertexAttrib2d",
	"glVertexAttrib2dv",
	"glVertexAttrib2f",
	"glVertexAttrib2fv",
	"glVertexAttrib2s",
	"glVertexAttrib2sv",
	"glVertexAttrib3d",
	"glVertexAttrib3dv",
	"glVertexAttrib3f",
	"glVertexAttrib3fv",
	"glVertexAttrib3s",
	"glVertexAttrib3sv",
	"glVertexAttrib4Nbv",
	"glVertexAttrib4Niv",
	"glVertexAttrib4Nsv",
	"glVertexAttrib4Nub",
	"glVertexAttrib4Nubv",
	"glVertexAttrib4Nuiv",
	"glVertexAttrib4Nusv",
	"glVertexAttrib4bv",
	"glVertexAttrib4d",
	"glVertexAttrib4dv",
	"glVertexAttrib4f",
	"glVertexAttrib4fv",
	"glVertexAttrib4iv",
	"glVertexAttrib4s",
	"glVertexAttrib4sv",
	"glVertexAttrib4ubv",
	"glVertexAttrib4uiv",
	"glVertexAttrib4usv",
	"glVertexAttribPointer",
	// 2.1
	"glUniformMatrix2x3fv",
	"glUniformMatrix3x2fv",
	"glUniformMatrix2x4fv",
	"glUniformMatrix4x2fv",
	"glUniformMatrix3x4fv",
	"glUniformMatrix4x3fv",
	// 3.0
	"glColorMaski",									
	"glGetBooleani_v",								
	"glGetIntegeri_v",								
	"glEnablei",									
	"glDisablei",									
	"glIsEnabledi",									
	"glBeginTransformFeedback",						
	"glEndTransformFeedback",						
	"glBindBufferRange",							
	"glBindBufferBase",								
	"glTransformFeedbackVaryings",					
	"glGetTransformFeedbackVarying",				
	"glClampColor",									
	"glBeginConditionalRender",						
	"glEndConditionalRender",						
	"glVertexAttribIPointer",						
	"glGetVertexAttribIiv",							
	"glGetVertexAttribIuiv",						
	"glVertexAttribI1i",							
	"glVertexAttribI2i",							
	"glVertexAttribI3i",							
	"glVertexAttribI4i",							
	"glVertexAttribI1ui",							
	"glVertexAttribI2ui",							
	"glVertexAttribI3ui",							
	"glVertexAttribI4ui",							
	"glVertexAttribI1iv",							
	"glVertexAttribI2iv",							
	"glVertexAttribI3iv",							
	"glVertexAttribI4iv",							
	"glVertexAttribI1uiv",							
	"glVertexAttribI2uiv",							
	"glVertexAttribI3uiv",							
	"glVertexAttribI4uiv",							
	"glVertexAttribI4bv",							
	"glVertexAttribI4sv",							
	"glVertexAttribI4ubv",							
	"glVertexAttribI4usv",							
	"glGetUniformuiv",								
	"glBindFragDataLocation",						
	"glGetFragDataLocation",						
	"glUniform1ui",									
	"glUniform2ui",									
	"glUniform3ui",									
	"glUniform4ui",									
	"glUniform1uiv",								
	"glUniform2uiv",								
	"glUniform3uiv",								
	"glUniform4uiv",								
	"glTexParameterIiv",							
	"glTexParameterIuiv",							
	"glGetTexParameterIiv",							
	"glGetTexParameterIuiv",						
	"glClearBufferiv",								
	"glClearBufferuiv",								
	"glClearBufferfv",								
	"glClearBufferfi",								
	"glGetStringi",									
	"glIsRenderbuffer",								
	"glBindRenderbuffer",							
	"glDeleteRenderbuffers",						
	"glGenRenderbuffers",							
	"glRenderbufferStorage",						
	"glGetRenderbufferParameteriv",					
	"glIsFramebuffer",								
	"glBindFramebuffer",							
	"glDeleteFramebuffers",							
	"glGenFramebuffers",							
	"glCheckFramebufferStatus",						
	"glFramebufferTexture1D",						
	"glFramebufferTexture2D",						
	"glFramebufferTexture3D",						
	"glFramebufferRenderbuffer",					
	"glGetFramebufferAttachmentParameteriv",		
	"glGenerateMipmap",								
	"glBlitFramebuffer",							
	"glRenderbufferStorageMultisample",				
	"glFramebufferTextureLayer",					
	"glMapBufferRange",								
	"glFlushMappedBufferRange",						
	"glBindVertexArray",							
	"glDeleteVertexArrays",							
	"glGenVertexArrays",							
	"glIsVertexArray",								
	// 3.1
	"glDrawArraysInstanced",
	"glDrawElementsInstanced",
	"glTexBuffer",
	"glPrimitiveRestartIndex",
	"glCopyBufferSubData",
	"glGetUniformIndices",
	"glGetActiveUniformsiv",
	"glGetActiveUniformName",
	"glGetUniformBlockIndex",
	"glGetActiveUniformBlockiv",
	"glGetActiveUniformBlockName",
	"glUniformBlockBinding",
	// 3.2
	"glDrawElementsBaseVertex",
	"glDrawRangeElementsBaseVertex",
	"glDrawElementsInstancedBaseVertex",
	"glMultiDrawElementsBaseVertex",
	"glProvokingVertex",
	"glFenceSync",
	"glIsSync",
	"glDeleteSync",
	"glClientWaitSync",
	"glWaitSync",
	"glGetInteger64v",
	"glGetSynciv",
	"glGetInteger64i_v",
	"glGetBufferParameteri64v",
	"glFramebufferTexture",
	"glTexImage2DMultisample",
	"glTexImage3DMultisample",
	"glGetMultisamplefv",
	"glSampleMaski",
	// 3.3
	"glBindFragDataLocationIndexed",
	"glGetFragDataIndex",
	"glGenSamplers",
	"glDeleteSamplers",
	"glIsSampler",
	"glBindSampler",
	"glSamplerParameteri",
	"glSamplerParameteriv",
	"glSamplerParameterf",
	"glSamplerParameterfv",
	"glSamplerParameterIiv",
	"glSamplerParameterIuiv",
	"glGetSamplerParameteriv",
	"glGetSamplerParameterIiv",
	"glGetSamplerParameterfv",
	"glGetSamplerParameterIuiv",
	"glQueryCounter",
	"glGetQueryObjecti64v",
	"glGetQueryObjectui64v",
	"glVertexAttribDivisor",
	"glVertexAttribP1ui",
	"glVertexAttribP1uiv",
	"glVertexAttribP2ui",
	"glVertexAttribP2uiv",
	"glVertexAttribP3ui",
	"glVertexAttribP3uiv",
	"glVertexAttribP4ui",
	"glVertexAttribP4uiv",
};

#if defined(AB_PLATFORM_WINDOWS)
#include <Windows.h>

static HMODULE g_dllHandle = 0;

bool32 AB::GL::LoadFunctions() {
	
	bool32 success = 1;
	for (unsigned int i = 0; i < AB_OPENGL_FUNCTIONS_COUNT; i++) {
		_ABOpenGLProcs.procs[i] = wglGetProcAddress(procNames[i]);
		if (_ABOpenGLProcs.procs[i] == 0 ||
			_ABOpenGLProcs.procs[i] == (void*)0x1 ||
			_ABOpenGLProcs.procs[i] == (void*)0x2 ||
			_ABOpenGLProcs.procs[i] == (void*)0x3 ||
			_ABOpenGLProcs.procs[i] == (void*)-1) 
		{
			if (!g_dllHandle) {
				g_dllHandle = LoadLibrary("opengl32.dll");
			}
			if (g_dllHandle)
				_ABOpenGLProcs.procs[i] = (AB_GLFUNCPTR)GetProcAddress(g_dllHandle, procNames[i]);
			else
				_ABOpenGLProcs.procs[i] = NULL;
			if (_ABOpenGLProcs.procs[i] == NULL) {
				AB_CORE_ERROR("ERROR: Failed to load OpenGL procedure: %s\n", procNames[i]);
				success = 0;
			}
		}
	}
		return success;
}

#elif defined(AB_PLATFORM_LINUX)
#include <dlfcn.h>

typedef void* def_glXGetProcAddress(const GLubyte *procname);
inline static def_glXGetProcAddress* glXGetProcAddress = nullptr;

bool32 AB::GL::LoadFunctions() {

	if (!glXGetProcAddress) {
		void* libgl = dlopen("libGL.so.1", RTLD_LAZY | RTLD_LOCAL);
		if (!libgl)
			return 0;
		glXGetProcAddress = (def_glXGetProcAddress*)dlsym(libgl, "glXGetProcAddress");
		if (!glXGetProcAddress)
			return 0;
		// TODO: Should dlclose be here?
	}

	bool32 success = 1;
	for (unsigned int i = 0; i < AB_OPENGL_FUNCTIONS_COUNT; i++) {
		_ABOpenGLProcs.procs[i] = (AB_GLFUNCPTR)glXGetProcAddress((const uchar*)procNames[i]);
		if (_ABOpenGLProcs.procs[i] == NULL) {
			// TODO:  get pid of printf
			AB_CORE_ERROR("ERROR: Failed to load OpenGL procedure: %s\n", procNames[i]);
			success = 0;
		}
	}
	return success;
}

#endif

namespace AB::GL {

	void InitAPI() {
		uint32 globalVAO;
		AB_GLCALL(glGenVertexArrays(1, &globalVAO));
		AB_GLCALL(glBindVertexArray(globalVAO));
		AB_GLCALL(glEnable(GL_BLEND));
		AB_GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		AB_GLCALL(glBlendEquation(GL_FUNC_ADD));
		AB_GLCALL(glEnable(GL_DEPTH_TEST));
		AB_GLCALL(glDepthMask(GL_FALSE));
		AB_GLCALL(glDepthFunc(GL_GREATER));
	}	

	static constexpr uint32 LOG_BUFFER_SIZE = 512;
	static char g_LogBuffer[LOG_BUFFER_SIZE];

	AB_API void _ClearErrorQueue() {
		while (_ABOpenGLProcs._glGetError() != GL_NO_ERROR);
	}

	AB_API bool32 _PeekError() {
		GLenum errorCode = _ABOpenGLProcs._glGetError();
		if (errorCode == GL_NO_ERROR)
			return false;

		const char* error;
		switch (errorCode) {
		case GL_INVALID_ENUM: { error = "GL_INVALID_ENUM"; } break;
		case GL_INVALID_VALUE: { error = "GL_INVALID_VALUE"; } break;
		case GL_INVALID_OPERATION: { error = "GL_INVALID_OPERATION"; } break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: { error = "GL_INVALID_FRAMEBUFFER_OPERATION"; } break;
		case GL_OUT_OF_MEMORY: { error = "GL_OUT_OF_MEMORY"; } break;
		default: {error = "UNKNOWN_ERROR"; } break;
		}

		/*if (size < 256) {
			if (size >= 16)
				buffer = "BUFFER OVERFLOW";
			else
				*buffer = '\0';
		}*/
		// TODO: This is actually unsafe
		FormatString(g_LogBuffer, LOG_BUFFER_SIZE, "A error caused by OpenGL call. Error: %s, code: %i32.", error, errorCode);
		return true;
	}

	AB_API void _GetLog(char* buffer, uint64 size) {
		if (size >= LOG_BUFFER_SIZE)
			memcpy(buffer, g_LogBuffer, LOG_BUFFER_SIZE);
		else
			*buffer = '\0';
	}
}