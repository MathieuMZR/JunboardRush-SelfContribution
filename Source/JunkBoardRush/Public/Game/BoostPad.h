
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/Tricks/ContextActionTarget.h"
#include "BoostPad.generated.h"

class AJBRCharacter;

UCLASS()
class JUNKBOARDRUSH_API ABoostPad : public AActor, public IContextActionTarget
{
	GENERATED_BODY()

public:
	ABoostPad();

	virtual void TriggerContextAction(AJBRCharacter* Initiator) override;

	UPROPERTY(EditDefaultsOnly)
	bool IsRamp;

	UFUNCTION(BlueprintCallable)
	void AddPlayerBoost(AJBRCharacter* Character);
};
