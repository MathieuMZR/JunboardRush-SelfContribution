
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Player/Tricks/TricksAnimDataBoard.h"

#include "TricksAnimData.generated.h"

UENUM(BlueprintType)
enum class TrickAnimID : uint8
{
	Null = 0 UMETA(DisplayName = "Null"),
	Default = 1 UMETA(DisplayName = "Default"),
	Air = 2 UMETA(DisplayName = "Air"),
	Rail = 3 UMETA(DisplayName = "Rail"),
	Transition = 100 UMETA(DisplayName = "Transition")
};

USTRUCT()
// Allow to return two animations from one function
struct FAnimSelection
{
	GENERATED_BODY()

	UAnimationAsset* BoardAnim = nullptr;
	FTricksAnimDataBoard TricksAnimDataBoard = FTricksAnimDataBoard();
	UAnimationAsset* PlayerAnim = nullptr;
};

USTRUCT(BlueprintType)
struct JUNKBOARDRUSH_API FTricksAnimData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta=(Tooltip = ""))
	TArray<FTricksAnimDataBoard> BoardAnims;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta=(Tooltip = ""))
	TArray<TObjectPtr<UAnimationAsset>> PlayerAnims;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta=(Tooltip = ""))
	TrickAnimID AnimID = TrickAnimID::Null;
};
