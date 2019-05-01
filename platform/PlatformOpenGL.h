#pragma once
#include "AB.h"
#include "OpenGL.h"
#include "Memory.h"

namespace AB
{
	struct LoadFunctionsResult
	{
		GLFuncTable* funcTable;
		b32 success;
	};
	
	LoadFunctionsResult OpenGLLoadFunctions(MemoryArena* memoryArena);
	void InitOpenGL(GLFuncTable* funcTable);
}
