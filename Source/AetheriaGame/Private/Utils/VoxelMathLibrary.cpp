// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#include "VoxelMathLibrary.h"

bool UVoxelMathLibrary::EqualEqual_VectorVector(FIntVector3 A, FIntVector3 B)
{
	return A == B;
}
bool UVoxelMathLibrary::NotEqual_VectorVector(FIntVector3 A, FIntVector3 B)
{
	return A != B;
}
FIntVector3 UVoxelMathLibrary::Add_IntVector3Int(FIntVector3 A, int32 B)
{
	return A + B;
}
FIntVector3 UVoxelMathLibrary::Add_IntVector3IntVector3(FIntVector3 A, FIntVector3 B)
{
	return A + B;
}
FIntVector3 UVoxelMathLibrary::Subtract_IntVector3Int(FIntVector3 A, int32 B)
{
	return A - B;
}
FIntVector3 UVoxelMathLibrary::Subtract_IntVector3IntVector3(FIntVector3 A, FIntVector3 B)
{
	return A - B;
}
FIntVector3 UVoxelMathLibrary::Multiply_IntVector3Int(FIntVector3 A, int32 B)
{
	return A * B;
}
FIntVector3 UVoxelMathLibrary::Multiply_IntVector3IntVector3(FIntVector3 A, FIntVector3 B)
{
	return A * B;
}
FIntVector3 UVoxelMathLibrary::Divide_IntVector3Int(FIntVector3 A, int32 B)
{
	return A / B;
}
FIntVector3 UVoxelMathLibrary::Divide_IntVector3IntVector3(FIntVector3 A, FIntVector3 B)
{
	return A / B;
}
FString UVoxelMathLibrary::Conv_IntVector3ToString(const FIntVector3& A)
{
	return A.ToString();
}
FVector UVoxelMathLibrary::Conv_IntVector3ToVector(const FIntVector3& A)
{
	return A.ToFloat();
}
FIntVector3 UVoxelMathLibrary::Conv_FloorVectorToIntVector3(const FVector& A)
{
	return FIntVector3::Floor(A);
}
FIntVector3 UVoxelMathLibrary::Conv_CeilVectorToIntVector3(const FVector& A)
{
	return FIntVector3::Ceil(A);
}
FIntVector3 UVoxelMathLibrary::MinIntVector3(const FIntVector3& A, const FIntVector3& B)
{
	return FIntVector3::Min(A, B);
}
FIntVector3 UVoxelMathLibrary::MaxIntVector3(const FIntVector3& A, const FIntVector3& B)
{
	return FIntVector3::Max(A, B);
}
FIntVector3 UVoxelMathLibrary::ClampIntVector3(const FIntVector3& A, const FIntVector3& MinA, const FIntVector3& MaxA)
{
	return FIntVector3::Clamp(A, MinA, MaxA);
}



