
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "TricksAnimDataBoard.generated.h"

// Allow the board animation to tell the system if a player anim should be forced play so we stock a linked anim
// complex struct like this allow multiples references / settings in one data
USTRUCT(BlueprintType)
struct JUNKBOARDRUSH_API FTricksAnimDataBoard : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta=(Tooltip = ""))
	TObjectPtr<UAnimationAsset> Anim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta=(Tooltip = ""))
	TObjectPtr<UAnimationAsset> LinkedAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta=(Tooltip = ""))
	bool ShouldPlayerBeLinkedOnBoard;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta=(Tooltip = ""))
	bool DisableNonPlayerAnimationSelection;
};
