#include "Framework/JBRCustomCMC.h"

#include <string>

#include "Components/CapsuleComponent.h"
#include "Framework/JBRCharacter.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "Game/Rails/GrindRail.h"
#include "Framework/JBRGameState.h"
#include "Player/Camera/PlayerCameraShakeComponent.h"
#include "Player/Hoverboard/Hoverboard.h"
#include "Sound/SoundManager.h"

#pragma region Saved Moves

FCharacterNetworkMoveDataContainer_JBR::FCharacterNetworkMoveDataContainer_JBR()
{
	NewMoveData = &CustomMoveData[0];
	PendingMoveData = &CustomMoveData[1];
	OldMoveData = &CustomMoveData[2];
}

void FCharacterNetworkMoveData_JBR::ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(ClientMove, MoveType);

	const FSavedMove_JBR* SavedMove = static_cast<const FSavedMove_JBR*>(&ClientMove);
    
	MoveRightInput = SavedMove->SavedMoveRightInput;
	MoveUpInput = SavedMove->SavedMoveUpInput;
	
	GrindDirection = SavedMove->SavedGrindDirection;
	RailDistance = SavedMove->SavedRailDistance;
	CurrentRail = SavedMove->SavedRail.Get();
	GrindSpeed = SavedMove->SavedGrindSpeed;
	
	GravityScale = SavedMove->SavedGravityScale;
	AirFriction = SavedMove->SavedAirFriction;
}

bool FCharacterNetworkMoveData_JBR::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar,
	UPackageMap* PackageMap, ENetworkMoveType MoveType)
{
	Super::Serialize(CharacterMovement, Ar, PackageMap, MoveType);
	
	SerializeOptionalValue<float>(Ar.IsSaving(), Ar, MoveRightInput, 0.0f);
	SerializeOptionalValue<float>(Ar.IsSaving(), Ar, MoveUpInput, 0.0f);
	
	SerializeOptionalValue<int32>(Ar.IsSaving(), Ar, GrindDirection, 1);
	SerializeOptionalValue<float>(Ar.IsSaving(), Ar, RailDistance, 0.0f);
	SerializeOptionalValue<float>(Ar.IsSaving(), Ar, GrindSpeed, 0.0f);
	
	SerializeOptionalValue<float>(Ar.IsSaving(), Ar, GravityScale, 1.0f);
    
	UObject* RailObj = CurrentRail;
	bool bHasRail = (RailObj != nullptr);
	Ar.SerializeBits(&bHasRail, 1);
	if (bHasRail)
	{
		Ar << RailObj;
		CurrentRail = Cast<AGrindRail>(RailObj);
	}
	else
	{
		CurrentRail = nullptr;
	}

	return !Ar.IsError();
}

FSavedMovePtr FNetworkPredictionData_Client_JBR::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_JBR());
}

void FSavedMove_JBR::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);
	UJBRCustomCMC* CMC = Cast<UJBRCustomCMC>(C->GetCharacterMovement());
	
	if (CMC)
	{
		SavedMoveRightInput = CMC->MoveRightInput;
		SavedMoveUpInput = CMC->MoveUpInput;

		bSavedWantsToJump = CMC->bWantsToJump;
		SavedJumpBufferTime = CMC->RemainingJumpBufferTime;
		bSavedJumpConsumed = CMC->bJumpConsumed;
		
		bSavedWantsToDash = CMC->bWantsToDash;

		bSavedWantsToBrake = CMC->bWantsToBrake;
		bSavedWantsToDrift = CMC->bWantsToDrift;
		
		SavedRailDistance = CMC->GrindState.CurrentDistance;
		SavedGrindDirection = CMC->GrindState.GrindDirection;
		SavedRail = CMC->GrindState.CurrentRail;
		SavedGrindSpeed = CMC->GrindState.CurrentSpeed;
		
		SavedGravityScale = CMC->GravityScale;
		SavedAirFriction = CMC->AirFriction;
	}
}

void FSavedMove_JBR::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);
	UJBRCustomCMC* CMC = Cast<UJBRCustomCMC>(C->GetCharacterMovement());

	if (CMC)
	{
		CMC->MoveRightInput = SavedMoveRightInput;
		CMC->MoveUpInput = SavedMoveUpInput;

		CMC->bWantsToJump = bSavedWantsToJump;
		CMC->RemainingJumpBufferTime = SavedJumpBufferTime;
		CMC->bJumpConsumed = bSavedJumpConsumed;
		
		CMC->VisualOnlyCurrentTurnInput = SavedMoveRightInput;

		CMC->bWantsToDash = bSavedWantsToDash;

		CMC->bWantsToBrake = bSavedWantsToBrake;
		CMC->bWantsToDrift = bSavedWantsToDrift;
		
		CMC->GravityScale = SavedGravityScale;
		CMC->AirFriction = SavedAirFriction;

		if (SavedRail.IsValid())
		{
			// During replay (pred), we do want to snap to specific saved positions
			// This is distinct from MoveAutonomous (serv only exec)
			if (CMC->GrindState.CurrentRail != SavedRail.Get())
			{
				CMC->EnterGrind(SavedRail.Get(), SavedGrindSpeed, SavedRailDistance, SavedGrindDirection);
			}
			else
			{
				CMC->GrindState.CurrentDistance = SavedRailDistance;
				CMC->GrindState.GrindDirection = SavedGrindDirection;
				CMC->GrindState.CurrentSpeed = SavedGrindSpeed;
			}
		}
		else if (CMC->IsGrinding())
		{
			CMC->ExitGrind();
		}
	}
}

FNetworkPredictionData_Client_JBR::FNetworkPredictionData_Client_JBR(const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{
}

bool FSavedMove_JBR::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	FSavedMove_JBR* NewJBRMove = static_cast<FSavedMove_JBR*>(NewMove.Get());
	
	if (bSavedWantsToJump != NewJBRMove->bSavedWantsToJump) return false;
	if (bSavedJumpConsumed != NewJBRMove->bSavedJumpConsumed) return false;

	if (bSavedWantsToBrake != NewJBRMove->bSavedWantsToBrake) return false;
	if (bSavedWantsToDrift != NewJBRMove->bSavedWantsToDrift) return false;

	if (FMath::Abs(SavedMoveRightInput - NewJBRMove->SavedMoveRightInput) > 0.01f) return false;

	if (bSavedWantsToDash != NewJBRMove->bSavedWantsToDash) return false;

	// We cannot combine moves if rail state changes, or precision will result in desync
	if (SavedRail != NewJBRMove->SavedRail) return false;
	if (SavedGrindDirection != NewJBRMove->SavedGrindDirection) return false;

	if (FMath::Abs(SavedAirFriction - NewJBRMove->SavedAirFriction) > 0.01f) return false;

	// Avoid if speed changed a lot, to avoid combining buffer frames where physics varied a lot
	if (FMath::Abs(SavedGrindSpeed - NewJBRMove->SavedGrindSpeed) > 10.0f) return false;
	
	if (FMath::Abs(SavedGravityScale - NewJBRMove->SavedGravityScale) > 0.01f) return false;
	if (FMath::Abs(SavedAirFriction - NewJBRMove->SavedAirFriction) > 0.01f) return false;

	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_JBR::Clear()
{
	Super::Clear();
	
	bSavedWantsToJump = false;
	SavedJumpBufferTime = 0.0f;
	bSavedJumpConsumed = false;
	
	bSavedWantsToBrake = false;
	bSavedWantsToDrift = false;
	
	SavedMoveRightInput = 0.0f;
	SavedMoveUpInput = 0.0f;
	
	bSavedWantsToDash = false;
	
	SavedRailDistance = 0.0f;
	SavedGrindDirection = 1;
	SavedRail = nullptr;
	SavedGrindSpeed = 0.0f;
	
	SavedGravityScale = 4.0f;
	SavedAirFriction = 0.f;
}

uint8 FSavedMove_JBR::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	
	if (bSavedWantsToJump) Result |= FLAG_Custom_0;
	if (bSavedWantsToDash) Result |= FLAG_Custom_1; 
	if (bSavedWantsToDrift) Result |= FLAG_Custom_2; 
	if (bSavedWantsToBrake) Result |= FLAG_Custom_3;
	if (bSavedJumpConsumed) Result |= FLAG_Reserved_1;
	
	return Result;
}

#pragma endregion Saved Moves

// CMC

#pragma region Init

UJBRCustomCMC::UJBRCustomCMC()
{
	bUseControllerDesiredRotation = false; 
	bOrientRotationToMovement = false; 
}

void UJBRCustomCMC::InitializeComponent()
{
	Super::InitializeComponent();
	JBRCharacterOwner = Cast<AJBRCharacter>(GetOwner());
	
	SetNetworkMoveDataContainer(NetworkMoveDataContainer);
}

void UJBRCustomCMC::BeginPlay()
{
	Super::BeginPlay();

	MaxWalkSpeed = JBRCharacterOwner->GetPlayerSpeedData().BaseMaxWalkSpeed;
	
	JBRCharacterOwner->OnLanded.AddDynamic(this, &UJBRCustomCMC::OnLand);
	
	JBRCharacterOwner->HealthComponent->OnDied.AddDynamic(this, &UJBRCustomCMC::ResetCmcStateOnRespawn);
	JBRCharacterOwner->HealthComponent->OnSpeedDebuffRequested.AddDynamic(this, &UJBRCustomCMC::AddSpeedDebuff);

	bJumpConsumed = false;
}

#pragma endregion Init

#pragma region Inputs

void UJBRCustomCMC::TryMoveFromBP(bool IsMoving)
{
	bIsMovingForward = IsMoving;
	if (IsMoving) JBRCharacterOwner->AddMovementInput(JBRCharacterOwner->GetActorForwardVector(), GetScalarMoveSpeed());
}

void UJBRCustomCMC::SetBrakeInput(bool IsBraking)
{
	if (!IsMovingOnGround()) return;
	if (IsGrinding()) return;
		
	bWantsToBrake = IsBraking;

	if (!bWantsToBrake) ElapsedTimeBrake = 0.f;
}

void UJBRCustomCMC::TryToDrift(bool IsDrifting)
{
	if (Velocity.Size2D() < GetPlayerData().MinSpeedForDrift && IsDrifting) return;
	if (IsFalling() && IsDrifting) return;

	if (JBRCharacterOwner->bForceDisableDrift) return;

	if (FMath::Abs(MoveRightInput) < GetPlayerData().DriftDeadZone && IsDrifting && bOldWantsToDrift)
	{
		ResetDriftState();
		return;
	}

	bOldWantsToDrift = bWantsToDrift;
	bWantsToDrift = IsDrifting;

	if (!bOldWantsToDrift && bWantsToDrift)
	{
		OnDriftEnter.Broadcast();
		if (!CharacterOwner->HasAuthority()) return;
		CachedDriftInitVelocitySizeXY = Velocity.Size2D();
	}

	if (bOldWantsToDrift && !bWantsToDrift)
	{
		PerformEndDrift();
		bOldWantsToDrift = false;
		OnDriftCancel.Broadcast();
	}
}

void UJBRCustomCMC::StartJumpCharge()
{
	bWantsToJump = true;
}

void UJBRCustomCMC::EndJumpCharge()
{
	bWantsToJump = false;
	bIsHoldingJump = false;
	CurrentJumpHoldTime = 0.f;

	// Only reset the jump ability when on ground
	if (IsMovingOnGround())
	{
		bJumpConsumed = false;
	}
}

// Reminder : when switching from direction, the 0 input is triggered (dead-point)
void UJBRCustomCMC::SetMoveRightInput(float Scalar)
{
	VisualOnlyCurrentTurnInput = Scalar;
	MoveRightInput = Scalar;

	if (!IsLateralMoveEnough() && !bHasResetMoveRightInput)
	{
		OnLateralMovementEnd.Broadcast();
		bHasResetMoveRightInput = true;
	}
	else if (IsLateralMoveEnough() && bHasResetMoveRightInput)
	{
		OnLateralMovementStart.Broadcast();
		bHasResetMoveRightInput = false;
	}
}

void UJBRCustomCMC::SetMoveUpInput(float Scalar)
{
	MoveUpInput = Scalar;
} 

#pragma endregion Inputs

#pragma region NetworkSync

void UJBRCustomCMC::UpdateFromCompressedFlags(uint8 Flags)
{
    Super::UpdateFromCompressedFlags(Flags);

	bOldWantsToDrift = bWantsToDrift;
	
    bWantsToJump = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	
	bWantsToDash = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
	bWantsToDrift = (Flags & FSavedMove_Character::FLAG_Custom_2) != 0;
	bWantsToBrake = (Flags & FSavedMove_Character::FLAG_Custom_3) != 0;
	
	bJumpConsumed = (Flags & FSavedMove_Character::FLAG_Reserved_1) != 0;
}

void UJBRCustomCMC::MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel)
{
	// MoveAutonomous runs on serv (processing client moves) and client (replaying pred)
    const FCharacterNetworkMoveData_JBR* MoveData = static_cast<const FCharacterNetworkMoveData_JBR*>(GetCurrentNetworkMoveData());
	
    if (MoveData)
    {
        // Alaways apply inputs
        MoveRightInput = MoveData->MoveRightInput;
    	MoveUpInput = MoveData->MoveUpInput;
    	
        VisualOnlyCurrentTurnInput = MoveData->MoveRightInput;
    	
    	GravityScale = MoveData->GravityScale;

        AGrindRail* DataRail = MoveData->CurrentRail;
        bool bDataHasRail = (DataRail != nullptr);
        bool bAmGrinding = IsGrinding();

        // If this desyncs, the character rotation flips 180 degrees and kinda starts to spin
        if (bDataHasRail)
        {
             GrindState.GrindDirection = MoveData->GrindDirection;
        }

        if (bDataHasRail && !bAmGrinding)
        {
            // Client entered a rail, server must enter too
            EnterGrind(DataRail, MoveData->GrindSpeed, MoveData->RailDistance, MoveData->GrindDirection);
        }
        else if (!bDataHasRail && bAmGrinding)
        {
            // Client exited rail
            ExitGrind();
        }
        else if (bDataHasRail && bAmGrinding)
        {
            // Continued Grind
            if (GrindState.CurrentRail != DataRail)
            {
               // Switching rails
               EnterGrind(DataRail, MoveData->GrindSpeed, MoveData->RailDistance, MoveData->GrindDirection);
            }
            else
            {
                // Monitor for massive desyncs
                float DistError = FMath::Abs(GrindState.CurrentDistance - MoveData->RailDistance);
                
                // If error is huge (approx 2 meters), snap to client
                // This prevents the server from simulating the player on a completely different section of the rail
                if (DistError > 200.0f) 
                {
                     GrindState.CurrentDistance = MoveData->RailDistance;
                     ForceUpdateGrindSpeed(MoveData->GrindSpeed);
                }
            }
        }
    }

    Super::MoveAutonomous(ClientTimeStamp, DeltaTime, CompressedFlags, NewAccel);
}

void UJBRCustomCMC::SmoothClientPosition(float DeltaTime)
{
	// Only apply rail smoothing for simulated proxies (other clients viewing this player)
	// Skip for authority (server's own simulation) and AutonomousProxy (owning client)
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		Super::SmoothClientPosition(DeltaTime);
		return;
	}

	if (IsGrinding() && GrindState.CurrentRail && GrindState.CurrentRail->Spline)
	{
		FVector NewLoc = GrindState.CurrentRail->Spline->GetLocationAtDistanceAlongSpline(
			GrindState.CurrentDistance, ESplineCoordinateSpace::World);

		if (CharacterOwner && CharacterOwner->GetCapsuleComponent())
		{
			NewLoc.Z += CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		}

		FVector Tangent = GrindState.CurrentRail->Spline->GetDirectionAtDistanceAlongSpline(GrindState.CurrentDistance, ESplineCoordinateSpace::World);
		Tangent *= GrindState.GrindDirection;
		FRotator NewRot = Tangent.Rotation();

		UpdatedComponent->SetWorldLocationAndRotation(NewLoc, NewRot, false, nullptr, ETeleportType::TeleportPhysics);
		return;
	}

	Super::SmoothClientPosition(DeltaTime);
}

void UJBRCustomCMC::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	// Pre cache velocity before any collision or movement response
	if (MovementMode == MOVE_Walking || MovementMode == MOVE_Falling)
	{
		CachedPreCollisionVelocity = Velocity;
		CachedPreCollisionVelocity.Z = 0.f;
	}
	
    // This whole thing runs on the server and client at the same simulation step, this is the new big function for the main logic
    Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	const FPlayerCMCData& Data = GetPlayerData();
	
	bDriftStarted = !bOldWantsToDrift && bWantsToDrift;
	bDriftEnded = bOldWantsToDrift && !bWantsToDrift;
	
	// CachedDriftInitVelocitySizeXY does not need replication if:
	// it's either derived from simulation state
	// or if it's sampled deterministically
	if (bDriftStarted) CachedDriftInitVelocitySizeXY = OldVelocity.Size2D();
	if (bDriftEnded) PerformEndDrift();
	
	MaxAcceleration = IsFalling() ? GetPlayerData().AccelerationRateInAir :  GetPlayerData().AccelerationRate;

	// Friction
	GroundFriction = HandleFriction();
	HandleAirFriction();

	// DEBUG only
	BrakingDecelerationWalking = JBRCharacterOwner->bForceStopAutoMoveState ? 10000.f : 0.f;
	
	// Hoverboard rotation
	RotationHoverboard(DeltaSeconds);

    // Auto move forward (replaces tick stuff)
    // Applied directly to acceleration / input
    if ((GetPlayerData().AutoMove && !JBRCharacterOwner->bForceStopAutoMoveState) && JBRCharacterOwner)
    {
    	bIsMovingForward = true;
        JBRCharacterOwner->AddMovementInput(JBRCharacterOwner->GetActorForwardVector(), GetScalarMoveSpeed());
    }
	
    // Variable gravity (replaces HandleGravity)
    // GravityZ is global, GravityScale is per character
	float BaseGravity = (IsFalling() ? Data.GravityWhileJumping : Data.Gravity);
	float FinalGravity = IsAirGliding() ? GetAirGlidingGravity() : BaseGravity;
	float GravitySpeed = IsAirGliding() ? Data.GravityUpdateSpeedGliding : Data.GravityUpdateSpeed;
    GravityScale = FMath::Lerp(GravityScale, FinalGravity, DeltaSeconds * GravitySpeed);
	if (bIsDashing) GravityScale = 0.f; // Bypass lerp

    // Air control (replaces HandleAirControl)
    AirControl = Data.AirControlBase;
    AirControlBoostVelocityThreshold = Data.AirControlBoostVelocityThreshold;
    if (GetMaxSpeed() > KINDA_SMALL_NUMBER)
    {
        AirControlBoostMultiplier = FMath::Lerp(Data.AirControlHighMultiplier,Data.AirControlLowMultiplier, JBRCharacterOwner->GetSpeedRatioFromAbsoluteVelocity());
    }

    // Update jump buffer (Coyote Time)
    if (RemainingJumpBufferTime > 0.0f)
    {
        RemainingJumpBufferTime -= DeltaSeconds;
    }

	if (bWantsToDash)
	{
		// Ensure that the player doesn't restart the dash if they're already in the middle of it 
		if (JBRCharacterOwner->DashComponent->CanDash())
		{
			// Trigger the physics setup
			StartDash(PendingDashForce);
		}
        
		// Consume the input flag so that the player doesn't dash forever
		bWantsToDash = false; 
	}

    ProcessJump(DeltaSeconds);
	
    CheckSlopeEjection(DeltaSeconds);
	
	// Draw dash homing range
	//UKismetSystemLibrary::DrawDebugSphere(GetWorld(), JBRCharacterOwner->GetActorLocation(), JBRCharacterOwner->GetPlayerData().DashHomingRange, 6, FLinearColor::Blue, 0.0f, 2.0f);
}

void UJBRCustomCMC::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	// Interrupt Walking behaviors
	if (MovementMode != MOVE_Walking && !IsDrifting())
	{
		ResetDriftState();
	}

	//Skip all movement logic
	//RailSystemComponent handle it
	if (MovementMode == MOVE_Custom || IsGrinding())
	{
		bIsHoldingJump = false;
		CurrentJumpHoldTime = 0.f;
		JumpStartVelocityZ = 0.f;
		
		if (!bWantsToJump)
		{
			bJumpConsumed = false;
		}
		
		return;
	}

	// Leaving the ground
    // Coyote time stuff
    if (PreviousMovementMode == MOVE_Walking && MovementMode == MOVE_Falling)
    {
        // If player didn't jump to get here, start buffer
        if (!bWantsToJump && Velocity.Z <= 0.0f) 
        {
            RemainingJumpBufferTime = GetPlayerData().JumpBufferDuration;
        }
    }

	if (MovementMode == MOVE_Walking)
	{
		bWasPreviouslyEjectedFromSlope = false;
		bIsHoldingJump = false;
		CurrentJumpHoldTime = 0.f;
		// Only reset the jump ability when releasing the input
		if (!bWantsToJump)
		{
			bJumpConsumed = false;
		}
	}
}

#pragma endregion NetworkSync

#pragma region Drift

void UJBRCustomCMC::ResetDriftState()
{
	if (!bWantsToDrift && !bOldWantsToDrift) return;

	bWantsToDrift = false;
	bOldWantsToDrift = false;

	ResetDriftBoostStates();
	OnDriftCancel.Broadcast();
}

void UJBRCustomCMC::PerformEndDrift()
{
	const float DriftInitSpeed = CachedDriftInitVelocitySizeXY;
	const float DriftEndSpeedBeforeBoost = Velocity.Size2D();

	float AdditiveBoost = 0.f;
	if (bIsInSmallDrift)
	{
		AdditiveBoost = GetPlayerSpeedData().DriftSmallBoostForce;
	}
	else if (bIsInBigDrift)
	{
		AdditiveBoost = GetPlayerSpeedData().DriftBigBoostForce;
	}

	const float Compensation = FMath::Abs(DriftInitSpeed - DriftEndSpeedBeforeBoost);
	const float TotalBoost = AdditiveBoost + Compensation;

	if (AdditiveBoost > 0.f)
	{
		LastBoostFromDrift = TotalBoost;
		
		AddSpeedBoost(TotalBoost, false, true);
		OnDriftRelease.Broadcast();
		
		Cast<AJBRGameState>(GetWorld()->GetGameState())->SoundManager->PlaySoundFromNameID(FName("PlayerDriftBoost"));
	}
	else
	{
		OnDriftReleasePoor.Broadcast();
	}

	ResetDriftBoostStates();
	bOldWantsToDrift = false;
}

void UJBRCustomCMC::TryUpdateDriftStates(float DeltaTime)
{
	if (!bWantsToDrift) return;

	if (Velocity.Size2D() < GetPlayerData().MinSpeedForDrift)
	{
		ResetDriftState();
		return;
	}
	
	ElapsedTimeDrift += DeltaTime;
		
	if (ElapsedTimeDrift >= GetPlayerData().DriftSmallBoostTiming && !bIsInSmallDrift)
	{
		OnSmallDriftEnter.Broadcast();
		
		bIsInSmallDrift = true;
	}
	if (ElapsedTimeDrift >= GetPlayerData().DriftBigBoostTiming && !bIsInBigDrift)
	{
		OnBigDriftEnter.Broadcast();
		
		bIsInBigDrift = true;
	}
}

void UJBRCustomCMC::ResetDriftBoostStates()
{
	bIsInSmallDrift = false;
	bIsInBigDrift = false;

	ElapsedTimeDrift = 0.f;
}

bool UJBRCustomCMC::IsDrifting()
{
	return bWantsToDrift;
}

void UJBRCustomCMC::TryComputeBrakeElapsed(float DeltaTime)
{
	if (!bWantsToBrake) return;

	ElapsedTimeBrake += DeltaTime;
}

#pragma endregion Drift

#pragma region Rotation

void UJBRCustomCMC::RotationHoverboard(float DeltaTime)
{
	VisualOnlyLastSentRotation = UpdatedComponent->GetComponentRotation();
    
	if (!IsGrinding() && IsLateralMoveEnough())
	{
		float TurnRate = GetTurnAmount() * MoveRightInput;
		FRotator RotationDelta = FRotator(0.f, TurnRate * DeltaTime, 0.f);
		MoveUpdatedComponent(FVector::ZeroVector, UpdatedComponent->GetComponentRotation() + RotationDelta, false);
	}
    
	VisualOnlyDesiredRotation = UpdatedComponent->GetComponentRotation();
}

void UJBRCustomCMC::StopHoverboardRotation()
{
	VisualOnlyCurrentTurnInput = .0f;
	
	MoveRightInput = .0f;
	MoveUpInput = .0f;
	
	if (CharacterOwner && !CharacterOwner->HasAuthority())
	{
		ServerStopHoverboardRotation();
	}
}

void UJBRCustomCMC::ServerStopHoverboardRotation_Implementation()
{
	VisualOnlyCurrentTurnInput = 0.0f;
	
	MoveRightInput = 0.0f;
	MoveUpInput = .0f;
}

float UJBRCustomCMC::GetTurnAmount() const
{
	float Value = 0.0f;
	
	if (!JBRCharacterOwner) return Value;
	const FPlayerCMCData& Data = GetPlayerData();

	if (IsFalling())
	{
		Value = Data.TurnForceInAir;
	}
	else if (bWantsToDrift)
	{
		Value = FMath::Lerp(JBRCharacterOwner->GetPlayerCMCData().DriftTurnForceDriftLow,JBRCharacterOwner->GetPlayerCMCData().DriftTurnForceDriftHigh,
		JBRCharacterOwner->GetSpeedRatioFromAbsoluteVelocity());
	}
	else
	{
		Value = Data.TurnForce;
	}

	return Value;
}

#pragma endregion Rotation

#pragma region Jump

void UJBRCustomCMC::ProcessJump(float DeltaTime)
{
    const FPlayerCMCData& Data = GetPlayerData();

    // Launch
    if (bWantsToJump && !bJumpConsumed && CanCmcJump())
    {
        if (IsGrinding())
        {
            if (FMath::Abs(MoveRightInput) > 0.2f)
            {
                bJumpConsumed = true;
                bIsHoldingJump = true;
                CurrentJumpHoldTime = 0.f;
                RemainingJumpBufferTime = 0.f;
                ExitGrindFromInput(MoveRightInput);
            	
                return;
            }
            ExitGrind();
        }

        JumpStartVelocityZ = Data.MinMaxJumpForce.X - 250.f;
        Velocity.Z = JumpStartVelocityZ;

        SetMovementMode(MOVE_Falling);
        EndDash();

        bJumpConsumed = true;
        bIsHoldingJump = true;
        CurrentJumpHoldTime = 0.01f; // Set this so we can't apply hold logic even if we reset 
        RemainingJumpBufferTime = 0.f;

        Cast<AJBRGameState>(GetWorld()->GetGameState())
            ->SoundManager->PlaySoundFromNameID(FName("PlayerJump"));
    }

    // Sustain
    if (!bWantsToJump)
    {
        bIsHoldingJump = false;
        CurrentJumpHoldTime = 0.f;
    }

    if (bIsHoldingJump
        && IsFalling()
        && Velocity.Z > 0.f
        && Velocity.Z < Data.MinMaxJumpForce.Y - 100.f
        && CurrentJumpHoldTime > 0
        && CurrentJumpHoldTime < Data.JumpMaxHoldTime)
    {
        CurrentJumpHoldTime += DeltaTime;
        float HoldRatio  = CurrentJumpHoldTime / Data.JumpMaxHoldTime;
        float EasedRatio = FMath::Pow(HoldRatio, 0.5f);

        float Gravity = FMath::Abs(GetGravityZ());
        float SustainForce = Gravity * (1.0f - EasedRatio)
            + (Data.MinMaxJumpForce.Y - JumpStartVelocityZ) / Data.JumpMaxHoldTime;

        Velocity.Z = FMath::Min(Velocity.Z + SustainForce * DeltaTime, Data.MinMaxJumpForce.Y);
    }
    else if (bWantsToJump && bJumpConsumed)
    {
        bIsHoldingJump = true;
    }
}

bool UJBRCustomCMC::CanCmcJump() const
{
	bool bHasCoyoteTime = MovementMode == MOVE_Falling && RemainingJumpBufferTime > 0.f;
	return (IsMovingOnGround() || IsGrinding() || bHasCoyoteTime)
		&& !bWasPreviouslyEjectedFromSlope;
}

void UJBRCustomCMC::CheckSlopeEjection(float DeltaSeconds)
{
    if (!IsMovingOnGround()) return;
	if (IsGrinding()) return;
    // Minimum speed to launch (prevent small jittery stuff)
    if (Velocity.SizeSquared() < FMath::Square(500.0f)) return;

	FVector CurrentFloorNormal = CurrentFloor.HitResult.Normal;
	float FloorDot = CurrentFloorNormal | FVector::UpVector;
	if (FloorDot >= 0.98f) return; //slope tolerance, closer to 1 is avoiding plane surfaces

	// Project velocity onto the current floor plane
	// This helps the prediction follow the slope angle better than raw Velocity, reducing the chance of predicting inside the slope
	FVector CurrentLoc = UpdatedComponent->GetComponentLocation();
	
    // Look ahead 0.15 seconds to predict future pos
	float LookAheadTime = FMath::Lerp(0.09f, 0.1f, JBRCharacterOwner->GetSpeedRatioBaseSpeedFromAbsoluteVelocity());
	FVector VelocityOnSlope = FVector::VectorPlaneProject(Velocity, CurrentFloorNormal).GetSafeNormal() * Velocity.Size();

	// Instead of checking from the predicted point downwards, we define kind of a probe window around the predicted height
	// This means that if the slope is thin, and we predicted slightly under it, we start the trace above the face so we can hit it
	float ProbeLift = 150.0f;
	// How far down to check for the floor before declaring it a cliff
	float ProbeDepth = 300.f;

	const int32 NumSamples = 3;
	float SampleTimes[NumSamples] = { LookAheadTime * 1.0f, LookAheadTime * 1.2f, LookAheadTime * 1.4f };
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	int32 MissCount = 0;

	for (int32 i = 0; i < NumSamples; i++)
	{
		FVector SampleLoc = CurrentLoc + VelocityOnSlope * SampleTimes[i];
		FVector TraceStart = SampleLoc + FVector(0, 0, ProbeLift);
		FVector TraceEnd = TraceStart - FVector(0, 0, ProbeDepth);

		FHitResult Hit;
		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

		if (!bHit) MissCount++;
	}

	if (MissCount == 0) return;

	FVector Velocity3D = Velocity.GetSafeNormal();
	FVector RampVelocityDir = FVector::VectorPlaneProject(Velocity3D, CurrentFloorNormal);

	if (RampVelocityDir.SizeSquared() < 0.001f)
	{
		RampVelocityDir = FVector::VectorPlaneProject(
			UpdatedComponent->GetForwardVector(), CurrentFloorNormal);
	}

	RampVelocityDir.Normalize();

	FVector UnderStart = CurrentLoc + FVector(0, 0, 10.f);
	FVector UnderEnd = CurrentLoc - FVector(0, 0, JBRCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FHitResult UnderHit;
	bool bGroundUnderFeet = GetWorld()->LineTraceSingleByChannel(UnderHit, UnderStart, UnderEnd, ECC_Visibility, Params);

	if (bGroundUnderFeet) return;
	
	float BoostedSpeed = Velocity.Size2D();
	float SlopeAngleRad = FMath::Asin(FMath::Clamp(RampVelocityDir.Z, -1.f, 1.f));
	FVector HorizontalDir = FVector(RampVelocityDir.X, RampVelocityDir.Y, 0.f).GetSafeNormal();
	FVector EjectVelocity;
	EjectVelocity.X = HorizontalDir.X * BoostedSpeed * GetPlayerSpeedData().SlopeExitSpeedMultiplier;
	EjectVelocity.Y = HorizontalDir.Y * BoostedSpeed * GetPlayerSpeedData().SlopeExitSpeedMultiplier;
	EjectVelocity.Z = FMath::Tan(SlopeAngleRad) * BoostedSpeed * GetPlayerSpeedData().SlopeExitSpeedMultiplier;

	if (EjectVelocity.Z <= 0.f) return;

	Velocity = EjectVelocity;
	
	FVector EjectOffset = CurrentFloorNormal * 5.0f;
	UpdatedComponent->MoveComponent(
		EjectOffset,
		UpdatedComponent->GetComponentQuat(),
		false
	);
	
	CurrentFloor.Clear();
	
	SetMovementMode(MOVE_Falling);
	bWasPreviouslyEjectedFromSlope = true;
}

#pragma endregion Jump

#pragma region Physic

void UJBRCustomCMC::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
    Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);

	TryApplyAirFriction();
	TryApplyBrake();
	TryApplySpeedDebuff();
	TryApplyInstabilityBoost(DeltaTime);
	
	Velocity = Velocity.GetClampedToSize(0.f, GetAbsoluteMaxSpeed());

	TryUpdateDriftStates(DeltaTime);
	TryComputeBrakeElapsed(DeltaTime);
	HandleAbsoluteMaxSpeed(DeltaTime);
}

void UJBRCustomCMC::PhysFalling(float DeltaTime, int32 Iterations)
{
	Super::PhysFalling(DeltaTime, Iterations);

	if (Velocity.Z < 0)
	{
		LastFallingZVelocity = Velocity.Z;
	}
}

void UJBRCustomCMC::PhysCustom(float deltaTime, int32 Iterations)
{
	if (IsGrinding())
	{
		PhysGrind(deltaTime, Iterations);
	}
	else
	{
		Super::PhysCustom(deltaTime, Iterations);
	}
}

void UJBRCustomCMC::HandleAirFriction()
{
	if (!IsFalling())
	{
		AirFriction = 0.f;
		return;
	}

	if (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy) return;

	float BaseAirFriction = FMath::Clamp(GetPlayerData().AirFriction, 0.f, GetPlayerData().AirFriction);
	float FrictionDerived = IsAirGliding() ? GetAirGlidingAirFriction() : BaseAirFriction;

	AirFriction = 1+FrictionDerived;
}

void UJBRCustomCMC::TryApplyAirFriction()
{
	if (IsFalling() && AirFriction > 0.f)
	{
		FVector HorizontalVel = FVector(Velocity.X, Velocity.Y, 0.f);
		float Speed = HorizontalVel.Size();

		if (Speed > KINDA_SMALL_NUMBER)
		{
			Velocity.X /= AirFriction;
			Velocity.Y /= AirFriction;
		}
	}
}

void UJBRCustomCMC::TryApplySpeedDebuff()
{
	if (JBRCharacterOwner->HealthComponent->bIsUnstable) return;
	
	Velocity /= FMath::Clamp(CurrentSpeedDebuff, 1.f, GetPlayerSpeedData().MaxSpeedDebuff);
}

void UJBRCustomCMC::TryApplyInstabilityBoost(float DeltaTime)
{
	if (!JBRCharacterOwner->HealthComponent->bIsUnstable) return;

	Velocity += Velocity.GetSafeNormal() * (DeltaTime * GetPlayerSpeedData().InstabilitySpeedPerSeconde);
}

float UJBRCustomCMC::HandleFriction() const
{
	const float Speed = Velocity.Size2D();
	const float MaxSpeed = GetAbsoluteMaxSpeed();
	const float LerpStartSpeed = GetPlayerSpeedData().BaseMaxWalkSpeed;

	const float BaseFriction = GetPlayerData().GroundFriction;
	const float MaxFriction  = GetPlayerData().GroundFrictionAbsoluteSpeed;

	float Friction = BaseFriction;

	if (Speed > LerpStartSpeed)
	{
		const float Alpha = FMath::GetMappedRangeValueClamped(
			FVector2D(LerpStartSpeed, MaxSpeed),
			FVector2D(0.f, 1.f),
			Speed
		);

		Friction = FMath::Lerp(BaseFriction, MaxFriction, Alpha);
	}

	float BaseDriftFactor = GetPlayerData().GroundFrictionWhileDriftingFactor;
	float DriftFactor = bWantsToDrift
		? BaseDriftFactor
		: 1.f;

	float SimulatedDriftFriction = Friction * BaseDriftFactor;

	// If the friction is already low, doesn't affect it with drift factor to protect exploit
	// where the drift boost is stronger when already at high speed
	if (SimulatedDriftFriction < (BaseFriction * 0.5f)) DriftFactor = 1.f;

	return Friction * DriftFactor;
}

void UJBRCustomCMC::TryApplyBrake()
{
	if (bWantsToBrake)
	{
		FVector HorizontalVel = FVector(Velocity.X, Velocity.Y, 0.f);
		float Speed = HorizontalVel.Size();

		if (Speed > KINDA_SMALL_NUMBER)
		{
			Velocity.X /= GetPlayerData().BrakeIntensity;
			Velocity.Y /= GetPlayerData().BrakeIntensity;
		}
	}
}

void UJBRCustomCMC::HandleAbsoluteMaxSpeed(float DeltaTime)
{
	CurrentSpeedDebuff = FMath::Clamp(CurrentSpeedDebuff - DeltaTime * GetPlayerSpeedData().SpeedDebuffDecaySpeed,
		1.f, GetPlayerSpeedData().MaxSpeedDebuff);
}

float UJBRCustomCMC::GetMaxSpeed() const
{
	float Speed = MaxWalkSpeed;
	
	if (IsFalling())
	{
		return FMath::Max(Velocity.Size(), Speed);
	}

	if (!JBRCharacterOwner) 
		return 0.f;
	
	return Speed;
}

float UJBRCustomCMC::GetScalarMoveSpeed() const
{
	float Value = 1.0f;
	if (bWantsToBrake) Value -= ElapsedTimeBrake * GetPlayerData().MovementBrakeScalarDeprecation;
	else if (bWantsToDrift) Value = GetPlayerData().MovementDriftScalarModifier;
	return FMath::Max(Value, 0.f);
}

float UJBRCustomCMC::GetAbsoluteMaxSpeed() const
{
	if (JBRCharacterOwner == nullptr) return GetPlayerSpeedData().AbsoluteMaxSpeed;
	if (JBRCharacterOwner->HasRelic()) return GetPlayerSpeedData().AbsoluteMaxSpeedHolderRelic;
	return GetPlayerSpeedData().AbsoluteMaxSpeed;
}

#pragma endregion Physic

#pragma region Speed Boost

void UJBRCustomCMC::ForceVelocityForward()
{
	if (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy) return;

	FVector Dir = GetForwardVector().GetSafeNormal();
	float VelSizeXY = Velocity.Size2D();
	FVector InstantApplyVelocity = Dir * VelSizeXY;
	Velocity = InstantApplyVelocity;
}

void UJBRCustomCMC::AddSpeedBoost(float Amount, bool TakeVelocityDir, bool ReplaceVelocity)
{
	if (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy) return;
	
	FVector Dir = GetForwardVector().GetSafeNormal();
	if (TakeVelocityDir) Dir = Velocity.GetSafeNormal();

	float MaxZProjection = GetPlayerData().MinMaxJumpForce.Y;

	float VelSizeXY = Velocity.Size2D();
	if (ReplaceVelocity)
	{
		FVector InstantApplyVelocity = Dir * (VelSizeXY + Amount);
		InstantApplyVelocity.Z = FMath::Clamp(InstantApplyVelocity.Z, 0.f, MaxZProjection);
		
		Velocity = InstantApplyVelocity;
	}
	else
	{
		FVector AddedVelocity = Dir * Amount;
		AddedVelocity.Z = FMath::Clamp(AddedVelocity.Z, 0.f, MaxZProjection);
		
		Velocity += AddedVelocity;
	}

	if (IsGrinding())
	{
		// if grinding, update the grind state so it used the new applied velocity
		ForceUpdateGrindSpeed();
	}
	
	if (CharacterOwner->HasAuthority())
	{
		OnSpeedBoostAdded.Broadcast(Amount);
		Client_SpeedBoostDelegateCall(Amount);
	}
	else if (CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy)
	{
		OnSpeedBoostAdded.Broadcast(Amount);
	}
}

void UJBRCustomCMC::AddSpeedDebuff(float Amount)
{
	CurrentSpeedDebuff = FMath::Clamp(CurrentSpeedDebuff + Amount, 1.f, GetPlayerSpeedData().MaxSpeedDebuff);
}

void UJBRCustomCMC::Client_SpeedBoostDelegateCall_Implementation(float Amount)
{
	OnSpeedBoostAdded.Broadcast(Amount);
}

#pragma endregion Speed Boost

#pragma region Movement

void UJBRCustomCMC::PerformMovement(float DeltaTime)
{
	Super::PerformMovement(DeltaTime);
}

bool UJBRCustomCMC::IsMovingOnGround() const
{
	return
	(MovementMode == MOVE_Walking || MovementMode == MOVE_NavWalking ||
	(IsGrinding()))
	&& UpdatedComponent;
}

bool UJBRCustomCMC::IsLateralMoveEnough()
{
	return FMath::Abs(MoveRightInput) > 0.1f;
}

void UJBRCustomCMC::OnLand()
{
	bWantsToBrake = false;
	ElapsedTimeBrake = 0.f;
}

#pragma endregion Movement

#pragma region Slope

void UJBRCustomCMC::ComputeFinalSlopeNormal(const TArray<USceneComponent*> TargetComponents)
{
	OldDetectedSlopeNormal = DetectedSlopeNormal;
	
	FVector Normal1 = ComputeSlopeNormal(200.0f, TargetComponents[0]);
	FVector Normal2 = ComputeSlopeNormal(200.0f, TargetComponents[0]);
	FVector Normal3 = ComputeSlopeNormal(200.0f, TargetComponents[0]);
	FVector Normal4 = ComputeSlopeNormal(200.0f, TargetComponents[0]);
	
	DetectedSlopeNormal = (Normal1 + Normal2 + Normal3 + Normal4) / 4.0f;

	ComputeSlopeSpeedBoost();
}

FVector UJBRCustomCMC::ComputeSlopeNormal(float DetectionLength, const USceneComponent* TargetComponent) const
{
	FVector StartPosition = TargetComponent->GetComponentToWorld().GetLocation();
	FVector EndPosition = StartPosition - FVector(0, 0, DetectionLength);
	
	FHitResult HitResult;

	GetWorld()->LineTraceSingleByChannel(
	HitResult,
	StartPosition, EndPosition,ECC_Visibility);
	
	if (HitResult.bBlockingHit)
	{
		return HitResult.ImpactNormal;
	}
	return FVector(0, 0, 0);
}

float UJBRCustomCMC::GetSlopeAngle(const FVector& Normal) const
{
	if (!JBRCharacterOwner) return 0.f;
	
	FVector FloorNormal = Normal;
    
	FVector MovementDirection = JBRCharacterOwner->GetActorForwardVector().GetSafeNormal();
    
	FVector RightVector = FVector::CrossProduct(FVector::UpVector, MovementDirection);
	FVector SlopeUpVector = FVector::CrossProduct(RightVector, FloorNormal).GetSafeNormal();
    
	float DotProduct = FVector::DotProduct(SlopeUpVector, FVector::UpVector);
	float AngleRadians = FMath::Asin(FMath::Clamp(DotProduct, -1.f, 1.f));
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);
    
	FVector SlopeDirection = FVector(SlopeUpVector.X, SlopeUpVector.Y, 0.0f).GetSafeNormal();
	float DirectionDot = FVector::DotProduct(MovementDirection, SlopeDirection);
    
	return AngleDegrees * FMath::Sign(DirectionDot);
}

void UJBRCustomCMC::ComputeSlopeSpeedBoost()
{
	float Angle = GetSlopeAngle(DetectedSlopeNormal);
	if (Angle >= 0 || IsGrinding())
	{
		SlopeSpeedBoost = 0.f;
		return;
	} 
	
	SlopeSpeedBoost = FMath::Abs(Angle) * GetPlayerSpeedData().SlopeSpeedBoostIntensity *
		FMath::Lerp(1.f, GetPlayerSpeedData().SlopeSpeedBoostMaxSpeedMultiplier, JBRCharacterOwner->GetSpeedRatioBaseSpeedFromAbsoluteVelocity());
	
	AddSpeedBoost(SlopeSpeedBoost, true, false);

	// Consume falling slope speed boost
	//FallingSlopeSpeedBoost = 1.f;
}

#pragma endregion Slope

#pragma region Dash

void UJBRCustomCMC::StartDash(float Force)
{
	if (GetOwnerRole() == ROLE_Authority || GetOwnerRole() == ROLE_AutonomousProxy)
	{
		CurrentDashForce = Force;

		// Calculate initial velocity
		Velocity.Z = 0.f;
		AddSpeedBoost(Force, IsAirGliding(), true);

		bIsDashing = true;
		GetWorld()->GetTimerManager().SetTimer(
				DashDelegateTimer,
				this,
				&UJBRCustomCMC::EndDash,
				GetPlayerData().DashDuration,
				false
			);

		// Reset after consuming pending dash for avoiding race conditions with CanDash()
		JBRCharacterOwner->DashComponent->ResetCooldown();

		Cast<AJBRGameState>(GetWorld()->GetGameState())->SoundManager->PlaySoundFromNameID(FName("PlayerDash"));

		if (bWantsToDrift)
		{
			ResetDriftBoostStates();
		}
	}
}

void UJBRCustomCMC::RequestDash(float Force)
{
	PendingDashForce = Force;

	// Picked up by the next SavedMove
	bWantsToDash = true;
}

void UJBRCustomCMC::EndDash()
{
	bIsDashing = false;
}

#pragma endregion Dash

#pragma region Rails

void UJBRCustomCMC::EnterGrind(AGrindRail* NewRail, float EnterVelocityXY, float StartDistance, int32 Direction)
{
    if (!NewRail) return;

	ResetDriftBoostStates();
	
	OnRailBegin.Broadcast();

    // Set state locally for immediate prediction
    GrindState.LastRail = GrindState.CurrentRail;
    GrindState.CurrentRail = NewRail;
    GrindState.CurrentDistance = StartDistance;
    GrindState.EnterSpeed = EnterVelocityXY;
	GrindState.CurrentSpeed = GrindState.EnterSpeed;
    GrindState.GrindDirection = Direction;
    
    SetMovementMode(MOVE_Custom, CMOVE_Grinding);
}

void UJBRCustomCMC::ExitGrind()
{
    if (IsGrinding())
    {
        // Convert spline tangent to velocity for inertia preservation
        if (GrindState.CurrentRail && GrindState.CurrentRail->Spline)
        {
        	FVector Tangent = GrindState.CurrentRail->Spline->GetTangentAtDistanceAlongSpline(GrindState.CurrentDistance, ESplineCoordinateSpace::World);
        	Tangent.Normalize();
        	
        	Velocity = Tangent * GrindState.GrindDirection * GrindState.CurrentSpeed;
        	
        	if (JBRCharacterOwner->RailSystemComponent)
        	{
        		JBRCharacterOwner->RailSystemComponent->IsOnRail = false; // camera only
        		JBRCharacterOwner->RailSystemComponent->SetLastRail(GrindState.CurrentRail);
        	}
        }
        
    	GrindState.LastRail = GrindState.CurrentRail;
        GrindState.CurrentRail = nullptr;
    	
        SetMovementMode(MOVE_Falling);

    	OnRailEnd.Broadcast();
    	TryClearLastRail();
    }
}

void UJBRCustomCMC::ExitGrindFromInput(float Scalar)
{
	if (!IsGrinding()) return;
	
	if (GrindState.CurrentRail && GrindState.CurrentRail->Spline)
	{
		int ScalarRounded = Scalar > 0 ? 1 : -1;
		FRotator Rotator = FRotator(0, ScalarRounded * GetPlayerData().RailFlickAngle, 0);
		FVector RedirectedForward = Rotator.RotateVector(JBRCharacterOwner->GetActorForwardVector());

		FVector HorizontalDirection = RedirectedForward;
		HorizontalDirection.Z = 0;
		HorizontalDirection.Normalize();

		float TotalHorizontalSpeed = GrindState.CurrentSpeed + GetPlayerData().RailFlickForce;
		FVector MainVector = HorizontalDirection * TotalHorizontalSpeed;
		FVector UpwardBoost = FVector(0, 0, GetPlayerData().RailFlickUpwardBoost);
	
		Velocity = (MainVector + UpwardBoost);

		if (JBRCharacterOwner->RailSystemComponent)
		{
			JBRCharacterOwner->RailSystemComponent->IsOnRail = false;
			JBRCharacterOwner->RailSystemComponent->SetLastRail(GrindState.CurrentRail);
		}
	}

	GrindState.LastRail = GrindState.CurrentRail;
	GrindState.CurrentRail = nullptr;
	
	SetMovementMode(MOVE_Falling);

	OnRailEnd.Broadcast();
	TryClearLastRail();
}

void UJBRCustomCMC::ForceUpdateGrindSpeed(float OverrideAmount)
{
	if (OverrideAmount > 0.0f) GrindState.CurrentSpeed = OverrideAmount;
	else GrindState.CurrentSpeed = Velocity.Size2D();
}

#pragma region Rail Timers

void UJBRCustomCMC::TryClearLastRail()
{
	if (CharacterOwner->GetLocalRole() >= ROLE_AutonomousProxy)
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(ClearLastRailTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(
				ClearLastRailTimerHandle,
				this,
				&UJBRCustomCMC::ClearOldRail,
				ClearRailDelay,
				false
			);
		}
	}
}

void UJBRCustomCMC::ClearOldRail()
{
	GrindState.LastRail = nullptr;
}

#pragma endregion Rail Timers

bool UJBRCustomCMC::IsGrinding() const
{
    return MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Grinding;
}

void UJBRCustomCMC::PhysGrind(float DeltaTime, int32 Iterations)
{
    if (DeltaTime < MIN_TICK_TIME) return;
    if (!GrindState.CurrentRail || !GrindState.CurrentRail->Spline)
    {
        ExitGrind();
        return;
    }

	if (JBRCharacterOwner->HealthComponent->bIsDead) return;

    RestorePreAdditiveRootMotionVelocity();
    
    float DistanceStep = GrindState.CurrentSpeed * DeltaTime * GrindState.GrindDirection;
    GrindState.CurrentDistance += DistanceStep;

	// Clamp current speed to handle both deceleration and max speed
	// Multiply by the adrenaline speed ratio to match the adapted speed
	GrindState.CurrentSpeed = FMath::Clamp(GrindState.CurrentSpeed - (DeltaTime * GetPlayerData().RailDecelerationRate),
		GetPlayerSpeedData().MinRailSpeed, GetAbsoluteMaxSpeed());

    USplineComponent* Spline = GrindState.CurrentRail->Spline;
    float SplineLen = Spline->GetSplineLength();

    bool bReachedEnd = (GrindState.GrindDirection > 0 && GrindState.CurrentDistance >= SplineLen) ||
    	(GrindState.GrindDirection < 0 && GrindState.CurrentDistance <= 0.0f);

    if (bReachedEnd)
    {
        ExitGrind();
        StartNewPhysics(DeltaTime, Iterations);
        return;
    }

    FVector NewLoc = Spline->GetLocationAtDistanceAlongSpline(GrindState.CurrentDistance, ESplineCoordinateSpace::World);
    
    // Offset player height, half-height
    NewLoc.Z += CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 1.f; 

    FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(GrindState.CurrentDistance, ESplineCoordinateSpace::World);
    Tangent *= GrindState.GrindDirection;
    
    FRotator NewRot = Tangent.Rotation();
	GrindState.TargetRotationRail = NewRot;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(PhysGrind), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(Params, ResponseParam);
    
	// Temporarily ignore the rail
	if (GrindState.CurrentRail)
	{
		Params.AddIgnoredActor(GrindState.CurrentRail);
	}

    // Move player, SafeMove to handle unexpected collisions cuz it out a Hit ref
    FHitResult Hit(1.f);
    SafeMoveUpdatedComponent(NewLoc - UpdatedComponent->GetComponentLocation(), NewRot, true, Hit);

    // Update internal velocity for post grind logic
    Velocity = Tangent * GrindState.CurrentSpeed;

    // Handle impacts if we hit something while grinding
    if (Hit.Time < 1.f)
    {
        HandleImpact(Hit, DeltaTime, FVector::ZeroVector);
        SlideAlongSurface(NewLoc - UpdatedComponent->GetComponentLocation(), 1.f - Hit.Time, Hit.Normal, Hit, true);
    }

	JBRCharacterOwner->CameraShakeComponent->StartCameraShake(CameraShakeEnum::RailConstant, 0.f);
}

#pragma endregion Rails

#pragma region Air Gliding

bool UJBRCustomCMC::IsAirGliding()
{
	return IsFalling() && FMath::Abs(MoveUpInput) > 0.1f;
}

float UJBRCustomCMC::GetAirGlidingAirFriction() const
{
	float MaxPitchAngle = JBRCharacterOwner->PlayerHoverboard->GetBoardData().MaxAirGlidePitchAngle;
	
	// 0 1 along -max angle and +max angle
	float PitchRange = FMath::GetMappedRangeValueClamped(FVector2D(-MaxPitchAngle, MaxPitchAngle),FVector2D(0.f, 1.f), 
		JBRCharacterOwner->PlayerHoverboard->GetActorRotation().Pitch); 
	
	return FMath::Lerp(GetPlayerData().AirGlideFrictionMinMax.Y, GetPlayerData().AirGlideFrictionMinMax.X, PitchRange);
}

float UJBRCustomCMC::GetAirGlidingGravity() const
{
	float MaxPitchAngle = JBRCharacterOwner->PlayerHoverboard->GetBoardData().MaxAirGlidePitchAngle;
	
	// 0 1 along -max angle and +max angle
	float PitchRange = FMath::GetMappedRangeValueClamped(FVector2D(-MaxPitchAngle, MaxPitchAngle),FVector2D(0.f, 1.f), 
		JBRCharacterOwner->PlayerHoverboard->GetActorRotation().Pitch); 
	
	return FMath::Lerp(GetPlayerData().AirGlideMinMaxGravity.Y, GetPlayerData().AirGlideMinMaxGravity.X, PitchRange);
}

#pragma endregion Air Gliding

#pragma region Data

const FPlayerCMCData& UJBRCustomCMC::GetPlayerData() const
{
	if (JBRCharacterOwner) 
		return JBRCharacterOwner->GetPlayerCMCData();
	
	static FPlayerCMCData EmptyData;
	return EmptyData;
}

const FPlayerSpeedData& UJBRCustomCMC::GetPlayerSpeedData() const
{
	if (JBRCharacterOwner) 
		return JBRCharacterOwner->GetPlayerSpeedData();
	
	static FPlayerSpeedData EmptyData;
	return EmptyData;
}

FNetworkPredictionData_Client* UJBRCustomCMC::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UJBRCustomCMC* MutableThis = const_cast<UJBRCustomCMC*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_JBR(*this);
	}
	
	return ClientPredictionData;
}

void UJBRCustomCMC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Only needed on simulated proxies, kinda mid but works (doesn't actually affect gameplay since it's only used for visual purposes)
	DOREPLIFETIME_CONDITION(UJBRCustomCMC, GrindState, COND_SimulatedOnly);
}

#pragma endregion Data

#pragma region Respawn

void UJBRCustomCMC::ResetCmcStateOnRespawn()
{
	// Reset jump state
	bWantsToJump = false;
	RemainingJumpBufferTime = 0.0f;
	bIsHoldingJump = false;
	bJumpConsumed = false;
	CurrentJumpHoldTime = 0.f;
	
	bWasPreviouslyEjectedFromSlope = false;

	// Reset drift/brake state
	ResetDriftBoostStates();
	bDriftEnded = false;

	// Reset movement input
	bIsMovingForward = false;
	
	MoveRightInput = .0f;
	MoveUpInput = .0f;
	
	VisualOnlyCurrentTurnInput = .0f;
	VisualOnlyLastSentRotation = FRotator::ZeroRotator;
	VisualOnlyDesiredRotation = FRotator::ZeroRotator;
	SetMoveRightInput(0.0f);

	// Reset dash state
	bWantsToDash = false;

	// Reset grinding state
	if (IsGrinding())
	{
		ExitGrind();
	}
	GrindState.CurrentRail = nullptr;
	GrindState.LastRail = nullptr;
	GrindState.CurrentDistance = 0.0f;
	GrindState.GrindDirection = 1;

	// Clear any pending timers
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ClearLastRailTimerHandle);
	}

	// Reset to default movement mode
	SetMovementMode(MOVE_Falling);
	Velocity = FVector::ZeroVector;

	// Reset slope detection
	DetectedSlopeNormal = FVector::ZeroVector;
	OldDetectedSlopeNormal = FVector::ZeroVector;
	
	ClientResetCmcStateOnRespawn();
}

void UJBRCustomCMC::ClientResetCmcStateOnRespawn_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 99.0f, FColor::Blue, TEXT("Client Reset CMC State on Respawn"));
	
	// Reset jump state
	bWantsToJump = false;
	RemainingJumpBufferTime = 0.0f;
	bIsHoldingJump = false;
	bJumpConsumed = false;
	CurrentJumpHoldTime = 0.f;
	
	bWasPreviouslyEjectedFromSlope = false;

	// Reset drift/brake state
	ResetDriftBoostStates();
	bDriftEnded = false;

	// Reset movement input
	bIsMovingForward = false;
	
	MoveRightInput = .0f;
	MoveUpInput = .0f;
	
	VisualOnlyCurrentTurnInput = .0f;
	VisualOnlyLastSentRotation = FRotator::ZeroRotator;
	VisualOnlyDesiredRotation = FRotator::ZeroRotator;
	SetMoveRightInput(.0f);

	// Reset dash state
	bWantsToDash = false;

	// Reset grinding state
	if (IsGrinding())
	{
		ExitGrind();
	}
	GrindState.CurrentRail = nullptr;
	GrindState.LastRail = nullptr;
	GrindState.CurrentDistance = 0.0f;
	GrindState.GrindDirection = 1;

	// Clear any pending timers
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ClearLastRailTimerHandle);
	}

	// Reset to default movement mode
	SetMovementMode(MOVE_Falling);
	Velocity = FVector::ZeroVector;

	// Reset slope detection
	DetectedSlopeNormal = FVector::ZeroVector;
	OldDetectedSlopeNormal = FVector::ZeroVector;
}

#pragma endregion Respawn