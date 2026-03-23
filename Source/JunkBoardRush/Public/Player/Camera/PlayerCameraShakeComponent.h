
#pragma once

#include "CoreMinimal.h"
#include "CameraShakeData.h"
#include "Components/ActorComponent.h"
#include "PlayerCameraShakeComponent.generated.h"

class AJBRCharacter;

UENUM(BlueprintType)
enum class CameraShakeEnum : uint8 {
	Base = 0 UMETA(DisplayName = "Camera Shake Base"),
	Damage = 1 UMETA(DisplayName = "Camera Shake Damage"),
	AdrenalineStart = 2 UMETA(DisplayName = "Camera Shake Adrenaline Start"),
	AdrenalineConstant = 3 UMETA(DisplayName = "Camera Shake Adrenaline Constant"),
	Landing = 4 UMETA(DisplayName = "Camera Shake Landing"),
	GadgetBoost = 5 UMETA(DisplayName = "Camera Shake Gadget Boost"),
	RailConstant = 6 UMETA(DisplayName = "Camera Shake Rail Constant"),
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class JUNKBOARDRUSH_API UPlayerCameraShakeComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPlayerCameraShakeComponent();

	AJBRCharacter* Character;

#pragma region Init

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
#pragma endregion Init

#pragma region Camera Shake
	
	void StartCameraShake(CameraShakeEnum CameraShake, float IncomingValue);

	const TSoftClassPtr<UCameraShakeBase>* GetShakeByEnum(CameraShakeEnum CameraShake);

#pragma endregion Camera Shake

	const FCameraShakeData& GetPlayerCMCData() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera - Data")
	FDataTableRowHandle CameraShakeData;
};
