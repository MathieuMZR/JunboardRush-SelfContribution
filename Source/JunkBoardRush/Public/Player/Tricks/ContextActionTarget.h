
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ContextActionTarget.generated.h"

class AJBRCharacter;

UINTERFACE()
class UContextActionTarget : public UInterface
{
	GENERATED_BODY()
};

// SERVER-ONLY INTERFACE !!
class JUNKBOARDRUSH_API IContextActionTarget
{
	GENERATED_BODY()

public:
	virtual void TriggerContextAction(AJBRCharacter* Initiator) = 0;
};
