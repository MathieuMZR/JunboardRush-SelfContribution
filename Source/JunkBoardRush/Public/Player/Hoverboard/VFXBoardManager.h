
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFXBoardManager.generated.h"

class UPointLightComponent;
class URectLightComponent;
class UJBRCustomCMC;
class AJBRCharacter;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class JUNKBOARDRUSH_API UVFXBoardManager : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UVFXBoardManager();

	AJBRCharacter* JBRCharacter;
	UJBRCustomCMC* CustomCMC;

	void Initialize(AJBRCharacter* Player, USkeletalMeshComponent* BoardMesh);

	UPROPERTY() URectLightComponent* BoardLight = nullptr;
	UPROPERTY() UPointLightComponent* BoardBoostLight = nullptr;

#pragma region VFX - Drift
	
	UPROPERTY(VisibleAnywhere) UNiagaraComponent* CompDriftVFX;
	UPROPERTY(VisibleAnywhere) UNiagaraComponent* CompDriftTransitionVFX;
	
	UPROPERTY(EditDefaultsOnly, Category = "VFX - Drift") UNiagaraSystem* LowDriftVFX;
	UPROPERTY(EditDefaultsOnly, Category = "VFX - Drift") UNiagaraSystem* MidDriftVFX;
	UPROPERTY(EditDefaultsOnly, Category = "VFX - Drift") UNiagaraSystem* HighDriftVFX;
	UPROPERTY(EditDefaultsOnly, Category = "VFX - Drift") UNiagaraSystem* TransitionLowToMidDriftVFX;
	UPROPERTY(EditDefaultsOnly, Category = "VFX - Drift") UNiagaraSystem* TransitionMidToHighDriftVFX;

#pragma endregion VFX - Drift

#pragma region VFX - Trail

	UPROPERTY(VisibleAnywhere) UNiagaraComponent* CompTrailVFX;

	UPROPERTY(EditDefaultsOnly, Category = "VFX - Trail") UNiagaraSystem* LowTrailVFX;
	UPROPERTY(EditDefaultsOnly, Category = "VFX - Trail") UNiagaraSystem* MidTrailVFX;
	UPROPERTY(EditDefaultsOnly, Category = "VFX - Trail") UNiagaraSystem* HighTrailVFX;
	
#pragma endregion VFX - Trail

protected:
	
	void InitializeSubscriptions();

	const FName DriftSocketName = TEXT("drift_socket");
	const FName LightRectSocketName = TEXT("light_rect_socket");

	const FColor LowColorLight = FColor(40, 156, 255);
	const FColor MidColorLight = FColor(255, 176, 40);
	const FColor HighColorLight = FColor(215, 37, 232);

#pragma region VFX Events

#pragma region Drift
	
	UFUNCTION() void OnDriftEnter();
	UFUNCTION() void OnDriftRelease();
	UFUNCTION() void OnDriftReleasePoor();
	UFUNCTION() void OnSmallDrift();
	UFUNCTION() void OnBigDrift();

#pragma endregion Drift

#pragma region Trail

	UFUNCTION() void OnSpeedBoostAdded(float Amount);
	UFUNCTION() void ResetTrail();

	FTimerHandle TrailResetTimer;
	const float TrailResetDuration = 1.f;
	
#pragma endregion Trail

#pragma region Light

	void LightColorBehavior(FColor Color);

#pragma endregion Light
	
#pragma endregion VFX Events

#pragma region VFX Creation

	UNiagaraComponent* TryCreateVFXComponent(UNiagaraSystem* DefaultAsset, FName SocketName,
		USkeletalMeshComponent* TargetMesh, bool InitialActivation) const;

	void TryCreateLightComponents(USkeletalMeshComponent* TargetMesh);

#pragma endregion VFX Creation
};
