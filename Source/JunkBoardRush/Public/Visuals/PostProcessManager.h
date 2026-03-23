
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PostProcessManager.generated.h"

class AExtractionGate;
class AJBRGameState;
class AJBRCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JUNKBOARDRUSH_API UPostProcessManager : public UActorComponent
{
	GENERATED_BODY()

public:

	UPostProcessManager();

	AJBRCharacter* LocalCharacter;
	AJBRGameState* GS;

	TArray<AExtractionGate*> Gates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int PlayerStencilID = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int GateStencilID = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RelicStencilID = 3;

protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void TrySetUpPlayer();

	void GetExtractionGates();

	bool bHasInitPlayer;

public:

	UFUNCTION() void DetermineGatesVisibility();
	UFUNCTION() void DetermineGatesVisibilityFromRelicHolder(AJBRCharacter* RelicHolder);
	UFUNCTION() void DetermineSelfPlayerVisibility();
	UFUNCTION() void DetermineRelicVisibility(AJBRCharacter* RelicHolder);
};
