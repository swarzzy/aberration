#include "Renderer.h"
#include "Memory.h"
#include "../AssetManager.h"

namespace AB
{
	RenderGroup* AllocateRenderGroup(MemoryArena* arena,
									 u32 rbSize,
									 u32 queueCapacity,
									 u32 lightBufCapacity)
	{
		// TODO: Check for free space
		// TODO: Temporary allocating rendergroup in sys storage (of not?)
		RenderGroup* group;
		group = (RenderGroup*)PushSize(arena,
									   sizeof(RenderGroup),
									   alignof(RenderGroup));
		AB_CORE_ASSERT(group, "Failed to allocate render group");

		group->commandQueueCapacity = queueCapacity;
		AB_CORE_ASSERT(sizeof(CommandQueueEntry) * queueCapacity <= 0xffffffff,
					   "RenderQueue size cannot be bigger than 4gb.");
		u32 commandQueueSize = sizeof(CommandQueueEntry) * queueCapacity;
		group->commandQueue = (CommandQueueEntry*)PushSize(arena,
														   commandQueueSize,
														   alignof(CommandQueueEntry));
		AB_CORE_ASSERT(group->commandQueue, "Failed to allocate render queue");
		group->tmpCommandQueue = (CommandQueueEntry*)PushSize(arena,
															  commandQueueSize,
															  alignof(CommandQueueEntry));
		AB_CORE_ASSERT(group->tmpCommandQueue, "Failed to allocate render queue");

		u32 lightBufSize = lightBufCapacity * sizeof(PointLight);
		group->pointLightsNum = lightBufCapacity;
		group->pointLights = (PointLight*)PushSize(arena,
												   lightBufSize,
												   alignof(PointLight));
		AB_CORE_ASSERT(group->pointLights, "Failed to allocate light buffer");

		group->renderBufferSize = rbSize;
		group->renderBufferFree = rbSize;
		group->renderBuffer = (byte*)PushSize(arena, rbSize, 16);
		AB_CORE_ASSERT(group->renderBuffer, "Failed to allocate render buffer");
		group->renderBufferAt = group->renderBuffer;

		return group;
	}

	inline static void* _PushRenderData(RenderGroup* group, u32 size, u32 aligment, void* data)
	{
		u32 padding = 0;
		u32 useAligment = 0;
		byte* currentAt = group->renderBufferAt;

		if (aligment == 0)
		{
			AB_CORE_WARN("Zero aligment specified to render data. Assuming aligment = 1");
			useAligment = 1;
		}
		else
		{
			useAligment = aligment;
		}

 		if ((uptr)currentAt % useAligment != 0)
		{
			// TODO: Check this padding calculation. It could be wrong
			padding = (useAligment - (uptr)currentAt % useAligment) % useAligment;
		}

		AB_CORE_ASSERT(size + padding < group->renderBufferFree,
					   "Not enough space in render buffer.");

		group->renderBufferAt += size + padding ;
		group->renderBufferFree -= size + padding;
		byte* nextAt = currentAt + padding;

		CopyBytes(size, nextAt, data);

		AB_CORE_ASSERT((uptr)nextAt % (uptr)useAligment == 0, "Wrong aligment");

		return (void*)nextAt;
	}

	static inline CommandQueueEntry*
	PushCommandQueueEntry(RenderGroup* renderGroup, CommandQueueEntry* cmd)
	{
		CommandQueueEntry* renderBucketDest = 
			renderGroup->commandQueue + renderGroup->commandQueueAt;	
		CopyScalar(CommandQueueEntry, renderBucketDest, cmd);	   	
		renderGroup->commandQueueAt++;
		return renderBucketDest;
	}
	
#define RENDER_KEY_INSERT_KIND(key, type)		 	key |= (u64)type << 63
#define RENDER_KEY_INSERT_BLEND_TYPE(key, type) 	key |= (u64)type << 62
#define RENDER_KEY_INSERT_DEPTH(key, depth)		\
	union {f32 flt; u32 uint;} _pun_##type;		\
	_pun_##type.flt = depth;					\
	key |= (u64)_pun_##type.uint << 30					

	static inline u64 _MakeKeyMesh(AssetManager* assetManager,
								   BlendMode blendMode,
								   RenderSortCriteria sortCriteria,
								   Transform* transform,
								   u32 meshHandle,
								   v3 cameraPosition)
	{
		RenderKeyBlendTypeBit blendBit = {};
		if (blendMode == BLEND_MODE_OPAQUE)
		{
			blendBit = BLEND_TYPE_BIT_OPAQUE;
		}
		else if (blendMode == BLEND_MODE_TRANSPARENT)
		{
			blendBit = BLEND_TYPE_BIT_TRANSPARENT;					
		}
		else
		{
			AB_CORE_FATAL("Invalid blend type");
		}
			
		f32 distanceToCamSq;
		if (sortCriteria == RENDER_SORT_CRITERIA_MESH_ORIGIN)
		{
			v3 meshCenter = GetPosition(&transform->worldMatrix);
			distanceToCamSq = DistanceSq(meshCenter, cameraPosition);
		}
		else if (sortCriteria == RENDER_SORT_CRITERIA_NEAREST_VERTEX)
		{
			Mesh* mesh = AssetGetMeshData(assetManager, meshHandle);
			AB_CORE_ASSERT(mesh, "Mesh pointer was nullptr");
			BBoxAligned bbox = mesh->aabb;

			v4 points[8];
			points[0] = V4(bbox.min, 1.0f);
			points[1] = V4(bbox.max.x, bbox.min.y, bbox.min.z, 1.0f);
			points[2] = V4(bbox.max.x, bbox.min.y, bbox.max.z, 1.0f);
			points[3] = V4(bbox.min.x, bbox.min.y, bbox.max.z, 1.0f);
			points[4] = V4(bbox.min.x, bbox.max.y, bbox.min.z, 1.0f);
			points[5] = V4(bbox.max.x, bbox.max.y, bbox.min.z, 1.0f);
			points[6] = V4(bbox.max.x, bbox.max.y, bbox.max.z, 1.0f);
			points[7] = V4(bbox.min.x, bbox.max.y, bbox.max.z, 1.0f);

			v4 point0 = MulM4V4(transform->worldMatrix,
								points[0]);
				
			f32 minDistanceSq = DistanceSq(V3(point0), cameraPosition);
				
			for (i32 i = 1; i < 8; i++)
			{
				v4 pointInWorld = MulM4V4(transform->worldMatrix,
										  points[i]);
				f32 d = DistanceSq(V3(pointInWorld), cameraPosition);
				if (d < minDistanceSq)
				{
					minDistanceSq = d;
				}
			}

			distanceToCamSq = minDistanceSq;
		}
		else
		{
			AB_CORE_FATAL("Invalid RenderSortCriteria.");
		}

		f32 sortValue;
		if (blendMode == BLEND_MODE_OPAQUE)
		{
			sortValue = distanceToCamSq;
		}
		else
		{
			// NOTE: Inverting distance in order to sort
			// transparent sprites from less significant to most
			if (distanceToCamSq < 0.001f)
			{
				sortValue = FLT_MAX;
			}
			else
			{
				sortValue = 1.0f / distanceToCamSq;
			}
			//DEBUG_OVERLAY_PUSH_VAR("Distance to cam", distanceToCamSq);
		}

		u64 sortKey = 0;
			
		RENDER_KEY_INSERT_KIND(sortKey, KIND_BIT_DRAW_CALL);
		RENDER_KEY_INSERT_BLEND_TYPE(sortKey, blendBit);
		RENDER_KEY_INSERT_DEPTH(sortKey, sortValue);

		return sortKey;
	}

	void RenderGroupPushCommand(RenderGroup* group,
								AssetManager* assetManager,
								RenderCommandType type,
								void* data)
	{
		// NOTE: Makes shure there is no pending instancing
		// array before execute other commands
		AB_ASSERT(!(!((type == RENDER_COMMAND_PUSH_DEBUG_CUBE_INSTANCE) ||
					  (type == RENDER_COMMAND_END_DEBUG_CUBE_INSTANCING)) &&
					group->pendingInstancingArray));

		// NOTE: Makes shure there is no pending line batch
		// before execute other commands
		AB_ASSERT(!(!((type == RENDER_COMMAND_PUSH_LINE_VERTEX) ||
					  (type == RENDER_COMMAND_DRAW_LINE_END)) &&
					group->pendingLineBatch));
				  
		
		void* renderDataPtr = nullptr;
		CommandQueueEntry command = {};

		switch (type)
		{
		case RENDER_COMMAND_DRAW_MESH:
		{
			RenderCommandDrawMesh* renderData = (RenderCommandDrawMesh*)data;

			u64 sortKey =  _MakeKeyMesh(assetManager,
										renderData->blendMode,
										renderData->sortCriteria,
										&renderData->transform,
										renderData->meshHandle,
										group->camera.position);

			command.sortKey = sortKey;
			command.commandType = RENDER_COMMAND_DRAW_MESH;

			renderDataPtr = _PushRenderData(group,
											sizeof(RenderCommandDrawMesh),
											alignof(RenderCommandDrawMesh),
											data);

			uptr offset = (uptr)renderDataPtr - (uptr)group->renderBuffer;
			command.rbOffset = SafeCastUptrU32(offset);
			PushCommandQueueEntry(group, &command);	
		}
		break;
		case RENDER_COMMAND_DRAW_MESH_WIREFRAME:
		{
			RenderCommandDrawMeshWireframe* renderData =
				(RenderCommandDrawMeshWireframe*)data;

			u64 sortKey =  _MakeKeyMesh(assetManager,
										renderData->blendMode,
										renderData->sortCriteria,
										&renderData->transform,
										renderData->meshHandle,
										group->camera.position);

				command.sortKey = sortKey;
				command.commandType = RENDER_COMMAND_DRAW_MESH_WIREFRAME;

				renderDataPtr =
					_PushRenderData(group,
									sizeof(RenderCommandDrawMeshWireframe),
									alignof(RenderCommandDrawMeshWireframe),
									data);
				
				uptr offset = (uptr)renderDataPtr - (uptr)group->renderBuffer;
				command.rbOffset = SafeCastUptrU32(offset);
				PushCommandQueueEntry(group, &command);	
		}
		break;
		case RENDER_COMMAND_DRAW_DEBUG_CUBE:
		{
			RenderCommandDrawDebugCube* renderData
				= (RenderCommandDrawDebugCube*)data;

			renderData->_meshHandle = ASSET_DEFAULT_CUBE_MESH_HANDLE;

			v3 cubeOrigin = GetPosition(&renderData->transform.worldMatrix);
			f32 distanceToCamSq = DistanceSq(cubeOrigin,
											 group->camera.position);

			u64 sortKey = 0;
			
			RENDER_KEY_INSERT_KIND(sortKey, KIND_BIT_DRAW_CALL);
			RENDER_KEY_INSERT_BLEND_TYPE(sortKey, BLEND_MODE_OPAQUE);
			RENDER_KEY_INSERT_DEPTH(sortKey, distanceToCamSq);

			command.sortKey = sortKey;
			command.commandType = RENDER_COMMAND_DRAW_DEBUG_CUBE;

			renderDataPtr = _PushRenderData(group,
											sizeof(RenderCommandDrawDebugCube),
											alignof(RenderCommandDrawDebugCube),
											data);
				
			uptr offset = (uptr)renderDataPtr - (uptr)group->renderBuffer;
			command.rbOffset = SafeCastUptrU32(offset);
			PushCommandQueueEntry(group, &command);	
		} break;
		case RENDER_COMMAND_SET_DIR_LIGHT:
		{
			RenderCommandSetDirLight* renderData = (RenderCommandSetDirLight*)data;
			group->dirLightEnabled = true;
			group->dirLight = renderData->light;
		} break;
		case RENDER_COMMAND_SET_POINT_LIGHT:
		{
			if (group->pointLightsAt <= group->pointLightsNum)
			{
				RenderCommandSetPointLight* renderData =
					(RenderCommandSetPointLight*)data;
				CopyScalar(PointLight,
						   &group->pointLights[group->pointLightsAt],
						   &renderData->light);
				group->pointLightsAt++;
			}
		} break;
		case RENDER_COMMAND_BEGIN_DEBUG_CUBE_INSTANCING:
		{
			RenderCommandBeginDebugCubeInctancing* renderData
				= (RenderCommandBeginDebugCubeInctancing*)data;
			renderData->_meshHandle = ASSET_DEFAULT_CUBE_MESH_HANDLE;

			u64 sortKey = 0;

			// TODO: Sorting for instanced batches
			// For now they always rendered last in the opaque queue
			RENDER_KEY_INSERT_KIND(sortKey, KIND_BIT_DRAW_CALL);
			RENDER_KEY_INSERT_BLEND_TYPE(sortKey, BLEND_MODE_OPAQUE);
			RENDER_KEY_INSERT_DEPTH(sortKey, FLT_MAX);

			command.sortKey = sortKey;
			command.commandType = RENDER_COMMAND_BEGIN_DEBUG_CUBE_INSTANCING;

			renderDataPtr =
				_PushRenderData(group,
								sizeof(RenderCommandBeginDebugCubeInctancing),
								1,
								data);
				
			uptr offset = (uptr)renderDataPtr - (uptr)group->renderBuffer;
			command.rbOffset = SafeCastUptrU32(offset);
			CommandQueueEntry* entry = PushCommandQueueEntry(group, &command);
			
			group->pendingInstancingArray = true;
			group->instancingArrayCount = 0;
			group->pendingInstancingCommandHeader = entry;

		} break;
		
		case RENDER_COMMAND_PUSH_DEBUG_CUBE_INSTANCE:
		{
			RenderCommandPushDebugCubeInstance* renderData
				= (RenderCommandPushDebugCubeInstance*)data;

			RCPushDebugCubeInstancePacked packedData = {};
			CopyArray(f32, 16, &packedData.worldMatrix,
					  renderData->worldMatrix.data);
			packedData.r = renderData->color.r;
			packedData.g = renderData->color.g;
			packedData.b = renderData->color.b;
			
			renderDataPtr =
				_PushRenderData(group,
								sizeof(RCPushDebugCubeInstancePacked),
								1,
								&packedData);
			group->instancingArrayCount++;
		} break;

		case RENDER_COMMAND_END_DEBUG_CUBE_INSTANCING:
		{
			group->pendingInstancingArray = false;
			group->pendingInstancingCommandHeader->instanceCount =
				group->instancingArrayCount;
			group->instancingArrayCount = 0;
		} break;
 
		case RENDER_COMMAND_DRAW_LINE_BEGIN:
		{
			RenderCommandDrawLineBegin* renderData
				= (RenderCommandDrawLineBegin*)data;

			u64 sortKey = 0;

			// TODO: Sorting for lines
			// For now they always rendered last in the opaque queue
			RENDER_KEY_INSERT_KIND(sortKey, KIND_BIT_DRAW_CALL);
			RENDER_KEY_INSERT_BLEND_TYPE(sortKey, BLEND_MODE_OPAQUE);
			RENDER_KEY_INSERT_DEPTH(sortKey, FLT_MAX);

			command.sortKey = sortKey;
			command.commandType = RENDER_COMMAND_DRAW_LINE_BEGIN;

			renderDataPtr =
				_PushRenderData(group,
								sizeof(RenderCommandDrawLineBegin),
								1,
								data);
				
			uptr offset = (uptr)renderDataPtr - (uptr)group->renderBuffer;
			command.rbOffset = SafeCastUptrU32(offset);
			CommandQueueEntry* entry = PushCommandQueueEntry(group, &command);
			
			group->pendingLineBatch = true;
			group->lineBatchCount = 0;
			group->pendingLineBatchCommandHeader = entry;

		} break;

		case RENDER_COMMAND_PUSH_LINE_VERTEX:
		{
			RenderCommandPushLineVertex* renderData
				= (RenderCommandPushLineVertex*)data;

			renderDataPtr =
				_PushRenderData(group,
								sizeof(RenderCommandPushLineVertex),
								1,
								(void*)renderData);
			group->lineBatchCount++;
		} break;

		case RENDER_COMMAND_DRAW_LINE_END:
		{
			group->pendingLineBatch = false;
			group->pendingLineBatchCommandHeader->instanceCount =
				group->lineBatchCount;
			group->lineBatchCount = 0;
		} break;
 
		INVALID_DEFAULT_CASE();
		}
	}

	void RenderGroupSetCamera(RenderGroup* group, v3 front, v3 position,
							  m4x4* lookAt)
	{
		group->camera.front = Normalize(front);
		group->camera.position = position;
		group->camera.lookAt = *lookAt;
	}

	void RenderGroupSetProjection(RenderGroup* group, m4x4* projection)
	{
		group->projectionMatrix = *projection;
	}


	void RenderGroupResetQueue(RenderGroup* group)
	{
		group->commandQueueAt = 0;
		group->renderBufferAt = group->renderBuffer;
		group->renderBufferFree = group->renderBufferSize;
		group->dirLightEnabled = false;
		group->pointLightsAt = 0;
	}

	CommandQueueEntry* RenderGroupSortCommandQueue(CommandQueueEntry* bufferA,
												   CommandQueueEntry* bufferB,
												   u32 beg, u32 end,
												   RenderGroupCommandQueueSortPred* pred)
	{
		CommandQueueEntry* targetBuffer = bufferA;
		if (beg < end)
		{
			u32 mid = beg + (end - beg) / 2;
			CommandQueueEntry* left = RenderGroupSortCommandQueue(bufferA,
																  bufferB,
																  beg, mid,
																  pred);
			CommandQueueEntry* right = RenderGroupSortCommandQueue(bufferA,
																   bufferB,
																   mid + 1,
																   end, pred);

			targetBuffer = left == bufferA ? bufferB : bufferA;

			u32 leftAt = beg;
			u32 rightAt = mid + 1;
			u32 tempAt = beg;
			while (leftAt <= mid && rightAt <= end)
			{
				AB_CORE_ASSERT(tempAt <= end, "Merge sort error. Buffer overflow.");
				if (pred(left[leftAt].sortKey, right[rightAt].sortKey))
				{
					targetBuffer[tempAt] = left[leftAt];
					tempAt++;
					leftAt++;
				}
				else
				{
					targetBuffer[tempAt] = right[rightAt];
					tempAt++;
					rightAt++;
					
				}
			}
			while (leftAt <= mid)
			{
				targetBuffer[tempAt] = left[leftAt];
				tempAt++;
				leftAt++;
			}
			while (rightAt <= end)
			{
				targetBuffer[tempAt] = right[rightAt];
				tempAt++;
				rightAt++;
			}
		}
		return targetBuffer;
	}

	//NOTE: Debug stuff
	void DrawDebugCube(RenderGroup* renderGroup,
					   AssetManager* assetManager,
					   v3 position, v3 scale, v3 color)
	{
		RenderCommandDrawDebugCube cubeCommand = {};
		m4x4 world = Identity4();
		world = Translate(world, position);
		world = Scale(world, scale);
		cubeCommand.transform.worldMatrix = world;
		cubeCommand.color = color;

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_DRAW_DEBUG_CUBE,
							   (void*)(&cubeCommand));

	}

	void DrawDebugMesh(RenderGroup* renderGroup,
					   AssetManager* assetManager,
					   v3 position, v3 scale, Mesh* mesh, b32 selected)
	{
		RenderCommandDrawMesh command = {};
		m4x4 world = Identity4();
		world = Translate(world, position);
		world = Scale(world, scale);
		command.transform.worldMatrix = world;
		command.meshHandle = mesh->index;
		command.highlight = selected;
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_DRAW_MESH,
							   (void*)(&command));

	}


	void DrawDebugCubeInstanced(RenderGroup* renderGroup,
								AssetManager* assetManager,
								v3 position, f32 scale, v3 color)
	{
		RenderCommandPushDebugCubeInstance cubeCommand = {};
		m4x4 world = Identity4();
		world = Translate(world, position);
		world = Scale(world, V3(scale));
		cubeCommand.worldMatrix = world;
		cubeCommand.color = color;

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_DEBUG_CUBE_INSTANCE,
							   (void*)(&cubeCommand));

	}

	void
	DrawAlignedBoxOutline(RenderGroup* renderGroup, AssetManager* assetManager,
						  v3 min, v3 max, v3 color, f32 lineWidth)
	{
		
		RenderCommandDrawLineBegin beginCommand = {};
		beginCommand.color = color;
		beginCommand.width = lineWidth;
		beginCommand.type = RENDER_LINE_TYPE_SEGMENTS;
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_DRAW_LINE_BEGIN,
							   (void*)(&beginCommand));

		RenderCommandPushLineVertex v0Command = {};
		v0Command.vertex = min;
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v0Command));

		RenderCommandPushLineVertex v1Command = {};
		v1Command.vertex = V3(max.x, min.y, min.z);
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v1Command));
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v1Command));


		RenderCommandPushLineVertex v2Command = {};
		v2Command.vertex = V3(max.x, min.y, max.z);
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v2Command));
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v2Command));


		RenderCommandPushLineVertex v3Command = {};
		v3Command.vertex = V3(min.x, min.y, max.z);
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v3Command));
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v3Command));

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v0Command));

		//
		RenderCommandPushLineVertex v4Command = {};
		v4Command.vertex =V3(min.x, max.y, min.z);
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v4Command));

		RenderCommandPushLineVertex v5Command = {};
		v5Command.vertex = V3(max.x, max.y, min.z);
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v5Command));
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v5Command));


		RenderCommandPushLineVertex v6Command = {};
		v6Command.vertex = V3(max.x, max.y, max.z);
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v6Command));
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v6Command));


		RenderCommandPushLineVertex v7Command = {};
		v7Command.vertex = V3(min.x, max.y, max.z);
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v7Command));
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v7Command));

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v4Command));

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v0Command));		
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v4Command));

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v1Command));
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v5Command));

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v2Command));
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v6Command));

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v3Command));
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v7Command));

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_DRAW_LINE_END,
							   (void*)(0));
 
	}

	void DrawStraightLine(RenderGroup* renderGroup,
						  AssetManager* assetManager,
						  v3 begin, v3 end, v3 color, f32 lineWidth)
	{
		RenderCommandDrawLineBegin beginCommand = {};
		beginCommand.color = color;
		beginCommand.width = lineWidth;
		beginCommand.type = RENDER_LINE_TYPE_SEGMENTS;
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_DRAW_LINE_BEGIN,
							   (void*)(&beginCommand));

		RenderCommandPushLineVertex v0Command = {};
		v0Command.vertex = begin;
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v0Command));

		RenderCommandPushLineVertex v1Command = {};
		v1Command.vertex = end;
		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_PUSH_LINE_VERTEX,
							   (void*)(&v1Command));

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_DRAW_LINE_END,
							   (void*)(0));				
	}



}
