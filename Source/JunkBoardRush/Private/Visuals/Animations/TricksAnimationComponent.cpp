
#include "Visuals/Animations/TricksAnimationComponent.h"

#include "Framework/JBRCharacter.h"
#include "Framework/JBRCustomCMC.h"
#include "Libraries/GenericFunction.h"
#include "Player/Hoverboard/Hoverboard.h"

UTricksAnimationComponent::UTricksAnimationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
    
    CurrentBoardTrickAnim = nullptr;
    CurrentBoardTrickStructAnim = FTricksAnimDataBoard();
    CurrentPlayerTrickAnim = nullptr;
    
    LastBoardTrickAnim = nullptr;
    LastBoardTrickStructAnim = FTricksAnimDataBoard();
    LastPlayerTrickAnim = nullptr;

    CurrentBoardTrickAnimData = FTricksAnimData();
    LastBoardTrickAnimData = FTricksAnimData();

    LastBoardTrickAnimData = FTricksAnimData();
    LastPlayerTrickAnimData = FTricksAnimData();
}

void UTricksAnimationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UTricksAnimationComponent, CurrentBoardTrickAnimData);
    DOREPLIFETIME(UTricksAnimationComponent, CurrentPlayerTrickAnimData);
    
    DOREPLIFETIME(UTricksAnimationComponent, LastBoardTrickAnimData);
    DOREPLIFETIME(UTricksAnimationComponent, LastPlayerTrickAnimData);
    
    DOREPLIFETIME(UTricksAnimationComponent, CurrentBoardTrickAnim);
    DOREPLIFETIME(UTricksAnimationComponent, CurrentPlayerTrickAnim);

    DOREPLIFETIME(UTricksAnimationComponent, LastBoardTrickAnim);
    DOREPLIFETIME(UTricksAnimationComponent, LastPlayerTrickAnim);
    
    DOREPLIFETIME(UTricksAnimationComponent, CurrentBoardTrickStructAnim);
    DOREPLIFETIME(UTricksAnimationComponent, LastBoardTrickStructAnim);
}

void UTricksAnimationComponent::BeginPlay()
{
	Super::BeginPlay();

    Character = Cast<AJBRCharacter>(GetOwner());
    TricksComponent = Character->TricksComponent;

    Character->OnLanded.AddDynamic(this, &UTricksAnimationComponent::StartLandAnimation);
}

#pragma region Animations

    #pragma region Animations Base Functions

void UTricksAnimationComponent::MulticastPlayTrickAnimation_Implementation(UAnimationAsset* AnimToPlayBoard, UAnimationAsset* AnimToPlayPlayer,
    const FTricksAnimDataBoard& NewBoardStructDataAnim,
    const FTricksAnimData& BoardTrickData, const FTricksAnimData& PlayerTrickData)
{
    if (!Character || !Character->PlayerHoverboard || !Character->PlayerHoverboard->BoardMesh)
    {
        return;
    }
    
    UpdateAnimTracking(BoardTrickData, PlayerTrickData, NewBoardStructDataAnim,
        AnimToPlayBoard, AnimToPlayPlayer);
    
    // Cast to UAnimSequence
    UAnimSequence* BoardSeq = Cast<UAnimSequence>(AnimToPlayBoard);
    UAnimSequence* PlayerSeq = Cast<UAnimSequence>(AnimToPlayPlayer);
    
    if (BoardSeq)
    {
        PreviousBoardAnimInstance = CurrentBoardAnimInstance;

        CurrentBoardAnimInstance = Character->PlayerHoverboard->BoardMesh->GetAnimInstance();
        if (CurrentBoardAnimInstance != nullptr)
        {
           PreviousBoardAnimMontage = CurrentBoardAnimMontage;
           CurrentBoardAnimInstance->Montage_Stop(0.0f, PreviousBoardAnimMontage);
            
           CurrentBoardAnimMontage = CurrentBoardAnimInstance->PlaySlotAnimationAsDynamicMontage(
                    BoardSeq,
                    FName("BoardTrickSlot"),
                    GetTricksData().BoardLerpInOutDuration.X,
                    GetTricksData().BoardLerpInOutDuration.Y,
                    1.0f,
                    1,
                    0.0f);
        }
    }
    if (PlayerSeq)
    {
        PreviousPlayerAnimInstance = CurrentPlayerAnimInstance;

        CurrentPlayerAnimInstance = Character->GetMesh()->GetAnimInstance();
        if (CurrentPlayerAnimInstance != nullptr)
        {
            PreviousPlayerAnimMontage = CurrentPlayerAnimMontage;
            CurrentPlayerAnimInstance->Montage_Stop(0.0f, PreviousPlayerAnimMontage);
            
            CurrentPlayerAnimMontage = CurrentPlayerAnimInstance->PlaySlotAnimationAsDynamicMontage(
                PlayerSeq,
                FName("PlayerTrickSlot"),
                GetTricksData().PlayerLerpInOutDuration.X,
                GetTricksData().PlayerLerpInOutDuration.Y,
                1.0f,
                1,
                0.0f
            );
            
            /*if (CurrentPlayerAnimMontage)
            {
                FOnMontageEnded MontageEndedDelegate;
                MontageEndedDelegate.BindUObject(this, &UTricksAnimationComponent::EndPlayerAnimationLogic);
                CurrentPlayerAnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, CurrentPlayerAnimMontage);
            }*/
        }
    }
}

void UTricksAnimationComponent::MulticastPlayInterruptAnimation_Implementation(UAnimationAsset* AnimToPlayBoard, UAnimationAsset* AnimToPlayPlayer,
    const FTricksAnimDataBoard& NewBoardStructDataAnim,
    const FTricksAnimData& BoardTrickData, const FTricksAnimData& PlayerTrickData, float BlendSpaceXvalue)
{
    if (!Character || !Character->PlayerHoverboard || !Character->PlayerHoverboard->BoardMesh)
    {
        return;
    }

    UpdateAnimTracking(BoardTrickData, PlayerTrickData, NewBoardStructDataAnim,
        AnimToPlayBoard, AnimToPlayPlayer);

    PreviousBoardAnimInstance = CurrentBoardAnimInstance;

    CurrentBoardAnimInstance = Character->PlayerHoverboard->BoardMesh->GetAnimInstance();
    if (CurrentBoardAnimInstance != nullptr)
    {
        PreviousBoardAnimMontage = CurrentBoardAnimMontage;
        CurrentBoardAnimInstance->Montage_Stop(0.0f, PreviousBoardAnimMontage);
        
        if (UBlendSpace* BoardBS = Cast<UBlendSpace>(AnimToPlayBoard))
        {
            FVector BlendSpacePosition(BlendSpaceXvalue, 0.f, 0.f);
            
            TArray<FBlendSampleData> BlendSampleDataCache;
            int32 TriangulationIndex = 0;
            
            BoardBS->UpdateBlendSamples(
                BlendSpacePosition,
                0.f,
                BlendSampleDataCache,
                TriangulationIndex
            );
            
            UAnimSequence* SelectedSeq = nullptr;
            float MaxWeight = 0.f;
            
            for (const FBlendSampleData& Sample : BlendSampleDataCache)
            {
                if (Sample.TotalWeight > MaxWeight)
                {
                    MaxWeight = Sample.TotalWeight;
                    SelectedSeq = Cast<UAnimSequence>(Sample.Animation);
                }
            }
            
            if (SelectedSeq)
            {
                CurrentBoardAnimMontage = CurrentBoardAnimInstance->PlaySlotAnimationAsDynamicMontage(
                    SelectedSeq,
                    FName("InterruptionSlot"),
                    GetTricksData().BoardLerpInOutDurationInterruption.X,
                    GetTricksData().BoardLerpInOutDurationInterruption.Y,
                    1.0f,
                    1,
                    0.0f
                );
                
                /*if (DynamicMontage)
                {
                    FOnMontageEnded MontageEndedDelegate;
                    MontageEndedDelegate.BindUObject(this, &UTricksAnimationComponent::EndMainAnimationLogic);
                    BoardAnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, DynamicMontage);
                }*/
            }
        }
        else if (UAnimSequence* BoardSeq = Cast<UAnimSequence>(AnimToPlayBoard))
        {
            CurrentBoardAnimMontage = CurrentBoardAnimInstance->PlaySlotAnimationAsDynamicMontage(
                BoardSeq,
                FName("InterruptionSlot"),
                GetTricksData().BoardLerpInOutDurationInterruption.X,
                GetTricksData().BoardLerpInOutDurationInterruption.Y,
                1.0f,
                1,
                0.0f
            );
            
            /*if (DynamicMontage)
            {
                FOnMontageEnded MontageEndedDelegate;
                MontageEndedDelegate.BindUObject(this, &UTricksAnimationComponent::EndMainAnimationLogic);
                BoardAnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, DynamicMontage);
            }*/
        }
    }
    
    PreviousPlayerAnimInstance = CurrentPlayerAnimInstance;

    CurrentPlayerAnimInstance = Character->GetMesh()->GetAnimInstance();
    if (CurrentPlayerAnimInstance != nullptr)
    {
        PreviousPlayerAnimMontage = CurrentPlayerAnimMontage;
        CurrentPlayerAnimInstance->Montage_Stop(0.0f, PreviousPlayerAnimMontage);
        
        if (UBlendSpace* PlayerBS = Cast<UBlendSpace>(AnimToPlayPlayer))
        {
            FVector BlendSpacePosition(1.f, 0.f, 0.f);
            
            TArray<FBlendSampleData> BlendSampleDataCache;
            int32 TriangulationIndex = 0;
            
            PlayerBS->UpdateBlendSamples(
                BlendSpacePosition,
                0.f,
                BlendSampleDataCache,
                TriangulationIndex
            );
            
            UAnimSequence* SelectedSeq = nullptr;
            float MaxWeight = 0.f;
            
            for (const FBlendSampleData& Sample : BlendSampleDataCache)
            {
                if (Sample.TotalWeight > MaxWeight)
                {
                    MaxWeight = Sample.TotalWeight;
                    SelectedSeq = Cast<UAnimSequence>(Sample.Animation);
                }
            }
            
            if (SelectedSeq)
            {
               CurrentPlayerAnimMontage = CurrentPlayerAnimInstance->PlaySlotAnimationAsDynamicMontage(
                    SelectedSeq,
                    FName("InterruptionSlot"),
                    GetTricksData().PlayerLerpInOutDurationInterruption.X,
                    GetTricksData().PlayerLerpInOutDurationInterruption.Y,
                    1.0f,
                    1,
                    0.0f
                );
                
                /*if (DynamicMontage)
                {
                    FOnMontageEnded MontageEndedDelegate;
                    MontageEndedDelegate.BindUObject(this, &UTricksAnimationComponent::EndPlayerAnimationLogic);
                    PlayerAnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, DynamicMontage);
                }*/
            }
        }
        else if (UAnimSequence* PlayerSeq = Cast<UAnimSequence>(AnimToPlayPlayer))
        {
            CurrentPlayerAnimMontage = CurrentPlayerAnimInstance->PlaySlotAnimationAsDynamicMontage(
                PlayerSeq,
                FName("InterruptionSlot"),
                0.1f,
                0.1f,
                1.0f,
                1,
                0.0f
            );
            
            /*if (DynamicMontage)
            {
                FOnMontageEnded MontageEndedDelegate;
                MontageEndedDelegate.BindUObject(this, &UTricksAnimationComponent::EndPlayerAnimationLogic);
                PlayerAnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, DynamicMontage);
            }*/
        }
    }
}

    #pragma endregion Animations Base Functions

    #pragma region Specific Tricks

// Start a basic trick animation
bool UTricksAnimationComponent::StartTrickAnimation()
{
    // We start getting the structure that we need
    FTricksAnimData TrickData;
    if (Character->CustomCMC->IsFalling())
    {
        TrickData = GetTricksData().AirTricksAnims;
    }
    else if (Character->CustomCMC->IsGrinding())
    {
        TrickData = GetTricksData().RailTricksAnims;
    }
    else
    {
        TrickData = GetTricksData().GroundTricksAnims;
    }

    // In case we use different data later
    FTricksAnimData BoardData = TrickData;
    FTricksAnimData PlayerData = TrickData;
    
    // Then return a random animation for each board and player, + an override animation if the selected
    // FTricksStructDataBoardAnimation has a valid PlayerAnim, or use a random one from PlayerData
    FAnimSelection NewAnimSelection = GetAnimForBoardAndPlayer(GetCorrectBoardAnimData(BoardData), GetCorrectPlayerAnimData(PlayerData));

    // if no anim found, return false
    if (NewAnimSelection.BoardAnim == nullptr) return false;
    
    // Play animation on ALL clients via multicast
    MulticastPlayTrickAnimation(NewAnimSelection.BoardAnim, NewAnimSelection.PlayerAnim,
        NewAnimSelection.TricksAnimDataBoard, BoardData, PlayerData);

    return true;
}

void UTricksAnimationComponent::StartLandAnimation()
{
    FTricksAnimData NewBoardAnim = GetTricksData().InterruptionAnims;
    FTricksAnimData NewPlayerAnim = GetTricksData().InterruptionAnims;

    float VelocityZ = FMath::Abs(Character->CustomCMC->LastFallingZVelocity);
    float RemappedValue = FMath::GetMappedRangeValueClamped(
        FVector2D(Character->GetPlayerSpeedData().BaseMaxWalkSpeed, Character->CustomCMC->GetAbsoluteMaxSpeed()),
        FVector2D(0.f, 100.f),
        VelocityZ);

    InterruptTrickWithData(NewBoardAnim, NewPlayerAnim, RemappedValue);
}

// ⚠️ REMINDER : NEED AN EMPTY PLAYER ANIM TO MAKE THE PLAYER BACK TO ITS ORIGINAL ANIM STATE
// It will continue the previous animation is not
void UTricksAnimationComponent::InterruptTrickWithData(FTricksAnimData BoardData, FTricksAnimData PlayerData, float BlendSpaceXvalue)
{
    // Significant falling check
    // If small bump, don't interrupt
    if (Character->CustomCMC->LastFallingZVelocity > -10.f) return;
    
    EndMainAnimationLogic();
    EndPlayerAnimationLogic();
    
    FAnimSelection NewAnimSelection = GetAnimForBoardAndPlayer(GetCorrectBoardAnimData(BoardData), GetCorrectPlayerAnimData(PlayerData));
    
    MulticastPlayInterruptAnimation(NewAnimSelection.BoardAnim, NewAnimSelection.PlayerAnim,
        NewAnimSelection.TricksAnimDataBoard, BoardData, PlayerData, BlendSpaceXvalue);
}

    #pragma endregion Specific Tricks

#pragma endregion Animations

#pragma region Utilities

bool UTricksAnimationComponent::IsBoardTrickingFromAnim()
{
    if (!CurrentBoardAnimInstance || !CurrentBoardAnimMontage) return false;

    return CurrentBoardAnimInstance->Montage_IsPlaying(CurrentBoardAnimMontage)
        || CurrentBoardAnimInstance->Montage_GetBlendTime(CurrentBoardAnimMontage) > 0.0f;
}

bool UTricksAnimationComponent::IsPlayerTrickingFromAnim()
{
    if (!CurrentPlayerAnimInstance || !CurrentPlayerAnimMontage) return false;
    
    return CurrentPlayerAnimInstance->Montage_IsPlaying(CurrentPlayerAnimMontage) || 
        CurrentPlayerAnimInstance->Montage_GetBlendTime(CurrentPlayerAnimMontage) > 0.0f;
}

bool UTricksAnimationComponent::AreFootCloseEnoughToBoard(float& DistR, float& DistL)
{
    const float DistMax = 19.f;

    if (!Character || !Character->PlayerHoverboard) return false;
    
    FTransform PlayerRFoot = Character->GetMesh()->GetSocketTransform(FName("foot_r"));
    FTransform PlayerLFoot = Character->GetMesh()->GetSocketTransform(FName("foot_l"));
        
    FTransform BoardRSocket = Character->PlayerHoverboard->BoardMesh->GetSocketTransform(FName("foot_r_socket"));
    FTransform BoardLSocket = Character->PlayerHoverboard->BoardMesh->GetSocketTransform(FName("foot_l_socket"));
    
    DistR = FMath::Abs(PlayerRFoot.GetLocation().Z - BoardRSocket.GetLocation().Z);
    DistL = FMath::Abs(PlayerLFoot.GetLocation().Z - BoardLSocket.GetLocation().Z);
    
    bool PosCondition = (DistR < DistMax || DistL < DistMax);
    
    //GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Purple, FString::Printf(TEXT("dist R : %f and dist L : %f"), DistR, DistL));
    
    return PosCondition;
}

bool UTricksAnimationComponent::ShouldAttachPlayerToBoard()
{
    bool bBoardIsActive = IsBoardTrickingFromAnim();

    // Linked explicitly
    if (bBoardIsActive && CurrentBoardTrickStructAnim.Anim != nullptr 
        && CurrentBoardTrickStructAnim.ShouldPlayerBeLinkedOnBoard)
        return true;

    // No player anim — but wait for the previous one to fully blend out first
    if (CurrentPlayerTrickAnim == nullptr && !IsPlayerTrickingFromAnim())
        return true;

    return false;
}

bool UTricksAnimationComponent::ShouldHardSetFootTransform()
{
    if (!Character || !Character->PlayerHoverboard) return true;
    
    float DistR = 0;
    float DistL = 0;
    
    return AreFootCloseEnoughToBoard(DistR, DistL) && !IsPlayerTrickingFromAnim();
}

#pragma endregion Utilities

#pragma region Animation Logic Flags

void UTricksAnimationComponent::EndMainAnimationLogic(UAnimMontage* Montage, bool bInterrupted)
{
    CurrentBoardTrickAnim = nullptr;
    CurrentBoardTrickAnimData = FTricksAnimData();
    CurrentBoardTrickStructAnim = FTricksAnimDataBoard();
}

void UTricksAnimationComponent::EndPlayerAnimationLogic(UAnimMontage* Montage, bool bInterrupted)
{
    CurrentPlayerTrickAnim = nullptr;
    CurrentPlayerTrickAnimData = FTricksAnimData();
}

#pragma endregion Animation Logic Flags

#pragma region Data

// Update current and last variables to keep track
void UTricksAnimationComponent::UpdateAnimTracking(FTricksAnimData NewBoardData, FTricksAnimData NewPlayerData,
    FTricksAnimDataBoard NewBoardStructDataAnim, UAnimationAsset* NewBoardAnim,  UAnimationAsset* NewPlayerAnim)
{
    // Update tracking
    LastBoardTrickAnimData = CurrentBoardTrickAnimData;
    LastBoardTrickStructAnim = CurrentBoardTrickStructAnim;
    LastBoardTrickAnim = CurrentBoardTrickAnim;
    LastPlayerTrickAnim = CurrentPlayerTrickAnim;
    
    CurrentBoardTrickAnim = NewBoardAnim;
    CurrentBoardTrickStructAnim = NewBoardStructDataAnim;
    CurrentPlayerTrickAnim = NewPlayerAnim;
    
    CurrentBoardTrickAnimData = NewBoardData;
    CurrentPlayerTrickAnimData = NewPlayerData;
}

// Returns the anim special board struct array based on a trick data (board only)
TArray<FTricksAnimDataBoard> UTricksAnimationComponent::GetCorrectBoardAnimData(FTricksAnimData BaseData)
{
    return BaseData.BoardAnims;
}

// Returns the anim array based on a trick data (player only)
TArray<TObjectPtr<UAnimationAsset>> UTricksAnimationComponent::GetCorrectPlayerAnimData(FTricksAnimData BaseData)
{
    return BaseData.PlayerAnims;
}

// Returns a combination of two animation : one for the board and the other for the player (override or not by the board struct)
FAnimSelection UTricksAnimationComponent::GetAnimForBoardAndPlayer(
    const TArray<FTricksAnimDataBoard>& BoardStructArray, const TArray<TObjectPtr<UAnimationAsset>>& PlayerAnimArray)
{
    FAnimSelection Result;

    if (BoardStructArray.Num() == 0)
    {
        return Result;
    }

    int32 RandomIndex = FMath::RandRange(0, BoardStructArray.Num() - 1);
    
    const FTricksAnimDataBoard& BoardStruct = BoardStructArray[RandomIndex];
    Result.BoardAnim = BoardStruct.Anim;
    Result.TricksAnimDataBoard = BoardStruct;

    // Pick the override animation if either its non-null or should be linked
    // (even if the override anim is empty, the player will just stand and follows the rotation)
    if (BoardStruct.LinkedAnim != nullptr || BoardStruct.ShouldPlayerBeLinkedOnBoard)
    {
        Result.PlayerAnim = BoardStruct.LinkedAnim;
    }
    // lese, play basic player animation from data arrays
    else if (PlayerAnimArray.Num() != 0)
    {
        RandomIndex = FMath::RandRange(0,PlayerAnimArray.Num() - 1);
        Result.PlayerAnim = PlayerAnimArray[RandomIndex];
        
        // Find a random non-null animation if forced to
        if (BoardStruct.DisableNonPlayerAnimationSelection && Result.PlayerAnim == nullptr)
        {
            TArray<UAnimationAsset*> ValidAnims = PlayerAnimArray.FilterByPredicate(
                [](const UAnimationAsset* Anim) { return Anim != nullptr; }
            );

            if (ValidAnims.Num() > 0)
            {
                Result.PlayerAnim = ValidAnims[FMath::RandRange(0, ValidAnims.Num() - 1)];
            }
        }
    }

    return Result;
}

const FTricksSystemData& UTricksAnimationComponent::GetTricksData() const
{
    return UGenericFunction::GetDataRow<FTricksSystemData>(TricksData, TEXT("TricksData"));
}

#pragma endregion Data

#pragma region Getters

UAnimationAsset* UTricksAnimationComponent::GetCurrentBoardAnim()
{
    return CurrentBoardTrickAnim;
}

FTricksAnimDataBoard UTricksAnimationComponent::GetCurrentBoardAnimStructData()
{
    return CurrentBoardTrickStructAnim;
}

UAnimationAsset* UTricksAnimationComponent::GetCurrentPlayerAnim()
{
    return CurrentPlayerTrickAnim;
}

FTricksAnimData UTricksAnimationComponent::GetCurrentBoardAnimData()
{
    return CurrentBoardTrickAnimData;
}

FTricksAnimData UTricksAnimationComponent::GetCurrentPlayerAnimData()
{
    return CurrentPlayerTrickAnimData;
}

#pragma endregion Getters
