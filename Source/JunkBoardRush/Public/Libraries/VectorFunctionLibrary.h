// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VectorFunctionLibrary.generated.h"

UCLASS()
class JUNKBOARDRUSH_API UVectorFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float GetAngleBetweenTwoVectorsDegrees(const FVector& VectorA, const FVector& VectorB);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float GetSignedAngleBetweenDirections(const FVector2D& VectorA, const FVector2D& VectorB);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float GetNormalizedValue(float CurrentAngle, float MinAngle, float MaxAngle);
};
