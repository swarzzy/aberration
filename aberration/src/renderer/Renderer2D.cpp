#include "Renderer2D.h"
#include "src/utils/Log.h"
#include <hypermath.h>
#include "src/platform/API/OpenGL/ABOpenGL.h"
#include "src/platform/Window.h"
#include "src/utils/ImageLoader.h"

namespace AB {

	const char* VERTEX_SOURCE = "#version 330 core\n"
		"layout (location = 0) in vec2 aPos;\n"
		"layout (location = 1) in vec4 aColor;\n"
		"layout (location = 2) in vec2 aUV;\n"
		"out vec4 v_Color;\n"
		"out vec2 v_UV;\n"
		"void main()\n"
		"{\n"
		"	v_UV = aUV;\n"
		"	v_Color = aColor;\n"
		"   gl_Position = vec4(aPos, 1.0,  1.0);\n"
		"}\0";

	const char* FRAGMENT_SOURCE = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec4 v_Color;\n"
		"in vec2 v_UV;\n"
		"uniform sampler2D tex;\n"
		"void main()\n"
		"{\n"
		"   FragColor = texture(tex, v_UV);\n"
		"}\n\0";

	struct VertexData {
		float32 x;
		float32 y;
		color32 color;
		float32 u;
		float32 v;
	};
	
	struct Renderer2DProperties {
		uint32 GLVBOHandle;
		uint32 shaderHandle;
		uint32 GLIBOHandle;
		uint64 vertexCount;
		uint64 indexCount;
		hpm::Vector2 viewSpaceDim;
		VertexData* vertexBuffer;
		// TODO: TEMPORARY
		uint32 textures[128];
		uint32 texturesUsed;
	};

	static void _GLInit(Renderer2DProperties* properties);

	void Renderer2D::Initialize(uint32 drawableSpaceX, uint32 drawableSpaceY) {
		if (!s_Properties) {
			s_Properties = (Renderer2DProperties*)std::malloc(sizeof(Renderer2DProperties));
			memset(s_Properties, 0, sizeof(Renderer2DProperties));
		} else {
			AB_CORE_WARN("2D renderer already initialized.");
		}
		_GLInit(s_Properties);
		s_Properties->viewSpaceDim = hpm::Vector2((float32)drawableSpaceX, (float32)drawableSpaceY);
	}

	void Renderer2D::Destroy() {
		AB_GLCALL(glDeleteBuffers(1, &s_Properties->GLVBOHandle));
		AB_GLCALL(glDeleteBuffers(1, &s_Properties->GLIBOHandle));
		AB_GLCALL(glDeleteProgram(s_Properties->shaderHandle));
		std::free(s_Properties->vertexBuffer);
		std::free(s_Properties);
		s_Properties = nullptr;
	}

	uint32 Renderer2D::LoadTexture(const char* filepath) {
		Image image = LoadBMP(filepath);
		if (image.bitmap) {
			GLuint texHandle;
			uint32 format = GL_RED;
			uint32 inFormat = GL_RED;
			switch(image.format) {
				case PixelFormat::RGB: {
					format = GL_RGB;
					inFormat = GL_RGB8;
				} break;
				case PixelFormat::RGBA: {
					format = GL_RGBA;
					inFormat = GL_RGBA8;
				} break;
				default: {
					// TODO: FIX THIS
					AB_CORE_ERROR("Wrong image format");
				} break;
			}
			AB_GLCALL(glGenTextures(1, &texHandle));
			AB_GLCALL(glBindTexture(GL_TEXTURE_2D, texHandle));

			AB_GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
			AB_GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
			AB_GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			AB_GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			AB_GLCALL(glTexImage2D(	GL_TEXTURE_2D, 0, inFormat,	image.width, image.height, 0, format, GL_UNSIGNED_BYTE,	image.bitmap));

			AB_GLCALL(glBindTexture(GL_TEXTURE_2D, 0));

			DeleteBitmap(image.bitmap);

			s_Properties->textures[s_Properties->texturesUsed] = texHandle;
			s_Properties->texturesUsed++;
			return s_Properties->texturesUsed - 1;

		} else {
			AB_CORE_ERROR("Failed to load texture. Cannot load image: ", filepath);
			return 0;
		}
	}

	void Renderer2D::DrawRectangle(hpm::Vector2 position, float32 angle, float32 anchor, hpm::Vector2 size, color32 color) {
		float32 sin = hpm::Sin(hpm::ToRadians(angle));
		float32 cos = hpm::Cos(hpm::ToRadians(angle));
		float32 invHalfW = (2 / s_Properties->viewSpaceDim.x);
		float32 invHalfH = (2 / s_Properties->viewSpaceDim.y);

		// rotation			// scale // translation	// ortho projection				   
		float32 originLBX = (((0 + anchor) * cos - (0 + anchor) * sin) * size.x + position.x) * invHalfW - 1;
		float32 originLBY = (((0 + anchor) * sin + (0 + anchor) * cos) * size.y + position.y) * invHalfH - 1;
		float32 originRBX = (((1 + anchor) * cos - (0 + anchor) * sin) * size.x + position.x) * invHalfW - 1;
		float32 originRBY = (((1 + anchor) * sin + (0 + anchor) * cos) * size.y + position.y) * invHalfH - 1;
		float32 originRTX = (((1 + anchor) * cos - (1 + anchor) * sin) * size.x + position.x) * invHalfW - 1;
		float32 originRTY = (((1 + anchor) * sin + (1 + anchor) * cos) * size.y + position.y) * invHalfH - 1;
		float32 originLTX = (((0 + anchor) * cos - (1 + anchor) * sin) * size.x + position.x) * invHalfW - 1;
		float32 originLTY = (((0 + anchor) * sin + (1 + anchor) * cos) * size.y + position.y) * invHalfH - 1;

		s_Properties->vertexBuffer[s_Properties->vertexCount].x = originLBX;
		s_Properties->vertexBuffer[s_Properties->vertexCount].y = originLBY;
		s_Properties->vertexBuffer[s_Properties->vertexCount].color = color;
		s_Properties->vertexBuffer[s_Properties->vertexCount].u = 0.0f;
		s_Properties->vertexBuffer[s_Properties->vertexCount].v = 0.0f;
		s_Properties->vertexCount++;
		//right bottom
		s_Properties->vertexBuffer[s_Properties->vertexCount].x = originRBX;
		s_Properties->vertexBuffer[s_Properties->vertexCount].y = originRBY;
		s_Properties->vertexBuffer[s_Properties->vertexCount].color = color;
		s_Properties->vertexBuffer[s_Properties->vertexCount].u = 1.0f;
		s_Properties->vertexBuffer[s_Properties->vertexCount].v = 0.0f;
		s_Properties->vertexCount++;
		//right top
		s_Properties->vertexBuffer[s_Properties->vertexCount].x = originRTX;
		s_Properties->vertexBuffer[s_Properties->vertexCount].y = originRTY;
		s_Properties->vertexBuffer[s_Properties->vertexCount].color = color;
		s_Properties->vertexBuffer[s_Properties->vertexCount].u = 1.0f;
		s_Properties->vertexBuffer[s_Properties->vertexCount].v = 1.0f;
		s_Properties->vertexCount++;
		// left top
		s_Properties->vertexBuffer[s_Properties->vertexCount].x = originLTX;
		s_Properties->vertexBuffer[s_Properties->vertexCount].y = originLTY;
		s_Properties->vertexBuffer[s_Properties->vertexCount].color = color;
		s_Properties->vertexBuffer[s_Properties->vertexCount].u = 0.0f;
		s_Properties->vertexBuffer[s_Properties->vertexCount].v = 1.0f;
		s_Properties->vertexCount++;

		s_Properties->indexCount += 6;
	}

	void Renderer2D::Flush() {
		AB_GLCALL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
		AB_GLCALL(glClear(GL_COLOR_BUFFER_BIT));

		AB_GLCALL(glBindBuffer(GL_ARRAY_BUFFER, s_Properties->GLVBOHandle));
		AB_GLCALL(glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * s_Properties->vertexCount, (void*)s_Properties->vertexBuffer, GL_DYNAMIC_DRAW));
		AB_GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_Properties->GLIBOHandle));

		AB_GLCALL(glEnableVertexAttribArray(0));
		AB_GLCALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0));
		AB_GLCALL(glEnableVertexAttribArray(1));
		AB_GLCALL(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexData), (void*)(sizeof(float32) * 2)));
		AB_GLCALL(glEnableVertexAttribArray(2));
		AB_GLCALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(float32) * 2 + sizeof(float) * 1)));

		AB_GLCALL(glUseProgram(s_Properties->shaderHandle));
		AB_GLCALL(glActiveTexture(GL_TEXTURE0));
		AB_GLCALL(glBindTexture(GL_TEXTURE_2D, s_Properties->textures[0]));

		AB_GLCALL(glUniform1i(glGetUniformLocation(s_Properties->shaderHandle, "tex"), 0));

		AB_GLCALL(glDrawElements(GL_TRIANGLES, (GLsizei)s_Properties->indexCount, GL_UNSIGNED_SHORT, 0));
		//AB_GLCALL(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)g_VertexCount));
		s_Properties->vertexCount = 0;
		s_Properties->indexCount = 0;
	}

	void _GLInit(Renderer2DProperties* properties) {
		uint32 winWidth;
		uint32 winHeight;
		Window::GetSize(winWidth, winHeight);
		AB_GLCALL(glViewport(0, 0, winWidth, winHeight));

		AB_GLCALL(glGenBuffers(1, &properties->GLVBOHandle));
		properties->vertexBuffer = (VertexData*)malloc(AB::Renderer2D::VERTEX_BUFFER_SIZE);
		properties->vertexCount = 0;

		uint16* indices = (uint16*)std::malloc(Renderer2D::INDEX_BUFFER_SIZE * sizeof(uint16));
		uint16 k = 0;
		for (uint32 i = 0; i < Renderer2D::INDEX_BUFFER_SIZE - 9; i += 6) {
			indices[i] = k;
			indices[i + 1] = k + 1;
			indices[i + 2] = k + 3;
			indices[i + 3] = k + 1;
			indices[i + 4] = k + 2;
			indices[i + 5] = k + 3;
			k += 4;
		}
		indices[Renderer2D::INDEX_BUFFER_SIZE - 1] = k;
		indices[Renderer2D::INDEX_BUFFER_SIZE - 2] = k + 1;

		AB_GLCALL(glGenBuffers(1, &properties->GLIBOHandle));
		AB_GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, properties->GLIBOHandle));
		AB_GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16) * 65536, indices, GL_STATIC_DRAW));
		AB_GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

		std::free(indices);

		int32 vertexShader;
		AB_GLCALL(vertexShader = glCreateShader(GL_VERTEX_SHADER));
		AB_GLCALL(glShaderSource(vertexShader, 1, &VERTEX_SOURCE, 0));
		AB_GLCALL(glCompileShader(vertexShader));

		int32 fragmentShader;
		AB_GLCALL(fragmentShader = glCreateShader(GL_FRAGMENT_SHADER));
		AB_GLCALL(glShaderSource(fragmentShader, 1, &FRAGMENT_SOURCE, 0));
		AB_GLCALL(glCompileShader(fragmentShader));

		int32 result = 0;
		AB_GLCALL(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result));
		if (!result) {
			int32 logLen;
			AB_GLCALL(glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLen));
			char* message = (char*)alloca(logLen);
			AB_GLCALL(glGetShaderInfoLog(vertexShader, logLen, NULL, message));
			AB_CORE_FATAL("Shader compilation error:\n", message);
		};

		AB_GLCALL(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result));
		if (!result) {
			int32 logLen;
			AB_GLCALL(glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLen));
			char* message = (char*)alloca(logLen);
			AB_GLCALL(glGetShaderInfoLog(fragmentShader, logLen, NULL, message));
			AB_CORE_FATAL("Shader compilation error:\n", message);
		};

		AB_GLCALL(properties->shaderHandle = glCreateProgram());
		AB_GLCALL(glAttachShader(properties->shaderHandle, vertexShader));
		AB_GLCALL(glAttachShader(properties->shaderHandle, fragmentShader));
		AB_GLCALL(glLinkProgram(properties->shaderHandle));
		AB_GLCALL(glGetProgramiv(properties->shaderHandle, GL_LINK_STATUS, &result));
		if (!result) {
			int32 logLen;
			AB_GLCALL(glGetProgramiv(properties->shaderHandle, GL_INFO_LOG_LENGTH, &logLen));
			char* message = (char*)alloca(logLen);
			AB_GLCALL(glGetProgramInfoLog(properties->shaderHandle, logLen, 0, message));
			AB_CORE_FATAL("Shader compilation error:\n", message);
		}

		AB_GLCALL(glDeleteShader(vertexShader));
		AB_GLCALL(glDeleteShader(fragmentShader));
	}


	void RenderGroupResizeCallback(uint32 width, uint32 height) {
		if (Renderer2D::s_Properties) {
			//Renderer2D::s_Properties->viewSpaceDim.x = (float32)width;
			//Renderer2D::s_Properties->viewSpaceDim.y = (float32)height;

			AB_GLCALL(glViewport(0, 0, width, height));
		}
	}
}
