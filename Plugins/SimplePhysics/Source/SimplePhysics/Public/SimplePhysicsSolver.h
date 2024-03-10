// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SimplePhysicsSolver.generated.h"

class USimplePhysicsRigidBodyComponent;

/**
 * 
 */
UCLASS()
class SIMPLEPHYSICS_API USimplePhysicsSolver : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	USimplePhysicsSolver();

	/** Called once all UWorldSubsystems have been initialized */
	virtual void PostInitialize() override;

	void SetSimulationEnabled(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, bool Enabled);

	/** Compute the distance we should move the RigidBody given time, at a given a velocity. */
	//FVector ComputeMoveDelta(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, const FVector& InVelocity, float DeltaTime) const;

	/**
     * Given an initial velocity and a time step, compute a new velocity
     * Default implementation applies the result of ComputeAcceleration() to velocity.
     *
     * @param  InitialVelocity Initial velocity.
     * @param  DeltaTime Time step of the update.
     * @return Velocity after DeltaTime time step.
     */
	//FVector ComputeVelocity(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, const FVector& InitialVelocity, float DeltaTime) const;

	/** Compute the acceleration that will be applied */
	//FVector ComputeAcceleration(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, const FVector& InitialVelocity, float DeltaTime) const;

	//FVector GetGravityZ() const;

	/** Begin UTickableWorldSubsystem Interface */
	virtual bool IsTickable() const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(FSimplePhysicsSolverStat, STATGROUP_Tickables); }
	/** End UTickableWorldSubsystem Interface */
	

private:

	TArray<TObjectPtr<USimplePhysicsRigidBodyComponent>> SimulatedRigidBodies;

	TArray<TObjectPtr<USimplePhysicsRigidBodyComponent>> InvalidRigidBodies;

	/** Values loaded from SimplePhysics_Settings */
	float GravityAcceleration;
	int32 MaxSimulationIterations;

	void ValidateRigidBodyTick(float DeltaTime);
	bool TickRigidBody(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, float DeltaTime);
	void ApplyRigidBodyMovement(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, float DeltaTime);

protected:
	/** Minimum delta time considered when ticking. Delta times below this are not considered. This is a very small non-zero positive value to avoid potential divide-by-zero in simulation code. */
	static const float MIN_TICK_TIME;
};
