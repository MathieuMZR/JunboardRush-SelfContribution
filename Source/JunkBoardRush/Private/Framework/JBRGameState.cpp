
#include "Framework/JBRGameState.h"
#include "Framework/JBRPlayerState.h"
#include "Game/KillFeedManager.h"
#include "Sound/SoundManager.h"
#include "Visuals/PostProcessManager.h"

AJBRGameState::AJBRGameState()
{
	AnnouncerComp = CreateDefaultSubobject<UAnnouncerComponent>(TEXT("AnnouncerComponent"));
	SoundManager = CreateDefaultSubobject<USoundManager>(TEXT("SoundManager"));
	PostProcessManager = CreateDefaultSubobject<UPostProcessManager>(TEXT("PostProcessManager"));
	KillFeedManager = CreateDefaultSubobject<UKillFeedManager>(TEXT("KillFeedManager"));
}

void AJBRGameState::BeginPlay()
{
	Super::BeginPlay();
}

void AJBRGameState::OnRep_MatchTimer()
{
	OnMatchTimerUpdated.Broadcast();
	if (Timer <= 0)
	{
		OnMatchTimerEnded.Broadcast();
	}
}

void AJBRGameState::OnRep_MatchStarted()
{
	// Could add stuff here if needed when match starts
}

void AJBRGameState::OnRep_TopThreeLeaderboard()
{
	OnLeaderboardUpdated.Broadcast();
}

void AJBRGameState::OnRep_FullLeaderboard()
{
	OnLeaderboardUpdated.Broadcast();
}

void AJBRGameState::RequestUpdateLeaderboard()
{
	if (HasAuthority())
	{
		UpdateLeaderboard();
	}
	else
	{
		RPC_UpdateLeaderboard();
	}
}

void AJBRGameState::UpdateLeaderboard()
{
	TArray<FLeaderboardEntry> AllPlayers;

	if (PlayerArray.Num() <= 0) return;
	
	// Gather all player states
	for (APlayerState* PS : PlayerArray)
	{
		AJBRPlayerState* JBRPS = Cast<AJBRPlayerState>(PS);
		if (JBRPS)
		{
			AllPlayers.Add(FLeaderboardEntry(
				JBRPS->GetSteamPlayerName(),
				JBRPS->Points,
				false,
				JBRPS->GetPlayerId()
			));
		}
	}
    
	// Sort by points descending
	AllPlayers.Sort([](const FLeaderboardEntry& A, const FLeaderboardEntry& B)
	{
		return A > B;
	});

	AllPlayers[0].IsFirst = true;
    
	TopThreePlayers.Empty();
	int32 Count = FMath::Min(3, AllPlayers.Num());
	for (int32 i = 0; i < Count; i++)
	{
		TopThreePlayers.Add(AllPlayers[i]);
	}
	
	// Update full leaderboard
	FullLeaderboard = AllPlayers;
    
	OnLeaderboardUpdated.Broadcast();
}

FLeaderboardEntry AJBRGameState::GetSelfLeaderboardEntry(int PlayerStateID)
{
	if (FullLeaderboard.Num() <= 0) return FLeaderboardEntry();
    
	FLeaderboardEntry* Found = FullLeaderboard.FindByPredicate([PlayerStateID](const FLeaderboardEntry& Entry)
	{
		return Entry.PlayerID == PlayerStateID;
	});

	return Found ? *Found : FLeaderboardEntry();
}

void AJBRGameState::RPC_UpdateLeaderboard_Implementation()
{
	UpdateLeaderboard();
}

// Server-only
void AJBRGameState::UpdateRelicHolder(AJBRCharacter* Holder, AJBRCharacter* PrevHolder)
{
	RelicHolderGlobalReference = Holder;

	OnRep_RelicHolder();

	if (Holder) Holder->RefreshRelicStealState();
	if (PrevHolder) PrevHolder->RefreshRelicStealState();
}

bool AJBRGameState::IsRelicClaimed() const
{
	return RelicHolderGlobalReference != nullptr;
}

int32 AJBRGameState::GetPlayerPlacementInLeaderboard(const FString& PlayerName) const
{
	if (FullLeaderboard.Num() <= 0) return -1;

	int32 Placement = 1;
	for (const FLeaderboardEntry& Entry : FullLeaderboard)
	{
		if (Entry.PlayerName == PlayerName)
		{
			return Placement;
		}
		Placement++;
	}

	return -1; // Not found
}

void AJBRGameState::OnRep_RelicHolder()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	if (RelicGlobalReference)
	{
		RelicGlobalReference->SetWaypointVisibility();
	}

	OnRelicHolderChanged.Broadcast(RelicHolderGlobalReference);
}

void AJBRGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AJBRGameState, Timer);
	DOREPLIFETIME(AJBRGameState, bMatchStarted);
	DOREPLIFETIME(AJBRGameState, TopThreePlayers);
	DOREPLIFETIME(AJBRGameState, FullLeaderboard);
	
	DOREPLIFETIME(AJBRGameState, RelicGlobalReference);
	DOREPLIFETIME(AJBRGameState, RelicHolderGlobalReference);
}