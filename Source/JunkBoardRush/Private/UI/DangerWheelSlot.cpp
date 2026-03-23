
#include "UI/DangerWheelSlot.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"

void UDangerWheelSlot::SetupAnimationAndVisuals(UCanvasPanelSlot* PanelSlot, float DistanceRatio, float DotVisibility)
{
	float ScaleUniform = FMath::Lerp(
					MinMaxScaleDistance.Y,
					MinMaxScaleDistance.X,
					DistanceRatio);
	ParentOverlay->SetRenderScale(FVector2D(ScaleUniform, ScaleUniform));
	
	float Opacity = FMath::Lerp(
					0.f,
					1.f,
					OpacityCurve->FloatCurve.Eval(DistanceRatio)) * DotVisibility;

	ParentOverlay->SetRenderOpacity(Opacity);

	UMaterialInstanceDynamic* Inst = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), DangerSlotMaterial);

	FLinearColor Color = DangerSlotData.SlotColorBase;
	if (DangerSlotData.AffectColorByDistance)
	{
		Color = UKismetMathLibrary::LinearColorLerp(DangerSlotData.SlotColorBase, DangerSlotData.SlotColorMax, DistanceRatio);
	}
	
	Inst->SetTextureParameterValue(FName("TexParameter"), DangerSlotData.SlotIcon);
	Inst->SetVectorParameterValue(FName("BlueReplaceColor"), Color);
	
	DangerSlotIcon->SetBrushFromMaterial(Inst);

	BP_SetupAnimationAndVisuals(PanelSlot, DistanceRatio);
}

void UDangerWheelSlot::SetupDangerSlot(UCanvasPanelSlot* WidgetSlot)
{
	WidgetSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	WidgetSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
	WidgetSlot->SetSize(ViewportSize2D);
	WidgetSlot->SetAutoSize(true);
	WidgetSlot->SetPosition(FVector2D(0.f, 0.f));
}
