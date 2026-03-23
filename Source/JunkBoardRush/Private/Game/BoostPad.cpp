
#include "Game/BoostPad.h"
#include "Framework/JBRCharacter.h"
#include "Framework/JBRCustomCMC.h"

ABoostPad::ABoostPad()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABoostPad::AddPlayerBoost(AJBRCharacter* Character)
{
	if (!Character->HasAuthority()) return;

	float BoostValue = IsRamp ? Character->GetPlayerSpeedData().BoostPadRampSpeedBoost : Character->GetPlayerSpeedData().BoostPadSpeedBoost;
	Character->CustomCMC->AddSpeedBoost(BoostValue, true, false);

	//Cast<AJBRGameState>(GetWorld()->GetGameState())->SoundManager->PlaySoundFromNameID(FName("BoostPad"));

	Character->TricksComponent->AttemptPossibleInteraction(this);
}

void ABoostPad::TriggerContextAction(AJBRCharacter* Initiator)
{
	FPlayerJunkData JunKData = Initiator->GetPlayerJunkData();
	FPlayerSpeedData SpeedData = Initiator->GetPlayerSpeedData();
	
	Initiator->MaterialBarComponent->AddJunk(JunKData.ActionBoostPadJunk);
	Initiator->CustomCMC->AddSpeedBoost(SpeedData.ActionBoostPadBoost, true, false);
}