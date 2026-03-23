
#include "Sound/SoundManager.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Libraries/GenericFunction.h"

#pragma region Init

USoundManager::USoundManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USoundManager::BeginPlay()
{
	Super::BeginPlay();
}

void USoundManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* This)
{
	Super::TickComponent(DeltaTime, TickType, This);
}

#pragma endregion Init

#pragma region Play Sounds

void USoundManager::PlaySoundFromNameID(FName NameID, FVector Position)
{
	FSoundEffectStructData Data = GetSoundFromTextID(NameID);
	PlaySound(Data, Position);
}

void USoundManager::PlaySound(FSoundEffectStructData Data, FVector Position)
{
	if (Data.Sounds.Num() == 0) return;
	
	int IndexCue = static_cast<int>(FMath::FRandRange(0.f, Data.Sounds.Num() - 1));
	USoundBase* Cue = Data.Sounds[IndexCue];

	float RandVolume = FMath::FRandRange(Data.MinMaxVolume.X, Data.MinMaxVolume.Y);
	float RandPitch = FMath::FRandRange(Data.MinMaxPitch.X, Data.MinMaxPitch.Y);

	UAudioComponent* CreatedSound = nullptr;

	if (Data.IsSpatialized)
	{
		CreatedSound = UGameplayStatics::SpawnSoundAtLocation(
			GetWorld(), 
			Cue, 
			Position,
			FRotator(),
			RandVolume, 
			RandPitch, 
			Data.StartTime, 
			Data.AttenuationSettings,
			nullptr,
			Data.AutoDestroy
		);
	}
	else
	{
		CreatedSound = UGameplayStatics::SpawnSound2D(GetWorld(), Cue, RandVolume, RandPitch, Data.StartTime, nullptr,
			false, Data.AutoDestroy);
	}

	CreatedSound->OnAudioFinishedNative.AddUObject(this, &USoundManager::RemoveSound);
	ActivesSounds.Add(CreatedSound);
}

void USoundManager::RemoveSound(UAudioComponent* SoundToRemove)
{
	if (SoundToRemove == nullptr) return;
	ActivesSounds.Remove(SoundToRemove);
}

#pragma endregion Play Sounds

#pragma region Utilities

FSoundEffectStructData USoundManager::GetSoundFromTextID(FName NameID)
{
	for (const FSoundEffectStructData& StructData : GetSoundEffectData().PlayerSounds)
	{
		if (StructData.NameID == NameID)
		{
			return StructData;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("No sound effect found for ID: %s"), *NameID.ToString());
	return FSoundEffectStructData();
}

#pragma endregion Utilities

#pragma region Data

const FSoundEffectData& USoundManager::GetSoundEffectData() const
{
	return UGenericFunction::GetDataRow<FSoundEffectData>(SoundEffectData, TEXT("SoundEffectData"));
}

#pragma endregion Data

// Example line : Cast<AJBRGameState>(GetWorld()->GetGameInstance())->SoundManager->PlaySoundFromTextID("SOUND_TEXT_ID");

