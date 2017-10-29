// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#include "VoxelCharacterMovementComponent.h"

#include "VoxelCharacter.h"

UVoxelCharacterMovementComponent::UVoxelCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCheatFlying = false;
	MaxFlySpeed = 4096.0f / 4.0f;
	MaxAcceleration = MaxFlySpeed * 16.0f;
	BrakingDecelerationFlying = MaxFlySpeed * 16.0f;

}

float UVoxelCharacterMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const AVoxelCharacter* CharOwner = Cast<AVoxelCharacter>(PawnOwner);
	if (CharOwner)
	{
		// If player is sprinting then max speed is multiplied
		if (CharOwner->IsSprinting())
		{
			MaxSpeed *= CharOwner->GetSprintingSpeedMultiplier();
		}
	}

	return MaxSpeed;
}


