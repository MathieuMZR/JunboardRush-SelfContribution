#pragma once

#include "CoreMinimal.h"

#include "Player/DashComponent.h"
#include "Gadgets/MaterialBarComponent.h"
#include "Gadgets/ScrappingComponent.h"
#include "Gadgets/CraftingComponent.h"
#include "Game/Rails/RailSystemComponent.h"
#include "Player/Tricks/TricksComponent.h"
#include "Player/HealthComponent.h"

#include "PlayerCMCData.h"
#include "Player/SpeedSystem/PlayerSpeedData.h"
#include "Player/PlayerJunkData.h"

#include "AbilitySystemInterface.h"
#include "Visuals/CharacterCosmeticsConfig.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/Tricks/ContextActionTarget.h"

#include "JBRCharacter.generated.h"

class UPlayerCameraShakeComponent;
class ARelic;
class UAdrenalineSystemComponent;
class AJBRPlayerState;
class AJBRController;
class UDynamicPlayerCamera;
class UJBRCustomCMC;
class AHoverboard;
class USphereComponent;
class UTricksAnimationComponent;

#pragma region States and Enums

UENUM(BlueprintType)
enum class CharacterRotationMode : uint8 {
	DefaultState = 0 UMETA(DisplayName = "Default State"),
	RailState = 1 UMETA(DisplayName = "Rail State"),
};

#pragma endregion States and Enums

//On name change
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerNameChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLanded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPossessedBy);

UCLASS()
class JUNKBOARDRUSH_API AJBRCharacter : public ACharacter, public IAbilitySystemInterface, public IContextActionTarget
{
	GENERATED_BODY()

public:
	AJBRCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	virtual void TriggerContextAction(AJBRCharacter* Initiator) override;

#pragma region Possession
	
	UPROPERTY(BlueprintAssignable)
	FOnPlayerNameChanged OnPlayerNameChanged;

	UPROPERTY(BlueprintAssignable)
	FOnPossessedBy OnPlayerPossessed;
	
	FString GetPlayerNameFromPlayerState();

#pragma endregion Possession

#pragma region Components

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - SubComponents")
	UDynamicPlayerCamera* DynamicPlayerCamera;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - SubComponents")
	UPlayerCameraShakeComponent* CameraShakeComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - SubComponents")
	UTricksComponent* TricksComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - SubComponents")
	UTricksAnimationComponent* TricksAnimationComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - SubComponents")
	UMaterialBarComponent* MaterialBarComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - SubComponents")
	UScrappingComponent* ScrappingComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - SubComponents")
	UCraftingComponent* CraftingComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - SubComponents")
	URailSystemComponent* RailSystemComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - SubComponents")
	UHealthComponent* HealthComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - SubComponents")
	UDashComponent* DashComponent;

	UPROPERTY(BlueprintReadOnly, Category = Component)
	UJBRCustomCMC* CustomCMC;
	AJBRPlayerState* PlayerState;

#pragma endregion Components

#pragma region Relic Steal Components
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - Components")
	USphereComponent* RelicStealingSphere;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - Components")
	USphereComponent* RelicAbsoluteStealingSphere;

#pragma endregion Relic Steal Components

#pragma region Native Components

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - Components")
	USpringArmComponent* SpringArm;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - Components")
	UCameraComponent* Camera;

#pragma endregion Native Components
	
#pragma region Visuals
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UCharacterCosmeticsConfig> CosmeticsConfig;
#pragma endregion Visuals

#pragma region Data

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - Data")
	FDataTableRowHandle PlayerCMCData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - Data")
	FDataTableRowHandle PlayerSpeedData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character - Data")
	FDataTableRowHandle PlayerJunkData;

#pragma endregion Data

#pragma region Physic

	FOnLanded OnLanded;

	// for debug purpose only
	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bForceStopAutoMoveState = false;

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bForceDisableDrift = false;

#pragma endregion Physic

#pragma region Relic

	UFUNCTION(BlueprintPure, Category = "Relic")
	bool HasRelic() const;
	
	UFUNCTION() void OnRelicStealingStartOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION() void OnRelicStealingEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION() void OnRelicAbsoluteStealingOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION(Server, Reliable)
	void ServerRPC_TryStealFromTricks();
	UFUNCTION() void TryStealFromTricks();
	
	// Retrigger steal colliders to resimulate collision when losing the relic (if still in range) so no re-enter range logic is needed
	void ReTriggerStealColliders();
	void RefreshRelicStealState();

	UPROPERTY(Replicated)
	bool bIsInRelicStealRange = false;
	
	UPROPERTY()
	AJBRCharacter* CurrentOverlappingRelicHolder;
	
	UFUNCTION()
	void HandleDeath();
	
#pragma endregion Relic

#pragma region GAS

	// Used to cache the pointer so we don't query PlayerState every frame
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	class UAttributeSet* AttributeSet;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

#pragma endregion GAS

#pragma region Data
	
	UFUNCTION(BlueprintCallable)
	const FPlayerCMCData& GetPlayerCMCData() const;

	UFUNCTION(BlueprintCallable)
	const FPlayerSpeedData& GetPlayerSpeedData() const;

	UFUNCTION(BlueprintCallable)
	const FPlayerJunkData& GetPlayerJunkData() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetSpeedRatioFromVelocity() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetSpeedRatioFromAbsoluteVelocity() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetSpeedRatioBaseSpeedFromAbsoluteVelocity() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetFallingSpeedRatio() const;

	UFUNCTION(BlueprintPure, Category = "Data")
	int32 GetPlayerRank();

#pragma endregion Data

#pragma region Board

	UFUNCTION(BlueprintCallable, Category = "Hoverboard")
	void ChangeBoard(AHoverboard* Hoverboard);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hoverboard")
	UChildActorComponent* HoverboardChildActorComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Hoverboard")
	AHoverboard* PlayerHoverboard;

#pragma endregion Board

	UPROPERTY(ReplicatedUsing = OnRep_CameraTransform)
	FVector ReplicatedCameraLocation;

	UPROPERTY(ReplicatedUsing = OnRep_CameraTransform)
	FRotator ReplicatedCameraRotation;
	
	UFUNCTION()
	void OnRep_CameraTransform() {}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
