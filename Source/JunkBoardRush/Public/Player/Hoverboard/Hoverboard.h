// Hoverboard.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HoverboardData.h"
#include "Hoverboard.generated.h"

class UVFXBoardManager;
class AJBRCharacter;
class UJBRCustomCMC;

enum class CharacterRotationMode : uint8;

UCLASS()
class JUNKBOARDRUSH_API AHoverboard : public AActor
{
    GENERATED_BODY()

public:
    AHoverboard();

    virtual void Tick(float DeltaTime) override;

    void AttachBoardToCharacter();

    UFUNCTION(BlueprintCallable)
    const FHoverboardData& GetBoardData() const;

    // Components and References
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Board")
    USkeletalMeshComponent* BoardMesh;

    UPROPERTY(BlueprintReadOnly, Category = "Board")
    AJBRCharacter* CharacterOwner;

    UPROPERTY(BlueprintReadOnly, Category = "Board")
    UJBRCustomCMC* CharacterCMC;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Board")
    FDataTableRowHandle BoardData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SubComponents")
    UVFXBoardManager* VFXBoardManager;

protected:
    virtual void BeginPlay() override;

private:

    // Transform State
    float TargetRoll = 0.f;
    float CurrentRoll = 0.f;
    float TargetPitch = 0.f;
    float CurrentPitch = 0.f;
    float PreviousSlopeAngle = 0.f;
    float PreviousActorYaw = 0.f;

    float CurrentLerpSpeedRoll = 0.f;
    float CurrentLerpSpeedPitch = 0.f;

    FRotator CurrentPlayerRotation = FRotator::ZeroRotator;
    FRotator TargetPlayerRotation  = FRotator::ZeroRotator;

    CharacterRotationMode CurrentRotationMode;

    // Air Glide State
    float TargetAirGlidePitch = 0.f;
    float AirGlidePitch = 0.f;

    // Update
    void UpdateRotationMode();
    void UpdateBoardTransform(float DeltaTime);
    void UpdateBoardTransformOnRails(float DeltaTime);

    // Slope & Rotation
    void  CalculateSlopePitchAndRoll(float& OutPitch, float& OutRoll);
    void  CalculateRailsLerpSpeeds();
    void  CalculateLerpSpeeds(float DeltaTime);
    float CalculateTargetRoll(float DeltaTime);
    float CalculateTargetPitch(float DeltaTime);
    float CalculateAirGlidingPitch(float DeltaTime);
    float GetSimulatedTurnInput(float DeltaTime);
    float GetMaxRollAngle() const;
    void  ResetAirGlideVisual();

    // Apply 
    void ApplyBoardRotation();
    void ApplyPlayerRotation(float DeltaTime, float BoardPitchMesh, float BoardRollMesh, float BoardYawMesh);
    FRotator GetBoneRootLocalRotation() const;
};