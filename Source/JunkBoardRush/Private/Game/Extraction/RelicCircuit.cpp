// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Extraction/RelicCircuit.h"

// Sets default values
ARelicCircuit::ARelicCircuit()
{
	PrimaryActorTick.bCanEverTick = false;

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	RootComponent = SplineComponent;
}

