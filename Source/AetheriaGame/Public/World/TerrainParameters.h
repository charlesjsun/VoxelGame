// Copyright (c) 2016, Charles JL Sun, All rights reserved.

#pragma once

#include "Runtime/Engine/Classes/Engine/DataTable.h"

#include "Engine/EngineTypes.h"
#include "TerrainParameters.generated.h"

/**
WIP. NOT USED IN CODE YET
The data table type for voxel data, so user defined voxel types would be easier to create
*/
USTRUCT(BlueprintType)
struct FVoxelTypeData : public FTableRowBase
{

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Type")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Type")
	FColor VoxelColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Type")
	TAssetPtr<class UMaterialInterface> VoxelMaterial;

	FVoxelTypeData() {};

};

/**
Stores information about a voxel using the flyweight design pattern.
Used by terrain, indexed by type id.
*/
USTRUCT(BlueprintType)
struct FVoxelType
{

	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel Type")
	FColor VoxelColor;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel Type")
	UMaterialInterface* VoxelMaterial;

	FVoxelType() {};

	FVoxelType(FColor InVoxelColor, UMaterialInterface* InVoxelMaterial)
		: VoxelColor(InVoxelColor), VoxelMaterial(InVoxelMaterial) {};

};

/**
Stores information about a terrain.
Seperate struct because it's easier to change and edit. Used by Voxel Terrain.
*/
USTRUCT(BlueprintType)
struct FTerrainParameters
{

	GENERATED_BODY()

	/* A radius around the player in which terrains are generated and rendered */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Parameters")
	int32 DrawDistanceInChunks;

	/* Number of extra chunk radius for expansion */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Parameters")
	int32 LoadExpansionRadius;

	/* A radius around the player in which collision meshes are generated */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Parameters")
	int32 CollisionDistanceInChunks;

	/* The blur radius for ambient occlusion calculation */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Parameters")
	int32 AOBlurRadus;

	/* The max time in seconds allowed between each update/tick */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Parameters")
	float MaxUpdateTime;

	/* All the Voxel Types and their index */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Parameters")
	TArray<FVoxelType> VoxelTypes;

};

/**
Default parameters used by the terrain generator to procedurally generate the terrain.
TODO: Replace by individual biome system
*/
USTRUCT(BlueprintType)
struct FTerrainGeneratorParameters
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Gen Parameter")
	bool bUseRidgedMulti;

	// The seed of our fractal
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Gen Parameter")
	int32 Seed;

	// The number of octaves that the noise generator will use
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Gen Parameter")
	int32 NoiseOctaves;

	// The frequency of the noise
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Gen Parameter")
	float NoiseFrequency;

	// Frequency is multiplied by this every octave. Bigger than 1.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Gen Parameter")
	float NoiseLacunarity;

	// Amptitude is multiplied by this every octave. Less than 1.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Gen Parameter")
	float NoisePersistance;

	// Scale
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Gen Parameter")
	float NoiseScale;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Gen Parameter")
	bool bCreateTrees;

	// Tree's Noise Scale
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Gen Parameter")
	float TreeScale;

	// Tree's Noise Octaves
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Gen Parameter")
	float TreeOctaves;

	// Tree's Density (between 0 - 1)
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Gen Parameter")
	float TreeDensity;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Gen Parameter")
	int32 SnowVoxel;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Gen Parameter")
	int32 GrassVoxel;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Gen Parameter")
	bool bSnowOnTrees;


};