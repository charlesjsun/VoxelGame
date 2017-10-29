// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#include "ChunkUtils.h"

FIntVector3 UChunkUtils::GetNormal(EVoxelFace Face)
{
	switch (Face)
	{
	case EVoxelFace::FRONT:
		return FIntVector3(0, 1, 0);
	case EVoxelFace::BACK:
		return FIntVector3(0, -1, 0);
	case EVoxelFace::LEFT:
		return FIntVector3(-1, 0, 0);
	case EVoxelFace::RIGHT:
		return FIntVector3(1, 0, 0);
	case EVoxelFace::TOP:
		return FIntVector3(0, 0, 1);
	case EVoxelFace::BOTTOM:
		return FIntVector3(0, 0, -1);
	}
	return FIntVector3(0, 0, 0);
}