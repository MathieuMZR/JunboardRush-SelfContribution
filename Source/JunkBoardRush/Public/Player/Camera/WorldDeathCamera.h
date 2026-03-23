
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldDeathCamera.generated.h"

class AJBRCharacter;
class UCapsuleComponent;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class JUNKBOARDRUSH_API AWorldDeathCamera : public AActor
{
	GENERATED_BODY()

public:

	AWorldDeathCamera();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	UCapsuleComponent* Capsule;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArm;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera;

	UPROPERTY()
	AJBRCharacter* FollowTarget = nullptr;

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	FVector CurrentPosition, DesiredPosition;
	FRotator CurrentRotation, DesiredRotation;
};
