// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#include "ChunkMeshComponent.h"

#include "ChunkUtils.h"

#include "VoxelTerrain.h"

#include "DebugLibrary.h"

#include "Engine/CollisionProfile.h"

#include "SceneManagement.h"
#include "RenderResource.h"
#include "RenderCore.h"
#include "RenderUtils.h"
#include "LocalVertexFactory.h"
#include "SceneView.h"
#include "PrimitiveSceneProxy.h"
#include "MeshBatch.h"
#include "Materials/MaterialInterface.h"

#include "Engine/Engine.h"

struct FChunkVertex
{

	uint8 X;
	uint8 Y;
	uint8 Z;
	uint8 W;
	uint8 B;
	uint8 G;
	uint8 R;
	uint8 A;

	FChunkVertex() {};

	FChunkVertex(const FIntVector3& LocalVertexCoord, const uint8& InR, const uint8& InG, const uint8& InB, const uint8& InA)
		: X(LocalVertexCoord.X), Y(LocalVertexCoord.Y), Z(LocalVertexCoord.Z), W(0),
		R(InR), G(InG), B(InB), A(InA) {}

};

class FChunkVertexBuffer : public FVertexBuffer
{
public:

	TArray<FChunkVertex> Vertices;

	virtual void InitRHI() override
	{
		if (Vertices.Num() > 0)
		{
			FRHIResourceCreateInfo ResourceCreateInfo;
			VertexBufferRHI = RHICreateVertexBuffer(Vertices.Num() * sizeof(FChunkVertex), BUF_Dynamic, ResourceCreateInfo);
			void* BufferData = RHILockVertexBuffer(VertexBufferRHI, 0, Vertices.Num() * sizeof(FChunkVertex), RLM_WriteOnly);
			FMemory::Memcpy(BufferData, Vertices.GetData(), Vertices.Num() * sizeof(FChunkVertex));
			RHIUnlockVertexBuffer(VertexBufferRHI);
		}
	}

};

class FChunkIndexBuffer : public FIndexBuffer
{
public:

	TArray<uint16> Indices;

	virtual void InitRHI() override
	{
		if (Indices.Num() > 0)
		{
			FRHIResourceCreateInfo ResourceCreateInfo;
			IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), Indices.Num() * sizeof(uint16), BUF_Static, ResourceCreateInfo);
			void* BufferData = RHILockIndexBuffer(IndexBufferRHI, 0, Indices.Num() * sizeof(uint16), RLM_WriteOnly);
			FMemory::Memcpy(BufferData, Indices.GetData(), Indices.Num() * sizeof(uint16));
			RHIUnlockIndexBuffer(IndexBufferRHI);
		}
	}

};

class FChunkTangentBuffer : public FVertexBuffer
{
public:

	virtual void InitRHI() override
	{
		FRHIResourceCreateInfo ResourceCreateInfo;
		VertexBufferRHI = RHICreateVertexBuffer(12 * sizeof(FPackedNormal), BUF_Dynamic, ResourceCreateInfo);
		FPackedNormal* BufferData = (FPackedNormal*)RHILockVertexBuffer(VertexBufferRHI, 0, 12 * sizeof(FPackedNormal), RLM_WriteOnly);
		for (int32 i = 0; i < 6; ++i)
		{
			const FVector Normal = UChunkUtils::GetNormal(EVoxelFace(i)).ToFloat();
			const FVector UnprojectedTangentX = FVector(+1, -1, 0).GetSafeNormal();
			const FVector UnprojectedTangentY(-1, -1, -1);
			const FVector ProjectedFaceTangentX = (UnprojectedTangentX - Normal * (UnprojectedTangentX | Normal)).GetSafeNormal();
			*(BufferData++) = ProjectedFaceTangentX;
			*(BufferData++) = FVector4(Normal, FMath::Sign(UnprojectedTangentY | (Normal ^ ProjectedFaceTangentX)));
		}
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}

};

// Global data because ALL vertex factories can use the SAME tangents
TGlobalResource<FChunkTangentBuffer> ChunkTangentBuffer;

class FChunkVertexFactory : public FLocalVertexFactory
{

private:
	// The face this vertex factory represents
	EVoxelFace Face;

	const FPrimitiveSceneProxy* ChunkSceneProxy;

public:

	// Called before BeginInitResources in the Game Thread
	void Init(const FPrimitiveSceneProxy* InChunkSceneProxy, EVoxelFace InFace, const FChunkVertexBuffer& VertexBuffer)
	{
		Face = InFace;
		ChunkSceneProxy = InChunkSceneProxy;

		FDataType NewData;
		NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&VertexBuffer, FChunkVertex, X, VET_UByte4N);
		NewData.TextureCoordinates.Add(STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&VertexBuffer, FChunkVertex, X, VET_UByte4N)); // Texture coord doesn't matter
		NewData.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&VertexBuffer, FChunkVertex, B, VET_Color);

		// Same Tangents for the same face
		NewData.TangentBasisComponents[0] = FVertexStreamComponent(&ChunkTangentBuffer, sizeof(FPackedNormal) * ((uint8)Face * 2), 0, VET_PackedNormal);
		NewData.TangentBasisComponents[1] = FVertexStreamComponent(&ChunkTangentBuffer, sizeof(FPackedNormal) * ((uint8)Face * 2 + 1), 0, VET_PackedNormal);

		check(!IsInRenderingThread());
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(InitializeData,
			FDataType, NewData, NewData,
			FChunkVertexFactory*, VertexFactory, this,
			{
				VertexFactory->SetData(NewData);
			});
	}

	virtual uint64 GetStaticBatchElementVisibility(const FSceneView& View, const struct FMeshBatch* Batch) const override
	{
		return IsStaticBatchVisible(View.ViewMatrices.GetViewOrigin(), Batch) ? 1 : 0;
	}
	virtual uint64 GetStaticBatchElementShadowVisibility(const FSceneView& View, const FLightSceneProxy* LightSceneProxy, const struct FMeshBatch* Batch) const override
	{
		return IsStaticBatchVisible(LightSceneProxy->GetPosition(), Batch) ? 1 : 0;
	}

private:

	bool IsStaticBatchVisible(const FVector4& ViewPosition, const FMeshBatch* Batch) const
	{
		const EVoxelFace VoxelFace = EVoxelFace(Batch->Elements[0].UserIndex);
		const FBox BoundingBox = ChunkSceneProxy->GetBounds().GetBox();
		const FVector MinRelativePosition = ViewPosition - BoundingBox.Min * ViewPosition.W;
		const FVector MaxRelativePosition = ViewPosition - BoundingBox.Max * ViewPosition.W;
		switch (VoxelFace)
		{
		case EVoxelFace::LEFT:
			return MaxRelativePosition.X < 0.0f;
		case EVoxelFace::RIGHT:
			return MinRelativePosition.X > 0.0f;
		case EVoxelFace::BACK:
			return MaxRelativePosition.Y < 0.0f;
		case EVoxelFace::FRONT:
			return MinRelativePosition.Y > 0.0f;
		case EVoxelFace::BOTTOM:
			return MaxRelativePosition.Z < 0.0f;
		case EVoxelFace::TOP:
			return MinRelativePosition.Z > 0.0f;
		default:
			return false;
		}
	}

};

class FChunkSceneProxy : public FPrimitiveSceneProxy
{
public:

	FChunkVertexBuffer VertexBuffer;
	FChunkIndexBuffer IndexBuffer;
	FChunkVertexFactory VertexFactories[6];

	// An unique array of voxel materials
	TArray<UMaterialInterface*> VoxelMaterials;

	// One element of the mesh, contains a section with the same materials
	// Synonymous with FBatchMeshElement
	struct FElement
	{
		uint32 FirstIndex; // Start index in IndexBuffer
		uint32 NumPrimitives; // Number of triangles
		uint32 MaterialIndex; // Index of the material in VoxelMaterials
		EVoxelFace Face; // Which face this element is representing
	};

	// Stores all the elements of the array
	TArray<FElement> Elements;

	// Reference to the meshing event
	FGraphEventRef MeshFinishEvent;

	TUniformBufferRef<FPrimitiveUniformShaderParameters> UniformBuffer;

	FMaterialRelevance MaterialRelevance;

	FChunkSceneProxy(UChunkMeshComponent* ChunkMeshComponent)
		: FPrimitiveSceneProxy(ChunkMeshComponent)
	{}

	void BeginInitResources()
	{
		// Sleep the Render Thread until the chunk finishes meshing
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(WaitUntilMeshingFinishes,
			FGraphEventRef, MeshFinishEvent, MeshFinishEvent,
			{
				FTaskGraphInterface::Get().WaitUntilTaskCompletes(MeshFinishEvent, ENamedThreads::RenderThread);
			});

		BeginInitResource(&VertexBuffer);
		BeginInitResource(&IndexBuffer);

		// Init the vertex factories for the 6 faces 
		for (int32 i = 0; i < 6; ++i)
		{
			VertexFactories[i].Init(this, EVoxelFace(i), VertexBuffer);
			BeginInitResource(&VertexFactories[i]);
		}
	}

	// Called in the Rendering Thread
	virtual ~FChunkSceneProxy()
	{
		VertexBuffer.ReleaseResource();
		IndexBuffer.ReleaseResource();
		for (int32 i = 0; i < 6; ++i)
		{
			VertexFactories[i].ReleaseResource();
		}
	}

	virtual uint32 GetMemoryFootprint(void) const override
	{
		return (sizeof(*this) + GetAllocatedSize());
	}

	uint32 GetAllocatedSize(void) const
	{
		return (FPrimitiveSceneProxy::GetAllocatedSize());
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Relevance;
		Relevance.bDrawRelevance = IsShown(View);
		Relevance.bDynamicRelevance = View->Family->EngineShowFlags.Wireframe || IsSelected();
		Relevance.bStaticRelevance = !Relevance.bDynamicRelevance;
		Relevance.bShadowRelevance = IsShadowCast(View);
		MaterialRelevance.SetPrimitiveViewRelevance(Relevance);
		return Relevance;
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	virtual void OnTransformChanged() override
	{
		// TODO: Find out why scale it by 255
		UniformBuffer = CreatePrimitiveUniformBufferImmediate(FScaleMatrix(255) * GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
	}

	void CreateMeshBatch(FMeshBatch& OutMeshBatch, const FElement& Element, FMaterialRenderProxy* WireframeMaterial) const
	{
		OutMeshBatch.VertexFactory = &VertexFactories[(int32)Element.Face];
		OutMeshBatch.MaterialRenderProxy = WireframeMaterial == nullptr ? VoxelMaterials[Element.MaterialIndex]->GetRenderProxy(IsSelected()) : WireframeMaterial;
		OutMeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();
		OutMeshBatch.bWireframe = WireframeMaterial != nullptr;
		OutMeshBatch.Type = PT_TriangleList;
		OutMeshBatch.DepthPriorityGroup = SDPG_World;
		OutMeshBatch.CastShadow = true;

		OutMeshBatch.Elements[0].FirstIndex = Element.FirstIndex;
		OutMeshBatch.Elements[0].NumPrimitives = Element.NumPrimitives;
		OutMeshBatch.Elements[0].MinVertexIndex = 0;
		OutMeshBatch.Elements[0].MaxVertexIndex = VertexBuffer.Vertices.Num() - 1;
		OutMeshBatch.Elements[0].IndexBuffer = &IndexBuffer;
		OutMeshBatch.Elements[0].UserIndex = (int32)Element.Face;
		OutMeshBatch.Elements[0].PrimitiveUniformBuffer = UniformBuffer;
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		// Set up the wireframe material Face.
		FColoredMaterialRenderProxy* WireframeMaterialFace = new FColoredMaterialRenderProxy(
			WITH_EDITOR ? GEngine->WireframeMaterial->GetRenderProxy(IsSelected()) : nullptr,
			FLinearColor(0, 0.5f, 1.f)
			);
		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialFace);

		// Draw the mesh elements in each view they are visible.
		for (int32 i = 0; i < Elements.Num(); ++i)
		{
			FMeshBatch& Batch = Collector.AllocateMesh();
			CreateMeshBatch(Batch, Elements[i], ViewFamily.EngineShowFlags.Wireframe ? WireframeMaterialFace : nullptr);

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ++ViewIndex)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					Collector.AddMesh(ViewIndex, Batch);
				}
			}
		}
	}

	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) override
	{
		for (int32 i = 0; i < Elements.Num(); ++i)
		{
			FMeshBatch Batch;
			CreateMeshBatch(Batch, Elements[i], nullptr);
			PDI->DrawMesh(Batch, FLT_MAX);
		}
	}

};

UChunkMeshComponent::UChunkMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	CastShadow = true;
	bUseAsOccluder = true;
	bAutoRegister = false;

	SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

}

void UChunkMeshComponent::DestroyComponent(bool bPromoteChildren)
{
	VoxelTerrain = nullptr;

	Super::DestroyComponent(bPromoteChildren);
}

FBoxSphereBounds UChunkMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds ResultBounds;
	ResultBounds.Origin = FVector(CHUNK_SIZE / 2);
	ResultBounds.BoxExtent = FVector(CHUNK_SIZE / 2);
	ResultBounds.SphereRadius = ResultBounds.BoxExtent.Size();
	return ResultBounds.TransformBy(LocalToWorld);
}

#define USE_CUSTOM_AO 0

/**
The following licensing information refers to the greedy meshing algorithm used to generate the voxel mesh.

The MIT Licence (MIT)

Copyright(c) 2013 Mikola Lysenko - https://github.com/mikolalysenko/mikolalysenko.github.com/blob/gh-pages/MinecraftMeshes/js/greedy.js

Adaptation to Java by Robert O'Leary - https://github.com/roboleary/GreedyMesh

Adaptation to C++ by Charles JL Sun

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
FPrimitiveSceneProxy* UChunkMeshComponent::CreateSceneProxy()
{


	UE_LOG(LogStats, Log, TEXT("<%d, %d, %d> Creating Chunk Scene Proxy"), ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z);

	TArray<uint8> Voxels;
	FIntVector3 ChunkVoxelCoord = AVoxelTerrain::ChunkToWorldCoord(ChunkCoord);
	VoxelTerrain->GetVoxelsArray(ChunkVoxelCoord - FIntVector3(1), ChunkVoxelCoord + FIntVector3(CHUNK_SIZE), Voxels);

	// TODO: This checks the voxels one layer around the chunk as well. Fix so that it only checks the voxels inside. Test for efficency.
	bool bChunkIsEmpty = true;
	for (int32 i = 0; i < Voxels.Num(); ++i)
	{
		if (Voxels[i] != 0)
		{
			bChunkIsEmpty = false;
			break;
		}
	}



	FChunkSceneProxy* NewSceneProxy = nullptr;

	if (!bChunkIsEmpty)
	{
		// Heightmap used for Ambient Occlusion Calculation
		TArray<int32> Heightmap;
		/// Getting Heightmap
		{
			const int32 VoxelX = ChunkCoord.X << CHUNK_SHIFT;
			const int32 VoxelY = ChunkCoord.Y << CHUNK_SHIFT;

			const int32 BlurRadius = VoxelTerrain->TerrainParameters.AOBlurRadus;
			const int32 BlurDiameter = BlurRadius * 2;
			const int32 AmbientMapSize = CHUNK_SIZE + 2;

			VoxelTerrain->GetHeightsArray(FIntVector2D(VoxelX - BlurRadius, VoxelY - BlurRadius), FIntVector2D(VoxelX, VoxelY) + FIntVector2D(AmbientMapSize + BlurRadius - 1), Heightmap);
		}

		const ERHIFeatureLevel::Type SceneFeatureLevel = GetScene()->GetFeatureLevel();

		// Reference to the TerrainParameters Voxel Types array
		const TArray<FVoxelType>& VoxelTypes = VoxelTerrain->TerrainParameters.VoxelTypes;

		// An Unique array of all voxel materials
		TArray<UMaterialInterface*> UniqueMaterials;
		// The index of the material for voxel type in UniqueMaterials
		TArray<int32> UniqueMaterialsIndex;
		UniqueMaterialsIndex.SetNumUninitialized(VoxelTypes.Num());

		for (int32 i = 0; i < VoxelTypes.Num(); ++i)
		{
			UniqueMaterialsIndex[i] = UniqueMaterials.AddUnique(VoxelTypes[i].VoxelMaterial);
		}

		NewSceneProxy = new FChunkSceneProxy(this);
		NewSceneProxy->MeshFinishEvent = FFunctionGraphTask::CreateAndDispatchWhenReady([=]
		{

			struct FFaceMesh
			{
				TArray<uint16> Indices;
			};

			struct FMaterialMesh
			{
				FFaceMesh FaceMeshes[6];
			};

			// Contains a Mesh for each unique Material
			TArray<FMaterialMesh> MaterialMeshes;
			MaterialMeshes.Init(FMaterialMesh(), UniqueMaterials.Num());

			// FString Matss("[");
			// for (int i = 0; i < UniqueMaterials.Num(); ++i)
			// {
			// 	Matss += (UniqueMaterials[i] == nullptr ? TEXT("nullptr") : UniqueMaterials[i]->GetName()) + TEXT(", ");
			// }
			// Matss += TEXT("]");
			// 
			// FString MatIndexs("[");
			// for (int i = 0; i < UniqueMaterialsIndex.Num(); ++i)
			// {
			// 	MatIndexs += INT_STRING(UniqueMaterialsIndex[i]) + TEXT(", ");
			// }
			// MatIndexs += TEXT("]");
			// 
			// UE_LOG(LogStats, Log, TEXT("<%d, %d, %d> Inside Meshing Task - MaterialMeshes: %d - UniqueMaterials: %d %s - UniqueMaterialIndex: %d %s"), ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z, MaterialMeshes.Num(), UniqueMaterials.Num(), *Matss, UniqueMaterialsIndex.Num(), *MatIndexs);

			const int32 VoxelX = ChunkCoord.X << CHUNK_SHIFT;
			const int32 VoxelY = ChunkCoord.Y << CHUNK_SHIFT;
			const int32 VoxelZ = ChunkCoord.Z << CHUNK_SHIFT;

			/// GREEDY MESHING ALGORITHM
			{

				struct FFaceValue
				{
					uint32 VoxelType = 0;
					uint32 AOValue = 0;

					FORCEINLINE bool operator==(const FFaceValue& B) const
					{
						return VoxelType == B.VoxelType && AOValue == B.AOValue;
					}

					FORCEINLINE bool operator!=(const FFaceValue& B) const
					{
						return VoxelType != B.VoxelType || AOValue != B.AOValue;
					}

				};

				int32 i, j, k, l, h, w, u, v, n, r, s, t;

				EVoxelFace Face;

				int32 x[] = { 0, 0, 0 };
				int32 q[] = { 0, 0, 0 };
				int32 du[] = { 0, 0, 0 };
				int32 dv[] = { 0, 0, 0 };

				FFaceValue Mask[CHUNK_SIZE * CHUNK_SIZE];

#if USE_CUSTOM_AO
				// Size = CHUNK_SIZE + 1, represents each vertex
				TArray<uint8> AmbientOcclusionValues;
				// Calculate the ambient occlusion map
				GenerateAO(AmbientOcclusionValues, Heightmap);
#endif

				for (bool bBackFace = true, b = false; b != bBackFace; bBackFace = bBackFace && b, b = !b)
				{
					for (int32 d = 0; d < 3; d++)
					{
						u = (d + 1) % 3;
						v = (d + 2) % 3;

						x[0] = 0;
						x[1] = 0;
						x[2] = 0;
						q[0] = 0;
						q[1] = 0;
						q[2] = 0;
						q[d] = 1;

						if (d == 0)
						{
							Face = bBackFace ? EVoxelFace::LEFT : EVoxelFace::RIGHT;
						}
						else if (d == 1)
						{
							Face = bBackFace ? EVoxelFace::BACK : EVoxelFace::FRONT;
						}
						else if (d == 2)
						{
							Face = bBackFace ? EVoxelFace::BOTTOM : EVoxelFace::TOP;
						}

						for (x[d] = 0; x[d] < CHUNK_SIZE; x[d]++)
						{
							n = 0;

							// For Ambient Occlusion. It's outside the loop so we don't assign it multiple times
							du[0] = 0;
							du[1] = 0;
							du[2] = 0;
							du[u] = 1;
							dv[0] = 0;
							dv[1] = 0;
							dv[2] = 0;
							dv[v] = 1;

							for (x[v] = 0; x[v] < CHUNK_SIZE; x[v]++)
							{
								for (x[u] = 0; x[u] < CHUNK_SIZE; x[u]++)
								{
									// Get World Voxel Positon
									const FIntVector3 LocalCoord(x[0] + 1, x[1] + 1, x[2] + 1);
									const int32 Size = CHUNK_SIZE + 2;

									// The voxel type of the current voxel
									const uint8& VoxelType = Voxels[LocalCoord.X + (LocalCoord.Y * Size) + (LocalCoord.Z * Size * Size)];

									// Get the voxel adjacent to the current voxel in the direction of the face
									const FIntVector3 FaceVoxel = LocalCoord + UChunkUtils::GetNormal(Face);
									bool bIsFaceVisible = Voxels[FaceVoxel.X + (FaceVoxel.Y * Size) + (FaceVoxel.Z * Size * Size)] == 0;

									// If the face isn't visible (blocked by another solid voxel), then don't mesh it
									Mask[n].VoxelType = bIsFaceVisible ? VoxelType : 0;

#if !USE_CUSTOM_AO
									Mask[n].AOValue = (255 << 24) | (255 << 16) | (255 << 8) | (255 << 0);
#else
									// Calculate Ambient Occlusion only if the face is visible and not 0
									if (Mask[n].VoxelType != 0)
									{
										/// Ambient Occlusion

										if (bBackFace)
										{
											r = x[0];
											s = x[1];
											t = x[2];
										}
										else
										{
											r = x[0] + q[0];
											s = x[1] + q[1];
											t = x[2] + q[2];
										}

										const FIntVector3 Vert0(r, s, t);
										const FIntVector3 Vert1(r + du[0], s + du[1], t + du[2]);
										const FIntVector3 Vert2(r + dv[0], s + dv[1], t + dv[2]);
										const FIntVector3 Vert3(r + du[0] + dv[0], s + du[1] + dv[1], t + du[2] + dv[2]);

										const int32 ChunkSizePlus1 = CHUNK_SIZE + 1;
										const int32 ChunkSizePlus1Squared = ChunkSizePlus1 * ChunkSizePlus1;

										const uint8 AO0 = AmbientOcclusionValues[Vert0.X + (Vert0.Y * ChunkSizePlus1) + (Vert0.Z * ChunkSizePlus1Squared)];
										const uint8 AO1 = AmbientOcclusionValues[Vert1.X + (Vert1.Y * ChunkSizePlus1) + (Vert1.Z * ChunkSizePlus1Squared)];
										const uint8 AO2 = AmbientOcclusionValues[Vert2.X + (Vert2.Y * ChunkSizePlus1) + (Vert2.Z * ChunkSizePlus1Squared)];
										const uint8 AO3 = AmbientOcclusionValues[Vert3.X + (Vert3.Y * ChunkSizePlus1) + (Vert3.Z * ChunkSizePlus1Squared)];

										if (!bBackFace)
										{
											Mask[n].AOValue = (AO0 << 24) | (AO2 << 16) | (AO1 << 8) | (AO3 << 0);
										}
										else
										{
											Mask[n].AOValue = (AO1 << 24) | (AO3 << 16) | (AO0 << 8) | (AO2 << 0);
										}
									}
#endif

									n++;
								}
							}

							n = 0;
							for (j = 0; j < CHUNK_SIZE; j++)
							{
								for (i = 0; i < CHUNK_SIZE; )
								{
									if (Mask[n].VoxelType != 0)
									{
										for (w = 1; w + i < CHUNK_SIZE && Mask[n + w].VoxelType != 0 && Mask[n + w] == Mask[n]; w++)
										{
										}

										bool bDone = false;

										for (h = 1; j + h < CHUNK_SIZE; h++)
										{
											for (k = 0; k < w; k++)
											{
												if (Mask[n + k + h * CHUNK_SIZE].VoxelType == 0 || Mask[n + k + h * CHUNK_SIZE] != Mask[n])
												{
													bDone = true;
													break;
												}
											}
											if (bDone)
											{
												break;
											}
										}

										x[u] = i;
										x[v] = j;
										du[0] = 0;
										du[1] = 0;
										du[2] = 0;
										du[u] = w;
										dv[0] = 0;
										dv[1] = 0;
										dv[2] = 0;
										dv[v] = h;

										if (bBackFace)
										{
											r = x[0];
											s = x[1];
											t = x[2];
										}
										else
										{
											r = x[0] + q[0];
											s = x[1] + q[1];
											t = x[2] + q[2];
										}

										FIntVector3 Vert0, Vert1, Vert2, Vert3;

										if (!bBackFace)
										{
											Vert0 = FIntVector3(r, s, t);
											Vert1 = FIntVector3(r + dv[0], s + dv[1], t + dv[2]);
											Vert2 = FIntVector3(r + du[0], s + du[1], t + du[2]);
											Vert3 = FIntVector3(r + du[0] + dv[0], s + du[1] + dv[1], t + du[2] + dv[2]);
										}
										else
										{
											Vert2 = FIntVector3(r, s, t);
											Vert0 = FIntVector3(r + du[0], s + du[1], t + du[2]);
											Vert3 = FIntVector3(r + dv[0], s + dv[1], t + dv[2]);
											Vert1 = FIntVector3(r + du[0] + dv[0], s + du[1] + dv[1], t + du[2] + dv[2]);
										}

										const FFaceValue& FaceValue = Mask[n];

										FFaceMesh& Mesh = MaterialMeshes[UniqueMaterialsIndex[FaceValue.VoxelType]].FaceMeshes[(int32)Face];

										TArray<FChunkVertex>& Vertices = NewSceneProxy->VertexBuffer.Vertices;
										const int32 VertCount = Vertices.Num();

										//UE_LOG(LogStats, Log, TEXT("Inside Meshing Loop - Vertices: %d"), Vertices.Num());

										Mesh.Indices.Add(VertCount + 2);
										Mesh.Indices.Add(VertCount + 0);
										Mesh.Indices.Add(VertCount + 1);
										Mesh.Indices.Add(VertCount + 1);
										Mesh.Indices.Add(VertCount + 3);
										Mesh.Indices.Add(VertCount + 2);

										const FColor& VoxelColor = VoxelTypes[FaceValue.VoxelType].VoxelColor;

										new(Vertices) FChunkVertex(Vert0, VoxelColor.R, VoxelColor.G, VoxelColor.B, (FaceValue.AOValue >> 24));
										new(Vertices) FChunkVertex(Vert1, VoxelColor.R, VoxelColor.G, VoxelColor.B, (FaceValue.AOValue >> 16) & 255);
										new(Vertices) FChunkVertex(Vert2, VoxelColor.R, VoxelColor.G, VoxelColor.B, (FaceValue.AOValue >> 8) & 255);
										new(Vertices) FChunkVertex(Vert3, VoxelColor.R, VoxelColor.G, VoxelColor.B, (FaceValue.AOValue) & 255);

										// Clear the mask
										for (l = 0; l < h; ++l)
										{
											for (k = 0; k < w; ++k)
											{
												Mask[n + k + l * CHUNK_SIZE].VoxelType = 0;
												Mask[n + k + l * CHUNK_SIZE].AOValue = 0;
											}
										}
										i += w;
										n += w;
									}
									else
									{
										i++;
										n++;
									}
								}
							}

						}

					}
				}

			} /// END GREEDY MESHING ALGORITHM

			UE_LOG(LogStats, Log, TEXT("<%d, %d, %d> Inside Meshing Task And Meshed"), ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z);

			// Calculates the total number of indices from all the elements
			int32 NumIndices = 0;
			for (int32 i = 0; i < MaterialMeshes.Num(); ++i)
			{
				for (int32 f = 0; f < 6; ++f)
				{
					NumIndices += MaterialMeshes[i].FaceMeshes[f].Indices.Num();
				}
			}
			// Setup the index buffer's indices
			NewSceneProxy->IndexBuffer.Indices.Empty(NumIndices);
			// For each material and face, create an Element to be rendered
			for (int32 i = 1; i < MaterialMeshes.Num(); ++i) // 0 is empty
			{
				UMaterialInterface* Material = UniqueMaterials[i];
				int32 MaterialIndex = NewSceneProxy->VoxelMaterials.Add(Material); // This index is used by the element
				NewSceneProxy->MaterialRelevance |= Material->GetRelevance_Concurrent(SceneFeatureLevel);

				// Do it for each face
				for (int32 f = 0; f < 6; ++f)
				{
					const FFaceMesh& Mesh = MaterialMeshes[i].FaceMeshes[f];
					// Only create an element if there is something to render
					if (Mesh.Indices.Num() > 0)
					{
						FChunkSceneProxy::FElement& Element = *new(NewSceneProxy->Elements) FChunkSceneProxy::FElement;
						Element.MaterialIndex = MaterialIndex;
						Element.FirstIndex = NewSceneProxy->IndexBuffer.Indices.Num();
						Element.NumPrimitives = Mesh.Indices.Num() / 3;
						Element.Face = EVoxelFace(f);

						NewSceneProxy->IndexBuffer.Indices.Append(Mesh.Indices);
					}
				}

			}

			UE_LOG(LogStats, Log, TEXT("<%d, %d, %d> Inside Meshing Task And Created Elements - Vertices: %d, Indices: %d, Elements: %d "), ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z, NewSceneProxy->VertexBuffer.Vertices.Num(), NewSceneProxy->IndexBuffer.Indices.Num(), NewSceneProxy->Elements.Num());

		}, GetStatID());

		NewSceneProxy->BeginInitResources();

	}

	UDebugLibrary::Println(this, 10, FString::Printf(TEXT("<%d, %d, %d> Chunk Scene Proxy Created"), ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z));
	UE_LOG(LogStats, Log, TEXT("<%d, %d, %d> Chunk Scene Proxy Created"), ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z);

	return NewSceneProxy;

}

// Generate Ambient Occlusion from height values.
// TODO: Change to using flood fill lighting instead
// Created by Charles JL Sun
void UChunkMeshComponent::GenerateAO(TArray<uint8>& AmbientOcclusionValues, const TArray<int32>& Heightmap)
{
	TIME_START();

	const int32 VoxelX = ChunkCoord.X << CHUNK_SHIFT;
	const int32 VoxelY = ChunkCoord.Y << CHUNK_SHIFT;
	const int32 VoxelZ = ChunkCoord.Z << CHUNK_SHIFT;

	const int32 BlurRadius = VoxelTerrain->TerrainParameters.AOBlurRadus;
	const int32 BlurDiameter = BlurRadius * 2;
	const int32 BlurAmount = FMath::Square(BlurDiameter + 1);

	// The chunk size, but with one more row/col/depth for the vertices. 
	const int32 ChunkVerticesSize = CHUNK_SIZE + 1;
	// The chunk size with one more voxels on each side
	const int32 AmbientMapSize = CHUNK_SIZE + 2;
	// The ambient map size with the radius on each side (thus diameter)
	const int32 FullMapSize = AmbientMapSize + BlurDiameter;

	AmbientOcclusionValues.SetNumUninitialized(ChunkVerticesSize * ChunkVerticesSize * ChunkVerticesSize);

	checkf(Heightmap.Num() == FullMapSize * FullMapSize, TEXT("Heightmap Wrong Size in Meshing!!!"));

	// The full amount of voxels we need to sample. The size of the ambient map + the radius for all sides.
	// The height is seperate from the blur. Radius 2 = <38, 38, 34>
	const FIntVector3 FullMapDimension(FullMapSize, FullMapSize, AmbientMapSize);
	// The dimensions of the voxels we need to sample after the X pass. 
	// The size of the ambient map + the radius on the Y axis. Radius 2 = <34, 38, 34>
	const FIntVector3 XPassDimension(AmbientMapSize, FullMapSize, AmbientMapSize);
	// The resulting dimension after both passes.
	// The size of the ambient map. Radius 2 = <34, 34, 34>
	const FIntVector3 YPassDimension(AmbientMapSize);
	// The averaged ao value output for the vertices. Chunk Size 32 = <33, 33, 33>
	// const FIntVector& AOValuesDimension = FIntVector(ChunkVerticesSize, ChunkVerticesSize, ChunkVerticesSize);

	// The array to store the x pass of the blur.
	// Stores a 2D map as it only needs one slice of the z value at a time
	TArray<uint8> XBlurPass;
	XBlurPass.SetNumUninitialized(XPassDimension.X * XPassDimension.Y);
	// The array to store the y pass of the blur.
	// Stores a 3D map for post processing
	TArray<uint8> YBlurPass;
	YBlurPass.SetNumUninitialized(YPassDimension.X * YPassDimension.Y * YPassDimension.Z);

	// Two pass "gaussian" blur for each z value
	for (int32 z = 0; z < FullMapDimension.Z; ++z)
	{
		const int32& VoxelOffsetZ = VoxelZ + z - 1;

		// X Pass first
		for (int32 x = 0; x < XPassDimension.X; ++x)
		{
			for (int32 y = 0; y < XPassDimension.Y; ++y)
			{
				uint32 SumVisibility = 0;
				for (int32 OffsetX = -BlurRadius; OffsetX <= BlurRadius; ++OffsetX)
				{
					const int32& LocalOffsetX = x + OffsetX - 1;
					const int32& LocalOffsetY = y - BlurRadius - 1;

					const int32 OffsetHeight = Heightmap[(LocalOffsetX + BlurRadius + 1) + ((LocalOffsetY + BlurRadius + 1) * FullMapSize)];

					if (VoxelOffsetZ > OffsetHeight)
					{
						SumVisibility += 1;
					}
					else
					{
						//const float VisibilityAmount = (float)FMath::Clamp(FMath::Abs(OffsetHeight - VoxelOffsetZ), 0, 32) / 1024.0f;
						//SumVisibility += VisibilityAmount;
						SumVisibility += 0;
					}
				}

				XBlurPass[x + (y * XPassDimension.X)] = (uint8)SumVisibility;

			}
		}

		// Y Pass second
		for (int32 y = 0; y < YPassDimension.Y; ++y)
		{
			for (int32 x = 0; x < YPassDimension.X; ++x)
			{
				uint32 SumVisibility = 0;
				for (int32 OffsetY = -BlurRadius; OffsetY <= BlurRadius; ++OffsetY)
				{
					const int32& LocalOffsetY = y + OffsetY + BlurRadius;
					SumVisibility += XBlurPass[x + (LocalOffsetY * XPassDimension.X)];
				}
				SumVisibility *= 255;
				SumVisibility /= BlurAmount;
				YBlurPass[x + (y * YPassDimension.X) + (z * YPassDimension.Y * YPassDimension.Y)] = (uint8)SumVisibility;
			}
		}

	}

	for (int32 z = 0; z < ChunkVerticesSize; ++z)
	{
		for (int32 y = 0; y < ChunkVerticesSize; ++y)
		{
			for (int32 x = 0; x < ChunkVerticesSize; ++x)
			{
				uint32 CornerAmbientValue = 0;
				for (int32 i = 0; i < 8; i++)
				{
					const int32 CornerX = x + (i & 1);
					const int32 CornerY = y + ((i >> 1) & 1);
					const int32 CornerZ = z + (i >> 2);
					CornerAmbientValue += YBlurPass[CornerX + (CornerY * YPassDimension.X) + (CornerZ * YPassDimension.Y * YPassDimension.Y)];
				}
				const uint8 FinalAOValue = (uint8)FMath::Min<uint32>(CornerAmbientValue / 4, 255);

				const int32 Index = x + (y * ChunkVerticesSize) + (z * ChunkVerticesSize * ChunkVerticesSize);
				checkf(Index >= 0 && Index < ChunkVerticesSize * ChunkVerticesSize * ChunkVerticesSize, TEXT("AOIndex is out of bound! Index is %d, size is %d"), Index, AmbientOcclusionValues.Num());
				AmbientOcclusionValues[Index] = FinalAOValue;
			}
		}
	}

	TIME_END();

	//PRINT_STRING(10, FString(TEXT("AO Gen: ") + FLOAT_STRING(ElapsedTime)));
}


