#pragma once
#include "Shared.h"
#include <hypermath.h>

// TODO: Sorting for instanced batches
// For now they always rendered last in the opaque queue

namespace AB
{
	struct DirectionalLight
	{
		v3 from;
		v3 target;
		v3 ambient;
		v3 diffuse;
		v3 specular;
		//v2 shadowCoverageRectMin;
		//v2 shadowCoverageRectMax;
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
		RENDER_COMMAND_DRAW_MESH_WIREFRAME,
		RENDER_COMMAND_SET_DIR_LIGHT,
		RENDER_COMMAND_SET_POINT_LIGHT,
		RENDER_COMMAND_DRAW_LINE_BEGIN,
		RENDER_COMMAND_PUSH_LINE_VERTEX,
		RENDER_COMMAND_DRAW_LINE_END,
		RENDER_COMMAND_DRAW_DEBUG_CUBE,
		RENDER_COMMAND_BEGIN_DEBUG_CUBE_INSTANCING,
		RENDER_COMMAND_PUSH_DEBUG_CUBE_INSTANCE,
		RENDER_COMMAND_END_DEBUG_CUBE_INSTANCING
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

	enum RenderLineType
	{
		RENDER_LINE_TYPE_SEGMENTS,
		RENDER_LINE_TYPE_STRIP
	};

 	struct RenderCommandDrawLineBegin
	{
		RenderLineType type;
		v3 color;
		f32 width;
	};

	struct RenderCommandPushLineVertex
	{
		v3 vertex;
	};

	struct RenderCommandBeginDebugCubeInctancing
	{
		BlendMode blendMode;
		RenderSortCriteria sortCriteria;
		i32 _meshHandle;
	};

	struct RenderCommandPushDebugCubeInstance
	{
		// NOTE: Using m4x4 instead of transform because this struct
		// will be used for tight packing data into instansing buffer
		m4x4 worldMatrix;
		v3 color;
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
		u16 instanceCount;
	   	byte _reserved0;
		RenderCommandType commandType;
	};

	struct CameraState
	{
		v3 position;
		v3 front;
		m4x4 lookAt;
	};

#pragma pack(push, 1)
	struct RCPushDebugCubeInstancePacked
	{
		// NOTE: Using m4x4 instead of transform because this struct
		// will be used for tight packing data into instansing buffer
		f32 worldMatrix[16];
		f32 r;
		f32 g;
		f32 b;
	};
#pragma pack(pop)

	struct RenderGroup
	{
		CameraState camera;
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

		b32 pendingInstancingArray;
		u16 instancingArrayCount;
		CommandQueueEntry* pendingInstancingCommandHeader;

		b32 pendingLineBatch;
		u16 lineBatchCount;
		CommandQueueEntry* pendingLineBatchCommandHeader;

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

	// NOTE: Debug stuff
	void DrawDebugCube(RenderGroup* renderGroup, AssetManager* assetManager,
					   v3 position, v3 scale, v3 color);

	void DrawDebugCubeInstanced(RenderGroup* renderGroup,
								AssetManager* assetManager,
								v3 position, f32 scale, v3 color);

	void DrawAlignedBoxOutline(v3 min, v3 max, v3 color, f32 lineWidth);

}
