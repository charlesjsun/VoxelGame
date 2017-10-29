// Fill out your copyright notice in the Description page of Project Settings.

#include "DebugLibrary.h"

#include "Runtime/Engine/Public/EngineGlobals.h"
#include "Engine/Engine.h"

void UDebugLibrary::Println(const UObject* const ContextObject, const float Time, const FString Message)
{
	UWorld* World = GEngine->GetWorldFromContextObject(ContextObject, false);
	FString NetType;
	if (World)
	{
		if (World->WorldType == EWorldType::PIE)
		{
			switch (World->GetNetMode())
			{
			case NM_Client:
				NetType = FString::Printf(TEXT("Client %d: "), GPlayInEditorID - 1);
				break;
			case NM_DedicatedServer:
			case NM_ListenServer:
				NetType = FString::Printf(TEXT("Server: "));
				break;
			case NM_Standalone:
				break;
			}
		}
	}
	FString ThreadId = ""; /*FString::Printf(TEXT("%d: "), GetCurrentThreadId());*/
	GEngine->AddOnScreenDebugMessage(-1, Time, FColor::White, NetType + ThreadId + Message);
}