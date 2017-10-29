// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"



#include "GameFramework/GameModeBase.h"
#include "AetheriaGameGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class AETHERIAGAME_API AAetheriaGameGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	AAetheriaGameGameModeBase(const class FObjectInitializer& Initializer);

	virtual void PreInitializeComponents() override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaSeconds) override;
	
};
