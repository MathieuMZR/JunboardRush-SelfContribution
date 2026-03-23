#include "UI/WaypointWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void UWaypointWidget::NativeConstruct()
{
    Super::NativeConstruct();

    PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    if (WaypointRoot)
    {
        WaypointCanvasSlot = Cast<UCanvasPanelSlot>(WaypointRoot->Slot);
    }
}

void UWaypointWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (TargetActor && PlayerController && WaypointCanvasSlot)
    {
        UpdateWaypointPosition(MyGeometry, InDeltaTime);
    }
}

void UWaypointWidget::UpdateWaypointPosition(const FGeometry& MyGeometry, float InDeltaTime)
{
    // Screen
    const FVector2D ScreenSize= MyGeometry.GetLocalSize();
    const FVector2D ScreenCenter = ScreenSize * 0.5f;
    const FVector2D WidgetSize = GetWidgetSize();
    
    // Add border offset (padding from screen edges)
    const float BorderOffset = 50.f;
    const FVector2D Margin = WidgetSize * 0.5f + FVector2D(BorderOffset, BorderOffset);

    // Camera
    FVector CamLoc;
    FRotator CamRot;
    PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
    
    // Target
    const FVector WorldPos = TargetActor->GetActorLocation() + Offset;

    // Camera-space direction (DO NOT normalize yet)
    const FVector ToTargetCS = CamRot.UnrotateVector(WorldPos - CamLoc);
    static constexpr float FrontDeadZone = 30.f;
    const bool bInFront = ToTargetCS.X > FrontDeadZone;

    // Try projection on screen only
    FVector2D ProjectedPos;
    const bool bProjected =
        UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
            PlayerController,
            WorldPos,
            ProjectedPos,
            false
        );

    const FVector2D MinBorder = Margin;
    const FVector2D MaxBorder = ScreenSize - Margin;
    
    const bool bOnScreen =
    bProjected &&
    bInFront &&
    ProjectedPos.X >= MinBorder.X && ProjectedPos.X <= MaxBorder.X &&
    ProjectedPos.Y >= MinBorder.Y && ProjectedPos.Y <= MaxBorder.Y;

    FVector2D FinalScreenPos;
    
    // ON-SCREEN
    if (bOnScreen)
    {
        FinalScreenPos = ProjectedPos;
    }
    // OFF-SCREEN
    else
    {
        // Screen-space direction (Y right, Z up)
        FVector2D Dir2D(ToTargetCS.Y, -ToTargetCS.Z);

        if (!Dir2D.Normalize())
        {
            Dir2D = FVector2D(0.f, 1.f);
        }

        // If behind camera, bias downward
        if (!bInFront)
        {
            Dir2D.Y = FMath::Lerp(Dir2D.Y, FMath::Abs(Dir2D.Y), InDeltaTime);
        }

        const FVector2D Min = Margin;
        const FVector2D Max = ScreenSize - Margin;

        float T = FLT_MAX;

        if (Dir2D.X > 0.f)
            T = FMath::Min(T, (Max.X - ScreenCenter.X) / Dir2D.X);
        else if (Dir2D.X < 0.f)
            T = FMath::Min(T, (Min.X - ScreenCenter.X) / Dir2D.X);

        if (Dir2D.Y > 0.f)
            T = FMath::Min(T, (Max.Y - ScreenCenter.Y) / Dir2D.Y);
        else if (Dir2D.Y < 0.f)
            T = FMath::Min(T, (Min.Y - ScreenCenter.Y) / Dir2D.Y);

        FinalScreenPos = ScreenCenter + Dir2D * T;
    }
    
    // Apply
    WaypointCanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));

    if (IsTargetNearEnoughAndVisible())
        FinalScreenPos = FVector2D(0, 100000); // tp out of screen, avoid hidden logic that conflict with other external scripts
    
    WaypointCanvasSlot->SetPosition(FinalScreenPos);
    
    // Distance
    const float Distance = FVector::Dist(WorldPos, CamLoc);
    const int32 DistanceMeters = FMath::RoundToInt(Distance / 100.f);

    if (DistanceText)
    {
        DistanceText->SetText(
            FText::FromString(FString::Printf(TEXT("%dm"), DistanceMeters))
        );
    }
}

FVector2D UWaypointWidget::GetWidgetSize() const
{
    if (WaypointRoot)
    {
        return WaypointRoot->GetDesiredSize();
    }

    return FVector2D(32.f, 32.f);
}

bool UWaypointWidget::IsTargetNearEnoughAndVisible() const
{
    APawn* Pawn = GetOwningPlayerPawn();
    if (!Pawn) return false;
    
    APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    if (!PC || !PC->PlayerCameraManager || !TargetActor) return false;
    
    FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
    FVector CamFwd = PC->PlayerCameraManager->GetCameraRotation().Vector();

    FVector DirToTarget = (TargetActor->GetActorLocation() - CamLoc).GetSafeNormal();
    float Dot = FVector::DotProduct(CamFwd, DirToTarget);

    return FVector::Dist(CamLoc, TargetActor->GetActorLocation()) <= MinDistanceShow && Dot > 0.5f;
}
