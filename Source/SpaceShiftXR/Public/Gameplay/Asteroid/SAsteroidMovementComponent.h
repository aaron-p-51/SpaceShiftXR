// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "SAsteroidMovementComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPACESHIFTXR_API USAsteroidMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()

public:	
	USAsteroidMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	///** Compute the distance we should move in the given time, at a given a velocity. */
	//FVector ComputeMoveDelta(const FVector& InVelocity, float DeltaTime) const;

	virtual FVector ComputeBounceResult(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta) override;

	//TObjectPtr<ASAsteroid>


public:

	UPROPERTY(EditAnywhere)
	float Mass;

	UPROPERTY(EditAnywhere)
	float DragCoefficient = 0.01f;

	UFUNCTION(BlueprintCallable)
	void SetInitialForce(const FVector& Force);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



};
