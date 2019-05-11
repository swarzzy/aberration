#include "GraphicsPipeline.h"
#include "OpenGL.h"
#include "Memory.h"

namespace AB
{
	inline static void _EnableDepthTest(b32 enable)
	{
		if (enable)
		{
			GLCall(glEnable(GL_DEPTH_TEST));	
		}
		else
		{
			GLCall(glDisable(GL_DEPTH_TEST));					
		}			
	}

	inline static void _WriteDepth(b32 enable)
	{
		if (enable)
		{
			GLCall(glDepthMask(GL_TRUE));	
		}
		else
		{
			GLCall(glDepthMask(GL_FALSE));					
		}
	}

	inline static void _SetDepthFunc(DepthFunc func)
	{
		GLuint glFunc;
		switch(func)
		{
		case DEPTH_FUNC_LESS: { glFunc = GL_LESS; } break;
		case DEPTH_FUNC_GREATER: { glFunc = GL_LESS; } break;
		INVALID_DEFAULT_CASE();
		}
		GLCall(glDepthFunc(glFunc));		
	}

	inline static void _EnableBlending(b32 val)
	{
		if (val)
		{
			GLCall(glEnable(GL_BLEND));
		}
		else
		{
			GLCall(glDisable(GL_BLEND));			
		}
	}

	inline static void _SetBlendFunc(BlendFunc func)
	{
		GLuint glFunc;
		switch(func)
		{
		case BLEND_FUNC_ADD: { glFunc = GL_FUNC_ADD; } break;
		INVALID_DEFAULT_CASE();
		}
		GLCall(glBlendEquation(glFunc));		
	}

	inline static void _SetBlendFactor(BlendFactor srcFac, BlendFactor dstFac)
	{
		GLuint src;
		GLuint dst;
		switch(srcFac)
		{
		case BLEND_FACTOR_SRC_ALPHA: { src = GL_SRC_ALPHA; } break;
		case BLEND_FACTOR_ONE_MINUS_SRC_ALPHA: { src = GL_ONE_MINUS_SRC_ALPHA; } break;
		INVALID_DEFAULT_CASE();
		}
		switch(dstFac)
		{
		case BLEND_FACTOR_SRC_ALPHA: { dst = GL_SRC_ALPHA; } break;
		case BLEND_FACTOR_ONE_MINUS_SRC_ALPHA: { dst = GL_ONE_MINUS_SRC_ALPHA; } break;
		INVALID_DEFAULT_CASE();
		}

		GLCall(glBlendFunc(src, dst));
	}

	inline static void _EnableFaceCulling(b32 val)
	{
		if (val)
		{
			GLCall(glEnable(GL_CULL_FACE));
		}
		else
		{
			GLCall(glDisable(GL_CULL_FACE));			
		}
	}

	inline static void _SetFaceCullingMode(FaceCullMode mode)
	{
		GLuint glMode;
		switch(mode)
		{
		case FACE_CULL_MODE_BACK: { glMode = GL_BACK; } break;
		case FACE_CULL_MODE_FRONT: { glMode = GL_FRONT; } break;
		INVALID_DEFAULT_CASE();
		}
		GLCall(glCullFace(glMode));
	}

	inline static void _SetDrawOrder(DrawOrder order)
	{
		GLuint glOrder;
		switch(order)
		{
		case DRAW_ORDER_CCW: { glOrder = GL_CCW; } break;
	   	INVALID_DEFAULT_CASE();
		}
		GLCall(glFrontFace(glOrder));
	}

	inline static void _SetPolygonFillMode(PolygonFillMode mode)
	{
		GLuint glMode;
		switch(mode)
		{
		case POLYGON_FILL_MODE_FILL: { glMode = GL_FILL; } break;
		case POLYGON_FILL_MODE_LINE: { glMode = GL_LINE; } break;
	   	INVALID_DEFAULT_CASE();
		}
		GLCall(glPolygonMode(GL_FRONT_AND_BACK, glMode));		
	}
	
	GraphicsPipeline* AllocatePipeline(MemoryArena* memory)
	{
		GraphicsPipeline* pipeline = (GraphicsPipeline*)PushSize(memory,
																 sizeof(GraphicsPipeline),
																 alignof(GraphicsPipeline));
		AB_CORE_ASSERT(pipeline, "Allocation failed.");
		u32 globalVao;
		GLCall(glGenVertexArrays(1, &globalVao));
		GLCall(glBindVertexArray(globalVao));
		
		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		GLCall(glBlendEquation(GL_FUNC_ADD));
		
		GLCall(glEnable(GL_DEPTH_TEST));
		GLCall(glDepthFunc(GL_LESS));
		GLCall(glDepthMask(GL_TRUE));	
		
		GLCall(glEnable(GL_CULL_FACE));
		GLCall(glCullFace(GL_BACK));
		GLCall(glFrontFace(GL_CCW));

		GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

		SetZeroScalar(GraphicsPipeline, pipeline);
		
		pipeline->currentState.depthTestState = true;
		pipeline->currentState.writeDepth = true;
		pipeline->currentState.depthFunc = DEPTH_FUNC_LESS;
		pipeline->currentState.blendingState = true;
		pipeline->currentState.blendFunc = BLEND_FUNC_ADD;
		pipeline->currentState.srcBlendFactor = BLEND_FACTOR_SRC_ALPHA;
		pipeline->currentState.dstBlendFactor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline->currentState.faceCullingState = true;
		pipeline->currentState.faceCullMode = FACE_CULL_MODE_BACK;
		pipeline->currentState.drawOrder = DRAW_ORDER_CCW;
		pipeline->currentState.polygonFillMode = POLYGON_FILL_MODE_FILL;

		pipeline->lastCommitedState = pipeline->currentState;

		return pipeline;
	}

	inline void PipelineCommitState(GraphicsPipeline* pipeline)
	{
		pipeline->lastCommitedState = pipeline->currentState;
	}

	inline void PipelineResetState(GraphicsPipeline* pipeline)
	{
		GraphicsPipelineState* c = &pipeline->currentState;
		GraphicsPipelineState* p = &pipeline->lastCommitedState;
		
		if (c->depthTestState != p->depthTestState)
		{
			_EnableDepthTest(p->depthTestState);
			c->depthTestState = p->depthTestState;
		}
		if (c->writeDepth != p->writeDepth)
		{
			_WriteDepth(p->writeDepth);
			c->writeDepth = p->writeDepth;
		}
		if (c->depthFunc != p->depthFunc)
		{
			_SetDepthFunc(p->depthFunc);
			c->depthFunc = p->depthFunc;
		}

		if (c->blendingState != p->blendingState)
		{
			_EnableBlending(p->blendingState);
			c->blendingState = p->blendingState;
		}
		if (c->blendFunc != p->blendFunc)
		{
			_SetBlendFunc(p->blendFunc);
			c->blendFunc = p->blendFunc;
		}
		if (c->srcBlendFactor != p->srcBlendFactor ||
			c->dstBlendFactor != p->dstBlendFactor)
		{
			_SetBlendFactor(p->srcBlendFactor, p->dstBlendFactor);
			p->srcBlendFactor = p->srcBlendFactor;
			p->dstBlendFactor = p->dstBlendFactor;
		}

		if (c->faceCullingState != p->faceCullingState)
		{
			_EnableFaceCulling(p->faceCullingState);
			c->faceCullingState = p->faceCullingState;
		}

		if (c->faceCullMode != p->faceCullMode)
		{
			_SetFaceCullingMode(p->faceCullMode);
			c->faceCullMode = p->faceCullMode;
		}

		if (c->drawOrder != p->drawOrder)
		{
			_SetDrawOrder(p->drawOrder);
			c->drawOrder = p->drawOrder;
		}

		if (c->polygonFillMode != p->polygonFillMode)
		{
			_SetPolygonFillMode(p->polygonFillMode);
			c->polygonFillMode = p->polygonFillMode;
		}
	}

	inline void PipelineResetStateNoBackend(GraphicsPipeline* pipeline)
	{
		pipeline->currentState = pipeline->lastCommitedState;	
	}

	inline void EnableDepthTest(GraphicsPipeline* pip, b32 val)
	{
		if (pip->currentState.depthTestState != val)
		{
			_EnableDepthTest(val);
			pip->currentState.depthTestState = val;
		}
	}

	inline void WriteDepth(GraphicsPipeline* pip, b32 val)
	{
		if (pip->currentState.writeDepth != val)
		{
			_WriteDepth(val);
			pip->currentState.writeDepth = val;
		}
	}

	inline void SetDepthFunc(GraphicsPipeline* pip, DepthFunc func)
	{
		if (pip->currentState.depthFunc != func)
		{
			_SetDepthFunc(func);
			pip->currentState.depthFunc = func;
		}
	}

	inline void EnableBlending(GraphicsPipeline* pip, b32 val)
	{
		if (pip->currentState.blendingState != val)
		{
			_EnableBlending(val);
			pip->currentState.blendingState = val;
		}
	}
	
	inline void SetBlendFunction(GraphicsPipeline* pip, BlendFunc val)
	{
		if (pip->currentState.blendFunc != val)
		{
			_SetBlendFunc(val);
			pip->currentState.blendFunc = val;
		}
	}

	inline void SetBlendFactor(GraphicsPipeline* pip,
							   BlendFactor srcFac, BlendFactor dstFac)
	{
		
		if (pip->currentState.srcBlendFactor != srcFac ||
			pip->currentState.dstBlendFactor != dstFac)
		{
			_SetBlendFactor(srcFac, dstFac);
			pip->currentState.srcBlendFactor = srcFac;
			pip->currentState.dstBlendFactor = dstFac;
		}
		

	}

	inline void EnableFaceCulling(GraphicsPipeline* pip, b32 val)
	{
		if (pip->currentState.faceCullingState != val)
		{
			_EnableFaceCulling(val);
			pip->currentState.faceCullingState = val;
		}
	}
	
	inline void SetFaceCullingMode(GraphicsPipeline* pip, FaceCullMode mode)
	{
		if (pip->currentState.faceCullMode != mode)
		{
			_SetFaceCullingMode(mode);
			pip->currentState.faceCullMode = mode;
		}
	}
	
	inline void SetDrawOrder(GraphicsPipeline* pip, DrawOrder order)
	{
		if (pip->currentState.drawOrder != order)
		{
			_SetDrawOrder(order);
			pip->currentState.drawOrder = order;
		}
	}

	inline void SetPolygonFillMode(GraphicsPipeline* pip, PolygonFillMode mode)
	{
		if (pip->currentState.polygonFillMode != mode)
		{
			_SetPolygonFillMode(mode);
			pip->currentState.polygonFillMode = mode;
		}
	}



	inline void PipelineResetBackend(GraphicsPipeline* pipeline)
	{
		_EnableDepthTest(pipeline->currentState.depthTestState);
		_WriteDepth(pipeline->currentState.writeDepth);
		_SetDepthFunc(pipeline->currentState.depthFunc);
		_EnableBlending(pipeline->currentState.blendingState);
		_SetBlendFunc(pipeline->currentState.blendFunc);
		_SetBlendFactor(pipeline->currentState.srcBlendFactor,
						pipeline->currentState.dstBlendFactor);
		_EnableFaceCulling(pipeline->currentState.faceCullingState);
		_SetFaceCullingMode(pipeline->currentState.faceCullMode);
		_SetDrawOrder(pipeline->currentState.drawOrder);
		_SetPolygonFillMode(pipeline->currentState.polygonFillMode);
	}




#if 0


	inline void PipelineRequireState(GraphicsPipelineState* pipeline, GraphicsPipelineConfig state)
	{
		if (state.depthTestState != pipeline->depthTestState)
		{
			if (state.depthTestState)
			{
				GLCall(glEnable(GL_DEPTH_TEST));	
			}
			else
			{
				GLCall(glDisable(GL_DEPTH_TEST));					
			}
			pipeline->depthTestState = state.depthTestState;
		}
		
		if (state.writeDepth != pipeline->writeDepth)
		{
			if (state.writeDepth)
			{
				GLCall(glDepthMask(GL_TRUE));	
			}
			else
			{
				GLCall(glDepthMask(GL_FALSE));					
			}
			pipeline->writeDepth = state.writeDepth;
		}

		if (state.depthFunc != pipeline->depthFunc)
		{
			GLuint func;
			switch(state.depthFunc)
			{
			case DEPTH_FUNC_LESS: { func = GL_LESS; } break;
			case DEPTH_FUNC_GREATER: { func = GL_LESS; } break;
			default: { AB_CORE_FATAL("Unknown depth func."); } break;
			}

			GLCall(glDepthFunc(func));
			pipeline->depthFunc = state.depthFunc;
		}
	}	
	
	inline void PipelineRequireDepthTest(GraphicsPipelineState* pipeline,
										 b32 depthTestEnabled,
										 b32 writeDepth,
										 DepthFunc depthFunc)
	{
		if (depthTestEnabled != pipeline->depthTestState)
		{
			if (depthTestEnabled)
			{
				GLCall(glEnable(GL_DEPTH_TEST));	
			}
			else
			{
				GLCall(glDisable(GL_DEPTH_TEST));					
			}
			pipeline->depthTestState = depthTestEnabled;
		}
		
		if (writeDepth != pipeline->writeDepth)
		{
			if (writeDepth)
			{
				GLCall(glDepthMask(GL_TRUE));	
			}
			else
			{
				GLCall(glDepthMask(GL_FALSE));					
			}
			pipeline->writeDepth = writeDepth;
		}

		if (depthFunc != pipeline->depthFunc)
		{
			GLuint func;
			switch(depthFunc)
			{
			case DEPTH_FUNC_LESS: { func = GL_LESS; } break;
			case DEPTH_FUNC_GREATER: { func = GL_LESS; } break;
			default: { AB_CORE_FATAL("Unknown depth func."); } break;
			}

			GLCall(glDepthFunc(func));
			pipeline->depthFunc = depthFunc;
		}
	}
#endif
}
