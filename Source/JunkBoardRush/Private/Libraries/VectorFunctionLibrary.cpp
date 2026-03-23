
#include "Libraries/VectorFunctionLibrary.h"

float UVectorFunctionLibrary::GetAngleBetweenTwoVectorsDegrees(const FVector& VectorA, const FVector& VectorB)
{
	FVector NormA = VectorA.GetSafeNormal();
	FVector NormB = VectorB.GetSafeNormal();
    
	float DotProduct = FVector::DotProduct(NormA, NormB);
	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);
    
	float AngleRadians = FMath::Acos(DotProduct);
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);
    
	return AngleDegrees;
}

float UVectorFunctionLibrary::GetSignedAngleBetweenDirections(const FVector2D& VectorA, const FVector2D& VectorB)
{
	float AngleA = FMath::Atan2(VectorA.Y, VectorA.X);
	float AngleB = FMath::Atan2(VectorB.Y, VectorB.X);
    
	float AngleDiff = AngleB - AngleA;
    
	while (AngleDiff > PI) AngleDiff -= 2.0f * PI;
	while (AngleDiff < -PI) AngleDiff += 2.0f * PI;
    
	return FMath::RadiansToDegrees(AngleDiff);
}

float UVectorFunctionLibrary::GetNormalizedValue(float CurrentAngle, float MinAngle, float MaxAngle)
{
	return FMath::Clamp((CurrentAngle - MinAngle) / (MaxAngle - MinAngle), 0.0f, 1.0f);
}