#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WaypointWidget.generated.h"

class UImage;
class UCanvasPanelSlot;
class UTextBlock;
class APlayerController;

UCLASS()
class JUNKBOARDRUSH_API UWaypointWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Root widget inside the Canvas (SizeBox / Overlay / Border) */
	UPROPERTY(meta = (BindWidget))
	UWidget* WaypointRoot;

	UPROPERTY(meta = (BindWidget))
	UImage* WaypointIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DistanceText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	AActor* TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FVector Offset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	float MinDistanceShow = 3000.f;

protected:
	APlayerController* PlayerController = nullptr;
	UCanvasPanelSlot* WaypointCanvasSlot = nullptr;

	void UpdateWaypointPosition(const FGeometry& MyGeometry, float InDeltaTime);
	FVector2D GetWidgetSize() const;

	bool IsTargetNearEnoughAndVisible() const;
};
