
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Gadgets/JunkPile.h"
#include "JBRController.generated.h"

class AWorldDeathCamera;
struct FKillEntry;
/**
 * 
 */
UCLASS(Blueprintable)
class JUNKBOARDRUSH_API AJBRController : public APlayerController
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, Category = "Camera")
	TSubclassOf<AWorldDeathCamera> WorldDeathCameraClass;
	
	UFUNCTION(Client, Reliable)
    void Client_OnJunkCollected(AJunkPile* JunkPile, float RespawnTime);

	UFUNCTION(Client, Unreliable)
	void Client_NotifyLocalKill(FKillEntry Entry);

	UFUNCTION(Server, Reliable)
	void Server_DeathCameraBehavior(AJBRCharacter* Killer);
	
	UFUNCTION()
	void EnterDeathCameraBehavior(AJBRCharacter* Killer);

	UFUNCTION()
	void ExitDeathCameraBehavior();

private:

	AWorldDeathCamera* LastDeathCam;
};
