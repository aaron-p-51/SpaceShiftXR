// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SAsteroidCollisionSolver.generated.h"


class USAsteroidMovementComponent;

UCLASS()
class SPACESHIFTXR_API USAsteroidCollisionSolver : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	USAsteroidCollisionSolver();

	void RegisterAsteroidMovementComponent(USAsteroidMovementComponent* MovementComp);
	void UnRegisterAsteroidMovementComponent(USAsteroidMovementComponent* MovementComp);

private:

	TArray<USAsteroidMovementComponent*> AstroidMovementComponents;

	int32 MaxSimulationIterations = 4;

public:	

	// Enum indicating how simulation should proceed after HandleBlockingHit() is called.
	enum class EHandleBlockingHitResult
	{
		Deflect,				/** Assume velocity has been deflected, and trigger HandleDeflection(). This is the default return value of HandleBlockingHit(). */
		AdvanceNextSubstep,		/** Advance to the next simulation update. Typically used when additional slide/multi-bounce logic can be ignored,
									such as when an object that blocked the projectile is destroyed and movement should continue. */
		Abort,					/** Abort all further simulation. Typically used when components have been invalidated or simulation should stop. */
	};

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void TickHandleAstroidMovement(float DeltaTime);

	virtual TStatId GetStatId() const override;

	FVector ComputeMoveDelta(USAsteroidMovementComponent* MovementComp, const FVector& InVelocity, float DeltaTime) const;
	FVector ComputeBounceResult(USAsteroidMovementComponent* MovementComp, const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta);
	bool HandleDeflection(USAsteroidMovementComponent* MovementComp, FHitResult& Hit, const FVector& OldVelocity, const uint32 NumBounces, float& SubTickTimeRemaining);
	FVector ComputeVelocity(USAsteroidMovementComponent* MovementComp, const FVector& InitialVelocity, float DeltaTime) const;
	FVector ComputeAcceleration(USAsteroidMovementComponent* MovementComp, const FVector& InVelocity, float DeltaTime) const;
	FVector LimitVelocity(USAsteroidMovementComponent* MovementComp, FVector NewVelocity) const;
	EHandleBlockingHitResult HandleBlockingHit(USAsteroidMovementComponent* MovementComp, const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining);
	void HandleImpact(USAsteroidMovementComponent* MovementComp, const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta);
	void StopSimulating(USAsteroidMovementComponent* MovementComp, const FHitResult& HitResult);


};
