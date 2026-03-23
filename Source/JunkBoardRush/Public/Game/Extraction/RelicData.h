
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RelicData.generated.h"


USTRUCT(BlueprintType)
struct JUNKBOARDRUSH_API FRelicData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = "Relic Config - Movement")
	float RelicSpeed = 600.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Relic Config - Respawn")
	float RelicRespawnShieldDuration = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Relic Config - Goal")
	int32 NbChargesToExtract = 3;

	UPROPERTY(EditDefaultsOnly, Category = "Relic Config - Visual")
	float OverrideScaleInHand = 0.75f;

	UPROPERTY(EditDefaultsOnly, Category = "Relic Config - Drop")
	FVector2D MinMaxLateralExpulsionDirection = FVector2D(-0.5f, 0.5f);
	
	UPROPERTY(EditDefaultsOnly, Category = "Relic Config - Drop")
	FVector2D MinMaxExpulsionForce = FVector2D(250.0f, 750.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Relic Config - Magnetic Shield")
	float RelicDefensiveShieldSize = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Relic Config - Magnetic Shield")
	float RelicDefensiveShieldDuration = 1.0f;
};
