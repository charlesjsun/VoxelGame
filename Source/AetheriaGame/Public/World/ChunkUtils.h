// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "IntVectors.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChunkUtils.generated.h"

#define WORLD_HEIGHT (128)
#define WORLD_HEIGHT_CHUNKS (4)

#define CHUNK_SIZE (32)

// Mask used when converting from world to local space. Used by x & CHUNK_SIZE_MASK. Equal CHUNK_SIZE - 1
#define CHUNK_SIZE_MASK (31)

// Default bitshift amount. Equal to log(CHUNK_SIZE)/log(2)
#define CHUNK_SHIFT (5)

// Amount to shift by using bitshift when converting from <x, y, z> format to int format
#define Y_SHIFT (5)
#define Z_SHIFT (10)

// Check if <x, y, z> is inside a chunk
#define IS_IN_CHUNK_BOUND(x, y, z) (x >= 0 && y >= 0 && z >= 0 && x < CHUNK_SIZE && y < CHUNK_SIZE && z < CHUNK_SIZE)

// Convert between <x, y, z> format to int format
// Linearizing 3D coordinates to store voxel data in a linear array.
#define COORD_FROM_INT(i) FIntVector3((i) % CHUNK_SIZE, ((i) >> Y_SHIFT) % CHUNK_SIZE, (i) >> Z_SHIFT)
#define COORD_TO_INT(x, y, z) ((x) | ((y) << Y_SHIFT) | ((z) << Z_SHIFT))

// Different faces of a cube
UENUM(BlueprintType)
enum class EVoxelFace : uint8
{
	TOP = 0 UMETA(DisplayName = "Top"),
	BOTTOM = 1 UMETA(DisplayName = "Bottom"),
	LEFT = 2 UMETA(DisplayName = "Left"),
	RIGHT = 3 UMETA(DisplayName = "Right"),
	FRONT = 4 UMETA(DisplayName = "Front"),
	BACK = 5 UMETA(DisplayName = "Back")
};

// Defines what a Chunk is. Used by Voxel Terrain. Indexed by Chunk Position
USTRUCT(BlueprintType)
struct FChunk
{

	GENERATED_BODY()

		// Coordinate in chunk space of a chunk.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel Chunk")
	FIntVector3 ChunkPosition;

	// Coodinate in world space of a chunk.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel Chunk")
	FIntVector3 ChunkPositionInVoxels;

	// Size -> 1 << (3 * CHUNK_SHIFT)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel Chunk")
	TArray<uint8> Voxels;

	// Size -> 1 << (2 * CHUNK_SHIFT) 
	TArray<int8> HighestSolidVoxels;

	FChunk() {}

	FChunk(FIntVector3 InChunkPosition)
	{
		ChunkPosition = InChunkPosition;
		ChunkPositionInVoxels = FIntVector3(ChunkPosition.X << CHUNK_SHIFT, ChunkPosition.Y << CHUNK_SHIFT, ChunkPosition.Z << CHUNK_SHIFT);
	}

};

/**
 * 
 */
UCLASS()
class AETHERIAGAME_API UChunkUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Chunk Utils")
	static FIntVector3 GetNormal(EVoxelFace Face);
	
	
};
