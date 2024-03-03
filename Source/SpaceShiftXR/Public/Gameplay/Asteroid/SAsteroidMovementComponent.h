// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/MovementComponent.h"
#include "SAsteroidMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class SPACESHIFTXR_API USAsteroidMovementComponent : public UMovementComponent
{
	GENERATED_BODY()

public:

	bool bEnableMovement = false;


	float MaxSpeed = 1000.f;

	//Begin UActorComponent Interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FVector ComputeMoveDelta(const FVector& InVelocity, float DeltaTime) const;
	FVector ComputeVelocity(FVector InitialVelocity, float DeltaTime) const;
	FVector LimitVelocity(FVector NewVelocity) const;
	FVector ComputeAcceleration(const FVector& InVelocity, float DeltaTime) const;
};
