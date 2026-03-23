
#include "Player/Camera/PlayerCameraShakeComponent.h"

#include "Framework/JBRCharacter.h"
#include "Libraries/GenericFunction.h"
#include "Player/Camera/CameraShakeData.h"

#pragma region Init

UPlayerCameraShakeComponent::UPlayerCameraShakeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerCameraShakeComponent::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<AJBRCharacter>(GetOwner());
}

void UPlayerCameraShakeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

#pragma endregion Init

#pragma region Camera Shake

void UPlayerCameraShakeComponent::StartCameraShake(CameraShakeEnum CameraShake, float IncomingValue)
{
	APlayerController* PC =
		Cast<APlayerController>(GetOwner()->GetInstigatorController());
	if (!PC) return;

	const TSoftClassPtr<UCameraShakeBase>* ShakePtr =
		GetShakeByEnum(CameraShake);
	if (!ShakePtr) return;

	TSubclassOf<UCameraShakeBase> ShakeClass = ShakePtr->LoadSynchronous();
	if (!ShakeClass) return;

	APlayerCameraManager* CamManager =
		PC->PlayerCameraManager;

	float Scale = 1.f;

	switch (CameraShake)
	{
	case CameraShakeEnum::Damage:
		{
			const FVector2D& MinMax =
				GetPlayerCMCData().CameraShakeDamageLowHighMult;
			
			float DamageIntensity = FMath::Clamp(FMath::Abs(IncomingValue) / Character->HealthComponent->GetMaxHealth(), 0.f, 1.f);
			Scale = FMath::Lerp(MinMax.X, MinMax.Y, DamageIntensity);
			
			break;
		}
	case CameraShakeEnum::Landing:
		{
			const FVector2D& MinMax =
				GetPlayerCMCData().CameraShakeLandingLowHighMult;

			float FallingIntensity = FMath::Clamp(FMath::Abs(IncomingValue) / Character->GetPlayerSpeedData().MaxFallingSpeed, 0.f, 1.f);
			Scale = FMath::Lerp(MinMax.X, MinMax.Y, FallingIntensity);
			
			break;
		}
	default:
		break;
	}

	CamManager->StartCameraShake(ShakeClass, Scale);
}

const TSoftClassPtr<UCameraShakeBase>* UPlayerCameraShakeComponent::GetShakeByEnum(CameraShakeEnum CameraShake)
{
	switch (CameraShake)
	{
		case CameraShakeEnum::Base:
			return &GetPlayerCMCData().CameraShakeBase;
			
		case CameraShakeEnum::Damage:
			return &GetPlayerCMCData().CameraShakeDamage;
			
		case CameraShakeEnum::AdrenalineStart:
			return &GetPlayerCMCData().CameraShakeAdrenalineStart;

		case CameraShakeEnum::AdrenalineConstant:
			return &GetPlayerCMCData().CameraShakeAdrenalineConstant;
			
		case CameraShakeEnum::Landing:
			return &GetPlayerCMCData().CameraShakeLanding;
			
		case CameraShakeEnum::GadgetBoost:
			return &GetPlayerCMCData().CameraShakeGadgetBoost;

		case CameraShakeEnum::RailConstant:
			return &GetPlayerCMCData().CameraShakeRailConstant;
		
	}
	return nullptr;
}

#pragma endregion Camera Shake

const FCameraShakeData& UPlayerCameraShakeComponent::GetPlayerCMCData() const
{
	return UGenericFunction::GetDataRow<FCameraShakeData>(CameraShakeData, TEXT("CameraShakeData"));
}

