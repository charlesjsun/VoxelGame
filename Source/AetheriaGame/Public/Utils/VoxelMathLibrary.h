// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "IntVectors.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelMathLibrary.generated.h"

/**
 * A Blueprint function library that handles every blueprint arithmetics with FIntVector3
 */
UCLASS()
class AETHERIAGAME_API UVoxelMathLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	/* Returns true if IntVector3 A is equal to IntVector3 B (A == B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (IntVector3)", CompactNodeTitle = "==", Keywords = "== equal"), Category = "Math|IntVector3")
	static bool EqualEqual_VectorVector(FIntVector3 A, FIntVector3 B);

	/* Returns true if IntVector3 A is not equal to IntVector3 B (A == B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Not Equal (IntVector3)", CompactNodeTitle = "!=", Keywords = "!= not equal"), Category = "Math|IntVector3")
	static bool NotEqual_VectorVector(FIntVector3 A, FIntVector3 B);

	/* Adds a int to each component of an IntVector3 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector3 + int", CompactNodeTitle = "+", Keywords = "+ add plus"), Category = "Math|IntVector3")
	static FIntVector3 Add_IntVector3Int(FIntVector3 A, int32 B);

	/* IntVector3 addition */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector3 + IntVector3", CompactNodeTitle = "+", Keywords = "+ add plus", CommutativeAssociativeBinaryOperator = "true"), Category = "Math|IntVector3")
	static FIntVector3 Add_IntVector3IntVector3(FIntVector3 A, FIntVector3 B);

	/* Subtracts an int to each component of an IntVector3 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector3 - int", CompactNodeTitle = "-", Keywords = "- subtract minus"), Category = "Math|IntVector3")
	static FIntVector3 Subtract_IntVector3Int(FIntVector3 A, int32 B);

	/* IntVector3 subtraction */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector3 - IntVector3", CompactNodeTitle = "-", Keywords = "- subtract minus", CommutativeAssociativeBinaryOperator = "true"), Category = "Math|IntVector3")
	static FIntVector3 Subtract_IntVector3IntVector3(FIntVector3 A, FIntVector3 B);

	/* Multiplies an int to each component of an IntVector3 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector3 * int", CompactNodeTitle = "*", Keywords = "* multiply"), Category = "Math|IntVector3")
	static FIntVector3 Multiply_IntVector3Int(FIntVector3 A, int32 B);

	/* IntVector3 multiplication */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector3 * IntVector3", CompactNodeTitle = "*", Keywords = "* multiply", CommutativeAssociativeBinaryOperator = "true"), Category = "Math|IntVector3")
	static FIntVector3 Multiply_IntVector3IntVector3(FIntVector3 A, FIntVector3 B);

	/* Divides a int to each component of an IntVector3 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector3 / int", CompactNodeTitle = "/", Keywords = "/ divide division"), Category = "Math|IntVector3")
	static FIntVector3 Divide_IntVector3Int(FIntVector3 A, int32 B = 1);

	/* IntVector3 division */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector3 / IntVector3", CompactNodeTitle = "/", Keywords = "/ divide division", CommutativeAssociativeBinaryOperator = "true"), Category = "Math|IntVector3")
	static FIntVector3 Divide_IntVector3IntVector3(FIntVector3 A, FIntVector3 B);

	/* Convert a IntVector3 to a String */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (IntVector3)", CompactNodeTitle = "->", Keywords = "cast convert", BlueprintAutocast), Category = "Math|Conversions")
	static FString Conv_IntVector3ToString(const FIntVector3& A);

	/* Convert a IntVector3 to a vector */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToVector (IntVector3)", CompactNodeTitle = "->", Keywords = "cast convert", BlueprintAutocast), Category = "Math|Conversions")
	static FVector Conv_IntVector3ToVector(const FIntVector3& A);

	/* Convert a vector to a IntVector3 by flooring every component */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Floor ToIntVector3 (vector)", CompactNodeTitle = "FLOOR", Keywords = "floor cast convert"), Category = "Math|Conversions")
	static FIntVector3 Conv_FloorVectorToIntVector3(const FVector& A);

	/* Convert a vector to a IntVector3 by ceiling every component */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ceil ToIntVector3 (vector)", CompactNodeTitle = "CEIL", Keywords = "ceil cast convert"), Category = "Math|Conversions")
	static FIntVector3 Conv_CeilVectorToIntVector3(const FVector& A);

	/* Returns a IntVector3 with the minimum values from A and B */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Min (IntVector3)", CompactNodeTitle = "MIN", CommutativeAssociativeBinaryOperator = "true"), Category = "Math|IntVector3")
	static FIntVector3 MinIntVector3(const FIntVector3& A, const FIntVector3& B);

	/* Returns a IntVector3 with the maximum values from A and B */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Max (IntVector3)", CompactNodeTitle = "MAX", CommutativeAssociativeBinaryOperator = "true"), Category = "Math|IntVector3")
	static FIntVector3 MaxIntVector3(const FIntVector3& A, const FIntVector3& B);

	/* Returns a IntVector3 with values clamped to be between MinA and MaxA (inclusive) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Clamp (float)"), Category = "Math|Float")
	static FIntVector3 ClampIntVector3(const FIntVector3& A, const FIntVector3& MinA, const FIntVector3& MaxA);

	
	
};
