#include "Visuals/PostProcessManager.h"

#include "EngineUtils.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"

#include "Game/Extraction/ExtractionGate.h"
#include "Framework/JBRCharacter.h"
#include "Framework/JBRGameState.h"
#include "Player/Hoverboard/Hoverboard.h"

#pragma region Init

UPostProcessManager::UPostProcessManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPostProcessManager::BeginPlay()
{
	Super::BeginPlay();

	GS = GetWorld()->GetGameState<AJBRGameState>();

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UPostProcessManager::GetExtractionGates);
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UPostProcessManager::TrySetUpPlayer);
}

void UPostProcessManager::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DetermineSelfPlayerVisibility();
	DetermineGatesVisibility();

	if (GS)
	{
		DetermineRelicVisibility(GS->RelicHolderGlobalReference);
	}
}

void UPostProcessManager::TrySetUpPlayer()
{
	if (bHasInitPlayer) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) 
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UPostProcessManager::TrySetUpPlayer);
		return;
	}

	APawn* Pawn = PC->GetPawn();
	if (!Pawn)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UPostProcessManager::TrySetUpPlayer);
		return;
	}

	LocalCharacter = Cast<AJBRCharacter>(Pawn);
	if (!LocalCharacter)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UPostProcessManager::TrySetUpPlayer);
		return;
	}

	if (LocalCharacter->CraftingComponent)
	{
		auto* CC = LocalCharacter->CraftingComponent;

		CC->OnRelicCaptured.AddUniqueDynamic(this, &UPostProcessManager::DetermineGatesVisibility);
		CC->OnRelicStolen.AddUniqueDynamic(this, &UPostProcessManager::DetermineGatesVisibility);
		CC->OnRelicScored.AddUniqueDynamic(this, &UPostProcessManager::DetermineGatesVisibility);
		CC->OnRelicLost.AddUniqueDynamic(this, &UPostProcessManager::DetermineGatesVisibility);
	}

	if (GS)
	{
		GS->OnRelicHolderChanged.AddUniqueDynamic(
			this, &UPostProcessManager::DetermineGatesVisibilityFromRelicHolder
		);

		GS->OnRelicHolderChanged.AddUniqueDynamic(
			this, &UPostProcessManager::DetermineRelicVisibility
		);
	}

	bHasInitPlayer = true;
}

#pragma endregion Init

#pragma region Gates

void UPostProcessManager::GetExtractionGates()
{
	Gates.Empty();

	for (TActorIterator<AExtractionGate> It(GetWorld()); It; ++It)
	{
		Gates.Add(*It);
	}

	// Ensure correct visibility once gates exist
	DetermineGatesVisibility();
}

void UPostProcessManager::DetermineGatesVisibility()
{
	if (!GS || !LocalCharacter) return;

	for (AExtractionGate* Gate : Gates)
	{
		if (!Gate) continue;

		const int32 TargetStencil = /*LocalCharacter->HasRelic() && */ Gate->bIsGateActive ? GateStencilID : 0;

		if (Gate->StencilID != TargetStencil)
		{
			Gate->UpdateStencil(TargetStencil);
		}
	}
}

void UPostProcessManager::DetermineGatesVisibilityFromRelicHolder(AJBRCharacter* RelicHolder)
{
	if (!LocalCharacter) return;

	const bool bIsRelicHolder = (LocalCharacter == RelicHolder);

	for (AExtractionGate* Gate : Gates)
	{
		if (!Gate) continue;

		const int32 TargetStencil = bIsRelicHolder && Gate->bIsGateActive ? GateStencilID : 0;
		
		if (Gate->StencilID != TargetStencil)
		{
			Gate->UpdateStencil(TargetStencil);
		}
	}
}

#pragma endregion Gates

#pragma region Player

void UPostProcessManager::DetermineSelfPlayerVisibility()
{
	if (!LocalCharacter) return;

	LocalCharacter->GetMesh()->SetCustomDepthStencilValue(4);

	if (LocalCharacter->PlayerHoverboard && LocalCharacter->PlayerHoverboard->BoardMesh)
	{
		LocalCharacter->PlayerHoverboard->BoardMesh->SetCustomDepthStencilValue(4);
	}
}

#pragma endregion Player

#pragma region Relic

void UPostProcessManager::DetermineRelicVisibility(AJBRCharacter* RelicHolder)
{
	if (!GS || !GS->RelicGlobalReference || !LocalCharacter) return;

	int32 TargetStencil = (RelicHolder == LocalCharacter) ? 0 : RelicStencilID;
	if (GS->RelicGlobalReference->CurrentState == ERelicState::WaitingForRespawn) TargetStencil = 0;

	GS->RelicGlobalReference->UpdateStencil(TargetStencil);
}

#pragma endregion Relic
