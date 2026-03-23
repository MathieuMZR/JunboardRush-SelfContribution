#include "Game/Extraction/Relic.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/DecalComponent.h"
#include "Framework/JBRGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Libraries/GenericFunction.h"
#include "UI/WaypointWidget.h"

class UWaypointWidget;

ARelic::ARelic()
{
	PrimaryActorTick.bCanEverTick = true;

	PhysicSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PhysicSphere"));
	RootComponent = PhysicSphere;

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(PhysicSphere);
	PickupSphere->SetSphereRadius(120.f);

	PickupSphere->OnComponentBeginOverlap.AddDynamic(this,&ARelic::OnPickupOverlap);

	RelicMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RelicMesh"));
	RelicMesh->SetupAttachment(PhysicSphere);

	RelicDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("RelicDecal"));
	RelicDecal->SetupAttachment(PhysicSphere);

	bReplicates = true;
	SetReplicateMovement(true);
	
	CurrentState = ERelicState::Moving;
}

// Called when the game starts or when spawned
void ARelic::BeginPlay()
{
	Super::BeginPlay();

	GS = GetWorld()->GetGameState<AJBRGameState>();
	
	WaypointWidget = CreateWidget<UWaypointWidget>(GetWorld()->GetFirstPlayerController(), WaypointWidgetClass);
	if (WaypointWidget)
	{
		WaypointWidget->TargetActor = this;
		WaypointWidget->AddToPlayerScreen(100);
		//WaypointWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ARelic::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (HasAuthority() && CurrentState == ERelicState::Moving)
	{
		UpdateMovementOnCircuit(DeltaTime);
	}
}

#pragma region UI

void ARelic::ShowWaypoint()
{
	if (WaypointWidget == nullptr) return;

	WaypointWidget->SetVisibility(ESlateVisibility::Visible);
}

void ARelic::HideWaypoint()
{
	if (WaypointWidget == nullptr) return;
	
	WaypointWidget->SetVisibility(ESlateVisibility::Hidden);
}

void ARelic::SetWaypointVisibility()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	AJBRCharacter* LocalChar = Cast<AJBRCharacter>(PC->GetPawn());
	
	if (!GS)
	{
		GS = GetWorld() ? GetWorld()->GetGameState<AJBRGameState>() : nullptr;
	}

	const bool bIsHolder = (LocalChar && LocalChar == GS->RelicHolderGlobalReference);

	if (CurrentState == ERelicState::WaitingForRespawn)
	{
		HideWaypoint();
	}
	else if (CurrentState == ERelicState::Carried)
	{
		if (bIsHolder) HideWaypoint();
		else ShowWaypoint();
	}
	else
	{
		ShowWaypoint();
	}
}

#pragma endregion UI

#pragma region Circuit

void ARelic::UpdateMovementOnCircuit(float DeltaTime)
{
	if (!CurrentCircuit || !CurrentCircuit->SplineComponent)
	{
		return;
	}

	// Calculate new distance
	float SplineLength = CurrentCircuit->SplineComponent->GetSplineLength();
	CurrentDistanceOnSpline += 	GetRelicData().RelicSpeed * DeltaTime;

	// Check if we reached the end of the current spline
	if (CurrentDistanceOnSpline >= SplineLength)
	{
		// If overshot, calculate how much excess distance
		float ExcessDistance = CurrentDistanceOnSpline - SplineLength;

		// Branching
		const TArray<ARelicCircuit*>& Branches = CurrentCircuit->NextCircuits;
		
		if (Branches.Num() > 0)
		{
			// Randomly choose the next branch
			int32 RandomIndex = FMath::RandRange(0, Branches.Num() - 1);
			ARelicCircuit* NextCircuit = Branches[RandomIndex];

			if (NextCircuit)
			{
				CurrentCircuit = NextCircuit;
				
				// Apply excess distance to the new spline so movement remains smooth
				CurrentDistanceOnSpline = ExcessDistance; 
			}
			else
			{
				// Loop back on current if connection is null
				CurrentDistanceOnSpline = 0.0f; 
			}
		}
		else
		{
			// No branches = loop back
			CurrentDistanceOnSpline = 0.0f;
		}
	}

	FVector NewLocation = CurrentCircuit->SplineComponent->GetLocationAtDistanceAlongSpline(CurrentDistanceOnSpline, ESplineCoordinateSpace::World);
	FRotator NewRotation = CurrentCircuit->SplineComponent->GetRotationAtDistanceAlongSpline(CurrentDistanceOnSpline, ESplineCoordinateSpace::World);

	SetActorLocationAndRotation(NewLocation, NewRotation);
}

void ARelic::SetInitialCircuit(ARelicCircuit* StartCircuit)
{
	if (StartCircuit)
	{
		CurrentCircuit = StartCircuit;
		CurrentDistanceOnSpline = 0.0f;
		
		FVector StartLoc = CurrentCircuit->SplineComponent->GetLocationAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
		SetActorLocation(StartLoc);
	}
}

#pragma endregion Circuit

#pragma region Pickup N Steal

void ARelic::TryAcquire(AJBRCharacter* Requester)
{
	if (!HasAuthority() || !Requester || !GS) return;

	AJBRCharacter* CurrentHolder = GS->RelicHolderGlobalReference;

	// Free pickup already handled by overlap
	if (!CurrentHolder) return;

	if (Requester == CurrentHolder) return;
	if (bIsShielded) return;
	
	if (bIsMagneticShieldActive) return;
	
	if (GetWorld()->TimeSeconds - LastTransferTime < Requester->GetPlayerCMCData().StealDelay) return;

	AcquireRelic(Requester, CurrentHolder, true);
}

void ARelic::ServerRPC_TryAcquire_Implementation(AJBRCharacter* Requester)
{
	TryAcquire(Requester);
}

// Allow player detection when not hold
void ARelic::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult & SweepResult)
{
	if (!HasAuthority()) return;
	if (!GS) return;
	
	if (bIsShielded) return;
	if (CurrentState == ERelicState::Carried) return;

	// Already owned → no free pickup
	if (GS->RelicHolderGlobalReference != nullptr) return;

	AJBRCharacter* Character = Cast<AJBRCharacter>(OtherActor);
	if (!Character) return;
	if (Character->HealthComponent->bIsDead) return;

	AcquireRelic(Character, nullptr, false);
}

// Server - only
void ARelic::AcquireRelic(AJBRCharacter* NewOwner, AJBRCharacter* PreviousOwner, bool bSteal)
{
	if (!NewOwner || !NewOwner->CraftingComponent) return;

	if (bSteal && PreviousOwner)
	{
		PreviousOwner->CraftingComponent->LostRelic();
	}

	FGadgetItem RelicItem;
	RelicItem.GadgetClass = ARelic::StaticClass();
	RelicItem.SlotIcon = SlotIcon;
	RelicItem.ActiveGadget = this;
	NewOwner->CraftingComponent->TryCaptureOrStealRelic(
		RelicItem,
		PreviousOwner
	);
	
	ResetAllCharges();

	Multicast_PlayRelicCatchVFX(NewOwner);

	GS->UpdateRelicHolder(NewOwner, PreviousOwner);
	MulticastRPC_NotifyRelicOwnershipChanged(NewOwner, PreviousOwner);

	ChangeStates(ERelicState::Carried);
	AttachToComponent(NewOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ThrowSocketName);
	RelicMesh->SetRelativeScale3D(FVector::OneVector * GetRelicData().OverrideScaleInHand);
	
	ActivateMagneticShield();

	LastTransferTime = GetWorld()->GetTimeSeconds();
}

void ARelic::MulticastRPC_NotifyRelicOwnershipChanged_Implementation(AJBRCharacter* NewOwner, AJBRCharacter* PreviousOwner)
{
	OnRelicOwnershipChanged.Broadcast(NewOwner, PreviousOwner);
}

#pragma endregion Pickup N Steal

#pragma region Drop

void ARelic::OnDrop(AJBRCharacter* Dropper)
{
	if (!HasAuthority()) return;

	GS->UpdateRelicHolder(nullptr, Dropper);
	if(Dropper) Dropper->CraftingComponent->LostRelic();

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	RelicMesh->SetRelativeScale3D(FVector::OneVector);
	
	ChangeStates(ERelicState::Dropped);
	
	ResetAllCharges();
	BreakMagneticShield();
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Relic Dropped"));
}

void ARelic::EnableDropPhysic()
{
	PhysicSphere->SetSimulatePhysics(true);

	FVector RandomOffset;
	RandomOffset.X = FMath::RandRange(GetRelicData().MinMaxLateralExpulsionDirection.X, GetRelicData().MinMaxLateralExpulsionDirection.Y);
	RandomOffset.Y = FMath::RandRange(GetRelicData().MinMaxLateralExpulsionDirection.X, GetRelicData().MinMaxLateralExpulsionDirection.Y);
	RandomOffset.Z = 1;
	float RandomForce = FMath::RandRange(GetRelicData().MinMaxExpulsionForce.X, GetRelicData().MinMaxExpulsionForce.Y);
	
	PhysicSphere->AddImpulse(RandomOffset * RandomForce, NAME_None, true);
}

void ARelic::DisableDropPhysic()
{
	PhysicSphere->SetSimulatePhysics(false);
}

#pragma endregion Drop

#pragma region Charge N Shield

bool ARelic::AddCharge()
{
	if (!HasAuthority()) return false;

	CurrentRelicCharges++;
	OnRep_CurrentRelicCharges();

	if (CurrentRelicCharges > GetRelicData().NbChargesToExtract)
	{
		return true; // Extracted
	}
	
	if (!bIsMagneticShieldActive)
	{
		ActivateMagneticShield();
	}
	
	return false; // Just Charged
}

void ARelic::ActivateMagneticShield()
{
	if (HasAuthority())
	{
		bIsMagneticShieldActive = true;
		OnRep_IsMagneticShieldActive();
	}
}

void ARelic::BreakMagneticShield()
{
	if (HasAuthority())
	{
		bIsMagneticShieldActive = false;
		OnRep_IsMagneticShieldActive();
	}
}

void ARelic::OnRep_IsMagneticShieldActive()
{
	OnMagneticShieldChanged.Broadcast(bIsMagneticShieldActive);

	if (bIsMagneticShieldActive)
	{
		if (UNiagaraSystem* ShieldSystem = MagneticShieldVFX.LoadSynchronous())
		{
			if (MagneticShieldVFXComponent)
			{
				MagneticShieldVFXComponent->DestroyComponent();
			}
			
			MagneticShieldVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				ShieldSystem,
				RelicMesh,
				FName("Root"),
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				false
			);

			if (MagneticShieldVFXComponent)
			{
				MagneticShieldVFXComponent->SetFloatParameter(TEXT("Size"), GetRelicData().RelicDefensiveShieldSize);
				MagneticShieldVFXComponent->SetFloatParameter(TEXT("Duration"), GetRelicData().RelicDefensiveShieldDuration);
			}
		}
	}
	else
	{
		if (UNiagaraSystem* BreakVFX = MagneticShieldBreakVFX.LoadSynchronous())
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				BreakVFX,
				RelicMesh,
				FName("Root"),
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				true
			);
		}
		
		if (MagneticShieldVFXComponent)
		{
			MagneticShieldVFXComponent->DestroyComponent();
			MagneticShieldVFXComponent = nullptr;
		}
	}
}

void ARelic::ResetAllCharges()
{
	if (!HasAuthority()) return;
	
	CurrentRelicCharges = 0;
	OnRep_CurrentRelicCharges();
}

void ARelic::OnShieldExpired()
{
	bIsShielded = false;
	//Multicast_DisableRelicShieldVFX();
	OnRep_IsShielded();
}

void ARelic::OnRep_IsShielded()
{
	OnRelicShieldChanged.Broadcast(bIsShielded);
	if (bIsShielded)
	{
		EnableRelicShieldVFX();
	}
	else
	{
		DisableRelicShieldVFX();
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Relic Shielded State: %s"), bIsShielded ? TEXT("Enabled") : TEXT("Disabled")));
}

void ARelic::OnRep_CurrentRelicCharges()
{
	OnRelicChargeChanged.Broadcast(CurrentRelicCharges, GetRelicData().NbChargesToExtract);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Relic Charges: %d"), CurrentRelicCharges));
	SwitchRelicAuraVFX();
}

#pragma endregion Charge N Shield

#pragma region Respawn

void ARelic::StartRespawnRelic()
{
	GS->UpdateRelicHolder(nullptr, nullptr);

	SetActorEnableCollision(false);
	PhysicSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); // Hide relic away from player
	SetActorLocation(FVector(0.f, 0.f, -10000.f));

	ChangeStates(ERelicState::WaitingForRespawn);
}

#pragma endregion Respawn

#pragma region Visuals

void ARelic::UpdateStencil(int ID)
{
	StencilID = ID;
	RelicMesh->SetCustomDepthStencilValue(StencilID);
}

void ARelic::EnableRelicShieldVFX()
{
	if (ShieldVFXComponent != nullptr)
	{
		return;
	}
	
	if (UNiagaraSystem* ShieldVFX = RelicShieldVFX.LoadSynchronous())
	{
		ShieldVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			ShieldVFX,
			RelicMesh,
				FName("Root"),                   
			FVector::ZeroVector,         
			FRotator::ZeroRotator,       
			EAttachLocation::SnapToTarget, 
			false                         
		);
	}
}

void ARelic::DisableRelicShieldVFX()
{
	if (ShieldVFXComponent != nullptr)
	{
		ShieldVFXComponent->DestroyComponent();
		ShieldVFXComponent = nullptr;
	}
}

void ARelic::SwitchRelicAuraVFX()
{
	UNiagaraSystem* RelicAuraVFX = nullptr;
	
	if (RelicAuraVFXComponent != nullptr)
	{
		RelicAuraVFXComponent->DestroyComponent();
		RelicAuraVFXComponent = nullptr;
	}
	
	switch (CurrentRelicCharges)
	{
		case 1:
			RelicAuraVFX = RelicAuraVFX1.LoadSynchronous();
			break;
		case 2:
			RelicAuraVFX = RelicAuraVFX2.LoadSynchronous();
			break;
		case 3:
			RelicAuraVFX = RelicAuraVFX3.LoadSynchronous();
			break;
	}
	
	if (!GS)
	{
		GS = GetWorld() ? GetWorld()->GetGameState<AJBRGameState>() : nullptr;
	}
	
	if (!GS || !GS->RelicHolderGlobalReference) return;
	
	if (RelicAuraVFX)
	{
		RelicAuraVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			RelicAuraVFX,
			RelicMesh,
			FName("Root"),                
			FVector::ZeroVector,         
			FRotator::ZeroRotator,       
			EAttachLocation::SnapToTarget, 
			false                         
		);
		
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("Switched Relic Aura VFX for %d Charges"), CurrentRelicCharges));
	}
}

void ARelic::HandleDecalBehavior()
{
	if (CurrentState == ERelicState::Carried || CurrentState == ERelicState::WaitingForRespawn)
	{
		RelicDecal->SetHiddenInGame(true);
		return;
	}

	RelicDecal->SetHiddenInGame(false);
}

void ARelic::Multicast_PlayRelicCatchVFX_Implementation(AJBRCharacter* Catcher)
{
	const AJBRCharacter* Character = Cast<AJBRCharacter>(Catcher);
    
	if (!Character || !Character->CosmeticsConfig)
	{
		return;
	}
    
	if (UNiagaraSystem* CatchVFX = Character->CosmeticsConfig->RelicCatchVFX.LoadSynchronous())
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			CatchVFX,
			RelicMesh,
				FName("Root"),                   
			FVector::ZeroVector,         
			FRotator::ZeroRotator,       
			EAttachLocation::SnapToTarget, 
			true                         
		);
	}
}

#pragma endregion Visuals

#pragma region States

void ARelic::ChangeStates(ERelicState State)
{
	if (!HasAuthority()) return;
	
	CurrentState = State;
	OnRep_CurrentState();
}

void ARelic::OnRep_CurrentState()
{
	SetWaypointVisibility();

	if (CurrentState == ERelicState::Dropped)
	{
		EnableDropPhysic();
		RelicMesh->SetRelativeScale3D(FVector::OneVector);
		OnRelicDropped.Broadcast();
	}
	else if (CurrentState == ERelicState::Carried)
	{
		DisableDropPhysic();
		RelicMesh->SetRelativeScale3D(FVector::OneVector * GetRelicData().OverrideScaleInHand);
	}
	else
	{
		DisableDropPhysic();
		RelicMesh->SetRelativeScale3D(FVector::OneVector);
	}

	if (CurrentState == ERelicState::WaitingForRespawn)
		OnRelicStartRespawn.Broadcast();
	else if (CurrentState == ERelicState::Moving)
		OnRelicRespawn.Broadcast();

	HandleDecalBehavior();
}

#pragma endregion States

const FRelicData& ARelic::GetRelicData() const
{
	return UGenericFunction::GetDataRow<FRelicData>(RelicConfig, TEXT("RelicConfig"));
}

void ARelic::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(ARelic, CurrentState, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(ARelic, bIsShielded);
	DOREPLIFETIME(ARelic, CurrentRelicCharges);
}

