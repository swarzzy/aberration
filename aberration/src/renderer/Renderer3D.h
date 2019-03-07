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

	AB_API void Init();
	AB_API void SetDirectionalLight(const DirectionalLight* light);
	AB_API int32 CreateMesh(float32* vertices, uint32 num_vertices);
	AB_API int32 CreateMaterial(const char* diff_path, const char* spec_path, float32 shininess);
	AB_API void SetCamera(hpm::Vector3 front, hpm::Vector3 position);
	AB_API void Submit(int32 mesh_handle, int32 material_handle, const hpm::Matrix4* transform);
	AB_API void Render();
}

#if 0
#include "ABHeader.h"
#include <hypermath.h>

namespace AB::Renderer3D {

	struct Material;
	struct MaterialProperties {
		const char* vertex_shader_path;
		const char* fragment_shader_path;
		uint32 num_uniforms;
		uint32 num_textures;
	};

	struct Renderer3D;
	Renderer3D* Initialize();
	Material* CreateMaterial(MaterialProperties* properties);
	void MaterialAddTexture(Material* mt, uint32 slot, const char* texture_path);
	void MaterialSetUniform(Material* mt, const char* name, float val);
	void MaterialSetUniform(Material* mt, const char* name, hpm::Vector2 val);
	void MaterialSetUniform(Material* mt, const char* name, hpm::Vector3 val);
	void MaterialSetUniform(Material* mt, const char* name, hpm::Vector4 val);
	void MaterialSetUniform(Material* mt, const char* name, hpm::Matrix4 val);
	void MaterialSetUniform(Material* mt, const char* name, int32 val);

	void Submit(uint32 meshHandle, uint32 materialHandle);
	void Render(Renderer3D* props);
	void SetCameraAngles(Renderer3D* props, float32 pitch, float32 yaw);
}
#endif