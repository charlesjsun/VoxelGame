// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#pragma once

#include "CoreMinimal.h"



#include "Kismet/BlueprintFunctionLibrary.h"
#include "DebugLibrary.generated.h"

#define BOOL_STRING(condition) ((condition) ? FString(TEXT("true")) : FString(TEXT("false")))
#define INT_STRING(i) FString::FromInt(i)
#define FLOAT_STRING(f) FString::SanitizeFloat(f)

#define GET_ROLE(role,string) FString string; \
	if (role == ROLE_Authority) string = TEXT("ROLE_Authority"); \
	else if (role == ROLE_AutonomousProxy) string = TEXT("ROLE_AutonomousProxy"); \
	else if (role == ROLE_SimulatedProxy) string = TEXT("ROLE_SimulatedProxy"); \
	else string = TEXT("ROLE_None");

#define GET_THIS_ROLE(string) GET_ROLE(Role, string)

#define TIME_START() const double __StartTime__ = FPlatformTime::Seconds();
#define TIME_END() const double __EndTime__ = FPlatformTime::Seconds(); const double ElapsedTime = __EndTime__ - __StartTime__;

#define TIME_START_NUM(num) const double __StartTime__##num = FPlatformTime::Seconds();
#define TIME_END_NUM(num) const double __EndTime__##num = FPlatformTime::Seconds(); const double ElapsedTime##num = __EndTime__##num - __StartTime__##num;	

/**
*
*/
UCLASS()
class AETHERIAGAME_API UDebugLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Debug Library")
	static void Println(const UObject* const ContextObject, const float Time, const FString Message);

};

