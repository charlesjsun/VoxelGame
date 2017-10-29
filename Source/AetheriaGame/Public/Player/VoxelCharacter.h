// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#pragma once

#include "CoreMinimal.h"



#include "GameFramework/Character.h"
#include "VoxelCharacter.generated.h"

UCLASS()
class AETHERIAGAME_API AVoxelCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVoxelCharacter(const class FObjectInitializer& ObjectInitializer);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//////////////////////////////////////////////////////////////////////////
	/// COMPONENTS
	//////////////////////////////////////////////////////////////////////////

protected:

	// Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* CameraComponent;

#if 0
	// Inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	class UInventoryComponent* InventoryComponent;
#endif

	// Terrain Reference
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient, Category = "Voxel Terrain")
	class AVoxelTerrain* VoxelTerrain;

	// The distance in UU that a character can reach
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Voxel Terrain")
	float ReachDistance;

	// The current selected voxel to place
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Voxel Terrain")
	int32 CurrentVoxel;

	//////////////////////////////////////////////////////////////////////////
	/// VOXEL TERRAIN
	//////////////////////////////////////////////////////////////////////////

public:

	// Breaks a voxel
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain")
	void BreakVoxel(const int32& x, const int32& y, const int32& z);

	// Places a voxel
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain")
	void PlaceVoxel(const int32& x, const int32& y, const int32& z, const uint8& Voxel);

	//////////////////////////////////////////////////////////////////////////
	/// INVENTORY
	//////////////////////////////////////////////////////////////////////////

	// Drop the current selected item
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DropItemInSlot(const int32& Slot);

	//////////////////////////////////////////////////////////////////////////
	/// MOUSE ACTIONS
	//////////////////////////////////////////////////////////////////////////

	void OnLeftClick();
	void OnRightClick();
	void OnMiddleClick();
	void OnScrollUp();
	void OnScrollDown();

	//////////////////////////////////////////////////////////////////////////
	/// MOVEMENT
	//////////////////////////////////////////////////////////////////////////

private:

	UPROPERTY(Transient)
	bool bIsSprinting;

	// Amount to multiply the speed by if player is sprinting
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintingSpeedMultiplier;

public:

	// Input Events for moving
	void OnMoveForward(float Val);
	void OnMoveRight(float Val);
	void OnMoveUp(float Val);

	// Input Events for sprinting
	void OnStartSprinting();
	void OnEndSprinting();

	void SetSprinting(bool NewSprinting);

	// Returns true if player is sprinting
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Movement")
	bool IsSprinting() const;

	// Get the SprintSpeedMultiplier
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Movement")
	float GetSprintingSpeedMultiplier() const;
	
};
