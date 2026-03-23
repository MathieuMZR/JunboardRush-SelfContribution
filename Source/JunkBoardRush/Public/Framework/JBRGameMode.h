// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Extraction/ExtractionGate.h"
#include "Framework/JBRGameState.h"
#include "GameFramework/GameModeBase.h"
#include "TrackersPlaytest/TelemetryDataVisualizer.h"

#include "JBRGameMode.generated.h"

class ABallRespawnPoint;
class ARelic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPostLogin);

UCLASS()
class JUNKBOARDRUSH_API AJBRGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AJBRGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintAssignable)
	FOnPostLogin OnPostLogin;

#pragma region Relic
	
	ARelic* Relic;
	
	UFUNCTION(BlueprintCallable)
	void ExtractRelic(int PointAdded, AJBRPlayerState* ScoringPlayer, AExtractionGate* ScoringGate, bool bIsFinalExtraction);
	UFUNCTION(BlueprintCallable)
	void SpawnRelic();
	UFUNCTION(BlueprintCallable)
	void RespawnRelic();
	
	ARelicCircuit* FindNearestCircuit();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Relic")
	TSubclassOf<AActor> RelicClass;

#pragma endregion Relic

#pragma region Match

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timer")
	bool bIsGameFinished;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timer")
	float MatchDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timer")
	float TimeBeforeRelicRespawn = 5.f;

#pragma endregion Match

// Later for custom matches with custom rules
#pragma region GadgetPool

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pools")
    UGadgetPoolData* RelicHolderGadgetPool;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pools")
    UGadgetPoolData* RelicPursuerGadgetPool;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pools")
    UGadgetPoolData* BasicGadgetPool;

#pragma endregion GadgetPool
	
#pragma region TrackersLog
	
	TArray<FTelemetryData> MatchTelemetryData;
	FString CurrentMatchID;

	FTimerHandle TelemetryTimerHandle;
	UPROPERTY(EditDefaultsOnly, Category = "Telemetry")
	float PositionTickRate = 1.0f; // Record position every 1 second

	void RecordPlayerPositions();
	
	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void LogDiscreteEvent(APlayerController* Player, ETelemetryEvent EventType, FVector Location);
	
#pragma endregion TrackersLog

protected:

	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	AActor* RespawnPoint;
	AActor* GetRespawnPoint() const;
	AActor* FindRespawnPoint();
	
	AJBRGameState* GS;

#pragma region Announcer
	
	bool bOneMinuteAnnounced = false;
	bool bThirtySecondsAnnounced = false;

	bool bScoreRecorded2Min = false;
	bool bScoreRecorded4Min = false;
	bool bScoreRecorded6Min = false;

	// ufunction attribute only for debug
	UFUNCTION(BlueprintCallable)
	void StartTimer();
	void HandleTimer(float DeltaTime);
	void CheckAssignScoreTrajectory();
	void RecordScoreTrajectory();
	void SendMatchData();
	
	ARelic* GetRelic() const;

	void StartRelicRespawnTimer();
	TArray<AExtractionGate*> GetAllExtractionGates();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Announcer")
	void RelicRespawnSoonAnnouncement();
	UFUNCTION(BlueprintImplementableEvent, Category = "Announcer")
	void RelicRespawnedAnnouncement();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Announcer")
	void OneMinuteRemainsAnnouncement();
	UFUNCTION(BlueprintImplementableEvent, Category = "Announcer")
	void ThirtySecondsRemainAnnouncement();
	
	UPROPERTY(EditDefaultsOnly)
	float ResetGateTimer = 10.0f; // Time to wait if all gates are down

	FTimerHandle TimerHandle_ResetGates;
};
