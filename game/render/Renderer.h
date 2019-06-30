#pragma once
#include "Shared.h"
#include <hypermath.h>
#include "../ExtendedMath.h"
#include "RenderGroup.h"
//#include "../GraphicsPipeline.h"
#include "World.h"

namespace AB::API
{
	struct Pipeline;
}

namespace AB
{
	const u32 RENDERER_MAX_CHUNK_QUADS =
		WORLD_CHUNK_DIM_TILES * WORLD_CHUNK_DIM_TILES * WORLD_CHUNK_DIM_TILES * 4 / 2;
	const u32 RENDERER_INDICES_PER_CHUNK_QUAD = 6;

	struct RenderCC
	{
		f32 gamma;
		f32 exposure;
	};

	struct  RendererConfig
	{
		u32 numSamples;
		u16 renderResolutionW;
		u16 renderResolutionH;
		u32 shadowMapRes;
	};

	struct Renderer
	{
		RenderCC cc;
		RendererConfig config;
		struct RendererImpl* impl;
	};

	Renderer* AllocateRenderer(MemoryArena* memory, MemoryArena* tempArena,
							   RendererConfig config);
	RendererConfig RendererGetConfig(Renderer* renderer);
    void RendererApplyConfig(Renderer* renderer, RendererConfig* newConfig);
	void RendererRender(Renderer* renderer,
						AssetManager* assetManager,
						RenderGroup* group);
}
