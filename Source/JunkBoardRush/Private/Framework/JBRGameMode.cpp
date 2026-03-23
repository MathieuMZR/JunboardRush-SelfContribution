
#include "Framework/JBRGameMode.h"

#include "Framework/JBRPlayerState.h"
#include "OnlineSubsystem.h"
#include "Game/Extraction/Relic.h"
#include "Game/Extraction/GateManager.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "TrackersPlaytest/MatchAnalytics.h"
#include "TrackersPlaytest/TelemetryDataVisualizer.h"

#pragma region Init

AJBRGameMode::AJBRGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AJBRGameMode::BeginPlay()
{
	Super::BeginPlay();
	GS = GetGameState<AJBRGameState>();
	RespawnPoint = FindRespawnPoint();
	SpawnRelic();
	
	AGateManager* GateManager = Cast<AGateManager>(
	   UGameplayStatics::GetActorOfClass(GetWorld(), AGateManager::StaticClass())
	);
	if (GateManager && Relic) GateManager->Init(Relic);

	StartTimer();
	
	
	if (GIsEditor)
	{
		return;
	}
	
	CurrentMatchID = FGuid::NewGuid().ToString();
	
	MatchTelemetryData.Reserve(5000); 
	GetWorldTimerManager().SetTimer(
		TelemetryTimerHandle, 
		this, 
		&AJBRGameMode::RecordPlayerPositions, 
		PositionTickRate, 
		true
	);
}

#pragma region TrackersLog
void AJBRGameMode::RecordPlayerPositions()
{
	if (GIsEditor)
	{
		return;
	}
	
	// Loop through all connected players
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC && PC->GetPawn() && PC->PlayerState)
		{
			FTelemetryData NewData;
            
			NewData.PlayerID = Cast<AJBRPlayerState>(PC->PlayerState)->GetSteamPlayerName();
			NewData.EventType = ETelemetryEvent::PositionTick;
			NewData.Location = PC->GetPawn()->GetActorLocation();
            
			NewData.Speed = PC->GetPawn()->GetVelocity().Size(); 
			NewData.Timestamp = GetWorld()->GetTimeSeconds();

			MatchTelemetryData.Add(NewData);
		}
	}
}

void AJBRGameMode::LogDiscreteEvent(APlayerController* Player, ETelemetryEvent EventType, FVector Location)
{
	if (GIsEditor)
	{
		return;
	}
	
	if (!Player || !Player->PlayerState) return;

	FTelemetryData EventData;
	EventData.PlayerID = Player->PlayerState->GetPlayerName();
	EventData.EventType = EventType;
	EventData.Location = Location;
    
	if (Player->GetPawn())
	{
		EventData.Speed = Player->GetPawn()->GetVelocity().Size();
	}
	else
	{
		EventData.Speed = 0.0f;
	}
    
	EventData.Timestamp = GetWorld()->GetTimeSeconds();

	MatchTelemetryData.Add(EventData);
}
#pragma endregion TrackersLog

void AJBRGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer) return;

	APlayerState* PlayerState = NewPlayer->PlayerState;
	if (!PlayerState) return;
    
	// Player's unique net ID
	FUniqueNetIdRepl UniqueNetIdRepl = PlayerState->GetUniqueId();

	if (UniqueNetIdRepl.IsValid())
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (Subsystem)
		{
			// Identity Interface = handles user accounts
			IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
			if (Identity.IsValid())
			{
				// Get the player's nickname from Steam
				FString SteamName = Identity->GetPlayerNickname(*UniqueNetIdRepl.GetUniqueNetId());
                
				if (AJBRPlayerState* PS = Cast<AJBRPlayerState>(PlayerState))
				{
					PS->SetSteamPlayerName(SteamName);
				}

				OnPostLogin.Broadcast();
			}
		}
	}
}

void AJBRGameMode::Tick(float DeltaTime)
{
	HandleTimer(DeltaTime);
}

#pragma endregion Init

#pragma region Relic

void AJBRGameMode::ExtractRelic(int PointAdded, AJBRPlayerState* ScoringPlayer, AExtractionGate* ScoringGate, bool bIsFinalExtraction)
{
	ScoringPlayer->AddPoints(PointAdded);

	if (bIsFinalExtraction)
	{
		StartRelicRespawnTimer();
		//ResetAllGates();
	}
}

void AJBRGameMode::SpawnRelic()
{
	if (RespawnPoint == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RespawnPoint is null"));
		return;
	}

	FActorSpawnParameters SpawnInfo;
	Relic = GetWorld()->SpawnActor<ARelic>(RelicClass, RespawnPoint->GetActorLocation(), RespawnPoint->GetActorRotation(), SpawnInfo);
	GS->RelicGlobalReference = Relic;

	ARelic* RelicObj = GetRelic();
	ARelicCircuit* NearestCircuit = FindNearestCircuit();

	if (NearestCircuit)
	{
		RelicObj->SetInitialCircuit(NearestCircuit);
		RelicObj->ChangeStates(ERelicState::Moving);
	}

	RelicObj->ResetAllCharges();
	RelicObj->bIsShielded = true;
	RelicObj->OnRep_IsShielded();
	GetWorld()->GetTimerManager().SetTimer(RelicObj->TimerHandle_ShieldExpire, RelicObj, &ARelic::OnShieldExpired, RelicObj->GetRelicData().RelicRespawnShieldDuration, false);
}

void AJBRGameMode::RespawnRelic()
{
	GetRelic()->SetActorEnableCollision(true);
	GetRelic()->PhysicSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	
	GetRelic()->SetActorLocation(GetRespawnPoint()->GetActorLocation());
	//GetRelic()->TriggerAreaRelic->SetCollisionResponseToAllChannels(ECR_Overlap);
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Relic Respawned"));
	
	RelicRespawnedAnnouncement();

	ARelic* RelicObj = GetRelic();
	ARelicCircuit* NearestCircuit = FindNearestCircuit();

	if (NearestCircuit)
	{
		RelicObj->SetInitialCircuit(NearestCircuit);
		RelicObj->ChangeStates(ERelicState::Moving);
	}

	RelicObj->ResetAllCharges();
	RelicObj->bIsShielded = true;
	RelicObj->OnRep_IsShielded();
	GetWorld()->GetTimerManager().SetTimer(RelicObj->TimerHandle_ShieldExpire, RelicObj, &ARelic::OnShieldExpired, RelicObj->GetRelicData().RelicRespawnShieldDuration, false);

	/*// Reset all gates
	for (AExtractionGate* Gate : ExtractionGates)
	{
		Gate->bIsGateActive = false;
		Gate->ChangeGateColor(FVector(1.f, 1.f, 1.f)); // Change to white for inactive gate
	}

	// Choose random gate to enable
	if (ExtractionGates.Num() > 0)
	{
		int RandomIndex = FMath::RandRange(0, ExtractionGates.Num() - 1);
		ExtractionGates[RandomIndex]->bIsGateActive = true;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Gate %d is now active!"), RandomIndex));
	}*/
}

ARelicCircuit* AJBRGameMode::FindNearestCircuit()
{
	TArray<AActor*> AllCircuits;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARelicCircuit::StaticClass(), AllCircuits);

	ARelicCircuit* NearestCircuit = nullptr;
	float MinDistSq = FLT_MAX;

	for (AActor* Actor : AllCircuits)
	{
		float DistSq = FVector::DistSquared(Actor->GetActorLocation(), GetRespawnPoint()->GetActorLocation());
		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
			NearestCircuit = Cast<ARelicCircuit>(Actor);
		}
	}

	return NearestCircuit;
}

AActor* AJBRGameMode::GetRespawnPoint() const
{
	return RespawnPoint;
}

ARelic* AJBRGameMode::GetRelic() const
{
	if (RespawnPoint == nullptr)
	{
		return nullptr;
	}
	return Relic;
}

#pragma endregion Relic

#pragma region Match

void AJBRGameMode::StartTimer()
{
	if (GS)
	{
		GS->bMatchStarted = true;
		GS->Timer = MatchDuration;
		GS->OnMatchTimerUpdated.Broadcast();
		// Test
	}
}

void AJBRGameMode::StartRelicRespawnTimer()
{
	GetRelic()->StartRespawnRelic();
	
	RelicRespawnSoonAnnouncement();
	
	FTimerHandle TimerRelicHandle;
	GetWorldTimerManager().SetTimer(TimerRelicHandle, this, &AJBRGameMode::RespawnRelic, TimeBeforeRelicRespawn, false);
}

void AJBRGameMode::HandleTimer(float DeltaTime)
{
	if (GS && GS->bMatchStarted && GS->Timer > 0.f)
	{
		GS->Timer -= DeltaTime;
		GS->OnMatchTimerUpdated.Broadcast();
	}

	if (!bOneMinuteAnnounced && GS->Timer <= 60.f)
	{
		bOneMinuteAnnounced = true;
		OneMinuteRemainsAnnouncement();
	}

	if (!bThirtySecondsAnnounced && GS->Timer <= 30.f)
	{
		bThirtySecondsAnnounced = true;
		ThirtySecondsRemainAnnouncement();
	}

	if (GS->Timer <= 0.f && !bIsGameFinished)
	{
		bIsGameFinished = true;
		GS->OnMatchTimerEnded.Broadcast();

		SendMatchData();
	}
}

void AJBRGameMode::CheckAssignScoreTrajectory()
{
	float ElapsedTime = MatchDuration - GS->Timer;

	// 2 min
	if (!bScoreRecorded2Min && ElapsedTime >= 120.0f)
	{
		RecordScoreTrajectory();
		bScoreRecorded2Min = true;
	}

	// 4 min
	if (!bScoreRecorded4Min && ElapsedTime >= 240.0f)
	{
		RecordScoreTrajectory();
		bScoreRecorded4Min = true;
	}

	// 6 min
	if (!bScoreRecorded6Min && ElapsedTime >= 360.0f)
	{
		RecordScoreTrajectory();
		bScoreRecorded6Min = true;
	}
}

void AJBRGameMode::RecordScoreTrajectory()
{
	if (!GS) return;

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (AJBRPlayerState* JBRPS = Cast<AJBRPlayerState>(PS))
		{
			FString ScoreStr = FString::FromInt(JBRPS->Points);

			if (JBRPS->ScoreTrajectory.IsEmpty())
			{
				JBRPS->ScoreTrajectory = ScoreStr;
			}
			else
			{
				JBRPS->ScoreTrajectory += TEXT(",") + ScoreStr;
			}
		}
	}
    
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Recorded Score Trajectory for all players."));
}

void AJBRGameMode::SendMatchData()
{
	TArray<AActor*> FoundAnalytics;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMatchAnalytics::StaticClass(), FoundAnalytics);
	if (FoundAnalytics.Num() > 0)		
	{
		AMatchAnalytics* AnalyticsActor = Cast<AMatchAnalytics>(FoundAnalytics[0]);
		if (AnalyticsActor)
		{
			TArray<FPlayerMatchStats> PlayerStats;
			for (APlayerState* PS : GS->PlayerArray)
			{
				AJBRPlayerState* JBRPS = Cast<AJBRPlayerState>(PS);
				if (JBRPS)
				{
					PlayerStats.Add(FPlayerMatchStats{
						JBRPS->GetSteamPlayerName(),
						GS->GetPlayerPlacementInLeaderboard(JBRPS->GetSteamPlayerName()),
						JBRPS->Points,
						JBRPS->ScoreTrajectory,
						JBRPS->TimeHoldingRelic,
						JBRPS->RelicStolenCount,
						JBRPS->DeathCount,
						JBRPS->ExtractionCount,
						JBRPS->KillCount,
						JBRPS->GadgetsAcquiredCount
					});
				}
			}
		
			AnalyticsActor->SendMatchStatsToGoogle(FString::Printf(TEXT("Match_%s"), *CurrentMatchID), MatchDuration, PlayerStats);
		}
	}
	
	ATelemetryDataVisualizer* TelemetryVisualizer = Cast<ATelemetryDataVisualizer>(UGameplayStatics::GetActorOfClass(GetWorld(), ATelemetryDataVisualizer::StaticClass()));
	if (TelemetryVisualizer)			
	{
		TelemetryVisualizer->UploadMatchDataToSupabase(FString::Printf(TEXT("Match_%s"), *CurrentMatchID), MatchTelemetryData);
	}
}

AActor* AJBRGameMode::FindRespawnPoint()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("RespawnPoint"), FoundActors);
	if (FoundActors.Num() > 0)
	{
		return FoundActors[0];
	}
	else if (FoundActors.Num() == 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No RespawnPoint found! Spawning a new one at origin. You need to add an actor wit the tag RespawnPoint to the map."));
		FActorSpawnParameters SpawnInfo;
		AActor* NewRespawnPoint = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
		NewRespawnPoint->Tags.Add(FName("RespawnPoint"));
		return NewRespawnPoint;
	}
	return nullptr;
}

#pragma endregion Match
