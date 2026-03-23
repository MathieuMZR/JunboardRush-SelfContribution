
#include "Game/Rails/RailPoint.h"

FRailPoint::FRailPoint() : Index(0), DistanceOnSpline(0), Position(), PreviousDirection(), NextDirection(), ArriveTangent(), LeaveTangent()
{}

FRailPoint::FRailPoint(int ActualIndex, float Dist, const FVector& Pos,
                       const FVector& PrevDir, const FVector& NextDir, const FVector& ArrTangent, const FVector& LeaTangent)
{
	Index = ActualIndex;
	DistanceOnSpline = Dist;
	
	Position = Pos;
	
	PreviousDirection = PrevDir;
	NextDirection = NextDir;
	
	ArriveTangent = ArrTangent;
	LeaveTangent = LeaTangent;
}

bool FRailPoint::IsExtremityPoint(int MaxIndex) const
{
	return Index == MaxIndex || Index == 0;
}
