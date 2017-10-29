// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "ChunkUtils.h"
#include "IntVectors.h"

#include "TerrainGenerator.h"

#include "Runnable.h"
#include "Queue.h"

class FTerrainGenerationThread : public FRunnable
{

private:

	friend class AVoxelTerrain;

	class AVoxelTerrain* VoxelTerrain;

	FRunnableThread* Thread;

	bool bIsThreadRunning;

	TQueue<FIntVector3> ChunksToGenerate;

	FEvent* ChunkAddedEvent;

public:

	FTerrainGenerationThread(class AVoxelTerrain* InVoxelTerrain);

	virtual ~FTerrainGenerationThread();

	virtual bool Init() override;

	virtual uint32 Run() override;

	virtual void Stop() override;

	void EnsureCompletion();

	void QueueChunk(const FIntVector3& ChunkCoord);

private:

	void NextChunk();

};