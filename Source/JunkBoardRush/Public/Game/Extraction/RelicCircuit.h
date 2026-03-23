// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "RelicCircuit.generated.h"

UCLASS()
class JUNKBOARDRUSH_API ARelicCircuit : public AActor
{
	GENERATED_BODY()
	
public:	
	ARelicCircuit();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USplineComponent* SplineComponent;

	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	TArray<ARelicCircuit*> NextCircuits;

};
