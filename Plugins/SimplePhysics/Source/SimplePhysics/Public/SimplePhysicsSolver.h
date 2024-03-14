// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SimplePhysics.h"
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

	/* Enable/Disable simulation for an actor with attached USimplePhysicsRigidBodyComponent */
	void SetSimulationEnabled(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, bool Enabled);

	/* Enable/Disable simulation for an actor. Will only simulate if actor has a USimplePhysicsRigidBodyComponent */
	UFUNCTION(BlueprintCallable)
	void SetSimulationEnabled(AActor* Actor, bool Enabled);

	UFUNCTION(BlueprintCallable)
	bool IsSimulating(const USimplePhysicsRigidBodyComponent* RigidBody) const;

	/** Begin UTickableWorldSubsystem Interface */
	virtual bool IsTickable() const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(FSimplePhysicsSolverStat, STATGROUP_Tickables); }
	/** End UTickableWorldSubsystem Interface */

protected:

	/** Applies deflection logic from colliding with a non Simple Physics Rigid Body actor. Will trigger RigidBodies OnRigidBodyBounce event. */
	virtual void HandleImpact(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta);

	/** Apply collision logic from two Simple Physics Rigidbodies collide with each other.  */
	virtual void HandleRigidBodyCollision(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, TObjectPtr<USimplePhysicsRigidBodyComponent> OtherRigidBody, const FHitResult& Hit);

	/** Compute the resulting velocities of two Simple Physics Rigidbodies collide with each other.  */
	virtual bool ComputeRigidBodyCollision(const FHitResult& Hit, TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody1, TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody2,	FMovementData& RigidBody1MovementData, FMovementData& RigidBody2MovementData) const;

	/** Computes the result of RigidBody bouncing off a non Simple Physics Rigid Body actor */
	virtual bool ComputeBounceResult(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta, FMovementData& ResultMovementData);

	/** Return true if RigidBody is able to simulate this frame */
	virtual bool CanSimulateRigidBodyMovement(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, float DeltaTime) const;

private:

	/** Current RigidBodies being simulated each frame */
	TArray<TObjectPtr<USimplePhysicsRigidBodyComponent>> SimulatedRigidBodies;

	/** RigidBodies to add to SimulatedRigidBodies, will be added at before SimulatedRigidBodies are updated the next frame */
	TArray<TObjectPtr<USimplePhysicsRigidBodyComponent>> AddRigidBodies;

	/** RigidBodies found to be invalid this frame. USimplePhysicsRigidBodyComponents in this arrya will be removed from SimulatedRigidBodies at the end of frame */
	TArray<TObjectPtr<USimplePhysicsRigidBodyComponent>> InvalidRigidBodies;

	/** Values loaded from SimplePhysics_Settings */
	int32 MaxSimulationIterations;
	float MinimumSimulationVelocity;

	void ValidateRigidBodyTick(float DeltaTime);
	/*bool TickRigidBody(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, float DeltaTime);*/
	void ApplyRigidBodyMovement(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, float DeltaTime);

	/** Return true if RigidBody velocity is below the velocity set in SimplePhysics_Settings */
	bool IsBelowSimulationVelocity(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody) const;

	/** Stop simulating RigidBody will broadcast */
	void StopSimulating(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody);

	/** Register all AddRigidBodies by adding them to SimulatedRigidBodies. Ensure all InvalidRigidBodies are removed from SimulatedRigidBodies */
	void RegisterRigidBodies();

	/** Map of new velocities calcuated this frame of rigid bodies colliding with each other */
	UPROPERTY()
	TMap<USimplePhysicsRigidBodyComponent*, FMovementData> RigidCollisionResultMap;

	/** Check if the Rigid body should abort its simulation this tick. */
	bool ShouldAbort(USimplePhysicsRigidBodyComponent* RigidBody, const FHitResult& Hit) const;

	/** Get the USimplePhysicsRigidBodyComponent if it exists from a FHitResult. Will return nullptr if hit actor does not have a USimplePhysicsRigidBodyComponent */
	TObjectPtr<USimplePhysicsRigidBodyComponent> GetOtherHitRigidBody(const FHitResult& Hit) const;

protected:
	/** Minimum delta time considered when ticking. Delta times below this are not considered. This is a very small non-zero positive value to avoid potential divide-by-zero in simulation code. */
	static const float MIN_TICK_TIME;
};
