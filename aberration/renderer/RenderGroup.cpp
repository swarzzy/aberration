#include "Renderer3D.h"
#include "Memory.h"

namespace AB
{
	RenderGroup* RendererAllocateRenderGroup(Memory* mem,
											 u32 rbSize,
											 u32 queueCapacity)
	{
		// TODO: Check for free space
		// TODO: Temporary allocating rendergroup in sys storage
		RenderGroup* group;
		group = (RenderGroup*)SysAllocAligned(sizeof(RenderGroup),
											  alignof(RenderGroup));
		AB_CORE_ASSERT(group, "Failed to allocate render group");

		group->commandQueueCapacity = queueCapacity;
		group->commandQueue = (RenderBucket*)SysAllocAligned(sizeof(RenderBucket),
															 alignof(RenderBucket));
		AB_CORE_ASSERT(group->commandQueue, "Failed to allocate render queue");
		group->tmpCommandQueue = (RenderBucket*)SysAllocAligned(sizeof(RenderBucket),
															 alignof(RenderBucket));
		AB_CORE_ASSERT(group->tmpCommandQueue, "Failed to allocate render queue");

		group->renderBufferSize = rbSize;
		group->renderBufferFree = rbSize;
		group->renderBuffer = (byte*)SysAllocAligned(rbSize, 16);
		AB_CORE_ASSERT(group->renderBuffer, "Failed to allocate render buffer");

		return group;
	}

}
