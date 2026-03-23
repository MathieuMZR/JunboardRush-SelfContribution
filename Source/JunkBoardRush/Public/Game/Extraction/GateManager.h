// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/JBRCharacter.h"
#include "GameFramework/Actor.h"
#include "Game/Extraction/ExtractionGate.h"
#include "GateManager.generated.h"

USTRUCT(BlueprintType)
struct FGateConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AExtractionGate*> Gates;
};

UCLASS()
class JUNKBOARDRUSH_API AGateManager : public AActor
{
	GENERATED_BODY()
	
public:	

	AGateManager();

	void Init(ARelic* Relic);

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TArray<FGateConfiguration> GateConfigurations;

private:
	int32 LastConfigurationIndex = -1;
	
	UFUNCTION()
	void OnRelicSpawned();
	
	UFUNCTION()
	void OnRelicStartRespawn();

	UFUNCTION()
	void OnRelicDropped();

	UFUNCTION()
	void OnRelicOwnershipChanged(AJBRCharacter* NewOwner, AJBRCharacter* PreviousOwner);
	
	UPROPERTY()
	TArray<AExtractionGate*> CurrentActiveSet;
	
	void SelectNewConfiguration();
	void DisableAllGates();
	void ReactivateCurrentSet();

};
