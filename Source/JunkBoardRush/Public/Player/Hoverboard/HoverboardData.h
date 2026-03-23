
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "HoverboardData.generated.h"

USTRUCT(BlueprintType)
struct JUNKBOARDRUSH_API FHoverboardData : public FTableRowBase 
{
	GENERATED_BODY()
	
#pragma region Rotation Roll
	
	UPROPERTY(EditDefaultsOnly, Category = "Rotation - Roll",
		meta = (ToolTip = "Définit l'angle maximal d'angle Roll quand le joueur tourne."))
	float MaxRollAngleLow = 45.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Rotation - Roll",
		meta = (ToolTip = "Définit l'angle maximal d'angle Roll quand le joueur tourne à vitesse maximale."))
	float MaxRollAngleHigh = 45.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Rotation - Roll",
		meta = (ToolTip = "Définit la vitesse d'update de la rotation Roll à vitesse minimale."))
	float RollSmoothingSpeedLow = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category = "Rotation - Roll",
		meta = (ToolTip = "Définit la vitesse d'update de la rotation Roll à vitesse maximale."))
	float RollSmoothingSpeedHigh = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Rotation - Roll",
		meta = (ToolTip = "Définit la manière dont le Roll va s'update le long d'une courbe."))
	TObjectPtr<UCurveFloat> RollCurve;

#pragma endregion Rotation Roll

#pragma region Rotation Pitch
	
	UPROPERTY(EditDefaultsOnly, Category = "Rotation - Pitch",
		meta = (ToolTip = "Définit l'angle maximal d'angle Pitch quand le joueur navigue sur une slope."))
	float MaxPitchAngle = 45.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Rotation - Pitch",
		meta = (ToolTip = "Définit la vitesse d'update de l'angle Pitch quand le joueur navigue sur une slope à vitesse minimale."))
	float PitchMinLerpSpeed = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Rotation - Pitch",
		meta = (ToolTip = "Définit la vitesse d'update de l'angle Pitch quand le joueur navigue sur une slope à vitesse maximale."))
	float PitchMaxLerpSpeed = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Rotation - Pitch",
		meta = (ToolTip = "Do not touch."))
	float PitchMinAngleChange = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Rotation - Pitch",
		meta = (ToolTip = "Do not touch."))
	float PitchMaxAngleChange = 10.0f;

#pragma endregion Rotation Pitch

#pragma region Air Glide

	UPROPERTY(EditDefaultsOnly, Category = "Rotation - Air Glide",
		meta = (ToolTip = "Définit l'angle maximal d'angle Pitch quand le joueur bouge son input en Y."))
	float MaxAirGlidePitchAngle = 45.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Rotation - Air Glide",
		meta = (ToolTip = "Définit la vitesse d'update du air glide visuel. Min = Input Y 0, Max = Input Y > 0"))
	FVector2D AirGlidePitchSpeed;
	
#pragma endregion Air Glide
	
};
