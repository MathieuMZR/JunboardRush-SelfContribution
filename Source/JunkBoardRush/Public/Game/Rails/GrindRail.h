
#pragma once

#include "CoreMinimal.h"

#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "RailPoint.h"
#include "Player/Tricks/ContextActionTarget.h"

#include "GrindRail.generated.h"

class UCapsuleComponent;
class AJBRCharacter;

UCLASS()
class JUNKBOARDRUSH_API AGrindRail : public AActor, public IContextActionTarget
{
	GENERATED_BODY()
	
public:

	virtual void TriggerContextAction(AJBRCharacter* Initiator) override;

#pragma region Init
	
	AGrindRail(const FObjectInitializer& ObjectInitializer);
	
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;

#pragma endregion Init

#pragma region Collisions
	
	void GenerateSplineCollisionPoints();
	UFUNCTION() void HandlePointsCollisions();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int SplinePointPerStep = 50;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float CollisionPointRadius = 20.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float CollisionPointHeight = 120.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float CollisionPointRadiusMultForFirstAndLast = 2.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rail")
	float OffsetY = 50.f;

#pragma endregion Collisions

#pragma region Start Rails
	
	void SnapPlayerToRail(AJBRCharacter* Character, FRailPoint* RailPoint);
	void StartGrind(AJBRCharacter* Character, FRailPoint* RailPoint);

#pragma endregion Start Rails

#pragma region Debug
	
	void ShowDebugPoints();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collisions")
	bool ShowDebug = false;

#pragma endregion Debug

#pragma region Distance
	
	float GetDistanceAtIndex(int Index, FVector& WorldPosition);
	float GetDistanceRatioAtIndex(int Index);

#pragma endregion Distance

#pragma region Direction
	
	FVector ComputePreviousDirection(int PreviousIndex, int ActualIndex);
	FVector ComputeNextDirection(int NextIndex, int ActualIndex);
	int ComputeGrindDirection(AJBRCharacter* Character, FRailPoint* RailPoint);

#pragma endregion Direction
	
	USplineComponent* Spline;
	UPROPERTY(BlueprintReadWrite)
	UCapsuleComponent* TrackingCapsule;

private:
	
	int RailPointCount = 30;
	TArray<FRailPoint> RailPoints;

	FTimerHandle CustomTick;

	UFUNCTION() void OnCapsuleOverlap(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	FRailPoint* FindNearestRailPoint(const FVector& WorldLocation);
};
