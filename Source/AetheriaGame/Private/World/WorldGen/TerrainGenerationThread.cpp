// Copyright (c) 2016, Charles JL Sun, All rights reserved.

#include "TerrainGenerationThread.h"

#include "VoxelTerrain.h"

#include "DebugLibrary.h"

#include "RunnableThread.h"
#include "PlatformProcess.h"
#include "ScopedEvent.h"
#include "Async.h"

FTerrainGenerationThread::FTerrainGenerationThread(AVoxelTerrain* InVoxelTerrain)
	: VoxelTerrain(InVoxelTerrain)
{
	Thread = FRunnableThread::Create(this, TEXT("Terrain Generation Thread"), 0, TPri_Normal);
	bIsThreadRunning = true;
	ChunkAddedEvent = nullptr;
}

FTerrainGenerationThread::~FTerrainGenerationThread()
{
	VoxelTerrain = nullptr;
	ChunkAddedEvent = nullptr;

	delete Thread;
	Thread = nullptr;
}

bool FTerrainGenerationThread::Init()
{
	return true;
}

void FTerrainGenerationThread::Stop()
{
	bIsThreadRunning = false;
	if (ChunkAddedEvent != nullptr)
	{
		ChunkAddedEvent->Trigger();
		ChunkAddedEvent = nullptr;
	}
}

void FTerrainGenerationThread::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

uint32 FTerrainGenerationThread::Run()
{

	FPlatformProcess::Sleep(0.5f);

	while (bIsThreadRunning)
	{
		// Runs until out of chunks to generate
		while (!ChunksToGenerate.IsEmpty())
		{
			NextChunk();
		}

		UDebugLibrary::Println(VoxelTerrain, 10, FString("World Gen Thread"));

		// Pauses this thread until another thread triggers the event
		FScopedEvent Event;
		ChunkAddedEvent = Event.Get();

		FPlatformProcess::Sleep(0.5f);
	}

	return 0;

}

void FTerrainGenerationThread::QueueChunk(const FIntVector3& ChunkCoord)
{

	ChunksToGenerate.Enqueue(ChunkCoord);

	// Triggers the event so the main loop/thread can update
	if (ChunkAddedEvent != nullptr)
	{
		ChunkAddedEvent->Trigger();
		ChunkAddedEvent = nullptr;
	}
}

void FTerrainGenerationThread::NextChunk()
{
	FIntVector3 ChunkCoord;
	ChunksToGenerate.Dequeue(ChunkCoord);

	// Generates the chunk
	FChunk Chunk(ChunkCoord);
	Chunk.Voxels.SetNumUninitialized(1 << (3 * CHUNK_SHIFT));
	Chunk.HighestSolidVoxels.SetNumUninitialized(1 << (2 * CHUNK_SHIFT));
	UTerrainGenerator::GenerateChunk(VoxelTerrain, Chunk, VoxelTerrain->TerrainGenParameters);

	// Tells the main game thread to add the chunk to the terrain and send it to the client
	AsyncTask(ENamedThreads::GameThread, [=]() mutable
	{
		VoxelTerrain->AddChunk(Chunk);
	});
}
