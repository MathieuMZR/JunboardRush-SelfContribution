
#include "Game/Extraction/GateManager.h"
#include "Framework/JBRGameState.h"

// Sets default values
AGateManager::AGateManager()
{

}

void AGateManager::Init(ARelic* Relic)
{
	if (!Relic) return;
	
	Relic->OnRelicRespawn.AddDynamic(this, &AGateManager::OnRelicSpawned);
	Relic->OnRelicStartRespawn.AddDynamic(this, &AGateManager::OnRelicStartRespawn);
	Relic->OnRelicOwnershipChanged.AddDynamic(this, &AGateManager::OnRelicOwnershipChanged);
	Relic->OnRelicDropped.AddDynamic(this, &AGateManager::OnRelicDropped);
	
	SelectNewConfiguration();
}

void AGateManager::OnRelicSpawned()
{
	SelectNewConfiguration();
}

void AGateManager::OnRelicStartRespawn()
{
	DisableAllGates();
	CurrentActiveSet.Empty();
}

void AGateManager::OnRelicOwnershipChanged(AJBRCharacter* NewOwner, AJBRCharacter* PreviousOwner)
{
	if (PreviousOwner != nullptr && NewOwner != PreviousOwner)
	{
		ReactivateCurrentSet();
	}
}

void AGateManager::OnRelicDropped()
{
	ReactivateCurrentSet();
}

void AGateManager::SelectNewConfiguration()
{
	DisableAllGates();
	CurrentActiveSet.Empty();

	if (GateConfigurations.Num() > 0)
	{
		int32 Idx = 0;

		if (GateConfigurations.Num() > 1)
		{
			do
			{
				Idx = FMath::RandRange(0, GateConfigurations.Num() - 1);
			}
			while (Idx == LastConfigurationIndex);
		}
		else
		{
			Idx = FMath::RandRange(0, GateConfigurations.Num() - 1);
		}

		LastConfigurationIndex = Idx;
		CurrentActiveSet = GateConfigurations[Idx].Gates;

		for (AExtractionGate* Gate : CurrentActiveSet)
		{
			if (Gate)
			{
				Gate->SetGateActive(true);
			}
		}
	}
}

void AGateManager::DisableAllGates()
{
	for (FGateConfiguration& Config : GateConfigurations)
	{
		for (AExtractionGate* Gate : Config.Gates)
		{
			if (Gate)
			{
				Gate->SetGateActive(false);
			}
		}
	}
}

void AGateManager::ReactivateCurrentSet()
{
	DisableAllGates();
	for (AExtractionGate* Gate : CurrentActiveSet)
	{
		if (Gate)
		{
			Gate->SetGateActive(true);
		}
	}
}

