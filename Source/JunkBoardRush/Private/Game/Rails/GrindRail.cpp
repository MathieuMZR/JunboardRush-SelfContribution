
#include "Game/Rails/GrindRail.h"

#include "Components/CapsuleComponent.h"
#include "Game/Rails/RailSystemComponent.h"

#include "Framework/JBRCharacter.h"
#include "Framework/JBRCustomCMC.h"
#include "Kismet/KismetMathLibrary.h"

#include "Kismet/KismetSystemLibrary.h"

#pragma region Init

AGrindRail::AGrindRail(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGrindRail::BeginPlay()
{
	Super::BeginPlay();

	Spline = FindComponentByClass<USplineComponent>();

	if (Spline == nullptr) return;

	TrackingCapsule = NewObject<UCapsuleComponent>(this, TEXT("TrackingCapsule"));
	TrackingCapsule->SetupAttachment(GetRootComponent());
	TrackingCapsule->RegisterComponent();
	TrackingCapsule->SetCapsuleSize(CollisionPointRadius, CollisionPointHeight);
	TrackingCapsule->SetCollisionObjectType(ECC_WorldDynamic);
	TrackingCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TrackingCapsule->SetGenerateOverlapEvents(true);
	TrackingCapsule->OnComponentBeginOverlap.AddDynamic(this, &AGrindRail::OnCapsuleOverlap);
	//TrackingCapsule->SetHiddenInGame(false);
	
	GenerateSplineCollisionPoints();

	GetWorld()->GetTimerManager().SetTimer(
		CustomTick, this, &AGrindRail::HandlePointsCollisions, 0.1f, true);
}

void AGrindRail::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void AGrindRail::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ShowDebug) ShowDebugPoints();
}

#pragma endregion Init

#pragma region Collisions

void AGrindRail::GenerateSplineCollisionPoints()
{
	float SplineLength = Spline->GetSplineLength();
	float StepDistance = SplinePointPerStep * GetActorScale3D().Y;
	RailPointCount = FMath::FloorToInt(SplineLength / StepDistance);
    
	for (int i = 0; i <= RailPointCount; ++i)
	{
		int ActualIndex = i;
		int PreviousIndex = ActualIndex - 1;
		int NextIndex = ActualIndex + 1;

		FVector PreviousDirection = ComputePreviousDirection(PreviousIndex, ActualIndex);
		FVector NextDirection = ComputeNextDirection(NextIndex, ActualIndex);

		FVector Position;
		float DistanceOnSpline = i * StepDistance;
		Position = Spline->GetLocationAtDistanceAlongSpline(DistanceOnSpline, ESplineCoordinateSpace::World);

		FVector UpVector = Spline->GetUpVectorAtDistanceAlongSpline(DistanceOnSpline, ESplineCoordinateSpace::World);
		Position += UpVector * OffsetY;
       
		FRailPoint* point = new FRailPoint(ActualIndex, DistanceOnSpline,
		   Position, PreviousDirection, NextDirection, FVector::ZeroVector, FVector::ZeroVector);
       
		if (point->IsExtremityPoint(RailPointCount))
		{
			point->ArriveTangent = Spline->GetSplinePointAt(ActualIndex, ESplineCoordinateSpace::World).ArriveTangent.GetClampedToSize(0.0f, 1.0f);
			point->LeaveTangent = Spline->GetSplinePointAt(ActualIndex, ESplineCoordinateSpace::World).LeaveTangent.GetClampedToSize(0.0f, 1.0f);
		}
       
		RailPoints.Add(*point);
	}
}

void AGrindRail::HandlePointsCollisions()
{
	APawn* NearestPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!NearestPawn) return;

	FVector PawnLoc = NearestPawn->GetActorLocation();

	FRailPoint* NearestPoint = FindNearestRailPoint(PawnLoc);
	if (!NearestPoint) return;
	
	TrackingCapsule->SetWorldLocation(NearestPoint->Position + FVector(0, 0, CollisionPointHeight * 0.5f));
	
	FVector ToPlayer = (PawnLoc - NearestPoint->Position).GetSafeNormal();
	FRotator Rot = ToPlayer.ToOrientationRotator();
	Rot.Pitch -= 90.0f;
	TrackingCapsule->SetWorldRotation(Rot);

	TArray<AActor*> OverlappingActors;
	TrackingCapsule->GetOverlappingActors(OverlappingActors, AJBRCharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		AJBRCharacter* Character = Cast<AJBRCharacter>(Actor);
		if (!Character) continue;
		if (Character->CustomCMC->IsGrinding()) continue;

		SnapPlayerToRail(Character, NearestPoint);
	}
}

void AGrindRail::OnCapsuleOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	AJBRCharacter* Character = Cast<AJBRCharacter>(OtherActor);
	if (!Character) return;
	if (Character->CustomCMC->IsGrinding()) return;

	FRailPoint* NearestPoint = FindNearestRailPoint(Character->GetActorLocation());
	if (!NearestPoint) return;
	
	SnapPlayerToRail(Character, NearestPoint);
}

FRailPoint* AGrindRail::FindNearestRailPoint(const FVector& WorldLocation)
{
	FRailPoint* Nearest = nullptr;
	float BestDistSq = FLT_MAX;

	for (FRailPoint& Point : RailPoints)
	{
		float DistSq = FVector::DistSquared(WorldLocation, Point.Position);
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Nearest = &Point;
		}
	}

	return Nearest;
}

#pragma endregion Collisions

#pragma region Start Grind

void AGrindRail::SnapPlayerToRail(AJBRCharacter* Character, FRailPoint* RailPoint)
{
	StartGrind(Character, RailPoint);
}

void AGrindRail::StartGrind(AJBRCharacter* Character, FRailPoint* RailPoint)
{
	UJBRCustomCMC* CMC = Cast<UJBRCustomCMC>(Character->GetCharacterMovement());
	if (!CMC) return;

	// No restart if already on this rail
	if (CMC->IsGrinding() && CMC->GrindState.CurrentRail == this) return;
	// No grind if last rail still in memory to avoid fas tsnap back when exiting
	if (CMC->GrindState.LastRail != nullptr) return;
    
	int32 Direction = ComputeGrindDirection(Character, RailPoint);

	// Swap control to CMC
	CMC->EnterGrind(this, CMC->Velocity.Size2D(), RailPoint->DistanceOnSpline, Direction);
	
	if (Character->RailSystemComponent)
	{
		Character->RailSystemComponent->IsOnRail = true; // Camera stuff only
	}

	if (Character->TricksComponent)
	{
		Character->TricksComponent->AttemptPossibleInteraction(this);
	}
}

#pragma endregion Start Grind

#pragma region Debug

void AGrindRail::ShowDebugPoints()
{
	int i = 0;
	for (FRailPoint RailPoint : RailPoints)
	{
		float Mult = ((i == 0) || i == RailPoints.Num() - 1) ? CollisionPointRadiusMultForFirstAndLast : 1.0f;
		float HalfHeight = CollisionPointHeight * 0.5f;
		
		UKismetSystemLibrary::DrawDebugCapsule(
			GetWorld(), 
			RailPoint.Position, 
			HalfHeight, 
			CollisionPointRadius * Mult, 
			FRotator::ZeroRotator,
			FLinearColor::Green
		);
		
		i++;
		
		UKismetSystemLibrary::DrawDebugLine(GetWorld(), RailPoint.Position,
			RailPoint.Position + RailPoint.NextDirection * 25.0f, FColor::Green, 0.0f, 5.0f);
		UKismetSystemLibrary::DrawDebugLine(GetWorld(), RailPoint.Position,
			RailPoint.Position + RailPoint.PreviousDirection * 25.0f, FColor::Red, 0.0f, 5.0f);
		
		UKismetSystemLibrary::DrawDebugLine(GetWorld(), RailPoint.Position,RailPoint.Position +
			RailPoint.ArriveTangent * 50.0f, FColor::Green, 0.0f, 5.0f);
		
		UKismetSystemLibrary::DrawDebugLine(GetWorld(), RailPoint.Position,RailPoint.Position -
			RailPoint.LeaveTangent * 50.0f, FColor::Red, 0.0f, 5.0f);
	}
}

#pragma endregion Debug

#pragma region Distance

float AGrindRail::GetDistanceAtIndex(int Index, FVector& WorldPosition)
{
	float SplineLength = Spline->GetSplineLength();
	float RatioDistance = SplineLength / static_cast<float>(RailPointCount);
	float Distance = Index * RatioDistance;

	WorldPosition = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	return Distance;
}

float AGrindRail::GetDistanceRatioAtIndex(int Index)
{
	float SplineLength = Spline->GetSplineLength();
	float RatioDistance = SplineLength / static_cast<float>(RailPointCount);
	float Distance = (Index * RatioDistance) / SplineLength;
	return Distance;
}

#pragma endregion Distance

#pragma region Directions

FVector AGrindRail::ComputePreviousDirection(int PreviousIndex, int ActualIndex)
{
	if (PreviousIndex >= 0)
	{
		FVector WorldPosActualIndex;
		FVector WorldPosPreviousIndex;

		GetDistanceAtIndex(ActualIndex, WorldPosActualIndex);
		GetDistanceAtIndex(PreviousIndex, WorldPosPreviousIndex);
			
		return (WorldPosPreviousIndex - WorldPosActualIndex).GetClampedToSize(0.0f, 1.0f);
	}
	
	FVector ArriveTangent = Spline->GetSplinePointAt(0, ESplineCoordinateSpace::World).LeaveTangent;
	return ArriveTangent.GetClampedToSize(0.0f, 1.0f);
}

FVector AGrindRail::ComputeNextDirection(int NextIndex, int ActualIndex)
{
	if (NextIndex <= RailPointCount)
	{
		FVector WorldPosActualIndex;
		FVector WorldPosNextIndex;

		GetDistanceAtIndex(ActualIndex, WorldPosActualIndex);
		GetDistanceAtIndex(NextIndex, WorldPosNextIndex);
			
		return (WorldPosNextIndex - WorldPosActualIndex).GetClampedToSize(0.0f, 1.0f);;
	}
	
	FVector ArriveTangent = Spline->GetSplinePointAt(RailPointCount, ESplineCoordinateSpace::World).ArriveTangent;
	return ArriveTangent.GetClampedToSize(0.0f, 1.0f);;
}

int AGrindRail::ComputeGrindDirection(AJBRCharacter* Character, FRailPoint* RailPoint)
{
	FVector ForwardVector = Character->GetActorForwardVector();
	ForwardVector.Z = 0.0f;
	ForwardVector.Normalize();

	FVector RailDir = RailPoint->NextDirection;
	RailDir.Z = 0.0f;
	RailDir.Normalize();

	float Dot = FVector::DotProduct(ForwardVector, RailDir);
	int Direction = (Dot >= 0.0f) ? 1 : -1;

	return Direction;
}

#pragma endregion Directions

void AGrindRail::TriggerContextAction(AJBRCharacter* Initiator)
{
	FPlayerJunkData JunKData = Initiator->GetPlayerJunkData();
	FPlayerSpeedData SpeedData = Initiator->GetPlayerSpeedData();
	
	Initiator->MaterialBarComponent->AddJunk(JunKData.ActionRailJunk);
	//Initiator->CustomCMC->AddSpeedBoost(SpeedData.ActionRailBoost, true, false);
}
