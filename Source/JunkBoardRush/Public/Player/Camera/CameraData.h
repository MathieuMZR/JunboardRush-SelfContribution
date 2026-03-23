
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "CameraData.generated.h"

USTRUCT(BlueprintType)
struct JUNKBOARDRUSH_API FCameraData : public FTableRowBase
{
	GENERATED_BODY()

#pragma region Socket Offset

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Arm Socket",
		meta = (ToolTip = "Définit de combien la camera va se déplacer en X pendant que le joueur tourne."))
	float SocketOffsetXTurn = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Arm Socket",
		meta = (ToolTip = "Définit la vitesse à laquelle le déplacement de la camera va s'update."))
	float SocketOffsetXTurnUpdateSpeed = 0.35f;

#pragma endregion Socket Offset

#pragma region Pitch Arm Rotation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Arm Rotation Pitch",
		meta = (ToolTip = "Définit l'angle pitch de base en fonction de la vitesse min et max."))
	FVector2D MinMaxDefaultPitchArmRotation = FVector2D(-20.f, -35.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Arm Rotation Pitch",
		meta = (ToolTip = "Aucune idée mais ça marche mieux avec ça."))
	float PitchMaxSpeedRotation = -25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Arm Rotation Pitch",
		meta = (ToolTip = "Définit l'angle pitch maximal lors du behavior de chute."))
	float PitchFallingMaxArmAngle = 55.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Arm Rotation Pitch",
		meta = (ToolTip = "Définit la vitesse d'update de l'angle pitch pendant le behavior de chute."))
	float PitchFallingArmAngleSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Arm Rotation Yaw",
		meta = (Toolip = "Définit l'angle en Pitch appliqué au Spring Arm quand le joueur tourne MANUELLEMENT sa caméra en Y."))
	float FreeModeMaxPitchAngle = 45.f;

#pragma endregion Pitch Arm Rotation

#pragma region Yaw Arm Rotation
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Arm Rotation Yaw",
		meta = (Toolip = "Définit l'angle en Yaw appliqué au Spring Arm quand le joueur tourne."))
	float YawTurnArmAngle = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Arm Rotation Yaw",
		meta = (Toolip = "Définit l'angle en Yaw appliqué au Spring Arm quand le joueur tourne MANUELLEMENT sa caméra en X."))
	float FreeModeMaxYawAngle = 45.f;

#pragma endregion Yaw Arm Rotation

#pragma region Roll Arm Rotation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Arm Rotation Roll",
		meta = (Toolip = "Définit l'angle en Roll appliqué au Spring Arm quand le joueur tourne."))
	float RollTurnArmAngle = 11.f;
	
#pragma endregion Roll Arm Rotation

#pragma region Arm Length

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Arm Rotation Length",
		meta = (Toolip = "Définit la distance au joueur appliquée au Spring Arm en fonction de la vitesse."))
	FVector2D MinMaxArmLengthAlongSpeed = FVector2D(300, 750);
	
#pragma endregion Arm Length

#pragma region Yaw Camera Rotation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Rotation Yaw",
		meta = (Toolip = "Définit l'angle en Roll appliqué à la caméra quand le joueur tourne."))
	float CameraYawTurnAngle = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Rotation Yaw",
		meta = (Toolip = "Définit la vitesse d'update de l'angle en Roll appliqué à la caméra quand le joueur tourne."))
	float CameraYawTurnAngleUpdateSpeed = 0.5f;
	
#pragma endregion Yaw Camera Rotation

#pragma region FOV
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV",
		meta = (Toolip = "Définit le FOV de base du joueur en fonction de la vitesse."))
	FVector2D MinMaxFOVAlongSpeed = FVector2D(90, 110);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV",
		meta = (Toolip = "Définit le multiplicateur du FOV quand le joueur drift."))
	float DriftFOVMultiplier = .75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV",
		meta = (Toolip = "Définit la vitesse d'update du FOV."))
	float FOVUpdateSpeed = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV",
		meta = (Toolip = "Définit à quel point la vitesse d'update de FOV est affectée quand le FOV est entrain de réduire."))
	float ReducingFOVSpeedMultiplier = .05f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV")
	float FOVKickStrength = 25.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV")
	float FOVKickMax = 10.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV")
	float FOVKickDecaySpeed = 10.f;

#pragma endregion FOV

#pragma region Falling Check

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection",
		meta = (Toolip = "Définit à partir de quelle hauteur la logique de falling de la caméra est appliquée."))
	float MinDistanceFromGroundForFallingState = 2000.f;
	
#pragma endregion Falling Check

#pragma region Lag

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default - Lag")
	float DefaultPositionLag = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default - Lag")
	float DefaultRotationLag = 4.f;

	//-----------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drift - Lag")
	float DriftPositionLag = 15.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drift - Lag")
	float DriftRotationLag = 10.f;

	//-----------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling - Lag")
	float FallingPositionLag = 5.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling - Lag")
	float FallingRotationLag = 60.f;

	//-----------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behind - Lag")
	float BehindPositionLag = 15.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behind - Lag")
	float BehindRotationLag = 150.f;

	//-----------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail - Lag")
	float RailPositionLag = 25.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail - Lag")
	float RailRotationLag = 25.f;

	//-----------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FreeMode - Lag")
	float FreeModePositionLag = 15.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FreeMode - Lag")
	float FreeModeRotationLag = 25.f;

	//-----------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic - Lag")
	float RelicPositionLag = 15.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic - Lag")
	float RelicRotationLag = 150.f;

#pragma endregion Lag

#pragma region Lerps

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lerps",
		meta = (Toolip = "Do not touch."))
	FVector2D MinMaxSpeedLerpFactor = FVector2D(2.f, 10.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lerps",
		meta = (Toolip = "Do not touch."))
	float DriftReleaseLerpFactorMultiplier = 25.f;

#pragma endregion Lerps
};
