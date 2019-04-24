#pragma once
#include "AB.h"

namespace AB::API
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
		FACE_CULL_MODE_BACK
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
	struct PipelineState
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

	struct Pipeline
	{
		PipelineState currentState;
		PipelineState lastCommitedState;
	};

	AB_API void InitPipeline(Pipeline* pipeline);

	inline void PipelineCommitState(Pipeline* pipeline);

	inline void EnableDepthTest(Pipeline* pip, b32 val);
	inline void WriteDepth(Pipeline* pip, b32 val);
	inline void SetDepthFunc(Pipeline* pip, DepthFunc func);

	inline void EnableBlending(Pipeline* pip, b32 val);
	inline void SetBlendFunction(Pipeline* pip, BlendFunc val);
	inline void SetBlendFactor(Pipeline* pip, BlendFactor srcFac, BlendFactor dstFac);

	inline void EnableFaceCulling(Pipeline* pip, b32 val);
	inline void SetFaceCullingMode(Pipeline* pip, FaceCullMode mode);
	inline void SetDrawOrder(Pipeline* pip, DrawOrder order);

	inline void SetPolygonFillMode(Pipeline* pip, PolygonFillMode mode);

	inline void PipelineResetBackend(Pipeline* pipeline);

	inline void PipelineResetState(Pipeline* pipeline);
	inline void PipelineResetStateNoBackend(Pipeline* pipeline);
#if 0
	inline void PipelineRecoverState(PipelineState* pipeline);
	inline void PipelineRequireState(PipelineConfig state);

	inline void PipelineRequireDepthTest(PipelineState* pipeline,
										 b32 depthTestEnabled,
										 b32 writeDepth,
										 DepthFunc depthFunc);
#endif
}
