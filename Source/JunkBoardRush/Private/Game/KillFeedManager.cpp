
#include "Game/KillFeedManager.h"

#include "Framework/JBRController.h"
#include "Framework/JBRPlayerState.h"

UKillFeedManager::UKillFeedManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Server-only
void UKillFeedManager::RegisterKill(AJBRCharacter* Killer, AJBRCharacter* Victim, float Damages)
{
	FKillEntry Entry;

	if (!Victim) return;

	if (Killer)
		Entry.KillerName = Cast<AJBRPlayerState>(Killer->GetPlayerState())->GetSteamPlayerName();

	Entry.VictimName = Cast<AJBRPlayerState>(Victim->GetPlayerState())->GetSteamPlayerName();
	Entry.Timestamp = GetWorld()->GetTimeSeconds();
	Entry.OneShot = Damages >= Victim->HealthComponent->GetMaxHealth();
	
	KillHistory.Add(Entry);

	Multicast_NotifyKill(Entry, Killer, Victim);

	// Notify kill for the correct killer client
	if (Killer)
	{
		if (AJBRController* KillerPC = Cast<AJBRController>(Killer->GetController()))
		{
			KillerPC->Client_NotifyLocalKill(Entry);
		}
		
		Killer->GetPlayerState<AJBRPlayerState>()->KillCount++;
	}
}

void UKillFeedManager::Multicast_NotifyKill_Implementation(const FKillEntry& Entry, AJBRCharacter* Killer, AJBRCharacter* Victim)
{
	FKillEntryLocal EntryLocal;

	if (!Victim) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* LocalCharacter = PC->GetPawn();

	if (Killer)
		EntryLocal.LocalPlayerKiller = Killer == LocalCharacter;
	EntryLocal.LocalPlayerVictim = Victim == LocalCharacter;
	
	// Allow kill feed row creation
	OnKillRegistered.Broadcast(Entry, EntryLocal);
}

void UKillFeedManager::ClientRpc_NotifyKill_Implementation(const FKillEntry& Entry, const FKillEntryLocal& EntryLocal,
	AJBRCharacter* Killer, AJBRCharacter* Victim)
{
	// Only show kill banner if the player killed someone
	if (Killer && EntryLocal.LocalPlayerKiller)
		OnKillLocal.Broadcast(Entry, EntryLocal);
}

void UKillFeedManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UKillFeedManager, KillHistory);
}

