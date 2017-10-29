// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "IntVectors.h"
#include "ChunkUtils.h"
#include "TerrainParameters.h"

#include "GameFramework/Actor.h"
#include "VoxelTerrain.generated.h"

UCLASS()
class AETHERIAGAME_API AVoxelTerrain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVoxelTerrain(const class FObjectInitializer& ObjectInitializer);

	virtual void BeginDestroy() override;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel Terrain")
	static FIntVector3 WorldToChunkCoord(const FIntVector3& WorldCoord);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel Terrain")
	static FIntVector3 ChunkToWorldCoord(const FIntVector3& ChunkCoord);


private:

	class FTerrainGenerationThread* TerrainGenerationThread;

	/* Stores all the Chunks in the terrain, indexed by LoadedChunksIndex */
	UPROPERTY(Transient, DuplicateTransient)
	TArray<FChunk> LoadedChunks;
	/* Stores the index of the Chunk at a Chunk Coordinate, to access the Chunk in LoadedChunks */
	TMap<FIntVector3, int32> LoadedChunksIndex;
	/* The Mutex for both LoadedChunksIndex AND LoadedChunks */
	FCriticalSection LoadedChunksMutex;

	/* Stores the Mesh Component for at a Chunk Coordinate. Not Thread Safe because only main game thread should create/modify UObject. */
	TMap<FIntVector3, class UChunkMeshComponent*> LoadedChunkMeshComponents;
	/* The Mutex for LoadedChunkMeshComponents */
	FCriticalSection LoadedChunkMeshesMutex;

	/* Stores the Collision Component for at a Chunk Coordinate. Not Thread Safe because only main game thread should create/modify UObject. */
	TMap<FIntVector3, class UChunkCollisionComponent*> LoadedChunkCollisionComponents;
	/* The Mutex for LoadedChunkCollisionComponents */
	FCriticalSection LoadedChunkCollisionsMutex;

	/* Chunks that are currently being loaded/generated */
	TSet<FIntVector3> PendingChunks;

	float time = 0.0f;

public:

	/* Parameter for terrain generation */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel Terrain")
	FTerrainGeneratorParameters TerrainGenParameters;

	/* Parameter for voxel terrain */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel Terrain")
	FTerrainParameters TerrainParameters;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UPDATE METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:

	virtual void Tick(float DeltaSeconds) override;

	bool IsChunkPending(const FIntVector3& ChunkCoord);

	void LoadChunk(const FIntVector3& ChunkCoord);

	/** Add a chunk to the client/server's loaded chunks */
	void AddChunk(FChunk& Chunk);

	/** Updates the light values in the verticle chunk */
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain")
	void UpdateChunkHeightmap(const FIntVector3& ChunkCoord);

	void GenerateMesh(const FIntVector3& ChunkCoord);

	void GenerateCollision(const FIntVector3& ChunkCoord);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VOXEL/CHUNK METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel Terrain")
	FVoxelType GetVoxelType(const uint8& Voxel) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel Terrain")
	int32 GetNumVoxelTypes() const;

	/**
	*  Get the voxel at voxel coordinate <x, y, z>
	*  @returns voxel type as uint8
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel Terrain")
	uint8 GetVoxelAt(const int32& x, const int32& y, const int32& z);

	/** Set the voxel at voxel coordinate <x, y, z> */
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain")
	void SetVoxelAt(const int32& x, const int32& y, const int32& z, uint8 Voxel);

	/**
	*  Mark an area of voxels to be 'dirty' and use that area to determine which chunks
	*  need mesh updates, ambient occlusion updates only, and/or collision update.
	*  Ambient Occlusion updates will be determined by the AORadius of the terrain.
	*	@param MinVoxelCoord - Lower bound of the area of voxels that are changed (Inclusive)
	*  @param MaxVoxelCoord - Upper bound of the area of voxels that are changed (Inclusive)
	*/
	void MarkVoxelAreaDirty(const FIntVector3& MinVoxelCoord, const FIntVector3& MaxVoxelCoord);

	/**
	*  Mark a single voxel to be 'dirty' and use that voxel to determine which chunks
	*  need mesh updates, ambient occlusion updates only, and/or collision update.
	*  Ambient Occlusion updates will be determined by the AORadius of the terrain.
	*/
	void MarkVoxelDirty(const FIntVector3& VoxelCoord);

	/**
	*  This method should be used with caution as it is the least imprecise and creates the most updates. Use Mark Voxel Dirty instead.
	*  Mark a single Chunk to be 'dirty', all chunks around will get a mesh update and a collision update.
	*  Also all chunks of all Z Coordinate in its Moore's neighbour will get an ambient occlusion update.
	*/
	void MarkChunkDirty(const FIntVector3& ChunkCoord);

	/**
	*  Get the voxels array at the chunk coordinate
	*  @param OutVoxelsArray - Empty TArray that the function outputs to
	*/
	void GetChunkVoxelsArray(const FIntVector3& ChunkCoord, TArray<uint8>& OutVoxelsArray);

	/**
	*  Gets all the voxels from MinVoxelCoord to MaxVoxelCoord and store in an array.
	*	@param MinVoxelCoord - Lower bound of the block of voxels to get (Inclusive)
	*  @param MaxVoxelCoord - Upper bound of the block of voxels to get (Inclusive)
	*  @param OutVoxelsArray - Empty TArray that the function outputs to.
	*/
	void GetVoxelsArray(const FIntVector3& MinVoxelCoord, const FIntVector3& MaxVoxelCoord, TArray<uint8>& OutVoxelsArray);


	/** Returns whether the chunk at chunk coord is loaded on this Client/Server */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel Terrain")
	bool HasChunk(const FIntVector3& ChunkCoord);

	bool HasAllNeighborChunks(const FIntVector3& ChunkCoord);

	/** Returns a FChunk Reference at ChunkCoord. Warning: Only use when ChunkCoord is sure to exist, raises error if not. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel Terrain")
	FChunk& GetChunkAt(const FIntVector3& ChunkCoord);

	/** Returns a FChunk Reference AND its index at ChunkCoord. Warning: Only use when ChunkCoord is sure to exist, raises error if not. */
	FChunk& GetChunkAndIndexAt(const FIntVector3& ChunkCoord, int32& OutChunkIndex);

	/** Returns the chunk's index at ChunkCoord. Warning: Only use when ChunkCoord is sure to exist, raises error if not. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel Terrain")
	int32 GetChunkIndexAt(const FIntVector3& ChunkCoord);

	/** Returns whether all the chunks at <x, y> are loaded. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel Terrain")
	bool HasChunkColumn(const FIntVector2D& ChunkColumnCoord);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CHUNK MESH METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/** Returns whether the Collision for the chunks at ChunkCoord is loaded. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel Terrain")
	bool HasChunkMesh(const FIntVector3& ChunkCoord);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CHUNK COLLISION METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/** Returns whether the Collision for the chunks at ChunkCoord is loaded. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel Terrain")
	bool HasChunkCollision(const FIntVector3& ChunkCoord);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// HEIGHTMAP METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/** Get the heightmap value at world voxel coordinate <x, y> */
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain")
	int32 GetHeightAt(const int32& x, const int32& y);

	/**
	*  Get the heightmap array at the chunk column coordinate
	*  @param OutVoxelsArray - Empty TArray that serves as output
	*/
	void GetChunkHeightsArray(const FIntVector2D& HeightmapCoord, TArray<int32>& OutHeightsArray);

	/**
	*  Gets all the heights from MinHeightCoord to MaxHeightCoord and store in an array.
	*  @param MinHeightCoord - Lower bound (in voxels) of the height to get (Inclusive)
	*  @param MaxHeightCoord - Upper bound (in voxels) of the height to get (Inclusive)
	*  @param OutHeightmapArray - Empty TArray that the function outputs to.
	*/
	void GetHeightsArray(const FIntVector2D& MinHeightCoord, const FIntVector2D& MaxHeightCoord, TArray<int32>& OutHeightsArray);

	
};
