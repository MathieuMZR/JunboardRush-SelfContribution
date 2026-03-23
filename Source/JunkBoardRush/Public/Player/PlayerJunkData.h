
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PlayerJunkData.generated.h"

USTRUCT(BlueprintType)
struct JUNKBOARDRUSH_API FPlayerJunkData : public FTableRowBase
{
	GENERATED_BODY()

#pragma region PVP
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PVP - Junk")
	int32 JunkDmgDropQuantity = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PVP - Junk")
	int32 JunkCrashDropQuantity = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PVP - Junk")
	float DropScatterForce = 500.0f;

#pragma endregion PVP

#pragma region Context Actions
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context Actions - Junk", meta=(Tooltip = ""))
	int ActionBoostPadJunk = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context Actions - Junk", meta=(Tooltip = ""))
	int ActionBumperJunk = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context Actions - Junk", meta=(Tooltip = ""))
	int ActionRailJunk = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context Actions - Junk", meta=(Tooltip = ""))
	int ActionJunkPileJunk = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context Actions - Junk", meta=(Tooltip = ""))
	int JunkDropsWhenContextAction = 5;

#pragma endregion Context Actions
	
};

