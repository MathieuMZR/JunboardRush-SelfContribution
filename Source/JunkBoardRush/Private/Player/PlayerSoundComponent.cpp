
#include "Player/PlayerSoundComponent.h"

UPlayerSoundComponent::UPlayerSoundComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerSoundComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerSoundComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

