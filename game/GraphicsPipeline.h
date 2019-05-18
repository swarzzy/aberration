#pragma once
#include "Shared.h"

namespace AB
{
	enum DepthFunc : u32
	{
		DEPTH_FUNC_LESS,
		DEPTH_FUNC_GREATER
	};

	enum BlendFunc : u32
	{
		BLEND_FUNC_ADD
	};

	enum BlendFactor : u32
	{
		BLEND_FACTOR_SRC_ALPHA,
		BLEND_FACTOR_ONE_MINUS_SRC_ALPHA	
	};

	enum FaceCullMode : u32
	{
		FACE_CULL_MODE_BACK,
		FACE_CULL_MODE_FRONT,
	};

	enum DrawOrder : u32
	{
		DRAW_ORDER_CCW
	};

	enum PolygonFillMode : u32
	{
		POLYGON_FILL_MODE_FILL,
		POLYGON_FILL_MODE_LINE
	};

	// TODO: Think about sizes of flags and enums
	struct GraphicsPipelineState
	{
		b32 depthTestState;
		b32 writeDepth;
		DepthFunc depthFunc;

		b32 blendingState;
		BlendFunc blendFunc;
		BlendFactor srcBlendFactor;
		BlendFactor dstBlendFactor;

		b32 faceCullingState;
		FaceCullMode faceCullMode;
		DrawOrder drawOrder;

		PolygonFillMode polygonFillMode;
	};

	struct GraphicsPipeline
	{
		GraphicsPipelineState currentState;
		GraphicsPipelineState lastCommitedState;
	};

	GraphicsPipeline* AllocatePipeline(MemoryArena* memory);

	inline void PipelineCommitState(GraphicsPipeline* pipeline);

	inline void EnableDepthTest(GraphicsPipeline* pip, b32 val);
	inline void WriteDepth(GraphicsPipeline* pip, b32 val);
	inline void SetDepthFunc(GraphicsPipeline* pip, DepthFunc func);

	inline void EnableBlending(GraphicsPipeline* pip, b32 val);
	inline void SetBlendFunction(GraphicsPipeline* pip, BlendFunc val);
	inline void SetBlendFactor(GraphicsPipeline* pip, BlendFactor srcFac, BlendFactor dstFac);

	inline void EnableFaceCulling(GraphicsPipeline* pip, b32 val);
	inline void SetFaceCullingMode(GraphicsPipeline* pip, FaceCullMode mode);
	inline void SetDrawOrder(GraphicsPipeline* pip, DrawOrder order);

	inline void SetPolygonFillMode(GraphicsPipeline* pip, PolygonFillMode mode);

	inline void PipelineResetBackend(GraphicsPipeline* pipeline);

	inline void PipelineResetState(GraphicsPipeline* pipeline);
	inline void PipelineResetStateNoBackend(GraphicsPipeline* pipeline);
#if 0
	inline void PipelineRecoverState(GraphicsPipelineState* pipeline);
	inline void PipelineRequireState(GraphicsPipelineState* pipeline,
									 GraphicsPipelineConfig state);

	inline void PipelineRequireDepthTest(GraphicsPipelineState* pipeline,
										 b32 depthTestEnabled,
										 b32 writeDepth,
										 DepthFunc depthFunc);
#endif
}
