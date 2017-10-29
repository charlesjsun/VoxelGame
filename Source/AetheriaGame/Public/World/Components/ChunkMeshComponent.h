/**
	Most of this code (except for GenerateAO) was not written by Charles JL Sun.
	Do not judge based on code from this file.
	This is only for learning purposes, it will be replaced.

	Copyright(c) 2014, Andrew Scheidecker All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met :

	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and / or other materials provided with the distribution.
	Neither the name of BrickGame nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.IN NO EVENT SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Unreal® is a trademark or registered trademark of Epic Games, Inc.in the United States of America and elsewhere

	Unreal® Engine, Copyright 1998 – 2014, Epic Games, Inc.All rights reserved.
*/

#pragma once

#include "CoreMinimal.h"

#include "IntVectors.h"

#include "Components/PrimitiveComponent.h"
#include "ChunkMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class AETHERIAGAME_API UChunkMeshComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
	
public:

	// The coordinate of this chunk.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chunk Mesh Component")
	FIntVector3 ChunkCoord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chunk Collision Component")
	class AVoxelTerrain* VoxelTerrain;

	// Whether this chunk has a low-priority update pending (e.g. ambient occlusion). These updates are spread over multiple frames.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chunk Mesh Component")
	bool HasLowPriorityUpdatePending;

public:

	UChunkMeshComponent(const FObjectInitializer& ObjectInitializer);

	virtual void DestroyComponent(bool bPromoteChildren = false) override;

	virtual FBoxSphereBounds CalcBounds(const FTransform & LocalToWorld) const override;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

private:

	void GenerateAO(TArray<uint8>& AmbientOcclusionValues, const TArray<int32>& Heightmap);

};
