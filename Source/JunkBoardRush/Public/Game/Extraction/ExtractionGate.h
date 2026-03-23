
#pragma once

#include "CoreMinimal.h"
#include "Framework/JBRCharacter.h"
#include "GameFramework/Actor.h"

#include "ExtractionGate.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRelicExtracted, AJBRCharacter*, Extractor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRelicCharged, AJBRCharacter*, Carrier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGateStateChanged, bool, bActive);

UCLASS()
class JUNKBOARDRUSH_API AExtractionGate : public AActor
{
	GENERATED_BODY()
	
public:	
	AExtractionGate();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_GateColor)
	FVector GateColor;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* RelicGateEntrySound;
	
	UFUNCTION()
	void OnRep_GateColor();
	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:	
	UFUNCTION(BlueprintCallable)
	void ProcessGateEntry(AJBRCharacter* Character);
	
	UFUNCTION(BlueprintCallable)
	void ChangeGateColor(FVector NewColor);

	UPROPERTY(Replicated)
	bool bIsGateActive = false;
	
	UFUNCTION(BlueprintCallable)
	void SetGateActive(bool bActive);
	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxGateHealth = 100.f;

	UPROPERTY(ReplicatedUsing=OnRep_GateHealth)
	float CurrentGateHealth;

	UPROPERTY(EditAnywhere)
	float GatePropulsionForce = 2000.f;

	UPROPERTY(EditAnywhere)
	float GateDownDuration = 10.0f;

	UPROPERTY(EditAnywhere)
	int32 ChargeVictoryPoints = 1;

	UPROPERTY(EditAnywhere)
	int32 ExtractVictoryPoints = 10;


	UPROPERTY(BlueprintAssignable)
	FOnRelicExtracted OnRelicExtracted;
	
	UPROPERTY(BlueprintAssignable)
	FOnRelicCharged OnRelicCharged;

	UPROPERTY(BlueprintAssignable)
	FOnGateStateChanged OnGateStateChanged;
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayFinalExtractionVFX(FVector Location, AJBRCharacter* Extractor);

	// Stencil
	int StencilID;
	void UpdateStencil(int ID);
	
private:
	UFUNCTION()
	void OnRep_IsGateActive();

	UFUNCTION()
	void OnRep_GateHealth();

	void HandleGateDeactivation();
};
