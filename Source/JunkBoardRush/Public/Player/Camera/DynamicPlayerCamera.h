
#pragma once

#include "CoreMinimal.h"
#include "CameraData.h"
#include "FCameraLerpBoost.h"
#include "Components/ActorComponent.h"
#include "DynamicPlayerCamera.generated.h"

class USpringArmComponent;
class AJBRGameState;
class UCameraComponent;
class AJBRCharacter;

#pragma region States and Enums

UENUM(BlueprintType)
enum class DynamicCameraState : uint8 {
	DefaultState = 0 UMETA(DisplayName = "Default State"),
	FreeModeState = 1 UMETA(DisplayName = "Free Mode State"),
	LookBehindState = 2 UMETA(DisplayName = "Look Behind State"),
	RailState = 3 UMETA(DisplayName = "Rail State"),
	DriftState = 4 UMETA(DisplayName = "Drift State"),
	FallingState = 5 UMETA(DisplayName = "Falling State"),
	Relic = 6 UMETA(DisplayName = "Relic State"),
};

#pragma endregion States and Enums

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class JUNKBOARDRUSH_API UDynamicPlayerCamera : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UDynamicPlayerCamera();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma region Data
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FDataTableRowHandle CameraData;

#pragma endregion Data

#pragma region Input
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	bool IsReceivingLookRightInput;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	bool IsReceivingLookUpInput;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	bool IsReceivingLookBehindInput;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	bool IsReceivingLookRelicInput;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	FVector2D CamXYInput;

#pragma endregion Input

#pragma region States
	
	UPROPERTY(BlueprintReadOnly, Category = "States")
	DynamicCameraState CameraState;
	
	UPROPERTY(BlueprintReadOnly, Category = "States")
	DynamicCameraState LastCameraState;

#pragma endregion States
	
#pragma region LogicInit

	UFUNCTION(BlueprintCallable)
	void SetCameraVariablesLook(FVector2D LookInput);
	
	UFUNCTION(BlueprintCallable)
	void SetCameraVariablesLookBehind(bool IsLookBehindInput);
	
	UFUNCTION(BlueprintCallable)
	void SetCameraVariablesLookRelic(bool IsLookRelic);
	
	void InitDefaultValues();

#pragma endregion LogicInit

#pragma region SocketOffset
	
	void ComputeSocketOffset(float DeltaTime);
	void ComputeTargetOffset(float DeltaTime);

#pragma endregion SocketOffset

#pragma region ArmCameraRotation
	
	void ComputeArmRotation(float DeltaTime);
	void ComputeCameraRotation(float DeltaTime);
	void ResetToDefaultArmRotation();

	void SetYaw(float DeltaTime);
	void SetPitch(float DeltaTime);
	void SetRoll(float DeltaTime);

#pragma endregion ArmCameraRotation

#pragma region Lag
	
	void ComputePositionLag(float DeltaTime);
	void ComputeRotationLag(float DeltaTime);

	void AddPositionLagBoost(float Multiplier, float Duration);
	float ComputePositionLagBoostMultiplier(float DeltaTime);

	void AddRotationLagBoost(float Multiplier, float Duration);
	float ComputeRotationLagBoostMultiplier(float DeltaTime);

#pragma endregion Lag

#pragma region Length
	
	void ComputeArmLength(float DeltaTime);

#pragma endregion Length

#pragma region FOV
	
	void ComputeFOV(float DeltaTime);

#pragma endregion FOV

#pragma region Logic

	void AutoCameraBehavior(float DeltaTime);

#pragma endregion Logic

#pragma region Lerp
	
	float GetLerpSpeed();

#pragma endregion Lerp

#pragma region Falling
	
	UFUNCTION()
	void TryEndFallLogicExecutionFromRail();
	UFUNCTION()
	void TryEndFallLogicExecution();

	bool IsCamFalling();
	bool HasLandFromHighFalling();

#pragma endregion Falling

#pragma region Data
	
	const FCameraData& GetCameraData() const;

#pragma endregion Data

	FVector BaseSocketOffset, BaseTargetOffset, SocketOffset, TargetOffset;
	
	float ArmAngleRoll, ArmAnglePitch, ArmAngleYaw;
	FRotator ArmRotation;
	
	float CameraAngleRoll, CameraAnglePitch, CameraAngleYaw;
	FRotator CameraRotation;
	
	float PositionLag, RotationLag;
	float ArmLength;
	float DesiredFOV;

	float CachedPositionLag;
	float CachedRotationLag;

	bool bHasAppliedFallLogic;

	float FromSpeedFOV = 90.f;

private:

	AJBRCharacter* Character;
	UCameraComponent* CameraComponent;
	USpringArmComponent* SpringArm;
	AJBRGameState* GS;

	UPROPERTY()
	TArray<FCameraLerpBoost> PositionLagBoosts;
	TArray<FCameraLerpBoost> RotationLagBoosts;
	
	float PreviousVelRatio = 0.f;
	float FOVKick = 0.f;
};
