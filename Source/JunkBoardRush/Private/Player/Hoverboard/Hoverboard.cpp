#include "Player/Hoverboard/Hoverboard.h"
#include "Libraries/GenericFunction.h"
#include "Framework/JBRCharacter.h"
#include "Framework/JBRCustomCMC.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/Hoverboard/VFXBoardManager.h"
#include "Visuals/Animations/TricksAnimationComponent.h"

#pragma region Init

AHoverboard::AHoverboard()
{
    PrimaryActorTick.bCanEverTick = true;

    BoardMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BoardMesh"));
    SetRootComponent(BoardMesh);
    BoardMesh->SetCollisionProfileName(TEXT("NoCollision"));

    VFXBoardManager = CreateDefaultSubobject<UVFXBoardManager>(TEXT("VFXBoardManager"));

    bReplicates = false;
}

void AHoverboard::BeginPlay()
{
    Super::BeginPlay();

    CharacterOwner = Cast<AJBRCharacter>(GetParentActor());
    if (!CharacterOwner) return;
    
    CharacterCMC = CharacterOwner->CustomCMC;
    AttachBoardToCharacter();
    PreviousActorYaw = CharacterOwner->GetActorRotation().Yaw;

    VFXBoardManager->Initialize(CharacterOwner, BoardMesh);
}

void AHoverboard::AttachBoardToCharacter()
{
    if (!CharacterOwner || !CharacterOwner->GetMesh()) return;

    FAttachmentTransformRules AttachmentRules(
        EAttachmentRule::KeepWorld,
        EAttachmentRule::KeepWorld,
        EAttachmentRule::KeepWorld,
        false
    );

    AttachToComponent(CharacterOwner->GetMesh(), AttachmentRules, FName("root"));
}

void AHoverboard::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!CharacterOwner || !CharacterCMC)
    {
        Destroy();
        return;
    }

    UpdateRotationMode();

    if (CurrentRotationMode == CharacterRotationMode::RailState)
    {
        UpdateBoardTransformOnRails(DeltaTime);
    }
    else
    {
        UpdateBoardTransform(DeltaTime);
    }
}

#pragma endregion Init

#pragma region Updates

void AHoverboard::UpdateRotationMode()
{
    CurrentRotationMode = CharacterCMC->IsGrinding()
        ? CharacterRotationMode::RailState
        : CharacterRotationMode::DefaultState;
}

void AHoverboard::UpdateBoardTransform(float DeltaTime)
{
    CalculateSlopePitchAndRoll(TargetPitch, TargetRoll);
    
    TargetPitch += CalculateTargetPitch(DeltaTime);
    TargetRoll += CalculateTargetRoll(DeltaTime);

    TargetPitch = FMath::Clamp(TargetPitch, -GetBoardData().MaxPitchAngle, GetBoardData().MaxPitchAngle);

    CalculateLerpSpeeds(DeltaTime);

    CurrentRoll = FMath::Lerp(CurrentRoll, TargetRoll, DeltaTime * CurrentLerpSpeedRoll);
    CurrentPitch = FMath::Lerp(CurrentPitch,TargetPitch,DeltaTime * CurrentLerpSpeedPitch);

    ApplyBoardRotation();

    FRotator BoneRot = GetBoneRootLocalRotation();
    ApplyPlayerRotation(DeltaTime, BoneRot.Pitch, BoneRot.Roll, BoneRot.Yaw);

    if (!CharacterCMC->IsAirGliding() && TargetAirGlidePitch != 0.f)
    {
        ResetAirGlideVisual();
    }
}

void AHoverboard::UpdateBoardTransformOnRails(float DeltaTime)
{
    TargetPitch = CalculateTargetPitch(DeltaTime);
    TargetRoll = CalculateTargetRoll(DeltaTime);

    CalculateRailsLerpSpeeds();

    CurrentRoll = FMath::Lerp(CurrentRoll,  TargetRoll,  DeltaTime * CurrentLerpSpeedRoll);
    CurrentPitch = FMath::Lerp(CurrentPitch, TargetPitch, DeltaTime * CurrentLerpSpeedPitch);

    ApplyBoardRotation();

    FRotator BoneRot = GetBoneRootLocalRotation();
    ApplyPlayerRotation(DeltaTime, BoneRot.Pitch, BoneRot.Roll, BoneRot.Yaw);
}

#pragma endregion Updates

#pragma region Slope & Rotation

void AHoverboard::CalculateSlopePitchAndRoll(float& OutPitch, float& OutRoll)
{
    if (CharacterCMC->IsAirGliding())
    {
        OutPitch = AirGlidePitch;
        OutRoll  = 0.f;
        return;
    }

    FVector SlopeNormal = CharacterCMC->DetectedSlopeNormal;

    FVector CharForward = CharacterOwner->GetActorForwardVector();
    FVector CharRight   = CharacterOwner->GetActorRightVector();
    CharForward.Z = 0.f; CharForward.Normalize();
    CharRight.Z   = 0.f; CharRight.Normalize();

    float ForwardDot = FVector::DotProduct(SlopeNormal, CharForward);
    float RightDot   = FVector::DotProduct(SlopeNormal, CharRight);

    OutPitch = FMath::RadiansToDegrees(FMath::Asin(-ForwardDot));
    OutRoll  = FMath::RadiansToDegrees(FMath::Asin(RightDot));
}

void AHoverboard::CalculateLerpSpeeds(float DeltaTime)
{
    float SpeedRatio = CharacterOwner->GetSpeedRatioBaseSpeedFromAbsoluteVelocity();

    // --- Roll ---
    float RollInputMagnitude = 0.f;
    if (CharacterOwner->IsLocallyControlled())
    {
        RollInputMagnitude = FMath::Abs(CharacterCMC->MoveRightInput);
    }
    else
    {
        RollInputMagnitude = FMath::Clamp(
            FMath::Abs(TargetRoll) / (GetMaxRollAngle() + KINDA_SMALL_NUMBER),
            0.f, 1.f
        );
    }

    float RollCurveVal = 1.f;
    if (GetBoardData().RollCurve)
    {
        RollCurveVal = GetBoardData().RollCurve->FloatCurve.Eval(RollInputMagnitude);
    }

    CurrentLerpSpeedRoll = FMath::Lerp(
        GetBoardData().RollSmoothingSpeedLow,
        GetBoardData().RollSmoothingSpeedHigh,
        SpeedRatio
    ) * RollCurveVal;

    // --- Pitch ---
    if (CharacterCMC->IsAirGliding())
    {
        CurrentLerpSpeedPitch = CharacterCMC->MoveUpInput > 0.f
            ? GetBoardData().AirGlidePitchSpeed.Y
            : GetBoardData().AirGlidePitchSpeed.X;
    }
    else
    {
        float AngleChange = FMath::Abs(TargetPitch - PreviousSlopeAngle);
        float NormChange  = FMath::GetMappedRangeValueClamped(
            FVector2D(GetBoardData().PitchMinAngleChange, GetBoardData().PitchMaxAngleChange),
            FVector2D(0.f, 1.f),
            AngleChange
        );

        if (AngleChange < 0.01f) NormChange = 1.f;

        CurrentLerpSpeedPitch = FMath::Lerp(
            GetBoardData().PitchMinLerpSpeed,
            GetBoardData().PitchMaxLerpSpeed,
            NormChange
        );
    }

    PreviousSlopeAngle = TargetPitch;
}

void AHoverboard::CalculateRailsLerpSpeeds()
{
    CurrentLerpSpeedRoll  = 25.f;
    CurrentLerpSpeedPitch = 25.f;
}

float AHoverboard::CalculateTargetRoll(float DeltaTime)
{
    if (CurrentRotationMode == CharacterRotationMode::RailState)
    {
        return CharacterCMC->GrindState.TargetRotationRail.Roll;
    }

    if (CharacterOwner->IsLocallyControlled())
    {
        if (CharacterCMC->bIsMovingForward && CharacterCMC->IsLateralMoveEnough())
        {
            return CharacterCMC->VisualOnlyCurrentTurnInput * GetMaxRollAngle();
        }
        return 0.f;
    }

    return GetSimulatedTurnInput(DeltaTime) * GetMaxRollAngle();
}

float AHoverboard::CalculateTargetPitch(float DeltaTime)
{
    if (CharacterCMC->IsAirGliding())
    {
        return CalculateAirGlidingPitch(DeltaTime);
    }

    if (CurrentRotationMode == CharacterRotationMode::RailState)
    {
        return CharacterCMC->GrindState.TargetRotationRail.Pitch;
    }

    return 0.f;
}

float AHoverboard::GetSimulatedTurnInput(float DeltaTime)
{
    if (DeltaTime < KINDA_SMALL_NUMBER) return 0.f;

    float CurrentYaw = CharacterOwner->GetActorRotation().Yaw;
    float DeltaYaw   = UKismetMathLibrary::NormalizeAxis(CurrentYaw - PreviousActorYaw);
    PreviousActorYaw = CurrentYaw;

    float AngularVelocity    = DeltaYaw / DeltaTime;
    float ExpectedMaxTurnRate = CharacterCMC->GetTurnAmount();

    return FMath::Clamp(AngularVelocity / (ExpectedMaxTurnRate + 0.1f), -1.f, 1.f);
}

float AHoverboard::GetMaxRollAngle() const
{
    if (!CharacterOwner) return 0.f;
    float ValueLerp = CharacterOwner->GetSpeedRatioFromAbsoluteVelocity();
    return FMath::Lerp(GetBoardData().MaxRollAngleLow, GetBoardData().MaxRollAngleHigh, ValueLerp);
}

#pragma endregion Slope & Rotation

#pragma region Air Glide

float AHoverboard::CalculateAirGlidingPitch(float DeltaTime)
{
    if (CurrentRotationMode == CharacterRotationMode::RailState) return 0.f;

    TargetAirGlidePitch = -CharacterCMC->MoveUpInput * GetBoardData().MaxAirGlidePitchAngle;
    AirGlidePitch = TargetAirGlidePitch;

    return AirGlidePitch;
}

void AHoverboard::ResetAirGlideVisual()
{
    AirGlidePitch = 0.f;
    TargetAirGlidePitch = 0.f;
}

#pragma endregion Air Glide

#pragma region Apply

void AHoverboard::ApplyBoardRotation()
{
    float CharYaw = CharacterOwner->GetActorRotation().Yaw;
    SetActorRotation(FRotator(CurrentPitch, CharYaw, CurrentRoll));
}

void AHoverboard::ApplyPlayerRotation(float DeltaTime, float BoardPitchMesh, float BoardRollMesh, float BoardYawMesh)
{
    USkeletalMeshComponent* CharMesh = CharacterOwner->GetMesh();
    if (!CharMesh) return;

    //bool bIsTricking    = CharacterOwner->TricksComponent->bIsPlayerTricking;
    bool bLinkedToBoard = CharacterOwner->TricksAnimationComponent->ShouldAttachPlayerToBoard();

    TargetPlayerRotation = /*(!bIsTricking || bLinkedToBoard)*/ bLinkedToBoard
        ? FRotator(CurrentPitch + BoardPitchMesh, BoardYawMesh, CurrentRoll + BoardRollMesh)
        : FRotator::ZeroRotator;

    CurrentPlayerRotation = bLinkedToBoard
        ? TargetPlayerRotation
        : FMath::RInterpTo(CurrentPlayerRotation, TargetPlayerRotation, DeltaTime, 15.f);

    CharMesh->SetRelativeRotation(CurrentPlayerRotation);
}

FRotator AHoverboard::GetBoneRootLocalRotation() const
{
    return BoardMesh->GetBoneTransform(FName("Root"), RTS_Component).Rotator();
}

#pragma endregion Apply

#pragma region Data

const FHoverboardData& AHoverboard::GetBoardData() const
{
    return UGenericFunction::GetDataRow<FHoverboardData>(BoardData, TEXT("BoardData"));
}

#pragma endregion Data