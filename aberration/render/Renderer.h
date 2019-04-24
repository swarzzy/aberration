#pragma once
#include "AB.h"
#include <hypermath.h>
#include "ExtendedMath.h"
#include "RenderGroup.h"

namespace AB::API
{
	struct Pipeline;
}

namespace AB {

	struct RenderCC
	{
		f32 gamma;
		f32 exposure;
	};

	struct AB_API RendererConfig
	{
		u32 numSamples;
		u16 renderResolutionW;
		u16 renderResolutionH;
	};


	struct Renderer
	{
		RenderCC cc;
		RendererConfig config;
		struct RendererImpl* impl;
		API::Pipeline* pipeline;
	};

	AB_API Renderer* RendererInit(RendererConfig config, API::Pipeline* pipeline);
	AB_API RendererConfig RendererGetConfig(Renderer* renderer);
	AB_API void RendererApplyConfig(Renderer* renderer, RendererConfig* newConfig);
	AB_API void RendererSetSkybox(Renderer* renderer, i32 cubemapHandle);
	AB_API void RendererRender(Renderer* renderer, RenderGroup* group);
}
