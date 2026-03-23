#include "Framework/JBRPlayerState.h"
#include "Engine.h"
#include "GAS/JBRAttributeSet.h"
#include "Framework/JBRCharacter.h"
#include "Framework/JBRGameState.h"
#include "Net/UnrealNetwork.h"

AJBRPlayerState::AJBRPlayerState()
{
	PrimaryActorTick.bCanEverTick = true;
	// Ensure tick only happens on server for game logic
	SetReplicates(true);
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UJBRAttributeSet>(TEXT("AttributeSet"));
	
	SetNetUpdateFrequency(100.0f);
	
}

UAbilitySystemComponent* AJBRPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AJBRPlayerState::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<AJBRCharacter>(GetPawn());
}

#pragma region Identity

void AJBRPlayerState::SetSteamPlayerName(const FString& NewName)
{
	// This function should only be called on the server
	if (HasAuthority())
	{
		SteamPlayerName = NewName;
		// Name change so need to manually call the RepNotify for the local server player
		// Clients will have their OnRep_SteamPlayerName called automatically by the replication system
		OnRep_SteamPlayerName();
	}
}

void AJBRPlayerState::OnRep_SteamPlayerName()
{
	Character = Cast<AJBRCharacter>(GetPawn());
    
	if (Character)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Player name updated to: %s"), *SteamPlayerName));
		Character->OnPlayerNameChanged.Broadcast();

		AJBRGameState* GameState = Cast<AJBRGameState>(UGameplayStatics::GetGameState(GetWorld()));
		if (GameState)
		{
			GameState->RequestUpdateLeaderboard();
		}
	}
	else
	{
		// Pawn not ready yet, retry on next tick
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Pawn not ready, retrying name update for: %s"), *SteamPlayerName));
        
		FTimerHandle LocalHandle;
		GetWorldTimerManager().SetTimer(
			LocalHandle,
			FTimerDelegate::CreateUObject(this, &AJBRPlayerState::OnRep_SteamPlayerName),
			0.25f,
			false
		);
	}
}

#pragma endregion Identity

#pragma region Game Points

void AJBRPlayerState::AddPoints(int PointsToAdd)
{
	// Ensure this only runs on server
	if (!HasAuthority())
	{
		return;
	}

	Points += PointsToAdd;
	OnPointsAdded.Broadcast(PointsToAdd);
	
	AJBRGameState* GS = GetWorld()->GetGameState<AJBRGameState>();
	if (GS)
	{
		GS->RequestUpdateLeaderboard();
	}
}

#pragma endregion Game Points

void AJBRPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AJBRPlayerState, SteamPlayerName);
	DOREPLIFETIME(AJBRPlayerState, Points);
	DOREPLIFETIME(AJBRPlayerState, StoredItemHandle);
}