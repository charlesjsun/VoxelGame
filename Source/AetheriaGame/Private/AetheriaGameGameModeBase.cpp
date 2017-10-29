// Fill out your copyright notice in the Description page of Project Settings.

#include "AetheriaGameGameModeBase.h"

#include "VoxelCharacter.h"

AAetheriaGameGameModeBase::AAetheriaGameGameModeBase(const class FObjectInitializer& Initializer)
	: Super(Initializer)
{
	DefaultPawnClass = AVoxelCharacter::StaticClass();
}

void AAetheriaGameGameModeBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AAetheriaGameGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

void AAetheriaGameGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AAetheriaGameGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}
