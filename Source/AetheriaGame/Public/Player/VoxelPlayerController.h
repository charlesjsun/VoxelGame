// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "IntVectors.h"

#include "GameFramework/PlayerController.h"
#include "VoxelPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class AETHERIAGAME_API AVoxelPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	AVoxelPlayerController(const class FObjectInitializer& ObjectInitializer);
	
protected:

	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

	virtual void Tick(float DeltaTime) override;

	// Update methods for chunks, meshes, and collision meshes
	void UpdateChunksAroundPlayer(const double& StartTime);
	void UpdateMeshesAroundPlayer(const double& StartTime);
	void UpdateCollisionAroundPlayer(const double& StartTime);

protected:

	// Test variable
	float time = 0.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient, Category = "Voxel Terrain")
	class AVoxelTerrain* VoxelTerrain;

public:

	/// ///////////////////////////////////////////
	/// CHUNK METHODS
	/// ///////////////////////////////////////////

	// Returns true if the voxel <x, y, z> is in a chunk that can be loaded
	bool IsVoxelInLoadDistance(const int32& x, const int32& y, const int32& z) const;

	// Interface method for setting a voxel
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel Player Controller")
	void SetVoxel(const int32& x, const int32& y, const int32& z, const uint8& Voxel);

	// Interface method for getting a voxel
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel Player Controller")
	uint8 GetVoxel(const int32& x, const int32& y, const int32& z);

	/**
	*  @param TraceDistance - the distance int Unreal Units that the trace extends
	*  @param OutVoxelCoord - the output of the hit voxel coordinate. <0, 0, -1> if not hit
	*  @param OutHitNormal - the output hit normal
	*  @returns whether a voxel was hit or not
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel Player Controller")
	bool RayTraceVoxelCoord(const float TraceDistance, FIntVector3& OutVoxelCoord, FIntVector3& OutHitNormal);


};
