#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Framework/JBRCharacter.h"
#include "KillFeedManager.generated.h"

class AJBRCharacter;

USTRUCT(BlueprintType)
struct FKillEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FString KillerName;
	UPROPERTY(BlueprintReadOnly) FString VictimName;
	UPROPERTY(BlueprintReadOnly) float Timestamp;
	UPROPERTY(BlueprintReadOnly) bool OneShot;
};

USTRUCT(BlueprintType)
struct FKillEntryLocal
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly) bool LocalPlayerKiller;
	UPROPERTY(BlueprintReadOnly) bool LocalPlayerVictim;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKillRegistered, const FKillEntry&, Entry, const FKillEntryLocal&, LocalEntry);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UKillFeedManager : public UActorComponent
{
	GENERATED_BODY()

	UKillFeedManager();

public:
	
	UPROPERTY(BlueprintAssignable)
	FOnKillRegistered OnKillRegistered;
	UPROPERTY(BlueprintAssignable)
	FOnKillRegistered OnKillLocal;

	UPROPERTY(BlueprintReadOnly, Replicated)
	TArray<FKillEntry> KillHistory;
	UPROPERTY(BlueprintReadOnly)
	TArray<FKillEntryLocal> LocalKillHistory;

	void RegisterKill(AJBRCharacter* Killer, AJBRCharacter* Victim, float Damages);

private:
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_NotifyKill(const FKillEntry& Entry, AJBRCharacter* Killer, AJBRCharacter* Victim);

	UFUNCTION(Client, Unreliable)
	void ClientRpc_NotifyKill(const FKillEntry& Entry, const FKillEntryLocal& EntryLocal, AJBRCharacter* Killer, AJBRCharacter* Victim);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
