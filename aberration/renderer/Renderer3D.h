#pragma once
#include "AB.h"
#include <hypermath.h>

namespace AB {
	struct AB_API DirectionalLight {
		Vector3 direction;
		Vector3 ambient;
		Vector3 diffuse;
		Vector3 specular;
	};

	struct AB_API PointLight {
		Vector3 position;
		Vector3 ambient;
		Vector3 diffuse;
		Vector3 specular;
		float32 linear;
		float32 quadratic;
	};

	struct AB_API RendererFlySettings {
		float32 gamma;
		float32 exposure;
	};

	struct AB_API RendererConfig {
		uint32 numSamples;
		uint16 renderResolutionW;
		uint16 renderResolutionH;
	};

	AB_API Renderer* RendererInit(RendererConfig config);
	AB_API RendererFlySettings* RendererGetFlySettings(Renderer* renderer);
	AB_API RendererConfig RendererGetConfig(Renderer* renderer);
	AB_API void RendererApplyConfig(Renderer* renderer, RendererConfig* newConfig);
	AB_API void RendererSetSkybox(Renderer* renderer, int32 cubemapHandle);
	AB_API void RendererSetDirectionalLight(Renderer* renderer, const DirectionalLight* light);
	AB_API void RendererSubmitPointLight(Renderer* renderer,  PointLight* light);
	//AB_API int32 CreateMaterial(const char* diff_path, const char* spec_path, float32 shininess);
	AB_API void RendererSetCamera(Renderer* renderer, hpm::Vector3 front, hpm::Vector3 position);
	AB_API void RendererSubmit(Renderer* renderer, int32 mesh_handle, const hpm::Matrix4* transform);
	AB_API void RendererRender(Renderer* renderer);
}
