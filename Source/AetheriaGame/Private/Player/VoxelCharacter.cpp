// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#include "VoxelCharacter.h"

#include "VoxelPlayerController.h"
#include "VoxelCharacterMovementComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

#include "Kismet/GameplayStatics.h"

#include "VoxelTerrain.h"

#include "DebugLibrary.h"

AVoxelCharacter::AVoxelCharacter(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UVoxelCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	UCharacterMovementComponent* MoveComponent = GetCharacterMovement();
	MoveComponent->GravityScale = 1.0f;

	CameraComponent = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->bUsePawnControlRotation = true;
	
#if 0
	InventoryComponent = ObjectInitializer.CreateDefaultSubobject<UInventoryComponent>(this, TEXT("InventoryComponent"));
#endif

	bIsSprinting = false;
	SprintingSpeedMultiplier = 8.0f;
	ReachDistance = 800.0f;
	CurrentVoxel = 0;

}

void AVoxelCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<AActor*> OutTerrain;
	UGameplayStatics::GetAllActorsOfClass(this, AVoxelTerrain::StaticClass(), OutTerrain);
	VoxelTerrain = Cast<AVoxelTerrain>(OutTerrain[0]);

}

void AVoxelCharacter::BeginDestroy()
{
	VoxelTerrain = nullptr;

	Super::BeginDestroy();
}


void AVoxelCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VOXEL TERRAIN
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// ///////////////////////////////////////////
/// VOXEL METHODS
/// ///////////////////////////////////////////


void AVoxelCharacter::BreakVoxel(const int32& x, const int32& y, const int32& z)
{
	const uint8 Voxel = VoxelTerrain->GetVoxelAt(x, y, z);

#if 0
	// WIP: Add the broken voxel to the character inventory
	UItemBasicVoxel* ItemBasicVoxel = NewObject<UItemBasicVoxel>(this);
	ItemBasicVoxel->Init(Voxel);
	InventoryComponent->AddToInventory(ItemBasicVoxel);
#endif

	VoxelTerrain->SetVoxelAt(x, y, z, 0);
}

void AVoxelCharacter::PlaceVoxel(const int32& x, const int32& y, const int32& z, const uint8& Voxel)
{
	VoxelTerrain->SetVoxelAt(x, y, z, Voxel);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// SETUP INPUT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AVoxelCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("LeftClick", IE_Pressed, this, &AVoxelCharacter::OnLeftClick);
	PlayerInputComponent->BindAction("RightClick", IE_Pressed, this, &AVoxelCharacter::OnRightClick);
	PlayerInputComponent->BindAction("MiddleClick", IE_Pressed, this, &AVoxelCharacter::OnMiddleClick);
	PlayerInputComponent->BindAction("ScrollUp", IE_Pressed, this, &AVoxelCharacter::OnScrollUp);
	PlayerInputComponent->BindAction("ScrollDown", IE_Pressed, this, &AVoxelCharacter::OnScrollDown);

	PlayerInputComponent->BindAxis("MoveForward", this, &AVoxelCharacter::OnMoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVoxelCharacter::OnMoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &AVoxelCharacter::OnMoveUp);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookRight", this, &APawn::AddControllerYawInput);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AVoxelCharacter::OnStartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AVoxelCharacter::OnEndSprinting);
}

//////////////////////////////////////////////////////////////////////////
/// INVENTORY
//////////////////////////////////////////////////////////////////////////

void AVoxelCharacter::DropItemInSlot(const int32& Slot)
{
#if 0
	// If the slot exist
	if (Slot >= 0 && Slot < InventoryComponent->NumSlots())
	{
		// Spawn the actor equivalent of the item
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AItemActor* NewItemActor = GetWorld()->SpawnActor<AItemActor>(AItemActor::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, SpawnInfo);
		if (NewItemActor)
		{
			NewItemActor->Init(InventoryComponent->GetInventoryItemAtSlot(Slot).Item);
		}
	}

#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MOUSE ACTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void AVoxelCharacter::OnLeftClick()
{
	if (Controller != nullptr)
	{
		// Raycast and break the selected voxel
		AVoxelPlayerController* const PlayerController = Cast<AVoxelPlayerController>(Controller);
		FIntVector3 OutVoxelCoord, OutHitNormal;
		if (PlayerController->RayTraceVoxelCoord(ReachDistance, OutVoxelCoord, OutHitNormal))
		{
			BreakVoxel(OutVoxelCoord.X, OutVoxelCoord.Y, OutVoxelCoord.Z);
		}
	}
}

void AVoxelCharacter::OnRightClick()
{
	if (Controller != nullptr)
	{
		// if the current voxel is not air
		if (CurrentVoxel != 0)
		{
			// Raycast and place the selected voxel on the face that it clicked
			AVoxelPlayerController* const PlayerController = Cast<AVoxelPlayerController>(Controller);
			FIntVector3 OutVoxelCoord, OutHitNormal;
			if (PlayerController->RayTraceVoxelCoord(ReachDistance, OutVoxelCoord, OutHitNormal))
			{
				const FIntVector3 PlaceCoord = OutVoxelCoord + OutHitNormal;
				PlaceVoxel(PlaceCoord.X, PlaceCoord.Y, PlaceCoord.Z, CurrentVoxel);
			}
		}
	}
}

void AVoxelCharacter::OnMiddleClick()
{
	if (Controller != nullptr)
	{
		// Select a voxel that's clicked
		AVoxelPlayerController* const PlayerController = Cast<AVoxelPlayerController>(Controller);
		FIntVector3 OutVoxelCoord, OutHitNormal;
		if (PlayerController->RayTraceVoxelCoord(ReachDistance, OutVoxelCoord, OutHitNormal))
		{
			const uint8 Voxel = VoxelTerrain->GetVoxelAt(OutVoxelCoord.X, OutVoxelCoord.Y, OutVoxelCoord.Z);
			if (Voxel != 0)
			{
				CurrentVoxel = Voxel;
			}
		}
	}
}

void AVoxelCharacter::OnScrollUp()
{
	// uint8 wraps around from 0 -> 255 when -1, so instead of modulo (%) we take the min of voxel and num voxel when voxel wraps to 255
	CurrentVoxel = FMath::Min<uint8>(CurrentVoxel - 1, VoxelTerrain->GetNumVoxelTypes() - 1);
	UDebugLibrary::Println(this, 10, FString::Printf(TEXT("Voxel Type: %d"), CurrentVoxel));
}

void AVoxelCharacter::OnScrollDown()
{
	// Next voxel
	++CurrentVoxel;
	CurrentVoxel %= VoxelTerrain->GetNumVoxelTypes();
	UDebugLibrary::Println(this, 10, FString::Printf(TEXT("Voxel Type: %d"), CurrentVoxel));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MOVEMENT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AVoxelCharacter::OnMoveForward(float Val)
{
	if (Controller != nullptr && Val != 0.0f)
	{
		// Basic movement
		const FRotator Rotation = GetActorRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);

		AddMovementInput(Direction, Val);
	}
}

void AVoxelCharacter::OnMoveRight(float Val)
{
	if (Controller != nullptr && Val != 0.0f)
	{
		// Basic movement
		const FRotator Rotation = GetActorRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, Val);
	}
}

void AVoxelCharacter::OnMoveUp(float Val)
{
	// DEBUG: FLYING ONLY
	if (Controller != nullptr && Val != 0.0f)
	{
		AddMovementInput(FVector::UpVector, Val);
	}
}

void AVoxelCharacter::OnStartSprinting()
{
	SetSprinting(true);
}

void AVoxelCharacter::OnEndSprinting()
{
	SetSprinting(false);
}

void AVoxelCharacter::SetSprinting(bool NewSprinting)
{
	bIsSprinting = NewSprinting;
}

bool AVoxelCharacter::IsSprinting() const
{
	return bIsSprinting;
}

float AVoxelCharacter::GetSprintingSpeedMultiplier() const
{
	return SprintingSpeedMultiplier;
}
