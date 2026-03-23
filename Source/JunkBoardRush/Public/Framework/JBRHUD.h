
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "JBRHUD.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHUDInitialized);

UCLASS()
class JUNKBOARDRUSH_API AJBRHUD : public AHUD
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintAssignable)
	FOnHUDInitialized OnHUDInitialized;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	TArray<TObjectPtr<UUserWidget>> ActiveWidgets;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	TArray<TObjectPtr<UUserWidget>> NeedDisableWidgets;
	
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void HideAllActiveWidgets();
	
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void HideWidgetsToDisable();
	
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void ShowAllActiveWidgets();
	//todo maybe we show smth that shouldn't be at T time
};
