#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UI/DangerWheelSlotData.h"
#include "DangerWheelDetected_Interface.generated.h"

UINTERFACE(BlueprintType)
class UDangerWheelDetected_Interface : public UInterface
{
	GENERATED_BODY()
};

class JUNKBOARDRUSH_API IDangerWheelDetected_Interface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FDangerWheelSlotData GetDangerWheelSlotData() const;
};