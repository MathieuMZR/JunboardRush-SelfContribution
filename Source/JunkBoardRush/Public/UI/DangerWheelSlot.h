
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/DangerWheelSlotData.h"
#include "DangerWheelSlot.generated.h"

class UOverlay;
class UImage;
class UCanvasPanelSlot;

UCLASS()
class JUNKBOARDRUSH_API UDangerWheelSlot : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UOverlay* ParentOverlay;
	
	UPROPERTY(meta = (BindWidget))
	UImage* DangerSlotIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D ViewportSize2D = FVector2D(128, 128);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D MinMaxScaleDistance = FVector2D(0.3f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterial> DangerSlotMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UCurveFloat> OpacityCurve;
	
	void SetupAnimationAndVisuals(UCanvasPanelSlot* PanelSlot, float DistanceRatio, float DotVisibility);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetupAnimationAndVisuals(UCanvasPanelSlot* PanelSlot, float DistanceRatio);

	void SetupDangerSlot(UCanvasPanelSlot* WidgetSlot);

	FDangerWheelSlotData DangerSlotData;
};
