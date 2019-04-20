#pragma once
#include "AB.h"
#include <hypermath.h>
#include "ExtendedMath.h"

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

	enum BlendMode : uint32 {
		BLEND_MODE_OPAQUE = 0,
		BLEND_MODE_TRANSPARENT = 1
	};

	enum RenderCommandType : byte {
		RENDER_COMMAND_DRAW_MESH = 0,
		RENDER_COMMAND_DRAW_MESH_WIREFRAME = 1
	};

	enum RenderSortCriteria : byte {
		RENDER_SORT_CRITERIA_MESH_ORIGIN = 0,
		RENDER_SORT_CRITERIA_NEAREST_VERTEX = 1
	};

	struct RenderCommandDrawMesh {
		Transform transform;
		int32 meshHandle;
		BlendMode blendMode;
		RenderSortCriteria sortCriteria;
	};

	struct RenderCommandDrawMeshWireframe {
		Transform transform;
		int32 meshHandle;
		BlendMode blendMode;
		float32 lineWidth;
		RenderSortCriteria sortCriteria;
	};

	enum RenderKeyKindBit : uint64 {
		KIND_BIT_PIPELINE_CONF = 0,
		KIND_BIT_DRAW_CALL = 1
   	};

	enum RenderKeyBlendTypeBit : uint64 {
		BLEND_TYPE_BIT_OPAQUE = 0,
		BLEND_TYPE_BIT_TRANSPARENT = 1
	};
	
	struct RenderBucket {
		uint64 sortKey;
		uint32 rbOffset;
		uint16 _reserved0;
	   	byte _reserved1;
		RenderCommandType commandType;
	};

	struct RenderGroup {
		byte* renderBuffer;
		byte* renderBufferAt;
		u32 renderBufferSize;
		u32 renderBufferFree;

		RenderBucket* commandQueue;
		RenderBucket* tmpCommandQueue;
		u32 commandQueueCapacity;
		u32 commandQueueAt;		
	};

	AB_API RenderGroup* RendererAllocateRenderGroup(Memory* mem,
													u32 rbSize,
													u32 queueCapacity);

	AB_API void RendererAllocateBuffers(Memory* memory, Renderer* renderer,
										uint32 rbSize, uint32 commandQueueCap);
	AB_API void RendererBeginFrame(Renderer* renderer);
	AB_API void RendererPushCommand(Renderer* renderer, RenderCommandType rcType, void* rc);
	AB_API void RendererEndFrame(Renderer* renderer);
	
	AB_API Renderer* RendererInit(RendererConfig config);
	AB_API RendererFlySettings* RendererGetFlySettings(Renderer* renderer);
	AB_API RendererConfig RendererGetConfig(Renderer* renderer);
	AB_API void RendererApplyConfig(Renderer* renderer, RendererConfig* newConfig);
	AB_API void RendererSetSkybox(Renderer* renderer, int32 cubemapHandle);
	AB_API void RendererSetDirectionalLight(Renderer* renderer, const DirectionalLight* light);
	AB_API void RendererSubmitPointLight(Renderer* renderer,  PointLight* light);
	//AB_API int32 CreateMaterial(const char* diff_path, const char* spec_path, float32 shininess);
	AB_API void RendererSetCamera(Renderer* renderer, hpm::Vector3 front, hpm::Vector3 position);
	AB_API void RendererSubmit(Renderer* renderer, int32 mesh_handle, const hpm::Matrix4* transform, BlendMode blendMode);
	AB_API void RendererRender(Renderer* renderer);
}
