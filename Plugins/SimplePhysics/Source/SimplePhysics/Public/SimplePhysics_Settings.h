// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SimplePhysics_Settings.generated.h"

/**
 * 
 */
UCLASS(config = SimplePhysicsSetting)
class SIMPLEPHYSICS_API USimplePhysics_Settings : public UObject
{
	GENERATED_BODY()

public:

	USimplePhysics_Settings(const FObjectInitializer& obj);

	/** Acceleration due to gravity in cm/s^2 */
	UPROPERTY(Config, EditAnywhere, Category = "Test Settings")
	float GravityAcceleration;

	UPROPERTY(Config, EditAnywhere, Category = "Test Settings")
	int32 MaxSimulationIterations;
	
};
