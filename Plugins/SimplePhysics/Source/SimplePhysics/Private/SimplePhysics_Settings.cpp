// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplePhysics_Settings.h"

USimplePhysics_Settings::USimplePhysics_Settings(const FObjectInitializer& obj)
{
	GravityAcceleration = 980.f;
	MaxSimulationIterations = 3;
}
