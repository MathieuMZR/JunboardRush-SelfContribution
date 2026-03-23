
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RailSystemComponent.generated.h"

struct FRailPoint;
class AGrindRail;
class AJBRCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JUNKBOARDRUSH_API URailSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URailSystemComponent();
	void SetLastRail(AGrindRail* Rail);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool IsOnRail;

	UPROPERTY()
	AGrindRail* LastRail;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FTimerHandle ClearLastRailTimerHandle;
	float ClearRailDelay = 0.2f;
	
	void ClearOldRail();
};
