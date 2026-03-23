
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "JBRPlayerState.generated.h"

class AJBRCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPointsAdded, int, Amount);

UCLASS()
class JUNKBOARDRUSH_API AJBRPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

	AJBRPlayerState();

	virtual void BeginPlay() override;

public:

	UPROPERTY(BlueprintreadWrite)
	AJBRCharacter* Character;
	
#pragma region Trackers
	UPROPERTY(Replicated)
	FString ScoreTrajectory;
	
	UPROPERTY(Replicated)
	float TimeHoldingRelic;

	UPROPERTY(Replicated)
	int RelicStolenCount = 0;
	
	UPROPERTY(Replicated)
	int DeathCount = 0;
	
	UPROPERTY(Replicated)
	int ExtractionCount = 0;
	
	UPROPERTY(Replicated)
	int KillCount = 0;
	
	UPROPERTY(Replicated)
	int GadgetsAcquiredCount = 0;
#pragma endregion Trackers

#pragma region Identity
	UFUNCTION(BlueprintPure, Category = "Player State")
	FString GetSteamPlayerName() const { return SteamPlayerName; } // Getter
	
	void SetSteamPlayerName(const FString& NewName); // Called on server
	
	UFUNCTION()
	void OnRep_SteamPlayerName();

#pragma endregion Identity

#pragma region Game Points
	
	UPROPERTY(Replicated)
	int Points = 0;

	void AddPoints(int PointsToAdd); // No need to make this a server rpc since it's the gamemode calling it, and the gamemode only exists on the server

	UPROPERTY(Blueprintassignable)
	FOnPointsAdded OnPointsAdded;

#pragma endregion Game Points
	
#pragma region GAS
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	class UJBRAttributeSet* AttributeSet;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Item Box")
	FGameplayAbilitySpecHandle StoredItemHandle;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
#pragma endregion
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Identity
	
	UPROPERTY(ReplicatedUsing = OnRep_SteamPlayerName)
	FString SteamPlayerName;

#pragma endregion Identity
};
