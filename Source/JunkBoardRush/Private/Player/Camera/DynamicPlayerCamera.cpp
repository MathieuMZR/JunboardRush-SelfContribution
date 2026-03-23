
#include "Player/Camera/DynamicPlayerCamera.h"
#include "Libraries/GenericFunction.h"
#include "Framework/JBRCharacter.h"
#include "Framework/JBRCustomCMC.h"
#include "Framework/JBRGameMode.h"
#include "Kismet/KismetMathLibrary.h"

UDynamicPlayerCamera::UDynamicPlayerCamera()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDynamicPlayerCamera::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<AJBRCharacter>(GetOwner());
	Character->OnLanded.AddDynamic(this, &UDynamicPlayerCamera::TryEndFallLogicExecution);
	Character->CustomCMC->OnRailBegin.AddDynamic(this, &UDynamicPlayerCamera::TryEndFallLogicExecutionFromRail);
	
	CameraComponent = Character->Camera;
	SpringArm = Character->SpringArm;
	
	GS = GetWorld()->GetGameState<AJBRGameState>();

	InitDefaultValues();
}

void UDynamicPlayerCamera::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AutoCameraBehavior(DeltaTime);
}

void UDynamicPlayerCamera::AutoCameraBehavior(float DeltaTime)
{
	ComputeSocketOffset(DeltaTime);
	SpringArm->SocketOffset = SocketOffset;

	ComputeArmRotation(DeltaTime);

	if (Character && Character->GetController())
	{
		FRotator NewRotation = FRotator(0.0f);
		
		if (IsReceivingLookBehindInput || IsReceivingLookRelicInput)
		{
			NewRotation = ArmRotation;
		}
		else
		{
			NewRotation = FMath::RInterpTo(
			Character->GetController()->GetControlRotation(),
			ArmRotation,
			DeltaTime,
			GetLerpSpeed()
		);
		}

		Character->GetController()->SetControlRotation(NewRotation);
	}

	ComputeRotationLag(DeltaTime);
	SpringArm->CameraRotationLagSpeed = RotationLag;

	ComputePositionLag(DeltaTime);
	SpringArm->CameraLagSpeed = PositionLag;

	ComputeArmLength(DeltaTime);
	SpringArm->TargetArmLength = ArmLength;

	ComputeFOV(DeltaTime);
	//BumpFOVLogic(DeltaTime);

	ComputeCameraRotation(DeltaTime);

	FRotator CamRot = FMath::RInterpTo(
		CameraComponent->GetRelativeRotation(),
		CameraRotation,
		DeltaTime,
		GetLerpSpeed() * GetCameraData().CameraYawTurnAngleUpdateSpeed
	);

	CameraComponent->SetRelativeRotation(CamRot);
}

#pragma region Init

void UDynamicPlayerCamera::InitDefaultValues()
{
	BaseSocketOffset = SpringArm->SocketOffset;
	BaseTargetOffset = SpringArm->TargetOffset;
}

#pragma endregion Init

#pragma region Offsets

void UDynamicPlayerCamera::ComputeSocketOffset(float DeltaTime)
{
	float InputRatio = Character->CustomCMC->VisualOnlyCurrentTurnInput;
	float BaseYValue = BaseSocketOffset.Y;
	float BaseComputedYValue = InputRatio * BaseYValue;
	float TurnOffset = InputRatio * GetCameraData().SocketOffsetXTurn;

	float SpeedFactor = 1.f;

	if (Character->CustomCMC->bWantsToDrift)
	{
		TurnOffset = 0.f;
		SpeedFactor = 2.f;
	}

	FVector TargetSocketOffset (
		BaseSocketOffset.X,
		BaseComputedYValue + TurnOffset,
		BaseSocketOffset.Z
	);

	SocketOffset = FMath::VInterpTo(
		SocketOffset,
		TargetSocketOffset,
		DeltaTime,
		GetCameraData().SocketOffsetXTurnUpdateSpeed * SpeedFactor
	);
}

void UDynamicPlayerCamera::ComputeTargetOffset(float DeltaTime)
{
	
}

#pragma endregion Offsets

#pragma region Rotations

void UDynamicPlayerCamera::ComputeArmRotation(float DeltaTime)
{
	if (!Character) return;

	SetYaw(DeltaTime);
	SetPitch(DeltaTime);
	SetRoll(DeltaTime);

	if (IsReceivingLookRelicInput)
	{
		if (!GS) return;

		ArmRotation = UKismetMathLibrary::FindLookAtRotation(
			Character->GetActorLocation(),
			GS->RelicGlobalReference->GetActorLocation()
		);
	}
	else if (IsReceivingLookBehindInput)
	{
		ArmRotation = UKismetMathLibrary::FindLookAtRotation(
			Character->GetActorLocation(),
			Character->GetActorLocation() - Character->GetActorForwardVector() * 100.f);
	}
	else
	{
		ArmRotation = FRotator(ArmAnglePitch, ArmAngleYaw, ArmAngleRoll);
	}
}

void UDynamicPlayerCamera::ComputeCameraRotation(float DeltaTime)
{
	float InputRatio = Character->CustomCMC->VisualOnlyCurrentTurnInput;
	float VelRatio = Character->GetSpeedRatioFromAbsoluteVelocity();

	CameraAngleYaw = InputRatio * GetCameraData().CameraYawTurnAngle * VelRatio;

	CameraRotation = FRotator(0.f, CameraAngleYaw, 0.f);
}

void UDynamicPlayerCamera::ResetToDefaultArmRotation()
{
	float VelRatio = Character->GetSpeedRatioFromAbsoluteVelocity();
	float VelAdditionalPitch = FMath::Lerp(0, GetCameraData().PitchMaxSpeedRotation, VelRatio);
	float DefaultPitch = FMath::Lerp(GetCameraData().MinMaxDefaultPitchArmRotation.X, GetCameraData().MinMaxDefaultPitchArmRotation.Y, VelRatio);
	ArmAnglePitch = DefaultPitch + VelAdditionalPitch;

	if (Character != nullptr && Character->GetController() != nullptr)
	{
		FRotator CharacterRotation = Character->GetActorRotation(); // Include character yaw rotation
		Character->GetController()->SetControlRotation(FRotator(ArmAnglePitch, CharacterRotation.Yaw, ArmAngleRoll));
	}
}

void UDynamicPlayerCamera::SetYaw(float DeltaTime)
{
	const float InputRatio = Character->CustomCMC->VisualOnlyCurrentTurnInput;

	float YawFromInput = InputRatio * GetCameraData().YawTurnArmAngle;

	if (IsReceivingLookRightInput)
	{
		YawFromInput = CamXYInput.X * GetCameraData().FreeModeMaxYawAngle;
	}
	else if (IsReceivingLookUpInput)
	{
		YawFromInput = 0.f;
	}

	const float BehindYaw = IsReceivingLookBehindInput ? 180.f : 0.f;

	ArmAngleYaw =
		YawFromInput +
		BehindYaw +
		Character->GetActorRotation().Yaw;
}

void UDynamicPlayerCamera::SetPitch(float DeltaTime)
{
	const float VelRatio = Character->GetSpeedRatioFromAbsoluteVelocity();

	// GROUND
	const FVector TraceStart = Character->GetActorLocation();
	const FVector TraceEnd = TraceStart - FVector(0.f, 0.f, 100000.f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, TraceStart, TraceEnd, ECC_Visibility, Params
	);

	const float Distance = bHit ? Hit.Distance : 100000.f;

	const bool bCanFall =
		Distance >= GetCameraData().MinDistanceFromGroundForFallingState &&
		Character->CustomCMC->IsFalling() &&
		Character->CustomCMC->Velocity.Z < 0.f;

	// FALL
	if (bCanFall)
	{
		bHasAppliedFallLogic = true;

		ArmAnglePitch = FMath::FInterpTo(
			ArmAnglePitch,
			-GetCameraData().PitchFallingMaxArmAngle,
			DeltaTime,
			GetCameraData().PitchFallingArmAngleSpeed
		);
		return;
	}

	if (bHasAppliedFallLogic)
	{
		return;
	}

	// MANUAL
	float PitchFromInput = 0.f;
	if (!IsReceivingLookRightInput && IsReceivingLookUpInput)
	{
		PitchFromInput = -CamXYInput.Y * GetCameraData().FreeModeMaxPitchAngle;
	}

	// ---- Velocity-based pitch
	const float VelPitch = FMath::Lerp(
		0.f,
		GetCameraData().PitchMaxSpeedRotation,
		VelRatio
	);

	const float DefaultPitch = FMath::Lerp(
		GetCameraData().MinMaxDefaultPitchArmRotation.X,
		GetCameraData().MinMaxDefaultPitchArmRotation.Y,
		VelRatio
	);

	ArmAnglePitch = DefaultPitch + VelPitch + PitchFromInput;
}

void UDynamicPlayerCamera::SetRoll(float DeltaTime)
{
	const float InputRatio = Character->CustomCMC->VisualOnlyCurrentTurnInput;

	const bool bDisableRoll =
		IsReceivingLookRightInput ||
		IsReceivingLookUpInput ||
		IsReceivingLookBehindInput;

	ArmAngleRoll = bDisableRoll
		? 0.f
		: InputRatio * GetCameraData().RollTurnArmAngle;
}

#pragma endregion Rotations

#pragma region Lag

void UDynamicPlayerCamera::ComputePositionLag(float DeltaTime)
{
	float Target = GetCameraData().DefaultPositionLag;

	if (IsReceivingLookRelicInput)
		Target = GetCameraData().RelicPositionLag;
	else if (IsReceivingLookBehindInput)
		Target = GetCameraData().BehindPositionLag;
	else if (IsReceivingLookRightInput || IsReceivingLookUpInput)
		Target = GetCameraData().FreeModePositionLag;
	else if (IsCamFalling())
		Target = GetCameraData().FallingPositionLag;
	else if (Character->CustomCMC->IsGrinding())
		Target = GetCameraData().RailPositionLag;
	else if (Character->CustomCMC->bWantsToDrift)
		Target = GetCameraData().DriftPositionLag;

	float LerpSpeed = GetLerpSpeed();

	LerpSpeed *= ComputePositionLagBoostMultiplier(DeltaTime);

	PositionLag = FMath::FInterpTo(
		PositionLag,
		Target,
		DeltaTime,
		LerpSpeed
	);
}

void UDynamicPlayerCamera::ComputeRotationLag(float DeltaTime)
{
	float Target = GetCameraData().DefaultRotationLag;

	if (IsReceivingLookRelicInput)
		Target = GetCameraData().RelicRotationLag;
	else if (IsReceivingLookBehindInput)
		Target = GetCameraData().BehindRotationLag;
	else if (IsReceivingLookRightInput || IsReceivingLookUpInput)
		Target = GetCameraData().FreeModeRotationLag;
	else if (IsCamFalling())
		Target = GetCameraData().FallingRotationLag;
	else if (Character->CustomCMC->IsGrinding())
		Target = GetCameraData().RailRotationLag;
	else if (Character->CustomCMC->bWantsToDrift)
		Target = GetCameraData().DriftRotationLag;

	float LerpSpeed = GetLerpSpeed();

	LerpSpeed *= ComputeRotationLagBoostMultiplier(DeltaTime);

	RotationLag = FMath::FInterpTo(
		RotationLag,
		Target,
		DeltaTime,
		LerpSpeed
	);
}

void UDynamicPlayerCamera::AddPositionLagBoost(float Multiplier, float Duration)
{
	FCameraLerpBoost& Boost = PositionLagBoosts.AddDefaulted_GetRef();
	Boost.Start(Multiplier, Duration);
}

float UDynamicPlayerCamera::ComputePositionLagBoostMultiplier(float DeltaTime)
{
	float FinalMultiplier = 1.f;

	for (int32 i = PositionLagBoosts.Num() - 1; i >= 0; --i)
	{
		FCameraLerpBoost& Boost = PositionLagBoosts[i];
		Boost.Tick(DeltaTime);

		const float Alpha = Boost.GetAlpha();

		// Ease-out decay
		const float EasedAlpha = FMath::InterpEaseOut(0.f, 1.f, Alpha, 2.f);

		FinalMultiplier *= FMath::Lerp(1.f, Boost.Multiplier, EasedAlpha);

		if (!Boost.IsActive())
		{
			PositionLagBoosts.RemoveAtSwap(i);
		}
	}

	return FinalMultiplier;
}

void UDynamicPlayerCamera::AddRotationLagBoost(float Multiplier, float Duration)
{
	FCameraLerpBoost& Boost = RotationLagBoosts.AddDefaulted_GetRef();
	Boost.Start(Multiplier, Duration);
}

float UDynamicPlayerCamera::ComputeRotationLagBoostMultiplier(float DeltaTime)
{
	float FinalMultiplier = 1.f;

	for (int32 i = RotationLagBoosts.Num() - 1; i >= 0; --i)
	{
		FCameraLerpBoost& Boost = RotationLagBoosts[i];
		Boost.Tick(DeltaTime);

		const float Alpha = Boost.GetAlpha();

		// Ease-out decay
		const float EasedAlpha = FMath::InterpEaseOut(0.f, 1.f, Alpha, 2.f);

		FinalMultiplier *= FMath::Lerp(1.f, Boost.Multiplier, EasedAlpha);

		if (!Boost.IsActive())
		{
			RotationLagBoosts.RemoveAtSwap(i);
		}
	}

	return FinalMultiplier;
}

#pragma endregion Lag

#pragma region Length

void UDynamicPlayerCamera::ComputeArmLength(float DeltaTime)
{
	float VelRatio = Character->GetSpeedRatioFromAbsoluteVelocity();
	float TargetLength = FMath::Lerp(
		GetCameraData().MinMaxArmLengthAlongSpeed.X,
		GetCameraData().MinMaxArmLengthAlongSpeed.Y,
		VelRatio
	);

	ArmLength = FMath::FInterpTo(
		ArmLength,
		TargetLength,
		DeltaTime,
		GetLerpSpeed()
	);
}

#pragma endregion Length

#pragma region FOV

void UDynamicPlayerCamera::ComputeFOV(float DeltaTime)
{
	const float VelRatio = Character->GetSpeedRatioBaseSpeedFromAbsoluteVelocity();

	// --- Derivative kick ---
	const float VelRatioDelta = (VelRatio - PreviousVelRatio) / FMath::Max(DeltaTime, KINDA_SMALL_NUMBER);
	PreviousVelRatio = VelRatio;

	// Only on positive acceleration (boosts, not braking)
	if (VelRatioDelta > 0.f)
		FOVKick += VelRatioDelta * GetCameraData().FOVKickStrength;

	FOVKick = FMath::Clamp(FOVKick, 0.f, GetCameraData().FOVKickMax);
	FOVKick = FMath::FInterpTo(FOVKick, 0.f, DeltaTime, GetCameraData().FOVKickDecaySpeed);
	// ----------------------

	const float DriftFactor = Character->CustomCMC->bWantsToDrift
		? GetCameraData().DriftFOVMultiplier
		: 1.f;

	FromSpeedFOV = FMath::Lerp(
		GetCameraData().MinMaxFOVAlongSpeed.X,
		GetCameraData().MinMaxFOVAlongSpeed.Y,
		VelRatio
	) * DriftFactor + FOVKick;

	DesiredFOV = FMath::FInterpTo(
		DesiredFOV,
		FromSpeedFOV,
		DeltaTime,
		GetCameraData().FOVUpdateSpeed
	);

	const bool bIsReducingFOV = DesiredFOV < CameraComponent->FieldOfView;
	const float DirectionalSpeedMultiplier = bIsReducingFOV
		? GetCameraData().ReducingFOVSpeedMultiplier
		: 1.f;

	CameraComponent->FieldOfView = FMath::FInterpTo(
		CameraComponent->FieldOfView,
		DesiredFOV,
		DeltaTime,
		GetLerpSpeed() * GetCameraData().FOVUpdateSpeed * DirectionalSpeedMultiplier
	);
}

#pragma endregion FOV

#pragma region Fall

void UDynamicPlayerCamera::TryEndFallLogicExecution()
{
	if (!HasLandFromHighFalling()) return;

	// Avoid reset if specific camera move logic
	if (!IsReceivingLookBehindInput && !IsReceivingLookRightInput && !IsReceivingLookRelicInput) ResetToDefaultArmRotation();
	bHasAppliedFallLogic = false;
}

void UDynamicPlayerCamera::TryEndFallLogicExecutionFromRail()
{
	if (!HasLandFromHighFalling()) return;

	// Avoid reset if specific camera move logic
	if (!IsReceivingLookBehindInput && !IsReceivingLookRightInput && !IsReceivingLookRelicInput) ResetToDefaultArmRotation();
	bHasAppliedFallLogic = false;
}

#pragma endregion Fall

#pragma region Check

float UDynamicPlayerCamera::GetLerpSpeed()
{
	float BaseLerpSpeedRelative = FMath::Lerp(GetCameraData().MinMaxSpeedLerpFactor.X,
		GetCameraData().MinMaxSpeedLerpFactor.Y, Character->GetSpeedRatioFromAbsoluteVelocity());
	float DriftBoostReleaseFactor = GetCameraData().DriftReleaseLerpFactorMultiplier;

	// Accelerate the lerp if the recent drift has a significant boost
	float FinalLerpSpeed = BaseLerpSpeedRelative +
		(Character->CustomCMC->bDriftEnded && Character->CustomCMC->LastBoostFromDrift > 0 ? DriftBoostReleaseFactor : 0.f);
	
	return FinalLerpSpeed;
}

bool UDynamicPlayerCamera::IsCamFalling()
{
	return bHasAppliedFallLogic && Character->CustomCMC->IsFalling() && Character->CustomCMC->Velocity.Z < 0.f;
}

bool UDynamicPlayerCamera::HasLandFromHighFalling()
{
	return bHasAppliedFallLogic;
}

#pragma endregion Utilities

#pragma region Utilities

const FCameraData& UDynamicPlayerCamera::GetCameraData() const
{
	return UGenericFunction::GetDataRow<FCameraData>(CameraData, TEXT("CameraData"));
}

void UDynamicPlayerCamera::SetCameraVariablesLook(FVector2D LookInput)
{
	IsReceivingLookRightInput = FMath::Abs(LookInput.X) > 0.75f;
	IsReceivingLookUpInput = FMath::Abs(LookInput.Y) > 0.75f;
	CamXYInput = LookInput;
}

void UDynamicPlayerCamera::SetCameraVariablesLookBehind(bool IsLookBehindInput)
{
	if (IsReceivingLookBehindInput && !IsLookBehindInput)
	{
		AddPositionLagBoost(12.f, .5f);
		AddRotationLagBoost(12.f, .5f);

		ResetToDefaultArmRotation();
	}
	
	IsReceivingLookBehindInput = IsLookBehindInput;
}

void UDynamicPlayerCamera::SetCameraVariablesLookRelic(bool IsLookRelic)
{
	AJBRGameState* GameState = Character ? Character->GetWorld()->GetGameState<AJBRGameState>() : nullptr;
	if (!GameState || !GameState->RelicGlobalReference)
	{
		IsReceivingLookRelicInput = false;
		return;
	}
	
	FGadgetItem RelicItem;
	RelicItem.GadgetClass = ARelic::StaticClass();

	if (Character && Character->CraftingComponent->HasItemTypeInInventory(RelicItem) &&
		GameState->RelicGlobalReference->CurrentState != ERelicState::WaitingForRespawn)
	{
		IsReceivingLookRelicInput = false;
		return;
	}

	if (IsReceivingLookRelicInput && !IsLookRelic)
	{
		AddPositionLagBoost(3.f, .5f);
		AddRotationLagBoost(3.f, .5f);
	}

	IsReceivingLookRelicInput = IsLookRelic;
}

#pragma endregion Utilities