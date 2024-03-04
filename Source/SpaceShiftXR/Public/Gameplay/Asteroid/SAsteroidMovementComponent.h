// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/MovementComponent.h"
#include "SAsteroidMovementComponent.generated.h"

class USAsteroidCollisionSolver;

/**
 * 
 */
UCLASS()
class SPACESHIFTXR_API USAsteroidMovementComponent : public UMovementComponent
{
	GENERATED_BODY()

public:

	USAsteroidMovementComponent();

	// From projectile movement
	float PreviousHitTime;
	bool bIsSliding;
	FVector PreviousHitNormal;
	/**
     * On the first few bounces (up to this amount), allow extra iterations over MaxSimulationIterations if necessary.
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "4", UIMin = "0", UIMax = "4"), Category = ProjectileSimulation)
	int32 BounceAdditionalIterations;

	/**
     * Max number of iterations used for each discrete simulation step.
     * Increasing this value can address precision issues with fast-moving objects or complex collision scenarios, at the cost of performance.
     *
     * WARNING: if (MaxSimulationTimeStep * MaxSimulationIterations) is too low for the min framerate, the last simulation step may exceed MaxSimulationTimeStep to complete the simulation.
     * @see MaxSimulationTimeStep, bForceSubStepping
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "25", UIMin = "1", UIMax = "25"), Category = ProjectileSimulation)
	int32 MaxSimulationIterations;

	// Enum indicating how simulation should proceed after HandleBlockingHit() is called.
	enum class EHandleBlockingHitResult
	{
		Deflect,				/** Assume velocity has been deflected, and trigger HandleDeflection(). This is the default return value of HandleBlockingHit(). */
		AdvanceNextSubstep,		/** Advance to the next simulation update. Typically used when additional slide/multi-bounce logic can be ignored,
									such as when an object that blocked the projectile is destroyed and movement should continue. */
		Abort,					/** Abort all further simulation. Typically used when components have been invalidated or simulation should stop. */
	};

	

	void SetMovementEnabled(bool Enabled);

	float MaxSpeed = 1000.f;

	//Begin UActorComponent Interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FVector ComputeMoveDelta(const FVector& InVelocity, float DeltaTime) const;
	FVector ComputeVelocity(FVector InitialVelocity, float DeltaTime) const;
	FVector LimitVelocity(FVector NewVelocity) const;
	FVector ComputeAcceleration(const FVector& InVelocity, float DeltaTime) const;
	EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining);
	void HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta);
	void StopSimulating(const FHitResult& HitResult);
	FVector ComputeBounceResult(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta);
	bool HandleDeflection(FHitResult& Hit, const FVector& OldVelocity, const uint32 NumBounces, float& SubTickTimeRemaining);

protected:

	virtual void BeginPlay() override;
	USAsteroidCollisionSolver* GetAsteroidCollisionSolver() const;

private:
	bool bEnableMovement = false;
	//ASAsteroidCollisionSolver* AsteroidCollisionSolver;

};
