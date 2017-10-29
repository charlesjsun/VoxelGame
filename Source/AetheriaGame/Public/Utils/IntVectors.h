#pragma once

#include "Engine/EngineTypes.h"
#include "IntVectors.generated.h"

USTRUCT(BlueprintType, Atomic)
struct FIntVector2D
{

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinates")
	int32 X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinates")
	int32 Y;

	FIntVector2D() {}
	FIntVector2D(int32 InX, int32 InY) : X(InX), Y(InY) {}
	FIntVector2D(int32 Scalar) : X(Scalar), Y(Scalar) {}

	FVector2D ToFloat() const { return FVector2D(X, Y); }

	friend uint32 GetTypeHash(const FIntVector2D& Vector)
	{
		return FCrc::MemCrc32(&Vector, sizeof(Vector));
	}

	FString ToString() const { FString::Printf(TEXT("X=%d Y=%d"), X, Y); }

	#define DEFINE_VECTOR_OPERATOR_2D(symbol) \
		FORCEINLINE FIntVector2D operator symbol(const FIntVector2D& Other) const \
		{ \
			return FIntVector2D(X symbol Other.X, Y symbol Other.Y); \
		} \
		FORCEINLINE FIntVector2D operator symbol(const int32& Scalar) const \
		{ \
			return FIntVector2D(X symbol Scalar, Y symbol Scalar); \
		}
	DEFINE_VECTOR_OPERATOR_2D(+);
	DEFINE_VECTOR_OPERATOR_2D(-);
	DEFINE_VECTOR_OPERATOR_2D(*);
	DEFINE_VECTOR_OPERATOR_2D(/ );
	DEFINE_VECTOR_OPERATOR_2D(<< );
	DEFINE_VECTOR_OPERATOR_2D(>> );

	#define DEFINE_ASSIGNMENT_OPERATOR_2D(symbol) \
		FORCEINLINE FIntVector2D operator symbol(const FIntVector2D& Other) \
		{ \
			X symbol Other.X; \
			Y symbol Other.Y; \
			return *this; \
		} \
		FORCEINLINE FIntVector2D operator symbol(const int32& Scalar) \
		{ \
			X symbol Scalar; \
			Y symbol Scalar; \
			return *this; \
		}
	DEFINE_ASSIGNMENT_OPERATOR_2D(+= );
	DEFINE_ASSIGNMENT_OPERATOR_2D(-= );
	DEFINE_ASSIGNMENT_OPERATOR_2D(*= );
	DEFINE_ASSIGNMENT_OPERATOR_2D(/= );
	DEFINE_ASSIGNMENT_OPERATOR_2D(<<= );
	DEFINE_ASSIGNMENT_OPERATOR_2D(>>= );

	friend bool operator==(const FIntVector2D& A, const FIntVector2D& B)
	{
		return A.X == B.X && A.Y == B.Y;
	}

	friend bool operator!=(const FIntVector2D& A, const FIntVector2D& B)
	{
		return A.X != B.X || A.Y != B.Y;
	}

	friend bool operator<(const FIntVector2D& A, const FIntVector2D& B)
	{
		return A.X < B.X && A.Y < B.Y;
	}

	friend bool operator<=(const FIntVector2D& A, const FIntVector2D& B)
	{
		return A.X <= B.X && A.Y <= B.Y;
	}

	friend bool operator>(const FIntVector2D& A, const FIntVector2D& B)
	{
		return A.X > B.X && A.Y > B.Y;
	}

	friend bool operator>=(const FIntVector2D& A, const FIntVector2D& B)
	{
		return A.X >= B.X && A.Y >= B.Y;
	}

	static FORCEINLINE FIntVector2D Max(const FIntVector2D& A, const FIntVector2D& B)
	{
		return FIntVector2D(FMath::Max(A.X, B.X), FMath::Max(A.Y, B.Y));
	}

	static FORCEINLINE FIntVector2D Min(const FIntVector2D& A, const FIntVector2D& B)
	{
		return FIntVector2D(FMath::Min(A.X, B.X), FMath::Min(A.Y, B.Y));
	}

	static FORCEINLINE FIntVector2D Clamp(const FIntVector2D& A, const FIntVector2D& MinA, const FIntVector2D& MaxA)
	{
		return Min(Max(A, MinA), MaxA);
	}

	static FORCEINLINE FIntVector2D Floor(const FVector2D& V)
	{
		return FIntVector2D(FMath::FloorToInt(V.X), FMath::FloorToInt(V.Y));
	}

	static FORCEINLINE FIntVector2D Ceil(const FVector2D& V)
	{
		return FIntVector2D(FMath::CeilToInt(V.X), FMath::CeilToInt(V.Y));
	}

};

USTRUCT(BlueprintType, Atomic)
struct FIntVector3
{

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinates")
	int32 X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinates")
	int32 Y;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinates")
	int32 Z;

	FIntVector3() {}
	FIntVector3(int32 InX, int32 InY, int32 InZ) : X(InX), Y(InY), Z(InZ) {}
	FIntVector3(int32 Scalar) : X(Scalar), Y(Scalar), Z(Scalar) {}

	FVector ToFloat() const { return FVector(X, Y, Z); }

	friend uint32 GetTypeHash(const FIntVector3& Vector)
	{
		return FCrc::MemCrc32(&Vector, sizeof(Vector));
	}

	FString ToString() const { return FString::Printf(TEXT("X=%d Y=%d Z=%d"), X, Y, Z); }

	#define DEFINE_VECTOR_OPERATOR_3(symbol) \
		FORCEINLINE FIntVector3 operator symbol(const FIntVector3& Other) const \
		{ \
			return FIntVector3(X symbol Other.X, Y symbol Other.Y, Z symbol Other.Z); \
		} \
		FORCEINLINE FIntVector3 operator symbol(const int32& Scalar) const \
		{ \
			return FIntVector3(X symbol Scalar, Y symbol Scalar, Z symbol Scalar); \
		}
	DEFINE_VECTOR_OPERATOR_3(+);
	DEFINE_VECTOR_OPERATOR_3(-);
	DEFINE_VECTOR_OPERATOR_3(*);
	DEFINE_VECTOR_OPERATOR_3(/ );
	DEFINE_VECTOR_OPERATOR_3(<< );
	DEFINE_VECTOR_OPERATOR_3(>> );

	#define DEFINE_ASSIGNMENT_OPERATOR_3(symbol) \
		FORCEINLINE FIntVector3 operator symbol(const FIntVector3& Other) \
		{ \
			X symbol Other.X; \
			Y symbol Other.Y; \
			Z symbol Other.Z; \
			return *this; \
		} \
		FORCEINLINE FIntVector3 operator symbol(const int32& Scalar) \
		{ \
			X symbol Scalar; \
			Y symbol Scalar; \
			Z symbol Scalar; \
			return *this; \
		}
	DEFINE_ASSIGNMENT_OPERATOR_3(+= );
	DEFINE_ASSIGNMENT_OPERATOR_3(-= );
	DEFINE_ASSIGNMENT_OPERATOR_3(*= );
	DEFINE_ASSIGNMENT_OPERATOR_3(/= );
	DEFINE_ASSIGNMENT_OPERATOR_3(<<= );
	DEFINE_ASSIGNMENT_OPERATOR_3(>>= );

	friend bool operator==(const FIntVector3& A, const FIntVector3& B)
	{
		return A.X == B.X && A.Y == B.Y && A.Z == B.Z;
	}

	friend bool operator!=(const FIntVector3& A, const FIntVector3& B)
	{
		return A.X != B.X || A.Y != B.Y || A.Z != B.Z;
	}

	friend bool operator<(const FIntVector3& A, const FIntVector3& B)
	{
		return A.X < B.X && A.Y < B.Y && A.Z < B.Z;
	}

	friend bool operator<=(const FIntVector3& A, const FIntVector3& B)
	{
		return A.X <= B.X && A.Y <= B.Y && A.Z <= B.Z;
	}

	friend bool operator>(const FIntVector3& A, const FIntVector3& B)
	{
		return A.X > B.X && A.Y > B.Y && A.Z > B.Z;
	}

	friend bool operator>=(const FIntVector3& A, const FIntVector3& B)
	{
		return A.X >= B.X && A.Y >= B.Y && A.Z >= B.Z;
	}

	static FORCEINLINE FIntVector3 Max(const FIntVector3& A, const FIntVector3& B)
	{
		return FIntVector3(FMath::Max(A.X, B.X), FMath::Max(A.Y, B.Y), FMath::Max(A.Z, B.Z));
	}

	static FORCEINLINE FIntVector3 Min(const FIntVector3& A, const FIntVector3& B)
	{
		return FIntVector3(FMath::Min(A.X, B.X), FMath::Min(A.Y, B.Y), FMath::Min(A.Z, B.Z));
	}

	static FORCEINLINE FIntVector3 Clamp(const FIntVector3& A, const FIntVector3& MinA, const FIntVector3& MaxA)
	{
		return Min(Max(A, MinA), MaxA);
	}

	static FORCEINLINE FIntVector3 Floor(const FVector& V)
	{
		return FIntVector3(FMath::FloorToInt(V.X), FMath::FloorToInt(V.Y), FMath::FloorToInt(V.Z));
	}

	static FORCEINLINE FIntVector3 Ceil(const FVector& V)
	{
		return FIntVector3(FMath::CeilToInt(V.X), FMath::CeilToInt(V.Y), FMath::CeilToInt(V.Z));
	}

};