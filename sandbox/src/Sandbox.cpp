#include <hypermath.h>
#include <Aberration.h>

const char* vertSource= R"(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    TexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
}
)";

const char* fragSource = R"(
#version 330 core
in vec2 TexCoord;

out vec4 color;

uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;

void main()
{
    color = mix(texture(ourTexture1, TexCoord), texture(ourTexture2, TexCoord), 0.2);
}
)";

	int32 LoadShaders(AB::Engine* engine) {
		auto* gl = engine->glGetFunctions();
		int32 vertexShader;
		vertexShader = gl->_glCreateShader(GL_VERTEX_SHADER);
		gl->_glShaderSource(vertexShader, 1, &vertSource, 0);
		gl->_glCompileShader(vertexShader);

		int32 fragmentShader;
		fragmentShader = gl->_glCreateShader(GL_FRAGMENT_SHADER);
		gl->_glShaderSource(fragmentShader, 1, &fragSource, 0);
		gl->_glCompileShader(fragmentShader);

		int32 result = 0;
		gl->_glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
		if (!result) {
			int32 logLen;
			gl->_glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLen);
			char* message = (char*)alloca(logLen);
			gl->_glGetShaderInfoLog(vertexShader, logLen, NULL, message);
			AB_FATAL("Shader compilation error:\n%s", message);
		};

		gl->_glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
		if (!result) {
			int32 logLen;
			gl->_glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLen);
			char* message = (char*)alloca(logLen);
			gl->_glGetShaderInfoLog(fragmentShader, logLen, NULL, message);
			AB_FATAL("Shader compilation error:\n%s", message);
		};

		int32 shaderHandle = gl->_glCreateProgram();
		gl->_glAttachShader(shaderHandle, vertexShader);
		gl->_glAttachShader(shaderHandle, fragmentShader);
		gl->_glLinkProgram(shaderHandle);
		gl->_glGetProgramiv(shaderHandle, GL_LINK_STATUS, &result);
		if (!result) {
			int32 logLen;
			gl->_glGetProgramiv(shaderHandle, GL_INFO_LOG_LENGTH, &logLen);
			char* message = (char*)alloca(logLen);
			gl->_glGetProgramInfoLog(shaderHandle, logLen, 0, message);
			AB_FATAL("Shader compilation error:\n%s", message);
		}

		gl->_glDeleteShader(vertexShader);
		gl->_glDeleteShader(fragmentShader);
		return shaderHandle;
	}

	float z = -7.0f;
	float x = 0.0f;
	AB::GameContext* HACK;

	void KeyCallback(AB::KeyboardKey key, bool32 currState, bool32 prevState, uint32 repeatCount) {
		if (key == AB::KeyboardKey::W && currState == 1) {
			HACK->camPos  = hpm::Add(hpm::Multiply(HACK->camFront, 0.2),  HACK->camPos);
		} if (key == AB::KeyboardKey::S && currState == 1) {
			HACK->camPos = hpm::Subtract(HACK->camPos, hpm::Multiply(HACK->camFront, 0.2));
		} if (key == AB::KeyboardKey::A && currState == 1) {
			hpm::Vector3 right = hpm::Normalize(hpm::Cross(HACK->camFront, HACK->camUp));
			HACK->camPos = hpm::Subtract(HACK->camPos, hpm::Multiply(right, 0.2));
		} if (key == AB::KeyboardKey::D && currState == 1) {
			hpm::Vector3 right = hpm::Normalize(hpm::Cross(HACK->camFront, HACK->camUp));
			HACK->camPos = hpm::Add(hpm::Multiply(right, 0.2), HACK->camPos);
		}
	}

	void MouseCallback(uint32 xPos, uint32 yPos) {
		HACK->pitch += (float32)yPos - (float32)HACK->yLastMouse;
		HACK->yaw += (float32)xPos - (float32)HACK->xLastMouse;
		HACK->xLastMouse = xPos;
		HACK->yLastMouse = yPos;

		if (HACK->pitch > 89.0f)
			HACK->pitch = 89.0f;
		if (HACK->pitch < -89.0f)
			HACK->pitch = -89.0f;
	}

extern "C" {
	ABERRATION_ENTRY void GameInitialize(AB::Engine* engine, AB::GameContext* gameContext) {
		gameContext->shaderHandle = LoadShaders(engine);
		auto* gl = engine->glGetFunctions();
		HACK = gameContext;
		engine->windowSetKeyCallback(KeyCallback);
		engine->windowSetMouseMoveCallback(MouseCallback);

		float32 vertices[] = {
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
			0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

			0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
			0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
		};

		gl->_glGenBuffers(1, &gameContext->vbo);
		gl->_glBindBuffer(GL_ARRAY_BUFFER, gameContext->vbo);
		gl->_glBufferData(GL_ARRAY_BUFFER, sizeof(float32) * 180, vertices, GL_STATIC_DRAW);
		gl->_glBindVertexArray(0);

		gl->_glGenTextures(1, &gameContext->texture);
		gl->_glBindTexture(GL_TEXTURE_2D, gameContext->texture);

		gl->_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		gl->_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		gl->_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl->_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		AB::Image img = engine->loadBMP("../../../assets/test.bmp");
		gl->_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bitmap);
		engine->deleteBitmap(img.bitmap);
		gl->_glBindTexture(GL_TEXTURE_2D, 0);


		gameContext->camFront = {0, 0, -1};
		gameContext->camPos = {0, 0, 3};
		gameContext->camUp = {0, 1, 0};
		gameContext->yaw = 0.0f;
		gameContext->pitch = 0.0f;
		engine->windowGetMousePositionCallback(&gameContext->xLastMouse, &gameContext->yLastMouse);
	}

	float a = 0.0f;

	ABERRATION_ENTRY void GameRender(AB::Engine* engine, AB::GameContext* gameContext) {
		hpm::Vector3 cubePositions[] = {
			hpm::Vector3{ 0.0f,  0.0f,  0.0f },
			hpm::Vector3{ 2.0f,  5.0f, -15.0f },
			hpm::Vector3{ -1.5f, -2.2f, -2.5f },
			hpm::Vector3{ -3.8f, -2.0f, -12.3f },
			hpm::Vector3{ 2.4f, -0.4f, -3.5f },
			hpm::Vector3{ -1.7f,  3.0f, -7.5f },
			hpm::Vector3{ 1.3f, -2.0f, -2.5f },
			hpm::Vector3{ 1.5f,  2.0f, -2.5f },
			hpm::Vector3{ 1.5f,  0.2f, -1.5f },
			hpm::Vector3{ -1.3f,  1.0f, -1.5f }
		};

		auto* gl = engine->glGetFunctions();

		gl->_glActiveTexture(GL_TEXTURE0);
		gl->_glBindTexture(GL_TEXTURE_2D, gameContext->texture);
		gl->_glUseProgram(gameContext->shaderHandle);
		gl->_glUniform1i(gl->_glGetUniformLocation(gameContext->shaderHandle, "ourTexture1"), 0);

		//hpm::Matrix4 view = hpm::Translation({ x, 0.0f, z });
		float32 xCam = hpm::Sin(a * 0.01f) * 10.2f;
		float32 zCam = hpm::Cos(a * 0.01f) * 10.2f;
		gameContext->camFront.x = hpm::Cos(hpm::ToRadians(gameContext->pitch)) * hpm::Cos(hpm::ToRadians(gameContext->yaw));
		gameContext->camFront.y = hpm::Sin(hpm::ToRadians(gameContext->pitch));
		gameContext->camFront.z = hpm::Cos(hpm::ToRadians(gameContext->pitch)) * hpm::Sin(hpm::ToRadians(gameContext->yaw));
		gameContext->camFront = hpm::Normalize(gameContext->camFront);
		hpm::Matrix4 view = hpm::LookAtRH(gameContext->camPos, 
									hpm::Add(gameContext->camPos, gameContext->camFront),
									gameContext->camUp);
		char buffer[512];
		// TODO: Debug functions for printing vectors
		engine->formatString(buffer, 128, "camPos: %f32 %f32 %f32\ncamFront: %f32 %f32 %f32\ncamUp: %f32 %f32 %f32\nyaw: %f32\npitch: %f32", gameContext->camPos.x, gameContext->camPos.y, gameContext->camPos.z,
								gameContext->camFront.x, gameContext->camFront.y, gameContext->camFront.z, gameContext->camUp.x, gameContext->camUp.y, gameContext->camUp.z, gameContext->yaw, gameContext->pitch);
		engine->debugDrawString({ 100, 100 }, 20, 0xffffffff, buffer);
		hpm::Matrix4 proj = hpm::PerspectiveRH(90.0f, 800.0f / 600.0f, 0.1f, 100.0f);
		//hpm::Matrix4 proj = hpm::OrthogonalRH(0, 8, 0, 6, 0.1, 100);


		int32 modelLoc = gl->_glGetUniformLocation(gameContext->shaderHandle, "model");
		int32 viewLoc = gl->_glGetUniformLocation(gameContext->shaderHandle, "view");
		int32 projLoc = gl->_glGetUniformLocation(gameContext->shaderHandle, "projection");
		
		gl->_glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.data);
		gl->_glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj.data);
		
		gl->_glBindBuffer(GL_ARRAY_BUFFER, gameContext->vbo);
		gl->_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		gl->_glEnableVertexAttribArray(0);
		gl->_glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		gl->_glEnableVertexAttribArray(2);
		gl->_glEnable(GL_DEPTH_TEST);
		for (GLuint i = 0; i < 10; i++) {
			hpm::Matrix4 tr = hpm::Translation(cubePositions[i]);
			GLfloat angle = 20.0f * i;
			hpm::Matrix4 rt = tr;//hpm::Rotate(tr, a, { 1.0f, 0.3f, 0.5f });
			hpm::Matrix4 sc = hpm::Identity4();//hpm::Scaling({1, 1, 1});
			rt = hpm::Scale(rt, { 2, 3, 0.2 });

			a += 0.1f;
			hpm::Matrix4 model = hpm::Multiply(rt, sc);
			gl->_glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.data);

			gl->_glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		gl->_glDisable(GL_DEPTH_TEST);

	}
	ABERRATION_ENTRY void GameUpdate(AB::Engine* engine, AB::GameContext* gameContext) {
		//engine->printString("%f32 %f32 %f32 %f32\n%f32 %f32 %f32 %f32\n%f32 %f32 %f32 %f32\n%f32 %f32 %f32 %f32\n",  mr._11, mr._12, mr._13, mr._14,
		//																												mr._21, mr._22, mr._23, mr._24,
		//																												mr._31, mr._32, mr._33, mr._34,
		//																										mr._41, mr._42, mr._43, mr._44);
	}
}
