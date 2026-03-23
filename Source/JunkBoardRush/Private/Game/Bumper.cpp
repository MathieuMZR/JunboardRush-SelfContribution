#include "Game/Bumper.h"

#include "Components/SphereComponent.h"
#include "Framework/JBRCharacter.h"
#include "Framework/JBRCustomCMC.h"

ABumper::ABumper()
{
	PrimaryActorTick.bCanEverTick = true;

	TriggerZone = CreateDefaultSubobject<USphereComponent>("TriggerZone");
}

void ABumper::BeginPlay()
{
	Super::BeginPlay();

	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &ABumper::OnOverlapBegin);
	TriggerZone->OnComponentEndOverlap.AddDynamic(this, &ABumper::OnOverlapEnd);
}

void ABumper::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	InZoneCharacter = Cast<AJBRCharacter>(OtherActor);
	if (!InZoneCharacter) return;

	InZoneCharacter->TricksComponent->AttemptPossibleInteraction(this);

	FVector JumpDir = GetPropulsionVector();
	Server_ApplyBumper(InZoneCharacter, JumpDir);
}

void ABumper::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (OtherActor != InZoneCharacter) return;

	InZoneCharacter = nullptr;
}

FVector ABumper::GetPropulsionVector()
{
	FVector Velocity = InZoneCharacter->CustomCMC->Velocity;
	FVector ProjectedVelocity = FVector(Velocity.X, Velocity.Y, BumperForce);
	return ProjectedVelocity;
}

void ABumper::SetPlayerRotationWallJump()
{
	FVector JumpDir = GetPropulsionVector();
	JumpDir.Z = 0.f;

	if (!JumpDir.IsNearlyZero())
	{
		FRotator DesiredRot(0.f, -JumpDir.Rotation().Yaw, 0.f);
		InZoneCharacter->CustomCMC->MoveUpdatedComponent(FVector::ZeroVector, DesiredRot, false);
	}
}

void ABumper::TriggerContextAction(AJBRCharacter* Initiator)
{
	FPlayerJunkData JunKData = Initiator->GetPlayerJunkData();
	
	Initiator->MaterialBarComponent->AddJunk(JunKData.ActionBumperJunk);
}

void ABumper::Server_ApplyBumper_Implementation(AJBRCharacter* Target, FVector PropulsionVector)
{
	if (!Target) return;

	Target->CustomCMC->SetMovementMode(MOVE_Falling);
	InZoneCharacter->LaunchCharacter(
	FVector(0, 0, PropulsionVector.Z),
	false,
	true
);
}
