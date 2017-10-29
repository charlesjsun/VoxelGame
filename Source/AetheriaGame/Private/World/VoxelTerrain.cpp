// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#include "VoxelTerrain.h"

#include "TerrainGenerator.h"
#include "TerrainGenerationThread.h"

#include "ChunkCollisionComponent.h"
#include "ChunkMeshComponent.h"

#include "VoxelPlayerController.h"

#include "DebugLibrary.h"

#include "ScopeLock.h"

#define TERRAIN_LOG 0

// Sets default values
AVoxelTerrain::AVoxelTerrain(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	PrimaryActorTick.bCanEverTick = true;

	TerrainGenParameters.bUseRidgedMulti = false;

	TerrainGenParameters.Seed = 123;
	TerrainGenParameters.NoiseOctaves = 2;
	TerrainGenParameters.NoiseFrequency = 1.0f;
	TerrainGenParameters.NoiseLacunarity = 2.0f;
	TerrainGenParameters.NoisePersistance = 0.5f;
	TerrainGenParameters.NoiseScale = 0.01f;

	TerrainGenParameters.bCreateTrees = true;
	TerrainGenParameters.TreeScale = 0.1f;
	TerrainGenParameters.TreeOctaves = 2;
	TerrainGenParameters.TreeDensity = 0.01f;

	TerrainGenParameters.SnowVoxel = 1;
	TerrainGenParameters.GrassVoxel = 10;
	TerrainGenParameters.bSnowOnTrees = false;

	TerrainParameters.DrawDistanceInChunks = 2;
	TerrainParameters.LoadExpansionRadius = 2;
	TerrainParameters.CollisionDistanceInChunks = 2;
	TerrainParameters.AOBlurRadus = 3;
	TerrainParameters.MaxUpdateTime = 0.001f;
	TerrainParameters.VoxelTypes.Add(FVoxelType(FColor(0, 0, 0), nullptr)); // Air, Material stays NULL
	TerrainParameters.VoxelTypes.Add(FVoxelType(FColor(1, 142, 14), nullptr)); // Grass
	TerrainParameters.VoxelTypes.Add(FVoxelType(FColor(55, 55, 55), nullptr)); // Stone
	TerrainParameters.VoxelTypes.Add(FVoxelType(FColor(255, 255, 255), nullptr)); // Snow
	TerrainParameters.VoxelTypes.Add(FVoxelType(FColor(0, 0, 255), nullptr)); // Blue
	TerrainParameters.VoxelTypes.Add(FVoxelType(FColor(255, 0, 0), nullptr)); // Red
	TerrainParameters.VoxelTypes.Add(FVoxelType(FColor(255, 0, 255), nullptr)); // Magenta
	TerrainParameters.VoxelTypes.Add(FVoxelType(FColor(0, 255, 255), nullptr)); // Cyan
	TerrainParameters.VoxelTypes.Add(FVoxelType(FColor(255, 255, 0), nullptr)); // Yellow

	// TESTING

#if 0
	static FString VoxelTerrainContext("VoxelTerrainContext");

	UDataTable* VoxelTypeData = ConstructorHelpers::FObjectFinder<UDataTable>(TEXT("DataTable'/Game/Data/Voxels/Voxels.Voxels'")).Object;
	VoxelTypeData->FindRow<FVoxelTypeData>(TEXT("1"), VoxelTerrainContext);
	VoxelTypeData->RowMap.Num();
#endif

	// END TESTING
}

void AVoxelTerrain::BeginDestroy()
{
	// Deletes the generation thread
	if (TerrainGenerationThread != nullptr)
	{
		TerrainGenerationThread->EnsureCompletion();
		delete TerrainGenerationThread;
		TerrainGenerationThread = nullptr;
		UE_LOG(LogStats, Log, TEXT("Terrain Generation Thread Destroyed!!!"));
	}

	//if (ChunkUpdaterThread != nullptr)
	//{
	//	ChunkUpdaterThread->EnsureCompletion();
	//	delete ChunkUpdaterThread;
	//	ChunkUpdaterThread = nullptr;
	//	UE_LOG(LogStats, Log, TEXT("Chunk Updater Thread Destroyed!!!"));
	//}

	// Empties all containers
	LoadedChunksIndex.Empty();
	LoadedChunkMeshComponents.Empty();
	LoadedChunkCollisionComponents.Empty();

	Super::BeginDestroy();
}

void AVoxelTerrain::BeginPlay()
{
	Super::BeginPlay();

	// Use a thread to generate the terrain
	TerrainGenerationThread = new FTerrainGenerationThread(this);
	UE_LOG(LogStats, Log, TEXT("Terrain Generation Thread Created!!!"));

}

FIntVector3 AVoxelTerrain::WorldToChunkCoord(const FIntVector3& WorldCoord)
{
	return FIntVector3(WorldCoord.X >> CHUNK_SHIFT, WorldCoord.Y >> CHUNK_SHIFT, WorldCoord.Z >> CHUNK_SHIFT);
}

FIntVector3 AVoxelTerrain::ChunkToWorldCoord(const FIntVector3& ChunkCoord)
{
	return FIntVector3(ChunkCoord.X << CHUNK_SHIFT, ChunkCoord.Y << CHUNK_SHIFT, ChunkCoord.Z << CHUNK_SHIFT);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UPDATE METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AVoxelTerrain::Tick(float DeltaSeconds)
{

	Super::Tick(DeltaSeconds);


	//time += DeltaSeconds;
	//if (time >= 2.0f)
	//{
	//	time -= 2.0f;
	//	PRINT_STRING(10, FString(TEXT("Main Thread")));
	//}


	//if (time < 0.5)
	//{
	//	time = 1;
	//	GET_ROLE(Role, RoleString);
	//	GET_ROLE(GetRemoteRole(), RemoteRoleString);
	//
	//	PRINT_STRING(1000, FString::Printf(TEXT("VoxelTerrain, Role: %s, RemoteRole: %s"), *RoleString, *RemoteRoleString));
	//}

}

bool AVoxelTerrain::IsChunkPending(const FIntVector3& ChunkCoord)
{
	return PendingChunks.Contains(ChunkCoord);
}

void AVoxelTerrain::LoadChunk(const FIntVector3& ChunkCoord)
{
	if (!HasChunk(ChunkCoord) && !IsChunkPending(ChunkCoord))
	{
		PendingChunks.Emplace(ChunkCoord);
		TerrainGenerationThread->QueueChunk(ChunkCoord);
	}
	//PRINT(*FString::Printf(TEXT("Chunk Generated: <%d, %d, %d>, Index: %d"), ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z, NewChunkIndex));
	//UE_LOG(LogStats, Log, TEXT("G - ChunkIndex: %d, Chunk Generated: <%d, %d, %d>, IsEmpty: %d"), NewChunkIndex, ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z, IsEmpty(NewChunk));
}

void AVoxelTerrain::AddChunk(FChunk& Chunk)
{

	{
		FScopeLock ChunkLock(&LoadedChunksMutex);

		const int32 NewChunkIndex = LoadedChunks.Emplace(Chunk);

		LoadedChunksIndex.Add(Chunk.ChunkPosition, NewChunkIndex);

#if TERRAIN_LOG
		GET_THIS_ROLE(ChunkRole);
		UE_LOG(LogStats, Log, TEXT("%s - ChunkIndex: %d, Chunk Added: <%d, %d, %d>"), *ChunkRole, NewChunkIndex, Chunk.ChunkPosition.X, Chunk.ChunkPosition.Y, Chunk.ChunkPosition.Z);
#endif
	}

	UDebugLibrary::Println(this, 10, FString::Printf(TEXT("Chunk Added %s"), *Chunk.ChunkPosition.ToString()));

	UpdateChunkHeightmap(Chunk.ChunkPosition);

	PendingChunks.Remove(Chunk.ChunkPosition);

	//PRINT_STRING(5, FString::Printf(TEXT("%s - ChunkIndex: %d, Chunk Added: <%d, %d, %d>"), *ChunkRole, NewChunkIndex, Chunk.ChunkPosition.X, Chunk.ChunkPosition.Y, Chunk.ChunkPosition.Z));
}

void AVoxelTerrain::UpdateChunkHeightmap(const FIntVector3& ChunkCoord)
{

	TIME_START();

	TArray<uint8> Voxels;

	GetChunkVoxelsArray(ChunkCoord, Voxels);

	if (Voxels.Num() == 0)
	{
		return;
	}

	TArray<int8> Heights;
	Heights.SetNumUninitialized(1 << (2 * CHUNK_SHIFT));

	// Loop through each column until a solid voxel, then set the height to that height
	for (int32 x = 0; x < CHUNK_SIZE; ++x)
	{
		for (int32 y = 0; y < CHUNK_SIZE; ++y)
		{
			int8 z = CHUNK_SIZE - 1;
			for (; z >= 0; --z)
			{
				if (Voxels[x | (y << Y_SHIFT) | (z << Z_SHIFT)] != 0)
				{
					break;
				}
			}
			Heights[x | (y << Y_SHIFT)] = z;
		}
	}

	FChunk& Chunk = GetChunkAt(ChunkCoord);

	{
		// Copies the generated height into the actual chunk's heightmap
		FScopeLock ChunksLock(&LoadedChunksMutex);
		FMemory::Memcpy(&Chunk.HighestSolidVoxels[0], &Heights[0], Heights.Num() * sizeof(int8));
	}

	TIME_END();

	//PRINT_STRING(10, FLOAT_STRING(ElapsedTime));

	//PRINT_STRING(10, FString::Printf(TEXT("Heighmap Updated <%d, %d>"), HeightmapCoord.X, HeightmapCoord.Y));

}

void AVoxelTerrain::GenerateMesh(const FIntVector3& ChunkCoord)
{

	FScopeLock MeshLock(&LoadedChunkMeshesMutex);
	UChunkMeshComponent* const MeshComponent = LoadedChunkMeshComponents.FindRef(ChunkCoord);

	if (MeshComponent == nullptr) // Make a new mesh component if it's generated
	{
		UChunkMeshComponent* NewMesh = NewObject<UChunkMeshComponent>(this);

		NewMesh->VoxelTerrain = this;
		NewMesh->ChunkCoord = ChunkCoord;
		NewMesh->HasLowPriorityUpdatePending = false;

		NewMesh->SetRelativeLocation(ChunkToWorldCoord(ChunkCoord).ToFloat());
		NewMesh->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
		NewMesh->RegisterComponent();

		// Add to the TMap that stores all the meshes
		LoadedChunkMeshComponents.Add(ChunkCoord, NewMesh);

#if TERRAIN_LOG
		UE_LOG(LogStats, Log, TEXT("<%d, %d, %d> Chunk Mesh Component Added to List"), ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z);
#endif
	}
	else if (MeshComponent->HasLowPriorityUpdatePending) // if it has a low priority update pending (meaning either far away or AO
	{
		MeshComponent->MarkRenderStateDirty();
		MeshComponent->HasLowPriorityUpdatePending = false;
	}

}

void AVoxelTerrain::GenerateCollision(const FIntVector3& ChunkCoord)
{
	FScopeLock CollisionLock(&LoadedChunkCollisionsMutex);

	const UChunkCollisionComponent* const CollisionComponent = LoadedChunkCollisionComponents.FindRef(ChunkCoord);

	if (CollisionComponent == nullptr) // Generate a collision mesh if it doesn't exist
	{
		UChunkCollisionComponent* NewCollision = NewObject<UChunkCollisionComponent>(this);
		NewCollision->VoxelTerrain = this;
		NewCollision->ChunkCoord = ChunkCoord;

		NewCollision->SetRelativeLocation(ChunkToWorldCoord(ChunkCoord).ToFloat());
		NewCollision->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
		NewCollision->RegisterComponent();

		LoadedChunkCollisionComponents.Add(ChunkCoord, NewCollision);

		UDebugLibrary::Println(this, 10, ChunkCoord.ToString());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VOXEL/CHUNK METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FVoxelType AVoxelTerrain::GetVoxelType(const uint8& Voxel) const
{
	checkf(Voxel < TerrainParameters.VoxelTypes.Num(), TEXT("There are NO Voxel Type (%d)!!!! There are only (%d) Voxel Types!!!"), Voxel, TerrainParameters.VoxelTypes.Num());
	return TerrainParameters.VoxelTypes[(int32)Voxel];
}

int32 AVoxelTerrain::GetNumVoxelTypes() const
{
	return TerrainParameters.VoxelTypes.Num();
}

uint8 AVoxelTerrain::GetVoxelAt(const int32& x, const int32& y, const int32& z)
{

	// Convert from global to local voxel coordinate
	const FIntVector3 ChunkCoord(x >> CHUNK_SHIFT, y >> CHUNK_SHIFT, z >> CHUNK_SHIFT);
	const int32& LocalX = x & CHUNK_SIZE_MASK;
	const int32& LocalY = y & CHUNK_SIZE_MASK;
	const int32& LocalZ = z & CHUNK_SIZE_MASK;

	FScopeLock LoadedChunksLock(&LoadedChunksMutex);

	const int32* const ChunkIndex = LoadedChunksIndex.Find(ChunkCoord);
	if (ChunkIndex != nullptr)
	{
		const FChunk& Chunk = LoadedChunks[*ChunkIndex];
		return Chunk.Voxels[LocalX | (LocalY << Y_SHIFT) | (LocalZ << Z_SHIFT)];
	}
	else // Return 0 (Air) if the chunk is not loaded
	{
		return (uint8)0;
	}

}

void AVoxelTerrain::SetVoxelAt(const int32& x, const int32& y, const int32& z, uint8 Voxel)
{

	// Convert from global to local voxel coordinate
	const FIntVector3 ChunkCoord(x >> CHUNK_SHIFT, y >> CHUNK_SHIFT, z >> CHUNK_SHIFT);
	const int32& LocalX = x & CHUNK_SIZE_MASK;
	const int32& LocalY = y & CHUNK_SIZE_MASK;
	const int32& LocalZ = z & CHUNK_SIZE_MASK;

	// Only mark chunk for re-mesh if something was changed
	bool bIsVoxelSet = false;

	{
		// Lock the container
		FScopeLock LoadedChunksLock(&LoadedChunksMutex);
		const int32* const ChunkIndex = LoadedChunksIndex.Find(ChunkCoord);
		if (ChunkIndex != nullptr) // only set voxel if the chunk is loaded
		{
			FChunk& Chunk = LoadedChunks[*ChunkIndex];
			Chunk.Voxels[LocalX | (LocalY << Y_SHIFT) | (LocalZ << Z_SHIFT)] = Voxel;
			bIsVoxelSet = true;
		}
	}

	if (bIsVoxelSet)
	{
		MarkVoxelDirty(FIntVector3(x, y, z));
	}


}

void AVoxelTerrain::MarkVoxelAreaDirty(const FIntVector3& MinVoxelCoord, const FIntVector3& MaxVoxelCoord)
{
	// TODO: Implement
	// Just add blur radius to all sides and modulo each one to get chunk coordinate and update
}

void AVoxelTerrain::MarkVoxelDirty(const FIntVector3& VoxelCoord)
{
	const FIntVector3 ChunkCoord = WorldToChunkCoord(VoxelCoord);

	const int32& LocalX = VoxelCoord.X & CHUNK_SIZE_MASK;
	const int32& LocalY = VoxelCoord.Y & CHUNK_SIZE_MASK;
	const int32& LocalZ = VoxelCoord.Z & CHUNK_SIZE_MASK;

	// Already lockes so we dont want to lock it again.
	UpdateChunkHeightmap(ChunkCoord);

	FScopeLock LoadedCollisionLock(&LoadedChunkCollisionsMutex);
	FScopeLock LoadedMeshLock(&LoadedChunkMeshesMutex);

	// Check all neighbor chunks and if the voxel is close enough to the edge for them to be remeshed
	TArray<bool> ShouldNeighborChunkUpdate;
	ShouldNeighborChunkUpdate.SetNumUninitialized(9);
	ShouldNeighborChunkUpdate[4] = true;
	ShouldNeighborChunkUpdate[3] = LocalX - TerrainParameters.AOBlurRadus < 0;
	ShouldNeighborChunkUpdate[5] = LocalX + TerrainParameters.AOBlurRadus >= CHUNK_SIZE;
	ShouldNeighborChunkUpdate[1] = LocalY - TerrainParameters.AOBlurRadus < 0;
	ShouldNeighborChunkUpdate[7] = LocalY + TerrainParameters.AOBlurRadus >= CHUNK_SIZE;
	ShouldNeighborChunkUpdate[0] = ShouldNeighborChunkUpdate[3] && ShouldNeighborChunkUpdate[1];
	ShouldNeighborChunkUpdate[2] = ShouldNeighborChunkUpdate[5] && ShouldNeighborChunkUpdate[1];
	ShouldNeighborChunkUpdate[6] = ShouldNeighborChunkUpdate[3] && ShouldNeighborChunkUpdate[7];
	ShouldNeighborChunkUpdate[8] = ShouldNeighborChunkUpdate[5] && ShouldNeighborChunkUpdate[7];

	// Moore's neighbor
	for (int32 i = 0; i < 9; ++i)
	{
		if (!ShouldNeighborChunkUpdate[i])
		{
			continue;
		}

		const int32 ChunkX = ChunkCoord.X + (i % 3 - 1);
		const int32 ChunkY = ChunkCoord.Y + (i / 3 - 1);

		// Update the meshes for all chunks in this column because ambient occlusion
		for (int32 ChunkZ = FMath::Min(WORLD_HEIGHT_CHUNKS - 1, ChunkCoord.Z + 1); ChunkZ >= 0; --ChunkZ)
		{
			const FIntVector3 CurrentChunkCoord(ChunkX, ChunkY, ChunkZ);
			UChunkMeshComponent* const ChunkMesh = LoadedChunkMeshComponents.FindRef(CurrentChunkCoord);
			if (ChunkMesh != nullptr)
			{
				// TODO: Only do this for adjacent chunks, otherwise use MarkRenderStateDirty
				ChunkMesh->HasLowPriorityUpdatePending = true;
			}
		}

		// Same for collision
		for (int32 ChunkZ = FMath::Min(WORLD_HEIGHT_CHUNKS - 1, ChunkCoord.Z + 1); ChunkZ >= FMath::Max(0, ChunkCoord.Z - 1); --ChunkZ)
		{
			const FIntVector3 CurrentChunkCoord(ChunkX, ChunkY, ChunkZ);
			UChunkCollisionComponent* const ChunkCollision = LoadedChunkCollisionComponents.FindRef(CurrentChunkCoord);
			if (ChunkCollision != nullptr)
			{
				ChunkCollision->MarkRenderStateDirty();
			}
		}

	}


}

void AVoxelTerrain::MarkChunkDirty(const FIntVector3& ChunkCoord)
{

	// Already lockes so we dont want to lock it again.
	UpdateChunkHeightmap(ChunkCoord);

	FScopeLock LoadedCollisionLock(&LoadedChunkCollisionsMutex);
	FScopeLock LoadedMeshLock(&LoadedChunkMeshesMutex);

	// Moore's neighbor
	for (int32 i = 0; i < 9; ++i)
	{
		const int32 ChunkX = ChunkCoord.X + (i % 3 - 1);
		const int32 ChunkY = ChunkCoord.Y + (i / 3 - 1);

		// Update the meshes for all chunks in this column because ambient occlusion
		for (int32 ChunkZ = FMath::Min(WORLD_HEIGHT_CHUNKS - 1, ChunkCoord.Z + 1); ChunkZ >= 0; --ChunkZ)
		{
			const FIntVector3 CurrentChunkCoord(ChunkX, ChunkY, ChunkZ);
			UChunkMeshComponent* const ChunkMesh = LoadedChunkMeshComponents.FindRef(CurrentChunkCoord);
			if (ChunkMesh != nullptr)
			{
				// TODO: Only do this for adjacent chunks,otherwise use MarkRenderStateDirty
				ChunkMesh->HasLowPriorityUpdatePending = true;
			}
		}

		// Same for collision
		for (int32 ChunkZ = FMath::Min(WORLD_HEIGHT_CHUNKS - 1, ChunkCoord.Z + 1); ChunkZ >= FMath::Max(0, ChunkCoord.Z - 1); --ChunkZ)
		{
			const FIntVector3 CurrentChunkCoord(ChunkX, ChunkY, ChunkZ);
			UChunkCollisionComponent* const ChunkCollision = LoadedChunkCollisionComponents.FindRef(CurrentChunkCoord);
			
			if (ChunkCollision != nullptr)
			{
				ChunkCollision->MarkRenderStateDirty();
			}

		}

	}
}

void AVoxelTerrain::GetChunkVoxelsArray(const FIntVector3& ChunkCoord, TArray<uint8>& OutVoxelsArray)
{
	FScopeLock LoadedChunksLock(&LoadedChunksMutex);
	const int32* const ChunkIndex = LoadedChunksIndex.Find(ChunkCoord);
	if (ChunkIndex != nullptr)
	{
		// Gets the chunk array
		const FChunk& Chunk = LoadedChunks[*ChunkIndex];
		OutVoxelsArray.SetNumUninitialized(Chunk.Voxels.Num());
		FMemory::Memcpy(&OutVoxelsArray[0], &Chunk.Voxels[0], Chunk.Voxels.Num() * sizeof(uint8));
	}
}

void AVoxelTerrain::GetVoxelsArray(const FIntVector3& MinVoxelCoord, const FIntVector3& MaxVoxelCoord, TArray<uint8>& OutVoxelsArray)
{
	FScopeLock LoadedChunksLock(&LoadedChunksMutex);

	// Voxel to chunk coord
	const FIntVector3& MinChunkCoord = WorldToChunkCoord(MinVoxelCoord);
	const FIntVector3& MaxChunkCoord = WorldToChunkCoord(MaxVoxelCoord);
	const FIntVector3& OutputDimensions = MaxVoxelCoord + FIntVector3(1) - MinVoxelCoord;

	OutVoxelsArray.SetNumUninitialized(OutputDimensions.X * OutputDimensions.Y * OutputDimensions.Z);

	// Loops through all chunks in between min and max
	for (int32 ChunkZ = MinChunkCoord.Z; ChunkZ <= MaxChunkCoord.Z; ++ChunkZ)
	{
		for (int32 ChunkY = MinChunkCoord.Y; ChunkY <= MaxChunkCoord.Y; ++ChunkY)
		{
			for (int32 ChunkX = MinChunkCoord.X; ChunkX <= MaxChunkCoord.X; ++ChunkX)
			{
				const FIntVector3 ChunkCoord(ChunkX, ChunkY, ChunkZ);
				const int32* const ChunkIndex = LoadedChunksIndex.Find(ChunkCoord);
				const FIntVector3 ChunkVoxelCoord(ChunkX << CHUNK_SHIFT, ChunkY << CHUNK_SHIFT, ChunkZ << CHUNK_SHIFT);

				// The min and max coord may be only part of the chunk, and we don't want index out of bound
				const FIntVector3 MinVoxelCoordInChunk = FIntVector3::Max(FIntVector3(0), MinVoxelCoord - ChunkVoxelCoord); // Inclusive
				const FIntVector3 MaxVoxelCoordInChunk = FIntVector3::Min(FIntVector3(CHUNK_SIZE - 1), MaxVoxelCoord - ChunkVoxelCoord); // Inclusive

				for (int32 LocalZ = MinVoxelCoordInChunk.Z; LocalZ <= MaxVoxelCoordInChunk.Z; ++LocalZ)
				{
					for (int32 LocalY = MinVoxelCoordInChunk.Y; LocalY <= MaxVoxelCoordInChunk.Y; ++LocalY)
					{
						const int32 OutputSizeX = MaxVoxelCoordInChunk.X - MinVoxelCoordInChunk.X + 1; // Num to copy from original array in the x axis
						const int32 LocalXStart = MinVoxelCoordInChunk.X; // Start copy from where in the original array? 

						const int32 LocalVoxelStartIndex = LocalXStart | (LocalY << Y_SHIFT) | (LocalZ << Z_SHIFT);

						const int32 OutputX = ChunkVoxelCoord.X + LocalXStart - MinVoxelCoord.X; // Location in output array
						const int32 OutputY = ChunkVoxelCoord.Y + LocalY - MinVoxelCoord.Y; // Location in output array
						const int32 OutputZ = ChunkVoxelCoord.Z + LocalZ - MinVoxelCoord.Z; // Location in output array

						const int32 OutputIndex = OutputX + OutputY * OutputDimensions.X + OutputZ * OutputDimensions.X * OutputDimensions.Y;

						if (ChunkIndex != nullptr)
						{
							FMemory::Memcpy(&OutVoxelsArray[OutputIndex], &LoadedChunks[*ChunkIndex].Voxels[LocalVoxelStartIndex], OutputSizeX * sizeof(uint8));
						}
						else
						{
							FMemory::Memset(&OutVoxelsArray[OutputIndex], 0, OutputSizeX * sizeof(uint8));
						}
					}
				}

			}
		}
	}

}

bool AVoxelTerrain::HasChunk(const FIntVector3& ChunkCoord)
{
	FScopeLock LoadedChunksLock(&LoadedChunksMutex);
	const int32* const ChunkIndex = LoadedChunksIndex.Find(ChunkCoord);
	return ChunkIndex != nullptr;
}

bool AVoxelTerrain::HasAllNeighborChunks(const FIntVector3& ChunkCoord)
{
	FScopeLock LoadedChunksLock(&LoadedChunksMutex);

	// Moore's neighbor
	for (int32 i = 0; i < 9; ++i)
	{
		const int32* const ChunkIndex = LoadedChunksIndex.Find(ChunkCoord + FIntVector3(i % 3 - 1, i / 3 - 1, 0));
		if (ChunkIndex == nullptr)
		{
			return false;
		}
	}
	return true;
}

FChunk& AVoxelTerrain::GetChunkAt(const FIntVector3& ChunkCoord)
{
	FScopeLock LoadedChunksLock(&LoadedChunksMutex);
	const int32* const ChunkIndex = LoadedChunksIndex.Find(ChunkCoord);

	checkf(ChunkIndex != nullptr, TEXT("FChunk& AVoxelTerrain::GetChunkAt(const FIntVector3&) -> Chunk <%s> is not loaded!!!!"), *ChunkCoord.ToString());

	return LoadedChunks[*ChunkIndex];
}

FChunk& AVoxelTerrain::GetChunkAndIndexAt(const FIntVector3& ChunkCoord, int32& OutChunkIndex)
{
	FScopeLock LoadedChunksLock(&LoadedChunksMutex);
	const int32* const ChunkIndex = LoadedChunksIndex.Find(ChunkCoord);

	checkf(ChunkIndex != nullptr, TEXT("FChunk& AVoxelTerrain::GetChunkAndIndexAt(const FIntVector3&, int32&) -> Chunk <%s> is not loaded!!!!"), *ChunkCoord.ToString());

	OutChunkIndex = *ChunkIndex;
	return LoadedChunks[*ChunkIndex];
}

int32 AVoxelTerrain::GetChunkIndexAt(const FIntVector3& ChunkCoord)
{
	FScopeLock LoadedChunksLock(&LoadedChunksMutex);
	const int32* const ChunkIndex = LoadedChunksIndex.Find(ChunkCoord);

	checkf(ChunkIndex != nullptr, TEXT("int32 AVoxelTerrain::GetChunkIndexAt(const FIntVector3&) -> Chunk <%s> is not loaded!!!!"), *ChunkCoord.ToString());

	return *ChunkIndex;
}

bool AVoxelTerrain::HasChunkColumn(const FIntVector2D& ChunkColumnCoord)
{
	// The Scope Lock is in HasChunk(), so we don't lock it here to prevent a dead lock
	for (int32 i = 0; i < WORLD_HEIGHT_CHUNKS; ++i)
	{
		if (!HasChunk(FIntVector3(ChunkColumnCoord.X, ChunkColumnCoord.Y, i)))
		{
			return false;
		}
	}
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CHUNK MESH METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Returns whether the mesh for the chunks at ChunkCoord is loaded. */
bool AVoxelTerrain::HasChunkMesh(const FIntVector3& ChunkCoord)
{
	FScopeLock MeshLock(&LoadedChunkMeshesMutex);
	const UChunkMeshComponent* const Mesh = LoadedChunkMeshComponents.FindRef(ChunkCoord);
	return Mesh != nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CHUNK COLLISION METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AVoxelTerrain::HasChunkCollision(const FIntVector3& ChunkCoord)
{
	FScopeLock CollisionLock(&LoadedChunkCollisionsMutex);
	const UChunkCollisionComponent* const Collision = LoadedChunkCollisionComponents.FindRef(ChunkCoord);
	return Collision != nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// HEIGHTMAP METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int32 AVoxelTerrain::GetHeightAt(const int32& x, const int32& y)
{
	FScopeLock LoadedChunksLock(&LoadedChunksMutex);

	const int32 ChunkX = x >> CHUNK_SHIFT;
	const int32 ChunkY = y >> CHUNK_SHIFT;

	const int32 LocalX = x & CHUNK_SIZE_MASK;
	const int32 LocalY = y & CHUNK_SIZE_MASK;

	int32 Height = -1;

	// We have to loop through the whole column and add the height values together
	for (int32 ChunkZ = WORLD_HEIGHT_CHUNKS - 1; ChunkZ >= 0; --ChunkZ)
	{
		const FIntVector3 ChunkCoord(ChunkX, ChunkY, ChunkZ);
		const int32* const ChunkIndex = LoadedChunksIndex.Find(ChunkCoord);

		if (ChunkIndex != nullptr)
		{
			const int32& LocalHeight = LoadedChunks[*ChunkIndex].HighestSolidVoxels[LocalX | (LocalY << Y_SHIFT)];
			if (LocalHeight >= 0)
			{
				const int32 ChunkVoxelZ = ChunkZ << CHUNK_SHIFT;
				Height = ChunkVoxelZ + LocalHeight;
				break;
			}
		}
	}

	return Height;
}

void AVoxelTerrain::GetChunkHeightsArray(const FIntVector2D& HeightmapCoord, TArray<int32>& OutHeightsArray)
{
	FScopeLock LoadedChunksLock(&LoadedChunksMutex);

	OutHeightsArray.Init(-1, 1 << (2 * CHUNK_SHIFT));

	const int32& ChunkX = HeightmapCoord.X;
	const int32& ChunkY = HeightmapCoord.Y;

	// Loops through the chunks and add the heights together
	for (int32 ChunkZ = WORLD_HEIGHT_CHUNKS - 1; ChunkZ >= 0; --ChunkZ)
	{
		const FIntVector3 ChunkCoord(ChunkX, ChunkY, ChunkZ);
		const int32* const ChunkIndex = LoadedChunksIndex.Find(ChunkCoord);

		if (ChunkIndex != nullptr)
		{
			for (int32 LocalY = 0; LocalY < CHUNK_SIZE; ++LocalY) // For each height in this chunk
			{
				for (int32 LocalX = 0; LocalX < CHUNK_SIZE; ++LocalX) // For each height in this chunk
				{
					const int32 HeightIndex = LocalX | (LocalY << Y_SHIFT);
					const int32& LocalHeight = LoadedChunks[*ChunkIndex].HighestSolidVoxels[HeightIndex]; // Height of this current voxel in chunk
					const int32& OutputHeight = OutHeightsArray[HeightIndex]; // Current height stored in Output Array

																			  // If the this height is larger than zero AND the output height does not already have a value
					if (LocalHeight >= 0 && OutputHeight < 0)
					{
						const int32 ChunkVoxelZ = ChunkZ << CHUNK_SHIFT;
						OutHeightsArray[HeightIndex] = ChunkVoxelZ + LocalHeight;
					}
				}
			}
		}
	}
}

void AVoxelTerrain::GetHeightsArray(const FIntVector2D& MinHeightCoord, const FIntVector2D& MaxHeightCoord, TArray<int32>& OutHeightsArray)
{
	FScopeLock LoadedChunksLock(&LoadedChunksMutex);

	const FIntVector2D& MinHeightmapCoord = MinHeightCoord >> CHUNK_SHIFT;
	const FIntVector2D& MaxHeightmapCoord = MaxHeightCoord >> CHUNK_SHIFT;
	const FIntVector2D& OutputDimensions = MaxHeightCoord + FIntVector2D(1) - MinHeightCoord;

	OutHeightsArray.Init(-1, OutputDimensions.X * OutputDimensions.Y);

	// Loops through all possible chunk <x, y> coordinate
	for (int32 ChunkY = MinHeightmapCoord.Y; ChunkY <= MaxHeightmapCoord.Y; ++ChunkY)
	{
		for (int32 ChunkX = MinHeightmapCoord.X; ChunkX <= MaxHeightmapCoord.X; ++ChunkX)
		{
			const FIntVector2D HeightmapVoxelCoord(ChunkX << CHUNK_SHIFT, ChunkY << CHUNK_SHIFT);

			// The min and max coord may be only part of the chunk, and we don't want index out of bound
			const FIntVector2D MinVoxelCoordInChunk = FIntVector2D::Max(FIntVector2D(0), MinHeightCoord - HeightmapVoxelCoord); // Inclusive
			const FIntVector2D MaxVoxelCoordInChunk = FIntVector2D::Min(FIntVector2D(CHUNK_SIZE - 1), MaxHeightCoord - HeightmapVoxelCoord); // Inclusive

																																			 // Add the heights together
			for (int32 ChunkZ = WORLD_HEIGHT_CHUNKS - 1; ChunkZ >= 0; --ChunkZ)
			{
				const FIntVector3 ChunkCoord(ChunkX, ChunkY, ChunkZ);
				const int32* const ChunkIndex = LoadedChunksIndex.Find(ChunkCoord);

				if (ChunkIndex != nullptr)
				{
					for (int32 LocalY = MinVoxelCoordInChunk.Y; LocalY <= MaxVoxelCoordInChunk.Y; ++LocalY) // For each height in this chunk
					{
						for (int32 LocalX = MinVoxelCoordInChunk.X; LocalX <= MaxVoxelCoordInChunk.X; ++LocalX) // For each height in this chunk
						{
							const int32 LocalHeightIndex = LocalX | (LocalY << Y_SHIFT);
							const int32& LocalHeight = LoadedChunks[*ChunkIndex].HighestSolidVoxels[LocalHeightIndex]; // Height of this current voxel in chunk

							const int32 OutputX = HeightmapVoxelCoord.X + LocalX - MinHeightCoord.X; // Location in output array
							const int32 OutputY = HeightmapVoxelCoord.Y + LocalY - MinHeightCoord.Y; // Location in output array
							const int32 OutputIndex = OutputX + OutputY * OutputDimensions.X;
							const int32& OutputHeight = OutHeightsArray[OutputIndex]; // Current height stored in Output Array
																					  // If the this height is larger than zero AND the output height does not already have a value
							if (LocalHeight >= 0 && OutputHeight < 0)
							{
								const int32 ChunkVoxelZ = ChunkZ << CHUNK_SHIFT;
								OutHeightsArray[OutputIndex] = ChunkVoxelZ + LocalHeight;
							}
						}
					}
				}

			}

		}
	}

}