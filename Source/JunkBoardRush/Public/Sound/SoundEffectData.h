
#pragma once

#include "CoreMinimal.h"
#include "SoundEffectStructData.h"
#include "Engine/DataTable.h"

#include "SoundEffectData.generated.h"

USTRUCT(BlueprintType)
struct JUNKBOARDRUSH_API FSoundEffectData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player", meta=(Tooltip = ""))
	TArray<FSoundEffectStructData> PlayerSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hoverboard", meta=(Tooltip = ""))
	TArray<FSoundEffectStructData> BoardSounds;
};