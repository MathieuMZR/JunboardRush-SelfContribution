
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "CameraShakeData.generated.h"

USTRUCT(BlueprintType)
struct JUNKBOARDRUSH_API FCameraShakeData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake - Base")
	TSoftClassPtr<UCameraShakeBase> CameraShakeBase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake - Damage")
	TSoftClassPtr<UCameraShakeBase> CameraShakeDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake - Damage")
	FVector2D CameraShakeDamageLowHighMult;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake - Adrenaline")
	TSoftClassPtr<UCameraShakeBase> CameraShakeAdrenalineStart;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake - Adrenaline")
	TSoftClassPtr<UCameraShakeBase> CameraShakeAdrenalineConstant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake - Landing")
	TSoftClassPtr<UCameraShakeBase> CameraShakeLanding;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake - Landing")
	FVector2D CameraShakeLandingLowHighMult;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake - Gadget")
	TSoftClassPtr<UCameraShakeBase> CameraShakeGadgetBoost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake - Rails")
	TSoftClassPtr<UCameraShakeBase> CameraShakeRailConstant;
};
