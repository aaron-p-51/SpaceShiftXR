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

	/* Enable/Disable the simulate for an actor with attached USimplePhysicsRigidBodyComponent */
	void SetSimulationEnabled(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, bool Enabled);

	UFUNCTION(BlueprintCallable)
	void SetSimulationEnabled(AActor* Actor, bool Enabled);

	
	/** Begin UTickableWorldSubsystem Interface */
	virtual bool IsTickable() const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(FSimplePhysicsSolverStat, STATGROUP_Tickables); }
	/** End UTickableWorldSubsystem Interface */

	

	bool HandleImpact(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta);

	void HandleRigidBodyCollision(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, TObjectPtr<USimplePhysicsRigidBodyComponent> OtherRigidBody);


	bool ComputeRigidBodyCollision(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody1, TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody2, FVector& V1, FVector& V2) const;

	float GetBounceFactor(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody1, TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody2) const;

private:

	/** Current RigidBodies being simulated each frame */
	TArray<TObjectPtr<USimplePhysicsRigidBodyComponent>> SimulatedRigidBodies;

	/** RigidBodies found to be invalid this frame. USimplePhysicsRigidBodyComponents in this arrya will be removed from SimulatedRigidBodies at the end of frame */
	TArray<TObjectPtr<USimplePhysicsRigidBodyComponent>> InvalidRigidBodies;

	/** Values loaded from SimplePhysics_Settings */
	int32 MaxSimulationIterations;
	float MinimumSimulationVelocity;


	void ValidateRigidBodyTick(float DeltaTime);
	bool TickRigidBody(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, float DeltaTime);
	void ApplyRigidBodyMovement(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, float DeltaTime);

	FVector ComputeBounceResult(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta);

	bool IsBelowSimulationVelocity(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody) const;

	void StopSimulating(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody);


	UPROPERTY()
	TMap<USimplePhysicsRigidBodyComponent*, FVector> RigidCollisionResultMap;

	/** Check if the Rigid body should abort its simulation this tick. */
	bool ShouldAbort(USimplePhysicsRigidBodyComponent* RigidBody, const FHitResult& Hit) const;

	/** Get the USimplePhysicsRigidBodyComponent if it exists from a FHitResult. Will return nullptr if hit actor does not have a USimplePhysicsRigidBodyComponent */
	TObjectPtr<USimplePhysicsRigidBodyComponent> GetOtherHitRigidBody(const FHitResult& Hit) const;

protected:
	/** Minimum delta time considered when ticking. Delta times below this are not considered. This is a very small non-zero positive value to avoid potential divide-by-zero in simulation code. */
	static const float MIN_TICK_TIME;
};
