// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#pragma once

#include "CoreMinimal.h"


#include "GameFramework/CharacterMovementComponent.h"
#include "VoxelCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class AETHERIAGAME_API UVoxelCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:

	UVoxelCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	// Get the speed of the player
	virtual float GetMaxSpeed() const override;
	
	
};
