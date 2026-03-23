#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/Tricks/ContextActionTarget.h"
#include "Bumper.generated.h"

class AJBRCharacter;
class USphereComponent;

UCLASS()
class JUNKBOARDRUSH_API ABumper : public AActor, public IContextActionTarget
{
	GENERATED_BODY()

public:

	ABumper();

	virtual void TriggerContextAction(AJBRCharacter* Initiator) override;

	UFUNCTION(Server, Reliable)
	void Server_ApplyBumper(AJBRCharacter* Target, FVector PropulsionVector);

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* TriggerZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	float BumperForce = 1000.f;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent*OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	FVector GetPropulsionVector();
	void SetPlayerRotationWallJump();

	AJBRCharacter* InZoneCharacter;
};
