
#include "Framework/JBRController.h"

#include "Framework/JBRGameState.h"
#include "Game/KillFeedManager.h"
#include "Player/Camera/WorldDeathCamera.h"

void AJBRController::Client_OnJunkCollected_Implementation(AJunkPile* JunkPile, float RespawnTime)
{
	if (JunkPile)
	{
		JunkPile->HideAndScheduleRespawn(RespawnTime);
	}
}

void AJBRController::Client_NotifyLocalKill_Implementation(FKillEntry Entry)
{
	AJBRGameState* GS = Cast<AJBRGameState>(GetWorld()->GetGameState());
	if (!GS || !GS->KillFeedManager) return;
	
	FKillEntryLocal EntryLocal;
	EntryLocal.LocalPlayerKiller = true;
	EntryLocal.LocalPlayerVictim = false;

	GS->KillFeedManager->OnKillLocal.Broadcast(Entry, EntryLocal);
}

void AJBRController::EnterDeathCameraBehavior(AJBRCharacter* Killer)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	
	LastDeathCam = GetWorld()->SpawnActor<AWorldDeathCamera>(
	WorldDeathCameraClass,
	GetPawn()->GetActorLocation(),
	FRotator(0, GetPawn()->GetActorRotation().Yaw, 0),
	SpawnParams
	);

	if (LastDeathCam == nullptr) return;

	SetViewTargetWithBlend(LastDeathCam, 0, VTBlend_Linear, 0);

	if (Killer == nullptr) return;
	
	FTimerHandle BlendTimer;
	GetWorld()->GetTimerManager().SetTimer(
		BlendTimer,
		[this, Killer]()
		{
			if (LastDeathCam && Killer)
			{
				LastDeathCam->FollowTarget = Killer;
				SetViewTargetWithBlend(LastDeathCam, 0, VTBlend_Linear, 0);
			}
		},
		2.f,
		false
	);
}

void AJBRController::Server_DeathCameraBehavior_Implementation(AJBRCharacter* Killer)
{
	EnterDeathCameraBehavior(Killer);
}

void AJBRController::ExitDeathCameraBehavior()
{
	SetViewTargetWithBlend(GetPawn(), 0, VTBlend_Linear, 0);

	if (LastDeathCam)
	{
		LastDeathCam->Destroy();
		LastDeathCam = nullptr;
	}
}
