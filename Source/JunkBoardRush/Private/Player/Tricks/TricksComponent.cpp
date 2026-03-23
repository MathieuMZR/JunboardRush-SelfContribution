#include "Player/Tricks/TricksComponent.h"

#include "Framework/JBRCharacter.h"
#include "Framework/JBRGameState.h"
#include "Libraries/GenericFunction.h"
#include "Player/Tricks/ContextActionTarget.h"
#include "Visuals/Animations/TricksAnimationComponent.h"

#pragma region Init


UTricksComponent::UTricksComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    bIsInExecuteWindow = false;
    bContextActionInputAvailable = true;
    
    SetIsReplicatedByDefault(true);
}

void UTricksComponent::BeginPlay()
{
    Super::BeginPlay();

    Character = Cast<AJBRCharacter>(GetOwner());
    TricksAnimationComponent = Character->TricksAnimationComponent;
}

void UTricksComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (Character && !Character->HasAuthority()) return;
    if (bIsInExecuteWindow) GetWindowTime += DeltaTime;
}

#pragma endregion Init

#pragma region Context Actions

void UTricksComponent::AttemptPossibleInteraction(AActor* ContextActor)
{
    if (!Character->HasAuthority()) Server_StartPossibleInteraction(ContextActor);
    else StartPossibleInteraction(ContextActor);
}

void UTricksComponent::Server_StartPossibleInteraction_Implementation(AActor* ContextActor)
{
    StartPossibleInteraction(ContextActor);
}

// Server-only
void UTricksComponent::StartPossibleInteraction(AActor* ContextActor)
{
    IContextActionTarget* ContextTarget = Cast<IContextActionTarget>(ContextActor);
    if (!ContextTarget) return;

    if (ContextTarget == Cast<IContextActionTarget>(CurrentInteractableActor)) return;
    
    LastInteractableActor = CurrentInteractableActor;
    CurrentInteractableActor = ContextActor;

    // Skip if non-valid interactable actor
    if (!CurrentInteractableActor) return;
    
    // Reset action input when new interaction starts
    ResetContextActionInputCooldown();

    // Start possible window to press the action input
    StartContextActionWindowTimer();
}

void UTricksComponent::AttemptContextAction()
{
    if (!Character->HasAuthority()) Server_AttemptContextAction();
    else ExecuteContextAction();
}

void UTricksComponent::Server_AttemptContextAction_Implementation()
{
    ExecuteContextAction();
}

// Server-only
void UTricksComponent::ExecuteContextAction()
{
    if (!bContextActionInputAvailable || !bIsInExecuteWindow) return;
    if (!CurrentInteractableActor) return;
    
    if (!TricksAnimationComponent->StartTrickAnimation()) return;
    
    StartContextActionInputCooldown();
    ResetContextActionWindow();
    
    Cast<IContextActionTarget>(CurrentInteractableActor)->TriggerContextAction(Character);
    CurrentInteractableActor = nullptr;

    Character->HealthComponent->SpawnJunkDrops(Character->GetPlayerJunkData().JunkDropsWhenContextAction, Character->GetActorLocation());
}

#pragma endregion Context Actions

#pragma region Context Actions Timers

void UTricksComponent::StartContextActionWindowTimer()
{
    ResetContextActionWindow();
    
    GetWorld()->GetTimerManager().SetTimer(
        ContextActionExecuteWindow,
        this,
        &UTricksComponent::EndContextActionWindow,
        GetTricksData().ActionWindowDuration,
        false);
    bIsInExecuteWindow = true;

    GetWindowTime = 0.f;
}

void UTricksComponent::ResetContextActionWindow()
{
    GetWorld()->GetTimerManager().ClearTimer(ContextActionExecuteWindow);
    bIsInExecuteWindow = false;
}

void UTricksComponent::EndContextActionWindow()
{
    ResetContextActionWindow();

    // Reset last actor when quit window
    CurrentInteractableActor = nullptr;
}

void UTricksComponent::StartContextActionInputCooldown()
{
    GetWorld()->GetTimerManager().SetTimer(
        ContextActionInputCooldownTimer,
        this,
        &UTricksComponent::ResetContextActionInputCooldown,
        GetTricksData().ActionCooldownDuration,
        false);
    bContextActionInputAvailable = false;
}

void UTricksComponent::ResetContextActionInputCooldown()
{
    GetWorld()->GetTimerManager().ClearTimer(ContextActionInputCooldownTimer);
    bContextActionInputAvailable = true;
}

#pragma endregion Context Actions Timers

#pragma region Utilities

const FTricksSystemData& UTricksComponent::GetTricksData() const
{
    return UGenericFunction::GetDataRow<FTricksSystemData>(TricksData, TEXT("TricksData"));
}

void UTricksComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UTricksComponent, bCanStealRelic);
}

#pragma endregion Utilities