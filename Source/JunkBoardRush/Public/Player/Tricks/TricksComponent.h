#pragma once

#include "CoreMinimal.h"
#include "TricksSystemData.h"
#include "Components/ActorComponent.h"
#include "TricksComponent.generated.h"

class UContextActionTarget;
class IContextActionTarget;
class UTricksAnimationComponent;
class AJBRCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPerfectZoneEnter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFalloffZoneEnter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFalloffZoneEnd);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTrickAllowsRelicSteal);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboIncrease, int, Value, int, OldValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboBreak, int, OldValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboReset);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCumulatedAPIncrease, float, Value, float, OldValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCumulatedAPBreak, float, OldValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCumulatedAPReset);

UENUM(BlueprintType)
enum class EComboEndReason : uint8
{
    None,
    Reset,
    Break
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class JUNKBOARDRUSH_API UTricksComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    
#pragma region Init
    
    UTricksComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
#pragma endregion Init
        
#pragma region Components
    
    AJBRCharacter* Character;
    UTricksAnimationComponent* TricksAnimationComponent;
    
#pragma endregion Components

    UPROPERTY(BlueprintAssignable)
    FOnTrickAllowsRelicSteal OnTrickAllowsRelicSteal;

#pragma region Context Actions

    void AttemptPossibleInteraction(AActor* ContextActor);
    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_StartPossibleInteraction(AActor* ContextActor);

    void StartPossibleInteraction(AActor* ContextActor);
    
    UFUNCTION(BlueprintCallable)
    void AttemptContextAction();
    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_AttemptContextAction();
    
    void ExecuteContextAction();
    
    void StartContextActionInputCooldown();
    void ResetContextActionInputCooldown();

    void StartContextActionWindowTimer();
    void ResetContextActionWindow();
    void EndContextActionWindow();

    FTimerHandle ContextActionExecuteWindow;
    FTimerHandle ContextActionInputCooldownTimer;

    UPROPERTY(BlueprintReadOnly, Replicated)
    bool bContextActionInputAvailable;
    UPROPERTY(BlueprintReadOnly, Replicated)
    bool bIsInExecuteWindow;

    UPROPERTY(BlueprintReadOnly, Replicated)
    float GetWindowTime;
    
#pragma endregion Context Actions
    
#pragma region Utilities

    UFUNCTION(BlueprintCallable)
    const FTricksSystemData& GetTricksData() const;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tricks - Data")
    FDataTableRowHandle TricksData;
    
    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma endregion Utilities

#pragma region Relic
    
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "Relic")
    bool bCanStealRelic;

#pragma endregion Relic

protected:
    
    AActor* CurrentInteractableActor = nullptr;
    AActor* LastInteractableActor = nullptr;
  
};