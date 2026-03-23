#pragma once

#include "CoreMinimal.h"
#include "FCameraLerpBoost.generated.h"

USTRUCT()
struct FCameraLerpBoost
{
	GENERATED_BODY()

	float Multiplier = 1.f;
	float Duration = 0.f;
	float TimeRemaining = 0.f;

	void Start(float InMultiplier, float InDuration)
	{
		Multiplier = InMultiplier;
		Duration = InDuration;
		TimeRemaining = InDuration;
	}

	float GetAlpha() const
	{
		return Duration > 0.f ? TimeRemaining / Duration : 0.f;
	}

	bool IsActive() const
	{
		return TimeRemaining > 0.f;
	}

	void Tick(float DeltaTime)
	{
		TimeRemaining = FMath::Max(0.f, TimeRemaining - DeltaTime);
	}
};
