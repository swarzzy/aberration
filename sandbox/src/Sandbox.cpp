#include <hypermath.h>
#include <Aberration.h>
#include "Application.h"
#include "InputManager.h"
#include "platform/Platform.h"

const char* vertSource= R"(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;

out vec2 TexCoord;
out vec3 norm;
out vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
	
	pos = (model * vec4(position, 1.0f)).xyz;
	// TODO: move it to cpu
	norm = mat3(transpose(inverse(model))) * normal;
    gl_Position = projection * view * model * vec4(position, 1.0f);
    TexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
}
)";

const char* fragSource = R"(
#version 330 core
in vec2 TexCoord;
in vec3 norm;
in vec3 pos;
out vec4 color;

#define POINT_LIGHTS_NUMBER  2

struct Material {
	sampler2D diffuse_map;
	sampler2D spec_map;
	float shininess;
};

struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float linear;
	float quadratic;
};

struct DirLight {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform Light lights[POINT_LIGHTS_NUMBER];
uniform DirLight dir_light;

uniform vec3 view_pos;

vec3 DirectionalLight(DirLight light, vec3 normal, vec3 view_dir, vec3 diff_sample, vec3 spec_sample) {
	vec3 light_dir = normalize(-light.direction);
	vec3 light_dir_reflected = reflect(-light_dir, normal);	

	float Kd = max(dot(normal, light_dir), 0.0);
	float Ks = pow(max(dot(view_dir, light_dir_reflected ), 0.0), material.shininess);
	
	vec3 ambient = light.ambient * diff_sample ;
	vec3 diffuse = Kd * light.diffuse * diff_sample;
	vec3 specular = Ks * light.specular * spec_sample;
	return ambient + diffuse + specular;
}

vec3 PointLight(Light light, vec3 normal, vec3 view_dir, vec3 diff_sample, vec3 spec_sample) {
	float distance = length(light.position - pos);
	float attenuation = 1.0f / (1.0f + 
								light.linear * distance + 
								light.quadratic * distance * distance);
	
	vec3 light_dir = normalize(light.position - pos);
	vec3 reflect_dir = reflect(-light_dir, normal);
	float Kd = max(dot(normal, light_dir), 0.0);
	float Ks = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
	vec3 ambient = light.ambient * diff_sample * attenuation;
	vec3 diffuse = light.diffuse * Kd * diff_sample * attenuation;
	vec3 specular = Ks * light.specular * spec_sample * attenuation;
	return ambient + diffuse + specular;
}

void main()
{
	vec3 normal = normalize(norm);
	vec3 view_dir = normalize(view_pos - pos);

	vec4 diff_texel = texture(material.diffuse_map, TexCoord);
	vec3 diff_sample = diff_texel.xyz;
	vec3 spec_sample = texture(material.spec_map, TexCoord).xyz;

	vec3 directional = DirectionalLight(dir_light, normal, view_dir, diff_sample, spec_sample);

	vec3 point;
	for (int i = 0; i < POINT_LIGHTS_NUMBER; i++) {
		point += PointLight(lights[i], normal, view_dir, diff_sample, spec_sample);
	}

	vec3 sum_point = clamp(point, 0.0f, 1.0f);

	color = vec4(sum_point + directional, diff_texel.a);
}
)";

const char* lightVertSource = R"(
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

const char* lightFragSource = R"(
#version 330 core
in vec2 TexCoord;

out vec4 color;

uniform vec3 lightColor;
void main()
{
    color = vec4(lightColor, 1.0f);
}
)";

	int32 LoadShader(AB::Engine* engine, const char* vSrc, const char* fSrc) {
		
		auto* gl = engine->glGetFunctions();
		int32 vertexShader;
		vertexShader = gl->_glCreateShader(GL_VERTEX_SHADER);
		gl->_glShaderSource(vertexShader, 1, &vSrc, 0);
		gl->_glCompileShader(vertexShader);

		int32 fragmentShader;
		fragmentShader = gl->_glCreateShader(GL_FRAGMENT_SHADER);
		gl->_glShaderSource(fragmentShader, 1, &fSrc, 0);
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
	AB::InputPropeties* g_Input = nullptr;

	void KeyCallback(AB::KeyboardKey key, bool32 currState, bool32 prevState, uint32 repeatCount) {
	
	}

	// TODO: Why pitch are upside down?
	void MouseCallback(AB::MouseMode mode, float32 x, float32 y, void* userData) {
		AB::GameContext* context = (AB::GameContext*)userData;
		if (context->mouseCaptured) {
			context->pitch += y;
			context->yaw += x;
			context->xLastMouse = x;
			context->yLastMouse = y;
			if (context->pitch > 89.0f)
				context->pitch = 89.0f;
			if (context->pitch < -89.0f)
				context->pitch = -89.0f;

			if (context->yaw > 360.0f)
				context->yaw -= 360.0f;
			if (context->yaw < -360.0f)
				context->yaw -= -360.0f;
		}
	}

extern "C" {

	ABERRATION_ENTRY void GameInitialize(AB::Engine* engine, AB::GameContext* gameContext) {
		g_Input = AB::InputInitialize(engine);
		AB::SetMouseMode(g_Input, AB::MouseMode::Cursor);
		gameContext->shaderHandle = LoadShader(engine, vertSource, fragSource);
		gameContext->lightShaderHandle= LoadShader(engine, lightVertSource, lightFragSource);

		auto* gl = engine->glGetFunctions();
		HACK = gameContext;
		
		engine->windowSetKeyCallback(KeyCallback);
		AB::SetMouseMoveCallback(g_Input, (void*)gameContext, MouseCallback);
		float32 vertices[288] = {
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,
			0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,

			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,

			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

			0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
			0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
			0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
			0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
			0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, -1.0f,  0.0f,
			0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
			0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f
		};

		gl->_glGenBuffers(1, &gameContext->vbo);
		gl->_glBindBuffer(GL_ARRAY_BUFFER, gameContext->vbo);
		gl->_glBufferData(GL_ARRAY_BUFFER, sizeof(float32) * 288, vertices, GL_STATIC_DRAW);
		gl->_glBindVertexArray(0);

		gl->_glGenBuffers(1, &gameContext->lightVbo);
		gl->_glBindBuffer(GL_ARRAY_BUFFER, gameContext->lightVbo);
		gl->_glBufferData(GL_ARRAY_BUFFER, sizeof(float32) * 288, vertices, GL_STATIC_DRAW);
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

		gl->_glGenTextures(1, &gameContext->spec_map);
		gl->_glBindTexture(GL_TEXTURE_2D, gameContext->spec_map);

		gl->_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		gl->_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		gl->_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl->_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		AB::Image img_spec = engine->loadBMP("../../../assets/spec.bmp");
		gl->_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img_spec.width, img_spec.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_spec.bitmap);
		engine->deleteBitmap(img_spec.bitmap);
		gl->_glBindTexture(GL_TEXTURE_2D, 0);


		gameContext->camFront = {0, 0, -1};
		gameContext->camPos = {0, 0, 3};
		gameContext->camUp = {0, 1, 0};
		gameContext->yaw = 0.0f;
		gameContext->pitch = 0.0f;
		gameContext->xLastMouse = 0;
		gameContext->yLastMouse = 0;
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
		gl->_glActiveTexture(GL_TEXTURE1);
		gl->_glBindTexture(GL_TEXTURE_2D, gameContext->spec_map);
		gl->_glUseProgram(gameContext->shaderHandle);

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

		DEBUG_OVERLAY_PUSH_V3("CamPos", gameContext->camPos);
		DEBUG_OVERLAY_PUSH_V3("CamFront", gameContext->camFront);
		DEBUG_OVERLAY_PUSH_V3("CamUP", gameContext->camUp);
		DEBUG_OVERLAY_PUSH_F32("Yaw", gameContext->yaw);
		DEBUG_OVERLAY_PUSH_F32("Pitch", gameContext->pitch);

		hpm::Matrix4 proj = hpm::PerspectiveRH(90.0f, 800.0f / 600.0f, 0.1f, 100.0f);
		//hpm::Matrix4 proj = hpm::OrthogonalRH(0, 8, 0, 6, 0.1, 100);

		//struct Material {
		//	sampler2D diffuse_map;
		//	vec3 specular;
		//	float shininess;
		//};
		//
		//struct Light {
		//	vec3 color;
		//	vec3 ambient;
		//	vec3 diffuse;
		//	vec3 specular;
		//	vec3 position;
		//};

		//struct DirLight {
		//	vec3 direction;
		//	vec3 ambient;
		//	vec3 diffuse;
		//	vec3 specular;
		//};
		
		int32 view_pos_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "view_pos");
		int32 light_pos_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "lights[0].position");
		int32 light_amb_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "lights[0].ambient");
		int32 light_diff_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "lights[0].diffuse");
		int32 light_spec_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "lights[0].specular");
		int32 light_ln_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "lights[0].linear");
		int32 light_qd_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "lights[0].quadratic");

		int32 light1_pos_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "lights[1].position");
		int32 light1_amb_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "lights[1].ambient");
		int32 light1_diff_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "lights[1].diffuse");
		int32 light1_spec_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "lights[1].specular");
		int32 light1_ln_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "lights[1].linear");
		int32 light1_qd_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "lights[1].quadratic");


		int32 material_shin_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "material.shininess");

		int32 dl_dir_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "dir_light.direction");
		int32 dl_amb_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "dir_light.ambient");
		int32 dl_dif_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "dir_light.diffuse");
		int32 dl_spc_loc = gl->_glGetUniformLocation(gameContext->shaderHandle, "dir_light.specular");
		
		int32 modelLoc = gl->_glGetUniformLocation(gameContext->shaderHandle, "model");
		int32 viewLoc = gl->_glGetUniformLocation(gameContext->shaderHandle, "view");
		int32 projLoc = gl->_glGetUniformLocation(gameContext->shaderHandle, "projection");

		DEBUG_OVERLAY_PUSH_SLIDER_F32("amb ", &(gameContext->light_props.r), 0, 1);
		DEBUG_OVERLAY_PUSH_SLIDER_F32("diff", &(gameContext->light_props.g), 0, 1);
		DEBUG_OVERLAY_PUSH_SLIDER_F32("spec", &(gameContext->light_props.b), 0, 1);
		DEBUG_OVERLAY_PUSH_V3("light_props: ", gameContext->light_props);

		//DEBUG_OVERLAY_PUSH_SLIDER_F32("dl_amb", &(gameContext->dl_props.r), 0, 1);
		//DEBUG_OVERLAY_PUSH_SLIDER_F32("dl_dif", &(gameContext->dl_props.g), 0, 1);
		//DEBUG_OVERLAY_PUSH_SLIDER_F32("dl_spc", &(gameContext->dl_props.b), 0, 1);
		//DEBUG_OVERLAY_PUSH_V3("dl_props: ", gameContext->dl_props);
		DEBUG_OVERLAY_PUSH_SLIDER_F32("x", &(gameContext->light_pos.r), -20, 20);
		DEBUG_OVERLAY_PUSH_SLIDER_F32("y", &(gameContext->light_pos.g), -20, 20);
		DEBUG_OVERLAY_PUSH_SLIDER_F32("z", &(gameContext->light_pos.b), -20, 20);
		DEBUG_OVERLAY_PUSH_V3("light_pos: ", gameContext->light_pos);

		DEBUG_OVERLAY_PUSH_SLIDER_F32("x", &(gameContext->light1_pos.r), -20, 20);
		DEBUG_OVERLAY_PUSH_SLIDER_F32("y", &(gameContext->light1_pos.g), -20, 20);
		DEBUG_OVERLAY_PUSH_SLIDER_F32("z", &(gameContext->light1_pos.b), -20, 20);
		DEBUG_OVERLAY_PUSH_V3("light1_pos: ", gameContext->light1_pos);

		DEBUG_OVERLAY_PUSH_SLIDER_F32("ln", &(gameContext->linear), 0.0001, 0.7);
		DEBUG_OVERLAY_PUSH_SLIDER_F32("qd", &(gameContext->quadratic), 0.000001, 1.8);

		DEBUG_OVERLAY_PUSH_F32("ln: ", gameContext->linear);
		DEBUG_OVERLAY_PUSH_F32("qd: ", gameContext->quadratic);



		gl->_glUniform3f(dl_dir_loc, gameContext->dl_dir.x, gameContext->dl_dir.y, gameContext->dl_dir.z);
		gl->_glUniform3f(dl_amb_loc, gameContext->dl_props.r, gameContext->dl_props.r, gameContext->dl_props.r);
		gl->_glUniform3f(dl_dif_loc, gameContext->dl_props.g, gameContext->dl_props.g, gameContext->dl_props.g);
		gl->_glUniform3f(dl_spc_loc, gameContext->dl_props.b, gameContext->dl_props.b, gameContext->dl_props.b);

		gl->_glUniform1i(gl->_glGetUniformLocation(gameContext->shaderHandle, "material.diffuse_map"), 0);
		gl->_glUniform1i(gl->_glGetUniformLocation(gameContext->shaderHandle, "material.spec_map"), 1);

		gl->_glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.data); 
		gl->_glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj.data);
		gl->_glUniform3f(view_pos_loc, gameContext->camPos.x, gameContext->camPos.y, gameContext->camPos.z);
		gl->_glUniform3f(light_pos_loc, gameContext->light_pos.x, gameContext->light_pos.y, gameContext->light_pos.z);
		gl->_glUniform3f(light_amb_loc, gameContext->light_props.r, gameContext->light_props.r, gameContext->light_props.r);
		gl->_glUniform3f(light_diff_loc, gameContext->light_props.g, gameContext->light_props.g, gameContext->light_props.g);
		gl->_glUniform3f(light_spec_loc, gameContext->light_props.b, gameContext->light_props.b, gameContext->light_props.b);
		gl->_glUniform1f(light_ln_loc, gameContext->linear);
		gl->_glUniform1f(light_qd_loc, gameContext->quadratic);

		gl->_glUniform3f(light1_pos_loc, gameContext->light1_pos.x, gameContext->light1_pos.y, gameContext->light1_pos.z);
		gl->_glUniform3f(light1_amb_loc, gameContext->light_props.r, gameContext->light_props.r, gameContext->light_props.r);
		gl->_glUniform3f(light1_diff_loc, gameContext->light_props.g, gameContext->light_props.g, gameContext->light_props.g);
		gl->_glUniform3f(light1_spec_loc, gameContext->light_props.b, gameContext->light_props.b, gameContext->light_props.b);
		gl->_glUniform1f(light1_ln_loc, gameContext->linear);
		gl->_glUniform1f(light1_qd_loc, gameContext->quadratic);


		gl->_glUniform1f(material_shin_loc, 128.0f);


		gl->_glBindBuffer(GL_ARRAY_BUFFER, gameContext->vbo);
		gl->_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		gl->_glEnableVertexAttribArray(0);
		gl->_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		gl->_glEnableVertexAttribArray(1);
		gl->_glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
		gl->_glEnableVertexAttribArray(2);
		gl->_glEnable(GL_DEPTH_TEST);
		for (GLuint i = 0; i < 10; i++) {
			hpm::Matrix4 tr = hpm::Translation(cubePositions[i]);
			GLfloat angle = 20.0f * i;
			hpm::Matrix4 rt = hpm::Rotate(tr, a, { 1.0f, 0.3f, 0.5f });
			hpm::Matrix4 sc = hpm::Identity4();//hpm::Scaling({1, 1, 1});
			//rt = hpm::Scale(rt, { 2, 3, 0.2 });

			hpm::Matrix4 model = hpm::Multiply(rt, sc);
			gl->_glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.data);

			gl->_glDrawArrays(GL_TRIANGLES, 0, 36);
		}
			a += 0.5f;

		gl->_glBindBuffer(GL_ARRAY_BUFFER, gameContext->vbo);
		gl->_glEnableVertexAttribArray(0);
		gl->_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		gl->_glUseProgram(gameContext->lightShaderHandle);
		int32 LModelLoc = gl->_glGetUniformLocation(gameContext->lightShaderHandle, "model");
		int32 LViewLoc = gl->_glGetUniformLocation(gameContext->lightShaderHandle, "view");
		int32 LProjLoc = gl->_glGetUniformLocation(gameContext->lightShaderHandle, "projection");
		int32 LColorLoc = gl->_glGetUniformLocation(gameContext->lightShaderHandle, "lightColor");

		gl->_glUniform3f(LColorLoc, 1, 1, 1);
		gl->_glUniformMatrix4fv(LViewLoc, 1, GL_FALSE, view.data);
		gl->_glUniformMatrix4fv(LProjLoc, 1, GL_FALSE, proj.data);
		gl->_glUniformMatrix4fv(LModelLoc, 1, GL_FALSE, hpm::Translation(gameContext->light_pos).data);

		gl->_glDrawArrays(GL_TRIANGLES, 0, 36);

		gl->_glUniformMatrix4fv(LModelLoc, 1, GL_FALSE, hpm::Translation(gameContext->light1_pos).data);

		gl->_glDrawArrays(GL_TRIANGLES, 0, 36);

		
		gl->_glDisable(GL_DEPTH_TEST);

	}
	ABERRATION_ENTRY void GameUpdate(AB::Engine* engine, AB::GameContext* gameContext) {
		AB::InputUpdate(g_Input);
		if (AB::InputKeyPressed(g_Input, AB::KeyboardKey::Tab)) {
			gameContext->mouseCaptured = !gameContext->mouseCaptured;
			if (gameContext->mouseCaptured) {
				AB::SetMouseMode(g_Input, AB::MouseMode::Captured);
			}
			else {
				AB::SetMouseMode(g_Input, AB::MouseMode::Cursor);
			}
		}
		if (AB::InputKeyHeld(g_Input, AB::KeyboardKey::W)) {
			HACK->camPos = hpm::Add(hpm::Multiply(HACK->camFront, 0.2), HACK->camPos);
		}
		if (AB::InputKeyHeld(g_Input, AB::KeyboardKey::S)) {
			HACK->camPos = hpm::Subtract(HACK->camPos, hpm::Multiply(HACK->camFront, 0.2));
		}
		if (AB::InputKeyHeld(g_Input, AB::KeyboardKey::A)) {
			hpm::Vector3 right = hpm::Normalize(hpm::Cross(HACK->camFront, HACK->camUp));
			HACK->camPos = hpm::Subtract(HACK->camPos, hpm::Multiply(right, 0.2));
		}
		if (AB::InputKeyHeld(g_Input, AB::KeyboardKey::D)) {
			hpm::Vector3 right = hpm::Normalize(hpm::Cross(HACK->camFront, HACK->camUp));
			HACK->camPos = hpm::Add(hpm::Multiply(right, 0.2), HACK->camPos);
		}
	}
}
