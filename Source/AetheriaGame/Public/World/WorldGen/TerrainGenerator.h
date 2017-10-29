// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "ChunkUtils.h"
#include "TerrainParameters.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "TerrainGenerator.generated.h"

/**
 * 
 */
UCLASS()
class AETHERIAGAME_API UTerrainGenerator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	// Procedurally generates a Chunk based on noise
	UFUNCTION(Category = "Terrain Generator", BlueprintCallable)
	static void GenerateChunk(const class AVoxelTerrain* const VoxelTerrain, FChunk& Chunk, const FTerrainGeneratorParameters& Parameter);

private:

	static void MakeTree(const FTerrainGeneratorParameters& Parameter, const uint8 LeafType, const FIntVector3& Position, const FIntVector3& ChunkPosInVoxels, TArray<uint8>& ExtraVoxels);

};
