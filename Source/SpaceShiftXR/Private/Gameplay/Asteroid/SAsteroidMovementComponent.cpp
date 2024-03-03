// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Asteroid/SAsteroidMovementComponent.h"

void USAsteroidMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(UpdatedComponent) || !bEnableMovement)
	{
		return;
	}

	const float MinTickTime = 1e-6f;
	float RemainingTime = DeltaTime;
	int32 NumImpacts = 0;
	int32 NumBounces = 0;
	int32 LoopCount = 0;
	int32 Iterations = 0;
	FHitResult Hit(1.f);
	while (bEnableMovement && RemainingTime >= MinTickTime && IsValid(UpdatedComponent))
	{

		LoopCount++;
		Iterations++;

		const float InitialTimeRemaining = RemainingTime;
		const float TimeTick = RemainingTime;
		RemainingTime -= TimeTick;

		Hit.Time = 1.f;
		const FVector OldVelocity = Velocity;
		const FVector MoveDelta = ComputeMoveDelta(OldVelocity, TimeTick);

		// Should bounce
		SafeMoveUpdatedComponent(MoveDelta, UpdatedComponent->GetComponentRotation(), false, Hit);

	}
}

FVector USAsteroidMovementComponent::ComputeMoveDelta(const FVector& InVelocity, float DeltaTime) const
{
	// Velocity Verlet integration (http://en.wikipedia.org/wiki/Verlet_integration#Velocity_Verlet)
// The addition of p0 is done outside this method, we are just computing the delta.
// p = p0 + v0*t + 1/2*a*t^2

// We use ComputeVelocity() here to infer the acceleration, to make it easier to apply custom velocities.
// p = p0 + v0*t + 1/2*((v1-v0)/t)*t^2
// p = p0 + v0*t + 1/2*((v1-v0))*t

	const FVector NewVelocity = ComputeVelocity(InVelocity, DeltaTime);
	const FVector Delta = (InVelocity * DeltaTime) + (NewVelocity - InVelocity) * (0.5f * DeltaTime);
	return Delta;
}

FVector USAsteroidMovementComponent::ComputeVelocity(FVector InitialVelocity, float DeltaTime) const
{
	// v = v0 + a*t
	const FVector Acceleration = ComputeAcceleration(InitialVelocity, DeltaTime);
	FVector NewVelocity = InitialVelocity + (Acceleration * DeltaTime);

	return LimitVelocity(NewVelocity);
}

FVector USAsteroidMovementComponent::ComputeAcceleration(const FVector& InVelocity, float DeltaTime) const
{
	FVector Acceleration(FVector::ZeroVector);

	//Acceleration.Z += GetGravityZ();

	//Acceleration += PendingForceThisUpdate;

	//if (bIsHomingProjectile && HomingTargetComponent.IsValid())
	//{
	//	Acceleration += ComputeHomingAcceleration(InVelocity, DeltaTime);
	//}

	return Acceleration;
}

FVector USAsteroidMovementComponent::LimitVelocity(FVector NewVelocity) const
{
	const float CurrentMaxSpeed = MaxSpeed;
	if (CurrentMaxSpeed > 0.f)
	{
		NewVelocity = NewVelocity.GetClampedToMaxSize(CurrentMaxSpeed);
	}

	return ConstrainDirectionToPlane(NewVelocity);
}
