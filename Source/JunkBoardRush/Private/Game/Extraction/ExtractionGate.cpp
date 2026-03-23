
#include "Game/Extraction/ExtractionGate.h"

#include "NiagaraFunctionLibrary.h"
#include "Framework/JBRCustomCMC.h"
#include "Framework/JBRGameMode.h"
#include "Framework/JBRPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "TrackersPlaytest/TelemetryDataVisualizer.h"

AExtractionGate::AExtractionGate()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AExtractionGate::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AExtractionGate, bIsGateActive);
	DOREPLIFETIME(AExtractionGate, GateColor);
	DOREPLIFETIME(AExtractionGate, CurrentGateHealth);
}

void AExtractionGate::OnRep_GateColor()
{
	TArray<UStaticMeshComponent*> MeshComponents;
	GetComponents<UStaticMeshComponent>(MeshComponents);
	for (UStaticMeshComponent* MeshComp : MeshComponents)
	{
		MeshComp->SetVectorParameterValueOnMaterials(TEXT("Color"), GateColor);
	}
}

void AExtractionGate::OnRep_IsGateActive()
{
	OnGateStateChanged.Broadcast(bIsGateActive);
	OnRep_GateColor();
}

void AExtractionGate::OnRep_GateHealth()
{
	// TODO: Maybe change gate color based on health idk
}

void AExtractionGate::BeginPlay()
{
	Super::BeginPlay();

	UpdateStencil(0);

	CurrentGateHealth = MaxGateHealth;
}

void AExtractionGate::ProcessGateEntry(AJBRCharacter* Character)
{
	if (!Character) return;
	
	if (!HasAuthority())
	{
		return;
	}
	
	FGadgetItem RelicItem = FGadgetItem();
	RelicItem.GadgetClass = ARelic::StaticClass();

	UCraftingComponent* CraftingComp = Character->CraftingComponent;
	bool bHasRelic = CraftingComp && CraftingComp->HasItemTypeInInventory(RelicItem);
	
	if (bHasRelic)
	{
		// Carrier stuff
		if (bIsGateActive)
		{
			AJBRGameState* GS = GetWorld()->GetGameState<AJBRGameState>();
			ARelic* RelicActor = GS->RelicGlobalReference;

			AJBRGameMode* GameMode = Cast<AJBRGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			AJBRPlayerState* PS = Cast<AJBRPlayerState>(Character->GetPlayerState());
			
			if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
			{
				if (RelicGateEntrySound)
				{
					PC->ClientPlaySound(RelicGateEntrySound);
				}
			}

			if (RelicActor)
			{
				bool bExtracted = RelicActor->AddCharge();

				if (bExtracted)
				{
					// Final extraction
					if (GameMode && PS)
					{
						GameMode->ExtractRelic(ExtractVictoryPoints, PS, this, true);
						Character->CustomCMC->AddSpeedBoost(Character->GetPlayerSpeedData().GateFinalExtractSpeedBoost, true, true);
						Multicast_PlayFinalExtractionVFX(GetActorLocation(), Character);
					}
					
					CraftingComp->ScoreRelic(); // Clears inventory
					
					Cast<AJBRGameMode>(GetWorld()->GetAuthGameMode())->LogDiscreteEvent(Cast<APlayerController>(Character->GetController()), ETelemetryEvent::GateScore, GetActorLocation());
					
					OnRelicExtracted.Broadcast(Character);
					HandleGateDeactivation();
				}
				else
				{
					// Base charge
					if (GameMode && PS)
					{
						GameMode->ExtractRelic(ChargeVictoryPoints, PS, this, false);
					}
					
					OnRelicCharged.Broadcast(Character);

					// Deactivate gate after carrier passes
					HandleGateDeactivation();
				}
			}
		}
	}

	float GateSpeedBoost = Character->GetPlayerSpeedData().GateSpeedBoost;
	if (Character->HasRelic()) GateSpeedBoost = Character->GetPlayerSpeedData().GateSpeedBoostHolder;
	Character->CustomCMC->AddSpeedBoost(GateSpeedBoost, true, true);
}

void AExtractionGate::HandleGateDeactivation()
{
	if (!bIsGateActive) return;

	SetGateActive(false);
	OnRep_IsGateActive();
}

void AExtractionGate::SetGateActive(bool bActive)
{
	if (HasAuthority())
	{
		bIsGateActive = bActive;
		
		if (bIsGateActive)
		{
			CurrentGateHealth = MaxGateHealth;
		}

		if (bIsGateActive)
			ChangeGateColor(FVector(0.f, 0.f, 1.f));
		else
			ChangeGateColor(FVector(1.f, 0.f, 0.f));
		
		OnRep_IsGateActive();
	}
}

float AExtractionGate::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || !bIsGateActive) return 0.f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	CurrentGateHealth -= DamageAmount;
	OnRep_GateHealth();

	if (CurrentGateHealth <= 0.f)
	{
		HandleGateDeactivation();
	}

	return ActualDamage;
}

void AExtractionGate::ChangeGateColor(FVector NewColor)
{
	if (HasAuthority())
	{
		GateColor = NewColor;
		OnRep_GateColor(); // Call locally on server
	}
}

void AExtractionGate::Multicast_PlayFinalExtractionVFX_Implementation(FVector Location, AJBRCharacter* Extractor)
{
	const AJBRCharacter* Character = Cast<AJBRCharacter>(Extractor);
    
	if (!Character || !Character->CosmeticsConfig)
	{
		return;
	}
    
	if (UNiagaraSystem* ExtractionVFX = Character->CosmeticsConfig->RelicExtractVFX.LoadSynchronous())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ExtractionVFX,
			Location
		);
	}
}

void AExtractionGate::UpdateStencil(int ID)
{
	TArray<UStaticMeshComponent*> MeshComponents;
	GetComponents<UStaticMeshComponent>(MeshComponents);
	
	StencilID = ID;
	for (auto mesh : MeshComponents)
	{
		mesh->SetCustomDepthStencilValue(StencilID);
	}
}

