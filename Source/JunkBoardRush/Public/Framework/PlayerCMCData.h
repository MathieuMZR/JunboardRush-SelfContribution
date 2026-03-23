
#pragma once

#include "CoreMinimal.h"

#include "Engine/DataTable.h"
#include "Curves/CurveFloat.h"

#include "PlayerCMCData.generated.h"

USTRUCT(BlueprintType)
struct JUNKBOARDRUSH_API FPlayerCMCData : public FTableRowBase
{
	GENERATED_BODY()
	
#pragma region Movement

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Movement",
	meta =(ToolTip = "Permet de tester le controlleur sans l'avance constante."))
	bool AutoMove = false;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Movement",
	meta =(ToolTip = "Définit la rapidité du gain de vitesse de déplacement que le joueur obtient sur le temps lorsqu’il se déplace sur le sol."))
	float AccelerationRate = 4000.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Movement",
	meta =(ToolTip = "Définit à quel point le mouvement du joueur sera affecté par la friction quand il bouge au sol."));
	float GroundFriction = 10.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Movement",
	meta =(ToolTip = "Définit à quel point le mouvement du joueur sera affecté par la friction quand il bouge au sol mais à vitesse absolue maximale."));
	float GroundFrictionAbsoluteSpeed = 2.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Movement",
	meta =(ToolTip = "Définit à quel point le mouvement du joueur sera affecté par la friction quand il freine."));
	float BrakeIntensity = 75.0f;

#pragma endregion Movement

#pragma region Direction

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Direction",
	meta = (ToolTip = "Définit la force appliquée quand le joueur décide de tourner à droite ou bien à gauche. Cette force détermine à quel point le virage est serré ou non."))
	float TurnForce = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Direction",
	meta = (ToolTip = "Définit la force appliquée quand le joueur décide d’effectuer un dérapage à vitesse minimale."))
	float DriftTurnForceDriftLow = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Direction",
	meta = (ToolTip = "Définit la force appliquée quand le joueur décide d’effectuer un dérapage à vitesse maximale."))
	float DriftTurnForceDriftHigh = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Direction",
	meta = (ToolTip = "Définit la douceur de changement de direction entre la direction d'input et celle reçu par le controlleur."))
	float TurnSmoothingSpeed = 100.0f;
    
#pragma endregion Direction

#pragma region Drift

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Drift",
	meta = (ToolTip = "Définit l'input minimale pour déclencher le drift."))
	float DriftDeadZone = 0.3f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Drift",
	meta = (ToolTip = "Définit la durée minimale pour effectuer un petit drift boost. En secondes."))
	float DriftSmallBoostTiming = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Drift",
	meta = (ToolTip = "Définit la durée minimale pour effectuer un gros drift boost. En secondes."))
	float DriftBigBoostTiming = 2.5f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Drift",
	meta =(ToolTip = "Définit à quel point le mouvement du joueur sera affecté par la friction quand il bouge au sol PENDANT le drift."));
	float GroundFrictionWhileDriftingFactor = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Drift",
	meta =(ToolTip = "Définit à quel point la vélocité du joueur va s'update rapidement (ou pas) quand il drift."));
	float MovementDriftScalarModifier = 0.5f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Drift",
	meta =(ToolTip = "Définit à quel point le player va appliquer de la force quand il freine."));
	float MovementBrakeScalarDeprecation = 0.5f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Drift",
	meta =(ToolTip = "Définit à la vitesse minimale requise pour drifter."));
	float MinSpeedForDrift = 1500;

#pragma endregion Drift

#pragma region Air

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Air",
	meta =(ToolTip = "Définit la frition que le joueur va subir pendant qu'il est dns le airs."))
	float AirFriction = 6000.0f;

#pragma endregion Air

#pragma region Air Control

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Air Control",
	meta =(ToolTip = "Définit la rapidité du gain de vitesse de déplacement que le joueur obtient quand il est dans les airs."))
	float AccelerationRateInAir = 6000.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Air Control",
	meta =(ToolTip = "Définit le air control de base. Compris entre 0 et 1."))
	float AirControlBase = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Air Control",
	meta =(ToolTip = "Si la Vélocité en X est inférieure à ce threshold, alors elle est multiplié par AirControlLow/HighMultiplier pour rattraper la valeur. Mesuré en unités d'Unreal."))
	float AirControlBoostVelocityThreshold = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Air Control",
	meta =(ToolTip = "Définit à quel point le air control est multiplié pour atteindre le AirControlBoostVelocityThreshold, à vitesse normale."))
	float AirControlLowMultiplier = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Air Control",
	meta =(ToolTip = "Définit à quel point le air control est multiplié pour atteindre le threshold, à vitesse maximale."))
	float AirControlHighMultiplier = 2.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Air Control",
	meta =(ToolTip = "Définit la gravité de base."))
	float Gravity = 6.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Air Control",
	meta =(ToolTip = "Définit la gravité de base."))
	float GravityWhileJumping = 8.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Air Control",
	meta =(ToolTip = "Définit la vitesse la gravité va s'update."))
	float GravityUpdateSpeed = 8.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Air Control",
	meta =(ToolTip = "Définit la vitesse la gravité va s'update pendant le air glide."))
	float GravityUpdateSpeedGliding = 8.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Air Control",
	meta = (ToolTip = "Définit la force appliquée quand le joueur décide de tourner à droite ou bien à gauche dans les airs. Cette force détermine à quel point le virage est serré ou non."))
	float TurnForceInAir = 100.0f;

#pragma endregion Air Control

#pragma region Jump
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Jump",
		meta = (ToolTip = "TODO"))
	float JumpMaxHoldTime = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Jump",
		meta = (ToolTip = "TODO"))
	float JumpSustainForce = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Jump",
		meta = (ToolTip = "TODO"))
	FVector2D MinMaxJumpForce = FVector2D(1500, 3000);

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Jump",
		meta = (ToolTip = "Définit la durée durant laquelle on peut sauter après avoir quitté le sol en TOMBANT."))
	float JumpBufferDuration = 0.5f;

#pragma endregion Jump
	
#pragma region Dash
	
	UPROPERTY(EditDefaultsOnly, Category = "CMC - Dash")
	float DashCooldown = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CMC - Dash")
	float DashDuration = 1.0f;
	
#pragma endregion Dash

#pragma region Rails
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Rail",
	meta =(ToolTip = "Définit la vitesse à laquelle le joueur va atteindre MinRailSpeed quand il reste sur une rail."))
	float RailDecelerationRate = 2.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Rail",
	meta =(ToolTip = "Définit à quel angle le joueur va être projeté si il se retire de la ramp manuellement."))
	float RailFlickAngle = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Rail",
	meta =(ToolTip = "Définit à quelle puissance le joueur va être projeté si il se retire de la ramp manuellement."))
	float RailFlickForce = 500.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Rail",
	meta =(ToolTip = "Définit à quelle puissance le joueur va être projeté VERTICALEMENT si il se retire de la ramp manuellement."))
	float RailFlickUpwardBoost = 200.0f;

#pragma endregion Rails

#pragma region Relic

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Relic",
	meta =(ToolTip = "Définit le radius de vol de relique via tricks."))
	float ManualRelicStealRadius = 400.f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Relic",
	meta =(ToolTip = "Définit le radius de vol de relique automatique."))
	float AbsoluteRelicStealRadius = 25.f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Relic",
	meta =(ToolTip = "Définit le delay de vol entre deux joueurs."))
	float StealDelay = 1.f;

#pragma endregion Relic

#pragma region Air Gliding

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Air Glide",
	meta =(ToolTip = "Définit à quel point la gravité du air glide est forte selon l'inclinaison."));
	FVector2D AirGlideMinMaxGravity;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CMC - Air Glide",
	meta =(ToolTip = "Définit à quel point la friction du air glide est forte selon l'inclinaison."));
	FVector2D AirGlideFrictionMinMax;
	
#pragma endregion Air Gliding
};
