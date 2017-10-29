// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "IntVectors.h"

#include "Components/PrimitiveComponent.h"
#include "ChunkCollisionComponent.generated.h"

/**
 * 
 */
UCLASS()
class AETHERIAGAME_API UChunkCollisionComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
	
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chunk Collision Component")
	FIntVector3 ChunkCoord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chunk Collision Component")
	class AVoxelTerrain* VoxelTerrain;

private:

	UPROPERTY(Transient, DuplicateTransient)
	class UBodySetup* CollisionBodySetup;

public:

	UChunkCollisionComponent(const FObjectInitializer& ObjectInitializer);

	virtual void DestroyComponent(bool bPromoteChildren = false) override;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual FBoxSphereBounds CalcBounds(const FTransform & LocalToWorld) const override;

	virtual class UBodySetup* GetBodySetup() override;

private:

	void CreateCollisionBodySetup();
	
	
};
