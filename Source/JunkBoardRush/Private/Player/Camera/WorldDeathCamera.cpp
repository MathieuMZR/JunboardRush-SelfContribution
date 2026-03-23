
#include "Player/Camera/WorldDeathCamera.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Framework/JBRCharacter.h"

AWorldDeathCamera::AWorldDeathCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(GetRootComponent());

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Capsule);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

void AWorldDeathCamera::BeginPlay()
{
	Super::BeginPlay();
}

void AWorldDeathCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FHitResult Hit;
	FVector Start = SpringArm->GetComponentLocation();
	FVector End = Camera->GetComponentLocation();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Camera, Params);
	if (bHit)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow,
			FString::Printf(TEXT("Actor: %s | Component: %s"), 
				*Hit.GetActor()->GetName(),
				*Hit.GetComponent()->GetName()));
	}

	if (FollowTarget == nullptr || (FollowTarget && FollowTarget->HealthComponent->bIsDead)) return;

	DesiredPosition = FollowTarget->ReplicatedCameraLocation;
	DesiredRotation = FollowTarget->ReplicatedCameraRotation;

	CurrentPosition = FMath::Lerp(CurrentPosition, DesiredPosition, DeltaTime * 10);
	CurrentRotation = FMath::Lerp(CurrentRotation, DesiredRotation, DeltaTime * 10);

	Camera->SetWorldLocation(CurrentPosition);
	Camera->SetWorldRotation(CurrentRotation);
}
