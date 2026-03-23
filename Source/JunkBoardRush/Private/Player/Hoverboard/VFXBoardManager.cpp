
#include "Player/Hoverboard/VFXBoardManager.h"

#include "NiagaraComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Framework/JBRCharacter.h"
#include "Framework/JBRCustomCMC.h"

UVFXBoardManager::UVFXBoardManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVFXBoardManager::Initialize(AJBRCharacter* Player, USkeletalMeshComponent* BoardMesh)
{
	AActor* HoverboardOwner = GetOwner();
	if (!HoverboardOwner) return;
	
	JBRCharacter = Player;
	CustomCMC = JBRCharacter->CustomCMC;
	
	CompDriftVFX = TryCreateVFXComponent(LowDriftVFX, DriftSocketName, BoardMesh, false);
	CompDriftTransitionVFX = TryCreateVFXComponent(TransitionLowToMidDriftVFX, DriftSocketName, BoardMesh, false);
	CompTrailVFX = TryCreateVFXComponent(LowTrailVFX, DriftSocketName, BoardMesh, true);

	TryCreateLightComponents(BoardMesh);

	InitializeSubscriptions();
}

void UVFXBoardManager::InitializeSubscriptions()
{
	CustomCMC->OnDriftEnter.AddDynamic(this, &UVFXBoardManager::OnDriftEnter);
	CustomCMC->OnDriftRelease.AddDynamic(this, &UVFXBoardManager::OnDriftRelease);
	CustomCMC->OnDriftReleasePoor.AddDynamic(this, &UVFXBoardManager::OnDriftReleasePoor);
	CustomCMC->OnDriftCancel.AddDynamic(this, &UVFXBoardManager::OnDriftRelease);
	CustomCMC->OnSmallDriftEnter.AddDynamic(this, &UVFXBoardManager::OnSmallDrift);
	CustomCMC->OnBigDriftEnter.AddDynamic(this, &UVFXBoardManager::OnBigDrift);
	
	CustomCMC->OnSpeedBoostAdded.AddDynamic(this, &UVFXBoardManager::OnSpeedBoostAdded);
}

#pragma region VFX Events

#pragma region Drift

void UVFXBoardManager::OnDriftEnter()
{
	if (!CompDriftVFX || !LowDriftVFX) return;

	CompDriftVFX->SetAsset(LowDriftVFX);
	CompDriftVFX->Activate();
}

void UVFXBoardManager::OnDriftRelease()
{
	if (!CompDriftVFX) return;
	
	CompDriftVFX->DeactivateImmediate();
	LightColorBehavior(LowColorLight);
}

void UVFXBoardManager::OnDriftReleasePoor()
{
	if (!CompDriftVFX) return;
	
	CompDriftVFX->DeactivateImmediate();
	LightColorBehavior(LowColorLight);
}

void UVFXBoardManager::OnSmallDrift()
{
	if (!CompDriftVFX || !CompDriftTransitionVFX || !MidDriftVFX || !TransitionLowToMidDriftVFX) return;

	CompDriftVFX->SetAsset(MidDriftVFX);
	CompDriftVFX->Activate();

	CompDriftTransitionVFX->SetAsset(TransitionLowToMidDriftVFX);
	CompDriftTransitionVFX->ResetSystem();

	LightColorBehavior(MidColorLight);
}

void UVFXBoardManager::OnBigDrift()
{
	if (!CompDriftVFX || !CompDriftTransitionVFX || !HighDriftVFX || !TransitionMidToHighDriftVFX) return;

	CompDriftVFX->SetAsset(HighDriftVFX);
	CompDriftVFX->Activate();

	CompDriftTransitionVFX->SetAsset(TransitionMidToHighDriftVFX);
	CompDriftTransitionVFX->ResetSystem();

	LightColorBehavior(HighColorLight);
}

#pragma endregion Drift

#pragma region Trail

void UVFXBoardManager::OnSpeedBoostAdded(float Amount)
{
	if (!CompTrailVFX || !LowTrailVFX || !MidTrailVFX || !HighTrailVFX) return;
	
	FPlayerSpeedData Data = JBRCharacter->GetPlayerSpeedData();
	if (Data.SpeedBoostThreshold.Num() < 3) return;

	GetWorld()->GetTimerManager().ClearTimer(TrailResetTimer);

	if(Amount < Data.SpeedBoostThreshold[1])
	{
		CompTrailVFX->SetAsset(LowTrailVFX);
		LightColorBehavior(LowColorLight);
		return;
	}
	
	CompDriftTransitionVFX->ResetSystem();
	
	int Index = 0;
	if (Amount < Data.SpeedBoostThreshold[2])
	{
		CompTrailVFX->SetAsset(MidTrailVFX);
		CompDriftTransitionVFX->SetAsset(TransitionLowToMidDriftVFX);
		LightColorBehavior(MidColorLight);
		Index = 1;
	}
	if (Amount >= Data.SpeedBoostThreshold[2])
	{
		CompTrailVFX->SetAsset(HighTrailVFX);
		CompDriftTransitionVFX->SetAsset(TransitionMidToHighDriftVFX);
		LightColorBehavior(HighColorLight);
		Index = 2;
	}

	float Duration = TrailResetDuration * Index;

	GetWorld()->GetTimerManager().SetTimer(
		TrailResetTimer,
		this,
		&UVFXBoardManager::ResetTrail,
		Duration,
		false
	);
}

void UVFXBoardManager::ResetTrail()
{
	if (!CompTrailVFX || !LowTrailVFX) return;
	
	CompTrailVFX->SetAsset(LowTrailVFX);
	LightColorBehavior(LowColorLight);
}

#pragma endregion Trail

#pragma region Light

void UVFXBoardManager::LightColorBehavior(FColor Color)
{
	if (!BoardLight || !BoardBoostLight) return;
	
	BoardLight->SetLightColor(Color);
	BoardBoostLight->SetLightColor(Color);
}

#pragma endregion Light

#pragma endregion VFX Events

#pragma region VFX Creation

UNiagaraComponent* UVFXBoardManager::TryCreateVFXComponent(UNiagaraSystem* DefaultAsset, FName SocketName,
                                                           USkeletalMeshComponent* TargetMesh, bool InitialActivation) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !DefaultAsset) return nullptr;
	if (!TargetMesh) return nullptr;

	UNiagaraComponent* Comp = NewObject<UNiagaraComponent>(Owner);
	Comp->SetAsset(DefaultAsset);
	Comp->SetAutoActivate(false);
	Comp->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
	Comp->RegisterComponent();

	if (InitialActivation) Comp->Activate();

	return Comp;
}

void UVFXBoardManager::TryCreateLightComponents(USkeletalMeshComponent* TargetMesh)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;
	
	BoardLight = NewObject<URectLightComponent>(Owner);
	BoardLight->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LightRectSocketName);
	BoardLight->RegisterComponent();
	BoardLight->SetIntensity(1000.f);
	BoardLight->SetIntensityUnits(ELightUnits::Unitless);
	BoardLight->SetAttenuationRadius(2000.f);
	BoardLight->SetSourceWidth(1.f);
	BoardLight->SetSourceHeight(136.f);
	BoardLight->SetLightColor(LowColorLight);

	BoardBoostLight = NewObject<UPointLightComponent>(Owner);
	BoardBoostLight->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, DriftSocketName);
	BoardBoostLight->SetLightColor(LowColorLight);
}

#pragma endregion VFX Creation
