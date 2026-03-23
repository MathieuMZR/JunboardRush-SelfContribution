
#pragma once

#include "CoreMinimal.h"
#include "Game/Announcer/AnnouncerComponent.h"
#include "Game/Extraction/Relic.h"
#include "GameFramework/GameStateBase.h"
#include "JBRGameState.generated.h"


class AGateManager;
class UKillFeedManager;
class UPostProcessManager;
class USoundManager;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchTimerUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchTimerEnded);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRelicHolderChanged, AJBRCharacter*, NewHolder);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeaderboardUpdated);

USTRUCT(BlueprintType)
struct FLeaderboardEntry
{
	GENERATED_BODY()
    
	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;
    
	UPROPERTY(BlueprintReadOnly)
	int32 Points;

	UPROPERTY(BlueprintReadOnly)
	bool IsFirst;

	UPROPERTY(BlueprintReadOnly)
	int32 PlayerID;

	FLeaderboardEntry()
		: PlayerName(TEXT("")), Points(0), IsFirst(false), PlayerID(0)
	{}
    
	FLeaderboardEntry(const FString& InName, int32 InPoints, bool InIsFirst, int32 InPlayerID)
		: PlayerName(InName), Points(InPoints), IsFirst(InIsFirst), PlayerID(InPlayerID)
	{}
    
	bool operator>(const FLeaderboardEntry& Other) const
	{
		return Points > Other.Points;
	}
};

UCLASS()
class JUNKBOARDRUSH_API AJBRGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AJBRGameState();

	virtual void BeginPlay() override;

#pragma region Match
	
	UPROPERTY(ReplicatedUsing=OnRep_MatchTimer, BlueprintReadOnly)
	float Timer;
    
	UPROPERTY(ReplicatedUsing=OnRep_MatchStarted, BlueprintReadOnly)
	bool bMatchStarted;
	
	UPROPERTY(BlueprintAssignable)
	FOnMatchTimerUpdated OnMatchTimerUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnMatchTimerEnded OnMatchTimerEnded;

	UPROPERTY(ReplicatedUsing=OnRep_TopThreeLeaderboard, BlueprintReadWrite)
	TArray<FLeaderboardEntry> TopThreePlayers;
	
	UPROPERTY(ReplicatedUsing=OnRep_FullLeaderboard, BlueprintReadWrite)
	TArray<FLeaderboardEntry> FullLeaderboard;
	
	UPROPERTY(BlueprintAssignable)
	FOnLeaderboardUpdated OnLeaderboardUpdated;
	
	UFUNCTION(BlueprintPure, BlueprintCallable)
	FLeaderboardEntry GetSelfLeaderboardEntry(int PlayerStateID);
	
	UFUNCTION()
	void UpdateLeaderboard();

	UFUNCTION(BlueprintCallable)
	void RequestUpdateLeaderboard();
	
	UFUNCTION(Server, BlueprintCallable, Reliable)
	void RPC_UpdateLeaderboard();

#pragma endregion Match
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnnouncerComponent* AnnouncerComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundManager* SoundManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UPostProcessManager* PostProcessManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UKillFeedManager* KillFeedManager;

#pragma region Relic

	UPROPERTY(ReplicatedUsing=OnRep_RelicHolder, BlueprintReadOnly)
	AJBRCharacter* RelicHolderGlobalReference;
	
	void UpdateRelicHolder(AJBRCharacter* Holder, AJBRCharacter* PrevHolder);
	
	UFUNCTION()
	void OnRep_RelicHolder();

	UPROPERTY(Replicated, BlueprintReadOnly)
	ARelic* RelicGlobalReference;
	
	UPROPERTY(BlueprintAssignable)
	FOnRelicHolderChanged OnRelicHolderChanged;
	
	UFUNCTION(BlueprintPure)
	bool IsRelicClaimed() const;

#pragma endregion Relic
	
	int32 GetPlayerPlacementInLeaderboard(const FString& PlayerName) const;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_MatchTimer();
	UFUNCTION()
	void OnRep_MatchStarted();
	UFUNCTION()
	void OnRep_TopThreeLeaderboard();
	UFUNCTION()
	void OnRep_FullLeaderboard();
};
