
#pragma once

#include "CoreMinimal.h"
#include "TricksAnimData.h"
#include "Engine/DataTable.h"

#include "TricksSystemData.generated.h"

USTRUCT(BlueprintType)
struct JUNKBOARDRUSH_API FTricksSystemData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Logic", meta=(Tooltip = ""))
	float ActionCooldownDuration = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Logic", meta=(Tooltip = ""))
	float ActionWindowDuration = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blend", meta=(Tooltip = ""))
	FVector2D BoardLerpInOutDuration = FVector2D(0.0f, 0.1f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blend", meta=(Tooltip = ""))
	FVector2D PlayerLerpInOutDuration = FVector2D(0.0f, 0.2f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blend", meta=(Tooltip = ""))
	FVector2D BoardLerpInOutDurationInterruption = FVector2D(0.1f, 0.1f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blend", meta=(Tooltip = ""))
	FVector2D PlayerLerpInOutDurationInterruption = FVector2D(0.0f, 0.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default", meta=(Tooltip = ""))
	FTricksAnimData GroundTricksAnims;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air", meta=(Tooltip = ""))
	FTricksAnimData AirTricksAnims;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rail", meta=(Tooltip = ""))
	FTricksAnimData RailTricksAnims;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions", meta=(Tooltip = ""))
	FTricksAnimData InterruptionAnims;
};
