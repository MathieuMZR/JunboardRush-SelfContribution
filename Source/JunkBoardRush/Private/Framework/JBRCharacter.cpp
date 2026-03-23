#include "Framework/JBRCharacter.h"
#include "Framework/JBRCustomCMC.h"

#include "Libraries/GenericFunction.h"
#include "Player/Camera/DynamicPlayerCamera.h"

#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Game/Extraction/Relic.h"
#include "Framework/JBRGameState.h"
#include "Framework/JBRPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Player/Hoverboard/Hoverboard.h"
#include "Kismet/GameplayStatics.h"
#include "Game/Rails/RailSystemComponent.h"
#include "Player/Camera/PlayerCameraShakeComponent.h"
#include "Sound/SoundManager.h"
#include "Visuals/Animations/TricksAnimationComponent.h"

AJBRCharacter::AJBRCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UJBRCustomCMC>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	
	CustomCMC = Cast<UJBRCustomCMC>(GetCharacterMovement());

	DynamicPlayerCamera = CreateDefaultSubobject<UDynamicPlayerCamera>(TEXT("DynamicPlayerCamera"));
	CameraShakeComponent = CreateDefaultSubobject<UPlayerCameraShakeComponent>(TEXT("CameraShakeComponent"));
	
	RailSystemComponent = CreateDefaultSubobject<URailSystemComponent>(TEXT("RailSystemComponent"));
	TricksComponent = CreateDefaultSubobject<UTricksComponent>(TEXT("TricksComponent"));
	TricksAnimationComponent = CreateDefaultSubobject<UTricksAnimationComponent>(TEXT("TricksAnimationComponent"));
	
	HoverboardChildActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("HoverboardChildActor"));
	HoverboardChildActorComponent->SetupAttachment(GetCapsuleComponent());
	HoverboardChildActorComponent->SetChildActorClass(AHoverboard::StaticClass());
	PlayerHoverboard = Cast<AHoverboard>(HoverboardChildActorComponent->GetChildActor());
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetCapsuleComponent());
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	MaterialBarComponent = CreateDefaultSubobject<UMaterialBarComponent>(TEXT("MaterialBarComponent"));
	ScrappingComponent = CreateDefaultSubobject<UScrappingComponent>(TEXT("ScrappingComponent"));
	CraftingComponent = CreateDefaultSubobject<UCraftingComponent>(TEXT("CraftingComponent"));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	DashComponent = CreateDefaultSubobject<UDashComponent>(TEXT("DashComponent"));
	
	RelicStealingSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RelicStealingSphere"));
	RelicAbsoluteStealingSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RelicAbsoluteStealingSphere"));
	RelicStealingSphere->SetupAttachment(GetCapsuleComponent());
	RelicAbsoluteStealingSphere->SetupAttachment(GetCapsuleComponent());
}

#pragma region Init

void AJBRCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	OnPlayerPossessed.Broadcast();

	AJBRPlayerState* PS = GetPlayerState<AJBRPlayerState>();
	if (PS)
	{
		// Set the ASC on the Server. Clients do this in OnRep_PlayerState()
		AbilitySystemComponent = PS->GetAbilitySystemComponent();
		// AI won't have PlayerControllers so we can init again here just to be sure. No harm in initing twice for players that have PlayerControllers
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);
	}
}

void AJBRCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AJBRPlayerState* PS = GetPlayerState<AJBRPlayerState>();
	if (PS)
	{
		// Set the ASC for clients. Server does this in PossessedBy
		AbilitySystemComponent = PS->GetAbilitySystemComponent();

		// Init ASC Actor Info for clients. Server will init its ASC when it possesses a new Actor
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
	}
}

void AJBRCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerState = Cast<AJBRPlayerState>(GetPlayerState());

	RelicStealingSphere->SetSphereRadius(GetPlayerCMCData().ManualRelicStealRadius);
	RelicAbsoluteStealingSphere->SetSphereRadius(GetPlayerCMCData().AbsoluteRelicStealRadius);

	if (TricksComponent) TricksComponent->OnTrickAllowsRelicSteal.AddDynamic(this, &AJBRCharacter::TryStealFromTricks);

	RelicStealingSphere->OnComponentBeginOverlap.AddDynamic(this, &AJBRCharacter::OnRelicStealingStartOverlap);
	RelicStealingSphere->OnComponentEndOverlap.AddDynamic(this, &AJBRCharacter::OnRelicStealingEndOverlap);
	RelicAbsoluteStealingSphere->OnComponentBeginOverlap.AddDynamic(this, &AJBRCharacter::OnRelicAbsoluteStealingOverlap);
}

#pragma endregion Init

void AJBRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PlayerHoverboard)
		PlayerHoverboard = Cast<AHoverboard>(HoverboardChildActorComponent->GetChildActor());

	if (HasAuthority() || IsLocallyControlled())
	{
		ReplicatedCameraLocation = Camera->GetComponentLocation();
		ReplicatedCameraRotation = Camera->GetComponentRotation();
	}
}

void AJBRCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Avoid small or micro land calls
	if (CustomCMC->LastFallingZVelocity > -100) return;
	
	OnLanded.Broadcast();
	
	CameraShakeComponent->StartCameraShake(CameraShakeEnum::Landing, CustomCMC->LastFallingZVelocity);
	
	Cast<AJBRGameState>(GetWorld()->GetGameState())->SoundManager->PlaySoundFromNameID(FName("PlayerLand"));
}

#pragma region Boards

void AJBRCharacter::ChangeBoard(AHoverboard* Hoverboard)
{
	PlayerHoverboard = Hoverboard;
}

#pragma endregion Boards

#pragma region Relic

void AJBRCharacter::OnRelicStealingStartOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult & SweepResult)
{
	if (!HasAuthority()) return;

	AJBRCharacter* C = Cast<AJBRCharacter>(OtherActor);
	if (C && C->HasRelic())
	{
		bIsInRelicStealRange = true;
		RefreshRelicStealState();

		TricksComponent->AttemptPossibleInteraction(this);
	}
}

void AJBRCharacter::OnRelicStealingEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;
	if (!Cast<AJBRCharacter>(OtherActor)) return; // Don't execute if the overlap end is not a character
	
	if (HasRelic()) return; // Don't execute if you are the holder
	
	// Else, execute anyway because in all other scenarios, you won't be allowed to steal the relic anymore when leaving a zone
	bIsInRelicStealRange = false;
	RefreshRelicStealState();
}

void AJBRCharacter::RefreshRelicStealState()
{
	if (!HasAuthority()) return;

	AJBRGameState* GS = GetWorld()->GetGameState<AJBRGameState>();
	if (!GS || !TricksComponent) return;

	AJBRCharacter* Victim = GS->RelicHolderGlobalReference;

	const bool bCanSteal =
		bIsInRelicStealRange &&
		Victim &&
		Victim != this &&
		GS->RelicGlobalReference &&
		(GetWorld()->TimeSeconds - GS->RelicGlobalReference->LastTransferTime) >= Victim->GetPlayerCMCData().StealDelay;

	TricksComponent->bCanStealRelic = bCanSteal;

	ReTriggerStealColliders();
}

void AJBRCharacter::HandleDeath()
{
	if (!HasAuthority())
		return;
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Player Died"));

	if (HasRelic())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Has relic"));
		AJBRGameState* GS = GetWorld()->GetGameState<AJBRGameState>();
		if (GS && GS->RelicGlobalReference)
		{
			GS->RelicGlobalReference->OnDrop(this);
		}
	}
}


void AJBRCharacter::OnRelicAbsoluteStealingOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	AJBRGameState* GS = GetWorld()->GetGameState<AJBRGameState>();
	if (!GS || !GS->RelicGlobalReference) return;

	AJBRCharacter* OverlappedCharacter = Cast<AJBRCharacter>(OtherActor);
	if (!OverlappedCharacter) return;

	if (!OverlappedCharacter->HasRelic()) return;

	GS->RelicGlobalReference->ServerRPC_TryAcquire(this);

	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, TEXT("OnRelicAbsoluteStealingOverlap"));
}


void AJBRCharacter::TryStealFromTricks()
{
	if (!HasAuthority())
	{
		ServerRPC_TryStealFromTricks();
		return;
	}
	
	if (!TricksComponent->bCanStealRelic)
	{
		return;
	}

	AJBRGameState* GS = GetWorld()->GetGameState<AJBRGameState>();
	if (!GS || !GS->RelicGlobalReference) return;

	AJBRCharacter* Victim = GS->RelicHolderGlobalReference;
	if (!Victim || Victim == this) return;
	
	const bool bWasShielded = GS->RelicGlobalReference->bIsMagneticShieldActive;

	UGameplayStatics::ApplyDamage(Victim, 1.0f, GetController(), this, UDamageType_PVP::StaticClass()); 

	if (!bWasShielded)
	{
		GS->RelicGlobalReference->TryAcquire(this);
	}
}

void AJBRCharacter::ServerRPC_TryStealFromTricks_Implementation()
{
	TryStealFromTricks();
}

// Server Only
void AJBRCharacter::ReTriggerStealColliders()
{
	AJBRGameState* GS = GetWorld()->GetGameState<AJBRGameState>();
	if (!GS || !TricksComponent) return;
	
	bIsInRelicStealRange = RelicStealingSphere->IsOverlappingActor(GS->RelicHolderGlobalReference);
}

#pragma endregion Relic

#pragma region Speed Ratios

float AJBRCharacter::GetSpeedRatioFromVelocity() const
{
	float MaxSpeed = CustomCMC->GetMaxSpeed();
	float CurrentSpeed = CustomCMC->Velocity.Size2D();
	float Ratio = CurrentSpeed / MaxSpeed;

	return FMath::Clamp(Ratio, 0.0f, 1.0f);
}

float AJBRCharacter::GetSpeedRatioFromAbsoluteVelocity() const
{
	float MaxSpeed = CustomCMC->GetAbsoluteMaxSpeed();
	float CurrentSpeed = CustomCMC->Velocity.Size2D();
	float Ratio = CurrentSpeed / MaxSpeed;

	return FMath::Clamp(Ratio, 0.f, 1.f);
}

float AJBRCharacter::GetSpeedRatioBaseSpeedFromAbsoluteVelocity() const
{
	const float MaxSpeed = CustomCMC->GetAbsoluteMaxSpeed();
	const float MinSpeed = GetPlayerSpeedData().BaseMaxWalkSpeed;
	const float CurrentSpeed = CustomCMC->Velocity.Size2D();

	const float ClampedSpeed = FMath::Clamp(CurrentSpeed, MinSpeed, MaxSpeed);

	const float Ratio = (ClampedSpeed - MinSpeed) / (MaxSpeed - MinSpeed);

	return Ratio;
}

float AJBRCharacter::GetFallingSpeedRatio() const
{
	float MaxSpeedZ = GetPlayerSpeedData().MaxFallingSpeed;
	float LastSpeedZ = CustomCMC->LastFallingZVelocity;
	float Ratio = FMath::Abs(LastSpeedZ) / FMath::Abs(MaxSpeedZ);

	return FMath::Clamp(Ratio, 0.f, 1.f);
}

#pragma endregion Speed Ratios

#pragma region Data

FString AJBRCharacter::GetPlayerNameFromPlayerState()
{
	FString DisplayName = *GetOwner()->GetName();
	if (APawn* PawnOwner = Cast<APawn>(GetOwner()))
	{
		if (AJBRPlayerState* PS = Cast<AJBRPlayerState>(PawnOwner->GetPlayerState()))
		{
			DisplayName = PS->GetSteamPlayerName();
		}
	}

	return DisplayName;
}

UAbilitySystemComponent* AJBRCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

const FPlayerCMCData& AJBRCharacter::GetPlayerCMCData() const
{
	return UGenericFunction::GetDataRow<FPlayerCMCData>(PlayerCMCData, TEXT("PlayerCMCData"));
}

const FPlayerSpeedData& AJBRCharacter::GetPlayerSpeedData() const
{
	return UGenericFunction::GetDataRow<FPlayerSpeedData>(PlayerSpeedData, TEXT("PlayerSpeedData"));
}

const FPlayerJunkData& AJBRCharacter::GetPlayerJunkData() const
{
	return UGenericFunction::GetDataRow<FPlayerJunkData>(PlayerJunkData, TEXT("PlayerJunkData"));
}

#pragma endregion Data

#pragma region Utilities

bool AJBRCharacter::HasRelic() const
{
	return Cast<AJBRGameState>(GetWorld()->GetGameState())->RelicHolderGlobalReference == this;
}

int32 AJBRCharacter::GetPlayerRank()
{
	const AJBRGameState* GS = GetWorld()->GetGameState<AJBRGameState>();
	
	if (!GS) return 1;

	const FString PlayerName = GetPlayerNameFromPlayerState();

	for (int32 i = 0; i < GS->FullLeaderboard.Num(); ++i)
	{
		if (GS->FullLeaderboard[i].PlayerName == PlayerName)
		{
			return i + 1;
		}
	}

	// Default case (should not happen)
	return 1;
}

void AJBRCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AJBRCharacter, bIsInRelicStealRange);
	
	DOREPLIFETIME(AJBRCharacter, bForceStopAutoMoveState);
	DOREPLIFETIME(AJBRCharacter, bForceDisableDrift);

	DOREPLIFETIME(AJBRCharacter, ReplicatedCameraLocation);
	DOREPLIFETIME(AJBRCharacter, ReplicatedCameraRotation);
}

#pragma endregion Utilities

void AJBRCharacter::TriggerContextAction(AJBRCharacter* Initiator)
{
	if (TricksComponent->bCanStealRelic)
	{
		TricksComponent->OnTrickAllowsRelicSteal.Broadcast();
	}
}
