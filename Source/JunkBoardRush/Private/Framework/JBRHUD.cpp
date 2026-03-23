
#include "Framework/JBRHUD.h"

#include "Blueprint/UserWidget.h"

void AJBRHUD::BeginPlay()
{
	Super::BeginPlay();

	OnHUDInitialized.Broadcast();
}

void AJBRHUD::HideAllActiveWidgets()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Hiding all active HUD widgets"));
	
	for (TObjectPtr<UUserWidget> Widget : ActiveWidgets)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Hiding widget: %s"), *Widget->GetName()));
	
		if (Widget)
		{
			Widget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AJBRHUD::HideWidgetsToDisable()
{
	for (TObjectPtr<UUserWidget> Widget : NeedDisableWidgets)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Hiding widget: %s"), *Widget->GetName()));
	
		if (Widget)
		{
			Widget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AJBRHUD::ShowAllActiveWidgets()
{
	for (TObjectPtr<UUserWidget> Widget : ActiveWidgets)
	{
		if (Widget)
		{
			Widget->SetVisibility(ESlateVisibility::Visible);
		}
	}
}
