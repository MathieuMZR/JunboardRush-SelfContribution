
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "SoundEffectStructData.generated.h"

USTRUCT(BlueprintType)
struct JUNKBOARDRUSH_API FSoundEffectStructData : public FTableRowBase
{
	GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ID", meta=(Tooltip = ""))
    FName NameID = "SoundCue_01";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cues", meta=(Tooltip = ""))
	TArray<TObjectPtr<USoundBase>> Sounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume", meta=(Tooltip = ""))
	FVector2D MinMaxVolume = FVector2D(1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pitch", meta=(Tooltip = ""))
	FVector2D MinMaxPitch = FVector2D(1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Start Time", meta=(Tooltip = ""))
	float StartTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatialization", meta=(Tooltip = ""))
	bool IsSpatialized = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatialization", meta=(Tooltip = ""))
	TObjectPtr<USoundAttenuation> AttenuationSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property", meta=(Tooltip = ""))
	bool Looping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property", meta=(Tooltip = ""))
	bool AutoDestroy = true;
};