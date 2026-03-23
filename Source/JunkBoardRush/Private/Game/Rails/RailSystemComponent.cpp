#include "Game/Rails/RailSystemComponent.h"
#include "Game/Rails/GrindRail.h"

URailSystemComponent::URailSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URailSystemComponent::SetLastRail(AGrindRail* Rail)
{
	LastRail = Rail;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ClearLastRailTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
			ClearLastRailTimerHandle,
			this,
			&URailSystemComponent::ClearOldRail,
			ClearRailDelay,
			false
		);
	}
}

void URailSystemComponent::ClearOldRail()
{
	LastRail = nullptr;
}

void URailSystemComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URailSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

