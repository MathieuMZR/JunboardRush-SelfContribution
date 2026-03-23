// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicCircuit.h"
#include "RelicData.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Framework/JBRCharacter.h"
#include "Relic.generated.h"

class UWaypointWidget;
class AJBRGameState;

UENUM(BlueprintType)
enum class ERelicState : uint8
{
	Moving              UMETA(DisplayName = "Moving on Circuit"),
	Carried             UMETA(DisplayName = "Carried by Player"),
	Dropped             UMETA(DisplayName = "Dropped on Ground"),
	WaitingForRespawn   UMETA(DisplayName = "Waiting for Respawn")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRelicOwnershipChanged, AJBRCharacter*, NewOwner, AJBRCharacter*, PreviousOwner);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRelicChargeChanged, int32, NewCharge, int32, MaxCharges);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRelicShieldChanged, bool, bIsShielded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMagneticShieldChanged, bool, bIsActive); // New Delegate

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRelicStartRespawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRelicRespawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRelicDropped);

UCLASS()
class JUNKBOARDRUSH_API ARelic : public ABaseGadget
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARelic();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Relic Config")
	FDataTableRowHandle RelicConfig;

#pragma region States

	UPROPERTY(ReplicatedUsing=OnRep_CurrentState, BlueprintReadOnly)
	ERelicState CurrentState;

	UFUNCTION()
	void OnRep_CurrentState();
	
#pragma endregion States

#pragma region Components
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Visual")
	TObjectPtr<USkeletalMeshComponent> RelicMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Visual")
	UDecalComponent* RelicDecal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Collisions")
	USphereComponent* PhysicSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Collisions")
	USphereComponent* PickupSphere;
	
#pragma endregion Components

#pragma region UI
	
	void ShowWaypoint();
	void HideWaypoint();

	void SetWaypointVisibility();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI")
	TSubclassOf<UUserWidget> WaypointWidgetClass;
	
#pragma endregion UI

#pragma region Shield N Charges

	UPROPERTY(ReplicatedUsing=OnRep_IsShielded, BlueprintReadOnly)
	bool bIsShielded = false;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentRelicCharges, BlueprintReadOnly)
	int32 CurrentRelicCharges = 0;

	FTimerHandle TimerHandle_ShieldExpire;
	void OnShieldExpired();
	
	UFUNCTION()
	void OnRep_IsShielded();

	UFUNCTION()
	void OnRep_CurrentRelicCharges();

	UPROPERTY(ReplicatedUsing=OnRep_IsMagneticShieldActive, BlueprintReadOnly)
	bool bIsMagneticShieldActive = false;

	UFUNCTION()
	void OnRep_IsMagneticShieldActive();
	
	UPROPERTY(BlueprintAssignable)
	FOnMagneticShieldChanged OnMagneticShieldChanged; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> MagneticShieldVFX;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> MagneticShieldBreakVFX;

	UPROPERTY()
	UNiagaraComponent* MagneticShieldVFXComponent;
 
	void ActivateMagneticShield();
	void BreakMagneticShield();
	
#pragma endregion Shield N Charges

#pragma region Pickup N Steal
	
	float LastTransferTime = 0.0f; // Only checked on server so no need to replicate

#pragma endregion Pickup N Steal

protected:
	
	virtual void BeginPlay() override;

	AJBRGameState* GS;
	UWaypointWidget* WaypointWidget;
	
	void UpdateMovementOnCircuit(float DeltaTime);
	
	UPROPERTY(VisibleAnywhere)
	ARelicCircuit* CurrentCircuit;

	float CurrentDistanceOnSpline;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> RelicShieldVFX;
	UPROPERTY()
	UNiagaraComponent* ShieldVFXComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> RelicAuraVFX1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> RelicAuraVFX2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> RelicAuraVFX3;
	UPROPERTY()
	UNiagaraComponent* RelicAuraVFXComponent;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	const FRelicData& GetRelicData() const;

#pragma region Movement
	
	void SetInitialCircuit(ARelicCircuit* StartCircuit);

#pragma endregion Movement

#pragma region Pickup N Steal
	
	UFUNCTION(BlueprintCallable)
	void AcquireRelic(AJBRCharacter* NewOwner, AJBRCharacter* PreviousOwner, bool bIsSteal);

	UFUNCTION(Server, Reliable)
	void ServerRPC_TryAcquire(AJBRCharacter* Requester);
	void TryAcquire(AJBRCharacter* Requester);
	UFUNCTION(BlueprintCallable)
	void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_NotifyRelicOwnershipChanged(AJBRCharacter* NewOwner, AJBRCharacter* PreviousOwner);
	
#pragma endregion Pickup N Steal

#pragma region Drop
	
	UFUNCTION(BlueprintCallable)
	void OnDrop(AJBRCharacter* Dropper);

	void EnableDropPhysic();
	void DisableDropPhysic();

#pragma endregion Drop

#pragma region Shield N Charges
	
	UFUNCTION(BlueprintCallable)
	bool AddCharge();
	
	UFUNCTION(BlueprintCallable)
	void ResetAllCharges();
	
#pragma endregion Shield N Charges

#pragma region Respawn
	
	void StartRespawnRelic();

#pragma endregion Respawn

	void ChangeStates(ERelicState State);

#pragma region Actions

	UPROPERTY(BlueprintAssignable)
	FOnRelicOwnershipChanged OnRelicOwnershipChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnRelicChargeChanged OnRelicChargeChanged;

	UPROPERTY(BlueprintAssignable)
	FOnRelicShieldChanged OnRelicShieldChanged;

	UPROPERTY(BlueprintAssignable)
	FOnRelicStartRespawn OnRelicStartRespawn;

	UPROPERTY(BlueprintAssignable)
	FOnRelicRespawn OnRelicRespawn;

	UPROPERTY(BlueprintAssignable)
	FOnRelicDropped OnRelicDropped;

#pragma endregion Actions

#pragma region Visuals

	// Stencil
	int StencilID;
	void UpdateStencil(int ID);
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayRelicCatchVFX(AJBRCharacter* Catcher);
	
	void EnableRelicShieldVFX();
	void DisableRelicShieldVFX();
	
	void SwitchRelicAuraVFX();

	void HandleDecalBehavior();

#pragma endregion Visuals
	
};
