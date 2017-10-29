// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#include "TerrainGenerator.h"

#include "SimplexNoise.h"

#include "VoxelTerrain.h"


void UTerrainGenerator::GenerateChunk(const AVoxelTerrain* const VoxelTerrain, FChunk& Chunk, const FTerrainGeneratorParameters& Parameter)
{

	const int X_EXTRA = 3;
	const int Y_EXTRA = 3;
	const int Z_EXTRA = 11;

	const FIntVector3& LowerBound = Chunk.ChunkPositionInVoxels;
	const FIntVector3& UpperBound = Chunk.ChunkPositionInVoxels + FIntVector3(CHUNK_SIZE - 1);

	// For trees
	const FIntVector3& LowerBoundExtra = LowerBound - FIntVector3(X_EXTRA, Y_EXTRA, Z_EXTRA);
	const FIntVector3& UpperBoundExtra = UpperBound + FIntVector3(X_EXTRA, Y_EXTRA, 0);
	const FIntVector3& ExtraDiff = UpperBoundExtra - LowerBoundExtra + FIntVector3(1);

	TArray<uint8> ExtraVoxels;
	ExtraVoxels.SetNumUninitialized(ExtraDiff.X * ExtraDiff.Y * ExtraDiff.Z);


	//GET_ROLE(VoxelTerrain->Role, RoleString);
	//PRINT(*FString::Printf(TEXT("%s - RAN"), *RoleString), 10);

	TArray<FIntVector3> TreePositions;
	TArray<uint8> LeaveTypes;

	for (int x = LowerBoundExtra.X; x <= UpperBoundExtra.X; ++x)
	{
		for (int y = LowerBoundExtra.Y; y <= UpperBoundExtra.Y; ++y)
		{

			//float Height = 16 * (FMath::Sin((float)x / 16.f) + FMath::Sin((float)y / 16.0f) + 2);

			for (int z = LowerBoundExtra.Z; z <= UpperBoundExtra.Z; ++z)
			{

				if (z < 0)
				{
					continue;
				}

				const int32 LocalX = x - LowerBoundExtra.X;
				const int32 LocalY = y - LowerBoundExtra.Y;
				const int32 LocalZ = z - LowerBoundExtra.Z;

				//uint8 Voxel = z < Height ? 1 : 0;

				// -1 -> 1
				float Noise;
				if (!Parameter.bUseRidgedMulti)
				{
					Noise = USimplexNoise::Fractal3D((x + Parameter.Seed * 59) * Parameter.NoiseScale, y * Parameter.NoiseScale, z * Parameter.NoiseScale, Parameter.NoiseOctaves, Parameter.NoiseFrequency, Parameter.NoiseLacunarity, Parameter.NoisePersistance);
				}
				else
				{
					Noise = USimplexNoise::RidgedMulti3D((x + Parameter.Seed * 59) * Parameter.NoiseScale, y * Parameter.NoiseScale, 0, Parameter.NoiseOctaves, Parameter.NoiseFrequency, Parameter.NoiseLacunarity);
				}

				// -0.25 - 0.25
				float ScaledNoise = Noise * 0.25f;
				// change z from 0 - 64 to 0 - 1
				float ScaledZ = (float)z / (float)WORLD_HEIGHT;

				float OffsetZ = ScaledZ - ScaledNoise;

				// Determines the voxel
				uint8 Voxel = 0;
				if (OffsetZ < 0.5f && OffsetZ >= 0.4f)	Voxel = Parameter.GrassVoxel; // 1 
				else if (OffsetZ < 0.4f)				Voxel = 2; // 2
				else									Voxel = 0;

				if (Voxel == 0)
				{
					const uint8 VoxelBeneath = (z == LowerBoundExtra.Z || z == 0) ? 0 : ExtraVoxels[LocalX + LocalY * ExtraDiff.X + (LocalZ - 1) * ExtraDiff.X * ExtraDiff.Y];
					if (VoxelBeneath != 0 && VoxelBeneath != Parameter.SnowVoxel)
					{

						// This means it is the top voxel

						Voxel = Parameter.SnowVoxel;

						// Check Tree Generation
						// -1 - 1
						const float TreeNoise = USimplexNoise::Fractal3D((x + Parameter.Seed * 31) * Parameter.TreeScale, (y + Parameter.Seed * 37) * Parameter.TreeScale, 0, Parameter.TreeOctaves, 1.0f, 2.0f, 0.5f);
						// 0 - 1
						const float TreeProb = (TreeNoise + 1.0f) * 0.5f;

						const float ColorNoise = USimplexNoise::Fractal3D((x + Parameter.Seed * 19) * Parameter.TreeScale, (y + Parameter.Seed * 17) * Parameter.TreeScale, 0, Parameter.TreeOctaves, 1.0f, 2.0f, 0.5f);

						if (TreeProb < Parameter.TreeDensity)
						{
							TreePositions.Add(FIntVector3(x, y, z));

							if (ColorNoise < -0.5f)
							{
								LeaveTypes.Add(11);
							}
							else if (ColorNoise < 0.3f)
							{
								LeaveTypes.Add(13);
							}
							else
							{
								LeaveTypes.Add(14);
							}
						}
					}
				}

				//if (Voxel != 0 && (LocalX == 0 || LocalX == CHUNK_SIZE - 1 || LocalY == 0 || LocalY == CHUNK_SIZE - 1 || LocalZ == 0 || LocalZ == CHUNK_SIZE - 1))
				//{
				//	Voxel = 6;
				//}

				ExtraVoxels[LocalX + LocalY * ExtraDiff.X + LocalZ * ExtraDiff.X * ExtraDiff.Y] = Voxel;

			}
		}
	}

	if (Parameter.bCreateTrees)
	{
		// Make Trees
		for (int i = 0; i < TreePositions.Num(); ++i)
		{
			if (LeaveTypes[i] == 11)
			{
				MakeTree(Parameter, LeaveTypes[i], TreePositions[i], Chunk.ChunkPositionInVoxels, ExtraVoxels);
			}
		}
		for (int i = 0; i < TreePositions.Num(); ++i)
		{
			if (LeaveTypes[i] == 13)
			{
				MakeTree(Parameter, LeaveTypes[i], TreePositions[i], Chunk.ChunkPositionInVoxels, ExtraVoxels);
			}
		}
		for (int i = 0; i < TreePositions.Num(); ++i)
		{
			if (LeaveTypes[i] == 14)
			{
				MakeTree(Parameter, LeaveTypes[i], TreePositions[i], Chunk.ChunkPositionInVoxels, ExtraVoxels);
			}
		}
	}

	// Copy the right values in
	for (int x = 0; x < CHUNK_SIZE; ++x)
	{
		for (int y = 0; y < CHUNK_SIZE; ++y)
		{
			for (int z = 0; z < CHUNK_SIZE; ++z)
			{
				Chunk.Voxels[x + (y << Y_SHIFT) + (z << Z_SHIFT)] = ExtraVoxels[(x + X_EXTRA) + (y + Y_EXTRA) * ExtraDiff.X + (z + Z_EXTRA) * ExtraDiff.X * ExtraDiff.Y];
			}
		}
	}

}

void UTerrainGenerator::MakeTree(const FTerrainGeneratorParameters& Parameter, const uint8 LeafType, const FIntVector3& Position, const FIntVector3& ChunkPosInVoxels, TArray<uint8>& ExtraVoxels)
{

	const int X_EXTRA = 3;
	const int Y_EXTRA = 3;
	const int Z_EXTRA = 11;

	const FIntVector3& LowerBound = ChunkPosInVoxels;
	const FIntVector3& UpperBound = ChunkPosInVoxels + FIntVector3(CHUNK_SIZE - 1);

	// For trees
	const FIntVector3& LowerBoundExtra = LowerBound - FIntVector3(X_EXTRA, Y_EXTRA, Z_EXTRA);
	const FIntVector3& UpperBoundExtra = UpperBound + FIntVector3(X_EXTRA, Y_EXTRA, 0);
	const FIntVector3& ExtraDiff = UpperBoundExtra - LowerBoundExtra + FIntVector3(1);

	int x = Position.X;
	int y = Position.Y;
	int z = Position.Z;

	const TArray<FIntVector3> TreeTrunk =
	{
		{ 0, 0, 1 }, { 0, 0, 2 }, { 0, 0, 3 }, { 0, 0, 4 }
	};

	const TArray<FIntVector3> TreeLeaves =
	{
		{ -1, -1, 5 },{ 0, -1, 5 },{ 1, -1, 5 }, // Square
		{ -1, 0, 5 },{ 0, 0, 5 },{ 1, 0, 5 },
		{ -1, 1, 5 },{ 0, 1, 5 },{ 1, 1, 5 },

		{ -1, -1, 6 },{  0, -1, 6 },{ 1, -1, 6 }, // Square
		{ -1,  0, 6 },{  0,  0, 6 },{ 1,  0, 6 },
		{ -1,  1, 6 },{  0,  1, 6 },{ 1,  1, 6 },
		{ -1, -2, 6 },{  0, -2, 6 },{  1, -2, 6 }, // Side
		{ -1,  2, 6 },{  0,  2, 6 },{  1,  2, 6 },
		{ -2,  1, 6 },{ -2,  0, 6 },{ -2, -1, 6 },
		{ 2 ,  1, 6 },{  2,  0, 6 },{  2, -1, 6 },

		{ -1, -1, 7 },{  0, -1, 7 },{ 1, -1, 7 }, // Square
		{ -1,  0, 7 },{  0,  0, 7 },{ 1,  0, 7 },
		{ -1,  1, 7 },{  0,  1, 7 },{ 1,  1, 7 },
		{ -1, -2, 7 },{  0, -2, 7 },{  1, -2, 7 }, // Side
		{ -1,  2, 7 },{  0,  2, 7 },{  1,  2, 7 },
		{ -2,  1, 7 },{ -2,  0, 7 },{ -2, -1, 7 },
		{ 2 ,  1, 7 },{  2,  0, 7 },{  2, -1, 7 },

		{ -1, -1, 8 },{  0, -1, 8 },{ 1, -1, 8 }, // Square
		{ -1,  0, 8 },{  0,  0, 8 },{ 1,  0, 8 },
		{ -1,  1, 8 },{  0,  1, 8 },{ 1,  1, 8 },
		{ -1, -2, 8 },{  0, -2, 8 },{  1, -2, 8 }, // Side
		{ -1,  2, 8 },{  0,  2, 8 },{  1,  2, 8 },
		{ -2,  1, 8 },{ -2,  0, 8 },{ -2, -1, 8 },
		{ 2 ,  1, 8 },{  2,  0, 8 },{  2, -1, 8 },

		{ -1, -1, 9 },{ 0, -1, 9 },{ 1, -1, 9 }, // Square
		{ -1,  0, 9 },{ 0,  0, 9 },{ 1,  0, 9 },
		{ -1,  1, 9 },{ 0,  1, 9 },{ 1,  1, 9 }

	};

	const TArray<FIntVector3> Snow =
	{
		{ -1, -2, 9 },{ 0, -2, 9 },{ 1, -2,  9 },
		{ -1,  2, 9 },{ 0,  2, 9 },{ 1,  2,  9 },
		{ -2,  1, 9 },{ -2, 0, 9 },{ -2, -1, 9 },
		{ 2 ,  1, 9 },{ 2,  0, 9 },{ 2, -1,  9 },

		{ -1, -1, 10 },{ 0, -1, 10 },{ 1, -1, 10 },
		{ -1,  0, 10 },{ 0,  0, 10 },{ 1,  0, 10 },
		{ -1,  1, 10 },{ 0,  1, 10 },{ 1,  1, 10 }

	};

	uint8 ExistingType = LeafType;

	for (int i = 0; i < TreeLeaves.Num(); ++i)
	{

		const int32 LocalX = x - LowerBoundExtra.X + TreeLeaves[i].X;
		const int32 LocalY = y - LowerBoundExtra.Y + TreeLeaves[i].Y;
		const int32 LocalZ = z - LowerBoundExtra.Z + TreeLeaves[i].Z;

		if (LocalX >= 0 && LocalX < ExtraDiff.X && LocalY >= 0 && LocalY < ExtraDiff.Y && LocalZ >= 0 && LocalZ < ExtraDiff.Z)
		{
			const uint8 CurrType = ExtraVoxels[LocalX + LocalY * ExtraDiff.X + LocalZ * ExtraDiff.X * ExtraDiff.Y];
			if (CurrType != LeafType && (CurrType == 11 || CurrType == 13 || CurrType == 14))
			{
				ExistingType = CurrType;
				break;
			}
		}
	}

	for (int i = 0; i < TreeLeaves.Num(); ++i)
	{

		const int32 LocalX = x - LowerBoundExtra.X + TreeLeaves[i].X;
		const int32 LocalY = y - LowerBoundExtra.Y + TreeLeaves[i].Y;
		const int32 LocalZ = z - LowerBoundExtra.Z + TreeLeaves[i].Z;

		if (LocalX >= 0 && LocalX < ExtraDiff.X && LocalY >= 0 && LocalY < ExtraDiff.Y && LocalZ >= 0 && LocalZ < ExtraDiff.Z)
		{
			ExtraVoxels[LocalX + LocalY * ExtraDiff.X + LocalZ * ExtraDiff.X * ExtraDiff.Y] = ExistingType;
		}
	}

	for (int i = 0; i < TreeTrunk.Num(); ++i)
	{

		const int32 LocalX = x - LowerBoundExtra.X + TreeTrunk[i].X;
		const int32 LocalY = y - LowerBoundExtra.Y + TreeTrunk[i].Y;
		const int32 LocalZ = z - LowerBoundExtra.Z + TreeTrunk[i].Z;

		if (LocalX >= 0 && LocalX < ExtraDiff.X && LocalY >= 0 && LocalY < ExtraDiff.Y && LocalZ >= 0 && LocalZ < ExtraDiff.Z)
		{
			ExtraVoxels[LocalX + LocalY * ExtraDiff.X + LocalZ * ExtraDiff.X * ExtraDiff.Y] = 12;
		}
	}

	if (Parameter.bSnowOnTrees)
	{
		for (int i = 0; i < Snow.Num(); ++i)
		{

			const int32 LocalX = x - LowerBoundExtra.X + Snow[i].X;
			const int32 LocalY = y - LowerBoundExtra.Y + Snow[i].Y;
			const int32 LocalZ = z - LowerBoundExtra.Z + Snow[i].Z;

			if (LocalX >= 0 && LocalX < ExtraDiff.X && LocalY >= 0 && LocalY < ExtraDiff.Y && LocalZ >= 0 && LocalZ < ExtraDiff.Z)
			{
				ExtraVoxels[LocalX + LocalY * ExtraDiff.X + LocalZ * ExtraDiff.X * ExtraDiff.Y] = 3;
			}
		}
	}


}

