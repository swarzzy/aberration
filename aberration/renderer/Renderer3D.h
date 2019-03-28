#pragma once
#include "AB.h"
#include <hypermath.h>

namespace AB {
	struct AB_API DirectionalLight {
		hpm::Vector3 direction;
		hpm::Vector3 ambient;
		hpm::Vector3 diffuse;
		hpm::Vector3 specular;
	};

	struct Renderer;

	struct AB_API PointLight {
		Vector3 position;
		Vector3 ambient;
		Vector3 diffuse;
		Vector3 specular;
		float32 linear;
		float32 quadratic;
	};

	AB_API Renderer* RendererInit();
	AB_API void RendererSetDirectionalLight(Renderer* renderer, const DirectionalLight* light);
	AB_API void RendererSetPointLight(Renderer* renderer, uint32 index, PointLight* light);
	//AB_API int32 CreateMaterial(const char* diff_path, const char* spec_path, float32 shininess);
	AB_API void RendererSetCamera(Renderer* renderer, hpm::Vector3 front, hpm::Vector3 position);
	AB_API void RendererSubmit(Renderer* renderer, int32 mesh_handle, int32 material_handle, const hpm::Matrix4* transform);
	AB_API void RendererRender(Renderer* renderer);
}
