
#pragma once

#include "CoreMinimal.h"
#include "SoundEffectData.h"
#include "GameFramework/Actor.h"
#include "SoundManager.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class JUNKBOARDRUSH_API USoundManager : public UActorComponent
{
	GENERATED_BODY()

public:
	
	USoundManager();

protected:

	virtual void BeginPlay() override;

public:

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* This) override;

#pragma region Play Sounds

	UFUNCTION(BlueprintCallable)
	void PlaySoundFromNameID(FName NameID, FVector Position = FVector(0,0,0));

	void PlaySound(FSoundEffectStructData Data, FVector Position = FVector(0,0,0));
	void RemoveSound(UAudioComponent* SoundToRemove);

	TArray<UAudioComponent*> ActivesSounds;

#pragma endregion Play Sounds

#pragma region Utilities
	
	FSoundEffectStructData GetSoundFromTextID(FName NameID);

	UFUNCTION(BlueprintCallable)
	const FSoundEffectData& GetSoundEffectData() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound - Data")
	FDataTableRowHandle SoundEffectData;

#pragma endregion Utilities
};
