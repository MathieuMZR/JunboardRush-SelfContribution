
#pragma once

#include "CoreMinimal.h"
#include "DangerWheelSlotData.h"
#include "Blueprint/UserWidget.h"
#include "DangerWheel.generated.h"

class UCanvasPanel;
class AJBRCharacter;

UCLASS()
class JUNKBOARDRUSH_API UDangerWheel : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	
	UPROPERTY(BlueprintReadOnly)
	AJBRCharacter* Player;

	FTimerHandle CustomTickTimerHandle;
	UFUNCTION()
	void CustomTick();

	void CreateDangerWheelSlot();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsFacingCamera(AActor* Target);
	int32 GetSlotSourcePriority(DangerWheelSlotSource Source) const;

#pragma region UI Elements

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCanvasPanel* SlotsContainer;
	
#pragma endregion UI Elements

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ToleranceCameraFacing = .65f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Margin = 650.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseYPosition = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D MinMaxDistance = FVector2D(500.f, 5000.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CurveYOffset = 85.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<AActor*> DetectedActors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UCurveFloat> ArcCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UCurveFloat> OpacityArcCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> DangerWheelSlotClass;
};
