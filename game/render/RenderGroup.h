#pragma once
#include "Shared.h"
#include <hypermath.h>

namespace AB
{
	struct DirectionalLight
	{
		v3 from;
		v3 target;
		v3 ambient;
		v3 diffuse;
		v3 specular;
	};

	struct PointLight
	{
		v3 position;
		v3 ambient;
		v3 diffuse;
		v3 specular;
		f32 linear;
		f32 quadratic;
	};

	enum BlendMode : byte
	{
		BLEND_MODE_OPAQUE = 0,
		BLEND_MODE_TRANSPARENT = 1
	};

	enum RenderCommandType : byte
	{
		RENDER_COMMAND_DRAW_MESH = 0,
		RENDER_COMMAND_DRAW_MESH_WIREFRAME = 1,
		RENDER_COMMAND_SET_DIR_LIGHT = 2,
		RENDER_COMMAND_SET_POINT_LIGHT = 3,
		RENDER_COMMAND_DRAW_DEBUG_CUBE = 4,
	};

	enum RenderSortCriteria : byte
	{
		RENDER_SORT_CRITERIA_MESH_ORIGIN = 0,
		RENDER_SORT_CRITERIA_NEAREST_VERTEX = 1
	};

	struct RenderCommandDrawMesh
	{
		Transform transform;
		i32 meshHandle;
		BlendMode blendMode;
		RenderSortCriteria sortCriteria;
	};

	struct RenderCommandDrawMeshWireframe
	{
		Transform transform;
		i32 meshHandle;
		BlendMode blendMode;
		f32 lineWidth;
		RenderSortCriteria sortCriteria;
	};

	struct  RenderCommandDrawDebugCube
	{
		Transform transform;
		v3 color;
		i32 _meshHandle;
	};

	struct RenderCommandSetDirLight
	{
		DirectionalLight light;		
	};

	struct RenderCommandSetPointLight
	{
		PointLight light;		
	};

	enum RenderKeyKindBit : u64
	{
		KIND_BIT_PIPELINE_CONF = 0,
		KIND_BIT_DRAW_CALL = 1
	};

	enum RenderKeyBlendTypeBit : u64
	{
		BLEND_TYPE_BIT_OPAQUE = 0,
		BLEND_TYPE_BIT_TRANSPARENT = 1
	};
	
	struct CommandQueueEntry
	{
		u64 sortKey;
		u32 rbOffset;
		u16 _reserved0;
	   	byte _reserved1;
		RenderCommandType commandType;
	};

	struct Camera
	{
		v3 position;
		v3 front;
		m4x4 lookAt;
	};

	struct RenderGroup
	{
		Camera camera;
		i32 skyboxHandle;

		b32 dirLightEnabled;
		DirectionalLight dirLight;

		PointLight* pointLights;
		u32 pointLightsNum;
		u32 pointLightsAt;
		
		byte* renderBuffer;
		byte* renderBufferAt;
		u32 renderBufferSize;
		u32 renderBufferFree;

		CommandQueueEntry* commandQueue;
		CommandQueueEntry* tmpCommandQueue;
		u32 commandQueueCapacity;
		u32 commandQueueAt;
		
		m4x4 projectionMatrix;
	};
	
	RenderGroup* AllocateRenderGroup(MemoryArena* mem,
											u32 rbSize,
											u32 queueCapacity,
											u32 lightBufCapacity);

	void RenderGroupPushCommand(RenderGroup* group,
									   RenderCommandType type,
									   void* command);
	void RenderGroupResetQueue(RenderGroup* group);
	
	void RenderGroupSetCamera(RenderGroup* group, v3 front, v3 position,
							  m4x4* lookAt);

	typedef b32(RenderGroupCommandQueueSortPred)(u64 a, u64 b);

	CommandQueueEntry* RenderGroupSortCommandQueue(CommandQueueEntry* bufferA,
												   CommandQueueEntry* bufferB,
												   u32 beg, u32 end,
												   RenderGroupCommandQueueSortPred* pred);


}
