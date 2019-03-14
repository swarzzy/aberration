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

	struct FPSCamera {
		
	};

}

namespace AB {

	AB_API void Renderer3DInit();
	AB_API void SetDirectionalLight(const DirectionalLight* light);
	//AB_API int32 CreateMaterial(const char* diff_path, const char* spec_path, float32 shininess);
	AB_API void SetCamera(hpm::Vector3 front, hpm::Vector3 position);
	AB_API void Submit(int32 mesh_handle, int32 material_handle, const hpm::Matrix4* transform);
	AB_API void Render();
}