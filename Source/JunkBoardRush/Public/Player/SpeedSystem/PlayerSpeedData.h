
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "PlayerSpeedData.generated.h"

USTRUCT(Blueprintable)
struct JUNKBOARDRUSH_API FPlayerSpeedData : public FTableRowBase 
{
	GENERATED_BODY()

#pragma region Controller

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CMC - Movement",
		meta = (ToolTip = "Définit la vitesse de base du joueur."))
	float BaseMaxWalkSpeed = 1200.f;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Movement",
	meta =(ToolTip = "Définit la vitesse MAXIMALE absolue."))
	float AbsoluteMaxSpeed = 5000.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Movement",
	meta =(ToolTip = "Définit la vitesse MAXIMALE absolue, en portant la relic."))
	float AbsoluteMaxSpeedHolderRelic = 4000.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Movement",
	meta =(ToolTip = "Définit la puissance (ou réduction) de sortie d'une slope au moment de décoler."))
	float SlopeExitSpeedMultiplier = 1.25f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Movement",
	meta =(ToolTip = "Définit les palliers de vitesse pour les VFX, ou autre logique relative aux palliers de vitesse."))
	TArray<float> SpeedBoostThreshold = TArray<float>({2000, 3000, 4000});

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CMC - Movement",
		meta = (ToolTip = "Définit le multiplicateur de vitesse sous instabilité à chaque seconde."))
	float InstabilitySpeedPerSeconde = 50.f;

#pragma endregion Controller
	
#pragma region Drift

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Drift",
	meta = (ToolTip = "Définit la force du boost quand on effecture un petit drift boost."))
	float DriftSmallBoostForce = 500.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Drift",
	meta = (ToolTip = "Définit la force du boost quand on effecture un gros drift boost."))
	float DriftBigBoostForce = 500.0f;

#pragma endregion Drift
	
#pragma region Falling

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Air Control",
	meta =(ToolTip = "Définit la vitesse maximum de chute."))
	float MaxFallingSpeed = 2048.f;

#pragma endregion Falling
	
#pragma region Dash

	UPROPERTY(EditDefaultsOnly, Category = "CMC - Dash")
	float DashForce = 3000.0f;

#pragma endregion Dash

#pragma region Speed Boost

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Speed Boost",
	meta =(ToolTip = "Définit la division maximale que le debuff peut opérer sur la vélocité."))
	float MaxSpeedDebuff = 1.2f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Speed Boost",
	meta =(ToolTip = "Définit à quelle vitesse le debuff de vitesse revient à sa valeur nulle une fois appliqué."))
	float SpeedDebuffDecaySpeed = 10.f;

#pragma endregion Speed Boost

#pragma region Speed Boost Specific

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Speed Boost - Specific")
	float BoostPadSpeedBoost = 1000.f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Speed Boost - Specific")
	float BoostPadRampSpeedBoost = 4000.f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Speed Boost - Specific")
	float GateSpeedBoost = 2000.f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Speed Boost - Specific")
	float GateSpeedBoostHolder = 2000.f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Speed Boost - Specific")
	float GateFinalExtractSpeedBoost = 3250.f;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Speed Boost - Specific")
	float JunkDropSpeedBoost = 150.f;

#pragma endregion Speed Boost Specific

#pragma region Rails

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Rail",
	meta =(ToolTip = "Définit la vitesse minimale du joueur sur les rails."))
	float MinRailSpeed = 500.0f;

#pragma endregion Rails

#pragma region Slope

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CMC - Slope", meta = (ToolTip = "INSERT TOOLTIP"))
	float SlopeSpeedBoostIntensity = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CMC - Slope", meta = (ToolTip = "INSERT TOOLTIP"))
	float SlopeSpeedBoostMaxSpeedMultiplier = .1f;

#pragma endregion Slope

#pragma region Context Actions

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context Actions - Boost", meta=(Tooltip = ""))
	float ActionBoostPadBoost = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context Actions - Boost", meta=(Tooltip = ""))
	float ActionRailBoost = 500.f;

#pragma endregion Context Actions
	
};
