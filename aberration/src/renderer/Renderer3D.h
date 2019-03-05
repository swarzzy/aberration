#pragma once
#include "ABHeader.h"
#include <hypermath.h>

namespace AB {

	struct DirectionalLight {
		hpm::Vector3 direction;
		hpm::Vector3 ambient;
		hpm::Vector3 diffuse;
		hpm::Vector3 specular;
	};

	void Renderer3DInit();
	void Renderer3DSetDirectionalLight(const DirectionalLight* light);
	int32 Renderer3DCreateMesh(float32* vertices, uint32 num_vertices);
	int32 Renderer3DCreateMaterial(const char* diff_path, const char* spec_path, float32 shininess);
	void Renderer3DSetCamera(float32 pitch, float32 yaw, hpm::Vector3 position);
	void Renderer3DSubmit(int32 mesh_handle, int32 material_handle, const hpm::Matrix4* transform);
	void Renderer3DRender();
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