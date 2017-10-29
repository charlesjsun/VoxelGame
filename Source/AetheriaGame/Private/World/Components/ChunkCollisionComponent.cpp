// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#include "ChunkCollisionComponent.h"

#include "PhysicsEngine/BodySetup.h"
#include "Engine/CollisionProfile.h"
#include "ChunkUtils.h"

#include "VoxelTerrain.h"

UChunkCollisionComponent::UChunkCollisionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	bVisible = true;
	bAutoRegister = false;
	bCanEverAffectNavigation = true;

	SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
}

void UChunkCollisionComponent::DestroyComponent(bool bPromoteChildren)
{
	VoxelTerrain = nullptr;

	Super::DestroyComponent(bPromoteChildren);
}

FPrimitiveSceneProxy* UChunkCollisionComponent::CreateSceneProxy()
{
	CreateCollisionBodySetup();

	return nullptr;
}

FBoxSphereBounds UChunkCollisionComponent::CalcBounds(const FTransform & LocalToWorld) const
{
	FBoxSphereBounds NewBounds;
	NewBounds.Origin = NewBounds.BoxExtent = FVector(CHUNK_SIZE) / 2.0f;
	NewBounds.SphereRadius = NewBounds.BoxExtent.Size();
	return NewBounds.TransformBy(LocalToWorld);
}

UBodySetup* UChunkCollisionComponent::GetBodySetup()
{
	// Create the collision if it hasn't already
	if (CollisionBodySetup == nullptr)
	{
		CreateCollisionBodySetup();
	}
	return CollisionBodySetup;
}

void UChunkCollisionComponent::CreateCollisionBodySetup()
{
	if (CollisionBodySetup == nullptr)
	{
		CollisionBodySetup = NewObject<UBodySetup>(this);
		CollisionBodySetup->CollisionTraceFlag = CTF_UseSimpleAsComplex;
	}

	CollisionBodySetup->AggGeom.BoxElems.Reset();

	const FIntVector3 ChunkVoxelCoord = AVoxelTerrain::ChunkToWorldCoord(ChunkCoord);

	// gets the voxels with one extra width on each side for adjacency testing
	TArray<uint8> Voxels;
	VoxelTerrain->GetVoxelsArray(ChunkVoxelCoord - FIntVector3(1), ChunkVoxelCoord + FIntVector3(CHUNK_SIZE), Voxels);

	const int32 Size = CHUNK_SIZE + 2;

	// Loop through each voxels in the actual chunk
	for (int32 z = 1; z <= CHUNK_SIZE; z++)
	{
		for (int32 y = 1; y <= CHUNK_SIZE; y++)
		{
			for (int32 x = 1; x <= CHUNK_SIZE; x++)
			{
				const FIntVector3 CurrentVoxelCoord(x, y, z);
				const uint8 CurrentVoxel = Voxels[x + y * Size + z * Size * Size];
				// Generate a box if the current voxel is not air
				if (CurrentVoxel != 0)
				{
					// Loops through each face to see if there is a neighbor that's empty
					bool bHasEmptyNeighbours = false;
					for (int32 Face = 0; Face < 6; Face++)
					{
						const FIntVector3 NeighbourCoord = CurrentVoxelCoord + UChunkUtils::GetNormal(EVoxelFace(Face));
						const uint8 NeighbourVoxel = Voxels[NeighbourCoord.X + NeighbourCoord.Y * Size + NeighbourCoord.Z * Size * Size];
						if (NeighbourVoxel == 0)
						{
							bHasEmptyNeighbours = true;
							break;
						}
					}

					// Generate a collision box only if there is an empty neighbor
					if (bHasEmptyNeighbours)
					{
						const FIntVector3 LocalVoxelCoord(x - 1, y - 1, z - 1);
						FKBoxElem* BoxElem = new(CollisionBodySetup->AggGeom.BoxElems) FKBoxElem;
						BoxElem->Center = LocalVoxelCoord.ToFloat() + FVector(0.5f);
						BoxElem->X = 1;
						BoxElem->Y = 1;
						BoxElem->Z = 1;
					}
				}
			}
		}
	}

	RecreatePhysicsState();

}
