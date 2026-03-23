#pragma once

#include "DangerWheelSlotData.generated.h"

UENUM(BlueprintType)
enum class DangerWheelSlotSource : uint8 {
	Gadget = 0 UMETA(DisplayName = "Gadget"),
	Player = 1 UMETA(DisplayName = "Player"),
	Other = 2 UMETA(DisplayName = "Other"),
};

USTRUCT(BlueprintType)
struct JUNKBOARDRUSH_API FDangerWheelSlotData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	DangerWheelSlotSource SlotSource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture> SlotIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SlotColorBase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SlotColorMax;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool AffectColorByDistance;
	
};
