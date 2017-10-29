// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#include "VoxelPlayerController.h"

#include "VoxelPlayerCameraManager.h"
#include "GameFramework/Character.h"

#include "Kismet/GameplayStatics.h"

#include "VoxelTerrain.h"

#include "ChunkUtils.h"

#include "DebugLibrary.h"

AVoxelPlayerController::AVoxelPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerCameraManagerClass = AVoxelPlayerCameraManager::StaticClass();

}

void AVoxelPlayerController::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> OutTerrain;
	UGameplayStatics::GetAllActorsOfClass(this, AVoxelTerrain::StaticClass(), OutTerrain);
	VoxelTerrain = Cast<AVoxelTerrain>(OutTerrain[0]);

}

void AVoxelPlayerController::BeginDestroy()
{
	VoxelTerrain = nullptr;

	Super::BeginDestroy();
}

void AVoxelPlayerController::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);

	// Just in case, get the voxel terrain ref
	if (VoxelTerrain == nullptr)
	{
		TArray<AActor*> OutTerrain;
		UGameplayStatics::GetAllActorsOfClass(this, AVoxelTerrain::StaticClass(), OutTerrain);
		VoxelTerrain = Cast<AVoxelTerrain>(OutTerrain[0]);
	}

	//UE_LOG(LogStats, Log, TEXT("TICK"));
	const double StartTime = FPlatformTime::Seconds();
	//UE_LOG(LogStats, Log, TEXT("TICK BEFORE CHUNK"));
	UpdateChunksAroundPlayer(StartTime);
	//UE_LOG(LogStats, Log, TEXT("TICK BEFORE MESH"));
	UpdateMeshesAroundPlayer(StartTime);
	//UE_LOG(LogStats, Log, TEXT("TICK BEFORE COLLISION"));
	UpdateCollisionAroundPlayer(StartTime);

}

void AVoxelPlayerController::UpdateChunksAroundPlayer(const double& StartTime)
{

	const FIntVector3 LoadDimensions(VoxelTerrain->TerrainParameters.DrawDistanceInChunks + VoxelTerrain->TerrainParameters.LoadExpansionRadius);
	const int32 LoadDistanceSquared = FMath::Square<int32>(LoadDimensions.X);

	// Player Variables
	const FVector PlayerPosition = GetFocalLocation();

	// Changing player position from world position into block then chunk position
	const FIntVector3 PlayerChunkPosition((int32)(PlayerPosition.X / 100.0f) >> CHUNK_SHIFT, (int32)(PlayerPosition.Y / 100.0f) >> CHUNK_SHIFT, 0);

	// Positions for looping
	const FIntVector3 MinLoadChunkCoord = PlayerChunkPosition - LoadDimensions;
	const FIntVector3 MaxLoadChunkCoord = PlayerChunkPosition + LoadDimensions;

	/// CHUNK GENERATION
	for (int32 ChunkX = MinLoadChunkCoord.X; ChunkX <= MaxLoadChunkCoord.X; ++ChunkX)
	{
		for (int32 ChunkY = MinLoadChunkCoord.Y; ChunkY <= MaxLoadChunkCoord.Y; ++ChunkY)
		{
			// Distance of chunk column to player in chunks, squared so we don't do sqrt which is slow
			const int32 DistanceToPlayerSquared = FMath::Square<int32>(ChunkX - PlayerChunkPosition.X) + FMath::Square<int32>(ChunkY - PlayerChunkPosition.Y);

			// Circular load
			if (DistanceToPlayerSquared < LoadDistanceSquared)
			{
				// Load a column of chunks
				for (int32 ChunkZ = 0; ChunkZ < WORLD_HEIGHT_CHUNKS; ++ChunkZ)
				{
					const FIntVector3 ChunkCoord(ChunkX, ChunkY, ChunkZ);
					if (!VoxelTerrain->HasChunk(ChunkCoord) && !VoxelTerrain->IsChunkPending(ChunkCoord))
					{
						UDebugLibrary::Println(this, 10, FString::Printf(TEXT("Load %s"), *ChunkCoord.ToString()));
						VoxelTerrain->LoadChunk(ChunkCoord);
						return;
					}
					// Only load if the chunk isn't loaded yet
				}

			}

		}//ChunkY

	}//ChunkX
}

void AVoxelPlayerController::UpdateMeshesAroundPlayer(const double& StartTime)
{

	const FVector PlayerPosition = GetFocalLocation();
	// Changing player position from world position into block then chunk position
	const FIntVector3 PlayerChunkPosition((int32)(PlayerPosition.X / 100.0f) >> CHUNK_SHIFT, (int32)(PlayerPosition.Y / 100.0f) >> CHUNK_SHIFT, 0);

	const FIntVector3 DrawDimensions(VoxelTerrain->TerrainParameters.DrawDistanceInChunks);
	const int32 DrawDistanceSquared = FMath::Square<int32>(VoxelTerrain->TerrainParameters.DrawDistanceInChunks);

	// Positions for looping
	const FIntVector3 MinDrawChunkCoord = PlayerChunkPosition - DrawDimensions;
	const FIntVector3 MaxDrawChunkCoord = PlayerChunkPosition + DrawDimensions;

	/// CHUNK GENERATION
	for (int32 ChunkX = MinDrawChunkCoord.X; ChunkX <= MaxDrawChunkCoord.X; ++ChunkX)
	{
		for (int32 ChunkY = MinDrawChunkCoord.Y; ChunkY <= MaxDrawChunkCoord.Y; ++ChunkY)
		{
			// Distance of chunk column to player in chunks, squared so we don't do sqrt which is slow
			const int32 DistanceToPlayerSquared = FMath::Square<int32>(ChunkX - PlayerChunkPosition.X) + FMath::Square<int32>(ChunkY - PlayerChunkPosition.Y);

			// Circular Draw
			if (DistanceToPlayerSquared < DrawDistanceSquared)
			{
				for (int32 ChunkZ = 0; ChunkZ < WORLD_HEIGHT_CHUNKS; ++ChunkZ)
				{
					const FIntVector3 ChunkCoord(ChunkX, ChunkY, ChunkZ);

					if (VoxelTerrain->HasAllNeighborChunks(ChunkCoord))
					{
						VoxelTerrain->GenerateMesh(ChunkCoord);
					}
				} // ChunkZ
			}
		} // ChunkY
	} // ChunkX

}

void AVoxelPlayerController::UpdateCollisionAroundPlayer(const double& StartTime)
{

	const FVector PlayerPosition = GetFocalLocation();
	// Changing player position from world position into block then chunk position
	const FIntVector3 PlayerChunkPosition((int32)(PlayerPosition.X / 100.0f) >> CHUNK_SHIFT, (int32)(PlayerPosition.Y / 100.0f) >> CHUNK_SHIFT, (int32)(PlayerPosition.Z / 100.0f) >> CHUNK_SHIFT);

	const FIntVector3 CollisionDimensions(VoxelTerrain->TerrainParameters.CollisionDistanceInChunks);
	const int32 CollisionDistanceSquared = FMath::Square<int32>(VoxelTerrain->TerrainParameters.CollisionDistanceInChunks);

	// Positions for looping
	const FIntVector3 UnboundMinCollisionChunkCoord = PlayerChunkPosition - CollisionDimensions;
	const FIntVector3 UnboundMaxCollisionChunkCoord = PlayerChunkPosition + CollisionDimensions;

	const FIntVector3 MinCollisionChunkCoord(UnboundMinCollisionChunkCoord.X, UnboundMinCollisionChunkCoord.Y, FMath::Max(0, UnboundMinCollisionChunkCoord.Z));
	const FIntVector3 MaxCollisionChunkCoord(UnboundMaxCollisionChunkCoord.X, UnboundMaxCollisionChunkCoord.Y, FMath::Min(WORLD_HEIGHT_CHUNKS - 1, UnboundMaxCollisionChunkCoord.Z));

	/// CHUNK GENERATION
	for (int32 ChunkX = MinCollisionChunkCoord.X; ChunkX <= MaxCollisionChunkCoord.X; ++ChunkX)
	{
		for (int32 ChunkY = MinCollisionChunkCoord.Y; ChunkY <= MaxCollisionChunkCoord.Y; ++ChunkY)
		{
			for (int32 ChunkZ = MinCollisionChunkCoord.Z; ChunkZ <= MaxCollisionChunkCoord.Z; ++ChunkZ)
			{
				// Distance of chunk column to player in cm, squared so we don't do sqrt which is slow
				const int32 DistanceToPlayerSquared = FMath::Square<int32>(ChunkX - PlayerChunkPosition.X) + FMath::Square<int32>(ChunkY - PlayerChunkPosition.Y) + FMath::Square<int32>(ChunkZ - PlayerChunkPosition.Z);

				// Spherical Generation
				if (DistanceToPlayerSquared < CollisionDistanceSquared)
				{
					FIntVector3 ChunkCoord(ChunkX, ChunkY, ChunkZ);
					if (VoxelTerrain->HasAllNeighborChunks(ChunkCoord))
					{
						VoxelTerrain->GenerateCollision(ChunkCoord);
					}
				}

			} // Chunk Z
		} // ChunkY
	} // ChunkX

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VOXEL TERRAIN
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// ///////////////////////////////////////////
/// CHUNK METHODS
/// ///////////////////////////////////////////

bool AVoxelPlayerController::IsVoxelInLoadDistance(const int32& x, const int32& y, const int32& z) const
{

	const int32 ChunkX = x >> CHUNK_SHIFT;
	const int32 ChunkY = y >> CHUNK_SHIFT;

	const int32 LoadDistanceSquared = FMath::Square<int32>(VoxelTerrain->TerrainParameters.DrawDistanceInChunks + VoxelTerrain->TerrainParameters.LoadExpansionRadius);

	const FVector PlayerPosition = GetFocalLocation();
	// Changing player position from world position into block then chunk position
	const FIntVector3 PlayerChunkPosition((int32)(PlayerPosition.X / 100.0f) >> CHUNK_SHIFT, (int32)(PlayerPosition.Y / 100.0f) >> CHUNK_SHIFT, 0);
	// Distance of chunk column to player in chunks, squared so we don't do sqrt which is slow
	const int32 DistanceToPlayerSquared = FMath::Square<int32>(ChunkX - PlayerChunkPosition.X) + FMath::Square<int32>(ChunkY - PlayerChunkPosition.Y);

	return DistanceToPlayerSquared <= LoadDistanceSquared;
}

void AVoxelPlayerController::SetVoxel(const int32& x, const int32& y, const int32& z, const uint8& Voxel)
{
	VoxelTerrain->SetVoxelAt(x, y, z, Voxel);
}

uint8 AVoxelPlayerController::GetVoxel(const int32& x, const int32& y, const int32& z)
{
	return VoxelTerrain->GetVoxelAt(x, y, z);
}

bool AVoxelPlayerController::RayTraceVoxelCoord(const float TraceDistance, FIntVector3& OutVoxelCoord, FIntVector3& OutHitNormal)
{
	// Self explanatory code

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerCameraManager->GetCameraViewPoint(CameraLocation, CameraRotation);
	const FVector EndTraceLocation = CameraLocation + CameraRotation.Vector() * TraceDistance;

	FCollisionQueryParams TraceParams(FName(TEXT("VoxelTrace")), false, GetCharacter());
	FHitResult HitResult;

	GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, EndTraceLocation, ECollisionChannel::ECC_Visibility, TraceParams);

	if (HitResult.bBlockingHit)
	{
		OutVoxelCoord = FIntVector3::Floor((HitResult.Location * 0.01f) - (HitResult.Normal * 0.25f));
		OutHitNormal = FIntVector3::Floor(HitResult.Normal);
		return true;
	}
	else
	{
		OutVoxelCoord = FIntVector3(0, 0, -1);
		OutHitNormal = FIntVector3(0, 0, 0);
		return false;
	}

}
