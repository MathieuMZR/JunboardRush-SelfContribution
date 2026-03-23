
#include "UI/DangerWheel.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Framework/JBRCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/Camera/DynamicPlayerCamera.h"
#include "UI/DangerWheelDetected_Interface.h"
#include "UI/DangerWheelSlot.h"

void UDangerWheel::NativeConstruct()
{
	Super::NativeConstruct();

	Player = Cast<AJBRCharacter>(GetOwningPlayerPawn());

	if (!Player) return;

	GetWorld()->GetTimerManager().ClearTimer(CustomTickTimerHandle);
	Player->GetWorldTimerManager().SetTimer(
			CustomTickTimerHandle,
			FTimerDelegate::CreateUObject(this, &UDangerWheel::CustomTick),
			0.1f,
			true
		);
}

void UDangerWheel::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (DetectedActors.Num() <= 0) return;
	if (!Player) return;
 
	int32 Iterator = 0;
	for (AActor* Actor : DetectedActors)
	{
		if (!Actor || !Actor->Implements<UDangerWheelDetected_Interface>())
		{
			Iterator++;
			continue;
		}

		UWidget* ChildWidget = SlotsContainer->GetChildAt(Iterator);
		UCanvasPanelSlot* ChildWidgetSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ChildWidget);
		UDangerWheelSlot* DangerSlot = Cast<UDangerWheelSlot>(ChildWidget);
		
		if (!ChildWidget || !ChildWidgetSlot || !DangerSlot)
		{
			Iterator++;
			continue;
		}
		
		float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
		FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld()) / ViewportScale;

		float CameraRelativeMargin = 
        FMath::Lerp(
            0,
            Player->DynamicPlayerCamera->GetCameraData().MinMaxFOVAlongSpeed.Y,
            Player->Camera->FieldOfView / Player->DynamicPlayerCamera->GetCameraData().MinMaxFOVAlongSpeed.Y);

		float RatioMargin = (ViewportSize.X * Margin) / 1920.f;
		float FinalMargin = RatioMargin - CameraRelativeMargin;

		FVector ActorLocation = Actor->GetActorLocation();
		FVector PlayerLocation = Player->GetActorLocation();
		float Distance = FVector::Distance(ActorLocation, PlayerLocation);

		FVector ToTarget2D = FVector(ActorLocation.X - PlayerLocation.X, ActorLocation.Y - PlayerLocation.Y, 0.f).GetSafeNormal();

		FRotator CamYaw = FRotator(0.f, Player->Camera->GetComponentRotation().Yaw, 0.f);
		FVector CamRight2D   = FRotator(0.f, CamYaw.Yaw + 90.f, 0.f).Vector();

		float HorizontalDot = FVector::DotProduct(ToTarget2D, CamRight2D);

		float CurveInput = UKismetMathLibrary::MapRangeClamped(HorizontalDot, -1.f, 1.f, 0.f, 1.f);

		float XScreenPos = FMath::Lerp(FinalMargin, ViewportSize.X - FinalMargin, CurveInput);
		XScreenPos = FMath::Clamp(XScreenPos, FinalMargin, ViewportSize.X - FinalMargin);

		float CurveEvalArc = ArcCurve->FloatCurve.Eval(CurveInput);
		float CurveEvalOpacityArc = OpacityArcCurve->FloatCurve.Eval(CurveInput);
		float YScreenPos = (ViewportSize.Y - BaseYPosition) + (CurveEvalArc * CurveYOffset);

		if (IsFacingCamera(Actor))
		{
			ChildWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		else
		{
			ChildWidget->SetVisibility(ESlateVisibility::Visible);
			ChildWidgetSlot->SetPosition(FVector2D(XScreenPos, YScreenPos));

			float Ratio = (Distance - MinMaxDistance.X) / (MinMaxDistance.Y - MinMaxDistance.X);
			Ratio = FMath::Clamp(Ratio, 0.f, 1.f);
			
			DangerSlot->DangerSlotData = IDangerWheelDetected_Interface::Execute_GetDangerWheelSlotData(Actor);
			DangerSlot->SetupAnimationAndVisuals(ChildWidgetSlot, Ratio, CurveEvalOpacityArc);
		}

		Iterator++;
	}
}

void UDangerWheel::CustomTick()
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsWithInterface(
		GetWorld(),
		UDangerWheelDetected_Interface::StaticClass(),
		OutActors
	);

	TArray<AActor*> NewDetected;
	for (AActor* Actor : OutActors)
	{
		float Dist = FVector::Dist(Actor->GetActorLocation(), Player->GetActorLocation());
		if (Dist >= MinMaxDistance.X && Dist <= MinMaxDistance.Y)
		{
			NewDetected.Add(Actor);
		}
	}

	NewDetected.Sort([this](const AActor& A, const AActor& B)
	{
		FDangerWheelSlotData DataA = IDangerWheelDetected_Interface::Execute_GetDangerWheelSlotData(&A);
		FDangerWheelSlotData DataB = IDangerWheelDetected_Interface::Execute_GetDangerWheelSlotData(&B);

		int32 PriorityA = GetSlotSourcePriority(DataA.SlotSource);
		int32 PriorityB = GetSlotSourcePriority(DataB.SlotSource);

		if (PriorityA != PriorityB)
			return PriorityA > PriorityB;

		float DistA = FVector::Dist(A.GetActorLocation(), Player->GetActorLocation());
		float DistB = FVector::Dist(B.GetActorLocation(), Player->GetActorLocation());
		return DistA > DistB;
	});

	if (NewDetected == DetectedActors) return;

	SlotsContainer->ClearChildren();
	DetectedActors = NewDetected;

	for (AActor* Actor : DetectedActors)
		CreateDangerWheelSlot();
}

void UDangerWheel::CreateDangerWheelSlot()
{
	UDangerWheelSlot* DangerSlot = CreateWidget<UDangerWheelSlot>(GetOwningPlayer(), DangerWheelSlotClass);
	if (!DangerSlot) return;

	UCanvasPanelSlot* CanvasSlot = SlotsContainer->AddChildToCanvas(DangerSlot);
	if (!CanvasSlot) return;

	DangerSlot->SetupDangerSlot(CanvasSlot);
}

bool UDangerWheel::IsFacingCamera(AActor* Target)
{
	if (!Target) return false;
	if (!Player || !Player->Camera) return false;

	FVector Forward = Player->Camera->GetForwardVector();
	FVector Direction = (Target->GetActorLocation() - Player->Camera->GetComponentLocation()).GetSafeNormal();
	float Dot = FVector::DotProduct(Direction, Forward);

	if (Dot > ToleranceCameraFacing) return true;

	return false;
}

int32 UDangerWheel::GetSlotSourcePriority(DangerWheelSlotSource Source) const
{
	switch (Source)
	{
	case DangerWheelSlotSource::Gadget:  return 0;
	case DangerWheelSlotSource::Player:  return 1;
	case DangerWheelSlotSource::Other:   return 2;
	default:                             return 99;
	}
}
	
