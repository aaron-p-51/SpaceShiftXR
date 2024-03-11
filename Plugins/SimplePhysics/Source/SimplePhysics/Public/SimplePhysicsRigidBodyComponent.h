// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/MovementComponent.h"
#include "SimplePhysics.h"
#include "SimplePhysicsRigidBodyComponent.generated.h"

class USimplePhysicsSolver;

/**
 * 
 */
UCLASS(ClassGroup = (SimplePhysics), meta = (BlueprintSpawnableComponent), EditInlineNew)
class SIMPLEPHYSICS_API USimplePhysicsRigidBodyComponent : public UMovementComponent
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRigidBodyBounceDelegate, const FHitResult&, ImpactResult, const FVector&, ImpactVelocity, const FVector&, ResultVelocity);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSimulationStopDelegate);

public:

	UPROPERTY(BlueprintAssignable)
	FOnRigidBodyBounceDelegate OnRigidBodyBounceDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnSimulationStopDelegate OnSimulationStopDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseGravity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Mass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Friction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinFrictionFraction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Bounciness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BounceAdditionalIterations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LinearDamping;

	/** Saved HitResult Time (0 to 1) from previous simulation step. Equal to 1.0 when there was no impact. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = ProjectileBounces)
	float PreviousHitTime;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = ProjectileBounces)
	bool bIsSliding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBounceCombine BounceCombine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBounceAngleAffectsFriction;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//bool bIsSliding;

	/** Saved HitResult Normal from previous simulation step that resulted in an impact. If PreviousHitTime is 1.0, then the hit was not in the last step. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = ProjectileBounces)
	FVector PreviousHitNormal;

	/** Custom gravity scale for this rigid body. Set to 0 for no gravity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float GravityScale;

	/** Get a pointer to the SimplePhysicsSolver Subsystem */
	USimplePhysicsSolver* GetSimplePhysicsSolver() const;

	/** Enable movement with the SimplePhysicsSolver Subsystem */

	UFUNCTION(BlueprintCallable)
	void SetSimulationEnabled(bool Enabled);

	UFUNCTION(BlueprintPure)
	bool IsSimulationEnabled() const { return bSimulationEnabled; }

	//Begin UMovementComponent Interface
	virtual float GetMaxSpeed() const override { return MaxSpeed; }
	virtual void InitializeComponent() override;
	//End UMovementComponent Interface

	void AddForce(const FVector& Force);
	bool HasPendingForce() const { return PendingForce.SquaredLength() > 0.f; }
	FVector GetPendingForce() const { return PendingForce; }
	void ClearPendingForce() { PendingForce = FVector::ZeroVector; }

	FVector GetLinearDragForce(const FVector& InVelocity) const;

	/** Compute the distance we should move the RigidBody given time, at a given a velocity. */
	FVector ComputeMoveDelta(const FVector& InVelocity, float DeltaTime) const;

	void SetVelocity(const FVector& NewVelocity, bool UpdateVelocity = true);

	/**
	 * Given an initial velocity and a time step, compute a new velocity
	 * Default implementation applies the result of ComputeAcceleration() to velocity.
	 *
	 * @param  InitialVelocity Initial velocity.
	 * @param  DeltaTime Time step of the update.
	 * @return Velocity after DeltaTime time step.
	 */
	FVector ComputeVelocity(const FVector& InitialVelocity, float DeltaTime) const;

	FVector LimitVelocity(FVector NewVelocity) const;
	FVector LimitVelocityFromCurrent();

	/** Compute the acceleration that will be applied */
	FVector ComputeAcceleration(const FVector& InitialVelocity, float DeltaTime) const;

	void SetMovementVelocityFromNoHit(const FVector& OldVelocity, float DeltaTime);

	FVector ComputeBounceResult(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta);

	void StopAllMovementImmediately();

	bool HasStoppedSimulation() const;

	float GetGravityZ() const override;

	EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining);
	void HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta);

	USimplePhysicsRigidBodyComponent();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bSimulationEnabled;

	FVector PendingForce;

	float GravityAcceleration;
	
};
