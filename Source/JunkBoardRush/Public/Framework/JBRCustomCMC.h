#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

#include "PlayerCMCData.h"
#include "Player/Camera/DynamicPlayerCamera.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/SpeedSystem/PlayerSpeedData.h"

#include "JBRCustomCMC.generated.h"

class ABumper;
class AGrindRail;
class AJBRController;
class AJBRCharacter;

#pragma region Delegates

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChargeJumpSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLateralMovementStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLateralMovementEnd);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDriftEnter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSmallDriftEnter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBigDriftEnter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDriftRelease);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDriftReleasePoor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDriftCancel);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpeedBoostAdded, float, Amount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRailBegin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRailEnd);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnDashHitSignature, const FHitResult&);

#pragma endregion Delegates

#pragma region Structs

struct FCharacterNetworkMoveData_JBR : FCharacterNetworkMoveData
{
	typedef FCharacterNetworkMoveData Super;
	
	float GravityScale = 1.0f;
	float AirFriction = 0.f;

	float MoveRightInput = 0.0f;
	float MoveUpInput = 0.0f;
	
	float RailDistance = 0.0f;
	AGrindRail* CurrentRail = nullptr;
	int32 GrindDirection = 1;
	
	float GrindSpeed = 0.0f;
	
	virtual void ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType) override;
	virtual bool Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType) override;
};

struct FCharacterNetworkMoveDataContainer_JBR : FCharacterNetworkMoveDataContainer
{
	typedef FCharacterNetworkMoveDataContainer Super;

	FCharacterNetworkMoveDataContainer_JBR();

    FCharacterNetworkMoveData_JBR CustomMoveData[3]; 
};

USTRUCT()
struct FGrindData
{
	GENERATED_BODY()

	UPROPERTY()
	AGrindRail* CurrentRail = nullptr;

	UPROPERTY()
	AGrindRail* LastRail = nullptr;

	UPROPERTY()
	float CurrentDistance = 0.f;

	UPROPERTY()
	float EnterSpeed = 0.f;

	UPROPERTY()
	float CurrentSpeed = 0.f;
	
	UPROPERTY()
	FRotator TargetRotationRail;

	UPROPERTY()
	int32 GrindDirection = 1; // 1 or -1
};

UENUM(BlueprintType)
enum ECustomMovementMode : uint8
{
	CMOVE_None = 0 UMETA(Hidden),
	CMOVE_Grinding = 1 UMETA(DisplayName = "Grinding"),
	CMOVE_MAX UMETA(Hidden),
};

#pragma endregion Structs

#pragma region Saved Moves

class FSavedMove_JBR : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;
	
	uint8 bSavedWantsToBrake : 1;
	uint8 bSavedWantsToDrift : 1;

	uint8 bSavedWantsToJump : 1;
	uint8 bSavedJumpConsumed : 1;
	
	// Send the timer so the server knows how much buffer is left
	float SavedJumpBufferTime;
	
	float SavedMoveRightInput = 0.0f;
	float SavedMoveUpInput = 0.0f;
	
	float SavedGravityScale = 1.0f;
	float SavedAirFriction = 0.f;
	
	uint8 bSavedWantsToDash : 1; 
	
	float SavedRailDistance = 0.0f;
	int32 SavedGrindDirection = 1;
	TWeakObjectPtr<AGrindRail> SavedRail = nullptr;
	float SavedGrindSpeed = 0.0f;
	
	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	virtual void PrepMoveFor(ACharacter* C) override;
};

class FNetworkPredictionData_Client_JBR : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_JBR(const UCharacterMovementComponent& ClientMovement);
	virtual FSavedMovePtr AllocateNewMove() override;
};

#pragma endregion Saved Moves

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class JUNKBOARDRUSH_API UJBRCustomCMC : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
	
public:
	UJBRCustomCMC();

    virtual void InitializeComponent() override;
    virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel) override;

	virtual void BeginPlay() override;

#pragma region Movement
	
	UFUNCTION(BlueprintCallable)
	void TryMoveFromBP(bool IsMoving);

	UPROPERTY(BlueprintReadOnly)
	float MoveRightInput = 0.0f;

#pragma region Lateral

	UFUNCTION(BlueprintCallable)
	void SetMoveRightInput(float Scalar);

	float GetTurnAmount() const;

#pragma endregion Lateral

#pragma region Up

	UFUNCTION(BlueprintCallable)
	void SetMoveUpInput(float Scalar);
	
	UPROPERTY(BlueprintReadOnly)
	float MoveUpInput = 0.0f;

#pragma endregion Up

#pragma region Rotation
	
	void RotationHoverboard(float DeltaTime);
	UFUNCTION(BlueprintCallable)
	void StopHoverboardRotation();
	UFUNCTION(Server, Reliable)
	void ServerStopHoverboardRotation(); //TODO: Maybe remove this since we use NetworkMoveData now

	FOnLateralMovementStart OnLateralMovementStart;
	FOnLateralMovementEnd OnLateralMovementEnd;

	bool bHasResetMoveRightInput;

	bool IsLateralMoveEnough();

	FVector CachedPreCollisionVelocity;

#pragma endregion Rotation

    // Networking overrides
    virtual void UpdateFromCompressedFlags(uint8 Flags) override;
    virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void SmoothClientPosition(float DeltaTime) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

#pragma region Brake & Drift

	UFUNCTION(BlueprintCallable)
	void SetBrakeInput(bool IsBraking);
	UFUNCTION(BlueprintCallable)
	void TryToDrift(bool IsDrifting);
	
	// Variables for SavedMoves
	UPROPERTY(BlueprintReadOnly)
	bool bWantsToBrake = false;
	bool bWantsToDrift = false;
	bool bOldWantsToDrift = false;

	UPROPERTY(BlueprintAssignable)
	FOnDriftEnter OnDriftEnter;
	UPROPERTY(BlueprintAssignable)
	FOnDriftRelease OnDriftRelease;
	UPROPERTY(BlueprintAssignable)
	FOnDriftReleasePoor OnDriftReleasePoor;
	UPROPERTY(BlueprintAssignable)
	FOnDriftRelease OnDriftCancel;
	
	bool bDriftStarted = false;
	bool bDriftEnded = false;

	float ElapsedTimeBrake = 0.f;
	void TryComputeBrakeElapsed(float DeltaTime);
	
#pragma endregion Brake & Drift
	
#pragma endregion Movement

#pragma region Drift
	
	float ElapsedTimeDrift = 0.f;
	bool bIsInSmallDrift = false;
	bool bIsInBigDrift = false;

	float CachedDriftInitVelocitySizeXY;

	float LastBoostFromDrift;

	void TryUpdateDriftStates(float DeltaTime);
	void ResetDriftBoostStates();

	UPROPERTY(BlueprintAssignable)
	FOnSmallDriftEnter OnSmallDriftEnter;
	UPROPERTY(BlueprintAssignable)
	FOnBigDriftEnter OnBigDriftEnter;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsDrifting();

#pragma endregion Drift

#pragma region JumpLogic

	UFUNCTION(BlueprintCallable)
	void StartJumpCharge();

	UFUNCTION(BlueprintCallable)
	void EndJumpCharge();
	
    // Variables for SavedMoves
	bool bWantsToJump = false;
	bool bJumpConsumed = false;
	bool bIsHoldingJump = false;
	float CurrentJumpHoldTime = 0.f;
	float JumpStartVelocityZ = 0.f;
    
    // Coyote Time
    float RemainingJumpBufferTime = 0.0f;

    // Slope State
    bool bWasPreviouslyEjectedFromSlope = false;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "CMC - Jump")
    FOnChargeJumpSignature OnFirstChargeJump;
    
    // Helper
    void ProcessJump(float DeltaTime);
    void CheckSlopeEjection(float DeltaTime);
    bool CanCmcJump() const;

#pragma endregion JumpLogic

#pragma region Physic
	
	virtual void PerformMovement(float DeltaTime) override;
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;

	virtual void PhysFalling(float DeltaTime, int32 Iterations) override;

	UPROPERTY(BlueprintReadOnly)
	float AirFriction;
	void HandleAirFriction();
	
	void TryApplyAirFriction();
	void TryApplySpeedDebuff();
	void TryApplyInstabilityBoost(float DeltaTime);

	UPROPERTY(BlueprintReadOnly)
	float LastFallingZVelocity = 0.f;

	virtual float GetMaxSpeed() const override;

	void ResetDriftState();
	void PerformEndDrift();

	float HandleFriction() const;
	void TryApplyBrake();
	
	float GetScalarMoveSpeed() const;

	virtual bool IsMovingOnGround() const override;

	UFUNCTION()
	void OnLand();

	UFUNCTION() void HandleAbsoluteMaxSpeed(float DeltaTime);
	UFUNCTION(BlueprintCallable, BlueprintPure) float GetAbsoluteMaxSpeed() const;

#pragma endregion Physic

#pragma region Speed Boost

	UFUNCTION() void ForceVelocityForward();
	UFUNCTION() void AddSpeedBoost(float Amount, bool TakeVelocityDir = false, bool ReplaceVelocity = false);
	UFUNCTION(BlueprintCallable) void AddSpeedDebuff(float Amount);

	UPROPERTY(BlueprintReadOnly)
	float CurrentSpeedDebuff;

	UPROPERTY(BlueprintAssignable)
	FOnSpeedBoostAdded OnSpeedBoostAdded;
	
	UFUNCTION(Client, Unreliable)
	void Client_SpeedBoostDelegateCall(float Amount);
	
#pragma endregion Speed Boost
	
#pragma region Infos
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsMovingForward = false;
	
	UPROPERTY(BlueprintReadWrite)
	float VisualOnlyCurrentTurnInput = 0.0f;
	float VisualOnlyMaxAngleSlide = -45.0f;

	FRotator VisualOnlyLastSentRotation = FRotator::ZeroRotator;
	FRotator VisualOnlyDesiredRotation = FRotator::ZeroRotator;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector OldDetectedSlopeNormal;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector DetectedSlopeNormal;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetSlopeAngle(const FVector& Normal) const;
	UFUNCTION(BlueprintCallable)
	void ComputeFinalSlopeNormal(const TArray<USceneComponent*> TargetComponents);
	FVector ComputeSlopeNormal(float DetectionLength, const USceneComponent* TargetComponent) const;

#pragma endregion Infos
	
#pragma region Dash
	
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	
	void StartDash(float Force);
	
	// This is the input flag driven by DashComponent, main thing that triggers a dash
	bool bWantsToDash = false; 

	// Used to configure the next dash (set by component before setting bWantsToDash)
	// Necessary to separate
	UPROPERTY(Transient)
	float PendingDashForce;

	float CurrentDashForce = 3000.0f;

	bool bIsDashing;
	
	FTimerHandle DashDelegateTimer;

	void RequestDash(float Force);
	void EndDash();
	
#pragma endregion Dash
	
#pragma region Rails
	
	UFUNCTION(BlueprintCallable)
	void EnterGrind(AGrindRail* NewRail, float EnterVelocityXY, float StartDistance, int32 Direction);

	UFUNCTION(BlueprintCallable)
	void ExitGrind();

	UFUNCTION(BlueprintCallable)
	void ExitGrindFromInput(float Scalar);

	UFUNCTION()
	void ForceUpdateGrindSpeed(float OverrideAmount = 0.0f);
	
	void TryClearLastRail();
	void ClearOldRail();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsGrinding() const;
	
	UPROPERTY(Transient, Replicated)
	FGrindData GrindState;

	UPROPERTY(BlueprintAssignable)
	FOnRailBegin OnRailBegin;
	UPROPERTY(BlueprintAssignable)
	FOnRailBegin OnRailEnd;
	
#pragma endregion Rails

#pragma region Slope

	UPROPERTY(BlueprintReadOnly)
	float SlopeSpeedBoost = 0.f;

	void ComputeSlopeSpeedBoost();
	
#pragma endregion Slope

#pragma region Air Gliding

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsAirGliding();

	float GetAirGlidingAirFriction() const;
	float GetAirGlidingGravity() const;
	
#pragma endregion Air Gliding

protected:
    UPROPERTY(Transient)
    AJBRCharacter* JBRCharacterOwner;
	
	// Similar to the dash physic but for grinding
	void PhysGrind(float DeltaTime, int32 Iterations);

	FTimerHandle ClearLastRailTimerHandle;
	float ClearRailDelay = 0.2f;
	
	UFUNCTION()
	void ResetCmcStateOnRespawn();
	
	UFUNCTION(Client, Reliable)
	void ClientResetCmcStateOnRespawn();
	
	mutable FCharacterNetworkMoveDataContainer_JBR NetworkMoveDataContainer;
	
	const FPlayerCMCData& GetPlayerData() const;
	const FPlayerSpeedData& GetPlayerSpeedData() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};