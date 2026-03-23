// AnimNotifyInterface.h
#pragma once

#include "UObject/Interface.h"
#include "TricksAnimNotifyInterface.generated.h"

UINTERFACE(Blueprintable)
class UTricksAnimNotifyInterface : public UInterface
{
	GENERATED_BODY()
};

class ITricksAnimNotifyInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnBoardAnimNotifyTriggered();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnPlayerAnimNotifyTriggered();
};
