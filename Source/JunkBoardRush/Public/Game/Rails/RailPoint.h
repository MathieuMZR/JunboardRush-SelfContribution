
#pragma once

#include "CoreMinimal.h"

#include "RailPoint.generated.h"

USTRUCT()
struct JUNKBOARDRUSH_API FRailPoint
{
	GENERATED_BODY()
	
public:
	FRailPoint();
	FRailPoint(int ActualIndex, float Dist, const FVector& Pos,
	const FVector& PrevDir, const FVector& NextDir, const FVector& ArrTangent, const FVector& LeaTangent);

	int Index;
	float DistanceOnSpline;
	
	FVector Position;
	
	FVector PreviousDirection;
	FVector NextDirection;
	FVector ArriveTangent;
	FVector LeaveTangent;

	bool IsExtremityPoint(int MaxIndex) const;
};
