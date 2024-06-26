// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/MovementComponent.h"
#include "SimplePhysics.h"
#include "SimplePhysicsRigidBodyComponent.generated.h"

class USimplePhysicsSolver;
class USphereComponent;




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

	/** Event when this RigidBody collides with another blocking surface */
	UPROPERTY(BlueprintAssignable)
	FOnRigidBodyBounceDelegate OnRigidBodyBounceDelegate;

	/** Called when simulation is stopped for this RigidBody. Will not be called when calling SetSimulationEnabled to stop simulation */
	UPROPERTY(BlueprintAssignable)
	FOnSimulationStopDelegate OnSimulationStopDelegate;

	/** Should this RigidBody simulate acceleration due to gravity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseGravity;

	/** If set to true then this RigidBody will start simulating*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableSimulationOnRigidBodyCollision;



	/** Coefficient of friction when sliding along a surface. Normal Range is [0,1], where 0 = No Friction and 1 = high friction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Friction;

	/**
     * When bounce angle affects friction, apply at least this fraction of normal friction.
     * Helps consistently slow objects sliding or rolling along surfaces or in valleys when the usual friction amount would take a very long time to settle.
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinFrictionFraction;

	/**
     * Percentage of velocity maintained after the bounce in the direction of the normal of impact (coefficient of restitution).
     * 1.0 = no velocity lost (elastic), 0.0 = no bounce (inelastic).
	 * @see BounceCombine
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Bounciness;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//int32 BounceAdditionalIterations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LinearDamping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngularDamping;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MomentOfInertia;

	/** Mass of this RigidBody */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Mass;

public:

	/** Max Speed (Linear Velocity) of this RigidBody. Set to 0 for no max speed limit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed;

	/** Max Angular Velocity of this RigidBody. Set to 0 for no max angular velocity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxAngularVelocity;

	/** Determine how to scale Bounciness when this RigidBody collides with another RigidBody  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBounceCombine BounceCombine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBounceAngleAffectsFriction;

	/** Custom gravity scale for this rigid body. Set to 0 for no gravity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float GravityScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector AngularVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TempScale;

	/** Get a pointer to the SimplePhysicsSolver Subsystem */
	USimplePhysicsSolver* GetSimplePhysicsSolver() const;

	/** Enable/Disable simulation for this RigidBody */
	UFUNCTION(BlueprintCallable)
	void SetSimulationEnabled(bool Enabled);

	//Begin UMovementComponent Interface
	virtual float GetMaxSpeed() const override { return MaxSpeed; }
	virtual void InitializeComponent() override;
	//End UMovementComponent Interface

	float GetMaxAngularVelocity() const { return MaxAngularVelocity; }

	virtual void AddForce(const FVector& Force);
	bool HasPendingForce() const { return PendingForce.SquaredLength() > 0.f; }
	FVector GetPendingForce() const { return PendingForce; }
	void ClearPendingForce() { PendingForce = FVector::ZeroVector; }

	virtual void AddTorque(const FVector& Torque);
	bool HasPendingTorque() const { return PendingTorque.SquaredLength() > 0.f; }
	FVector GetPendingTorque() const { return PendingTorque; }
	void ClearPendingTorque() { PendingTorque = FVector::ZeroVector; }


	FVector ComputeAngularVelocityDelta(const FVector& InAngularVelocity, float DeltaTime) const;

	FVector ComputeAngularVelocity(const FVector& InitialAngularVelocity, float DeltaTime) const;

	FVector ComputeAngularAcceleration(const FVector& InitialAngularVelocity, float DeltaTime) const;

	virtual FVector GetAngularDragTorque(const FVector& InAngularVelocity) const;

	/** Get the linear drag force acting on the RigidBody given a velocity */
	virtual FVector GetLinearDragForce(const FVector& InVelocity) const;

	/** Compute the distance we should move the RigidBody given time, at a given a velocity. */
	FVector ComputeMoveDelta(const FVector& InVelocity, float DeltaTime) const;

	/** Set the velocity of the UpdatedComponent. Be default will call UpdateComponentVelocity in base class */
	void SetVelocity(const FVector& NewVelocity, bool UpdateVelocity = true);

	void SetAngularVelocity(const FVector& NewAngularVelocity);

	void SetMovementData(const FMovementData& MovementData);

	/** Calculate the RestitutionCoefficient to apply to collision calculations when two RigidBodies collide */
	float GetRestitutionCoefficient(TObjectPtr<USimplePhysicsRigidBodyComponent> OtherRidigBody) const;

	//FRotator ComputeRotationDelta

	/**
	 * Given an initial velocity and a time step, compute a new velocity
	 * Default implementation applies the result of ComputeAcceleration() to velocity.
	 *
	 * @param  InitialVelocity Initial velocity.
	 * @param  DeltaTime Time step of the update.
	 * @return Velocity after DeltaTime time step.
	 */
	FVector ComputeVelocity(const FVector& InitialVelocity, float DeltaTime) const;

	/** Limit NewVelocity to have a length no grater than MaxSpeed */
	FVector LimitVelocity(FVector NewVelocity) const;
	/** Limit Velocity to have a length no greater than MaxSpeed */
	FVector LimitVelocityFromCurrent();

	FVector LimitAngularVelocity(FVector NewAngularVelocity) const;

	/** Compute the acceleration that will be applied */
	FVector ComputeAcceleration(const FVector& InitialVelocity, float DeltaTime) const;


	/** Call after moving the updated component. Will change Velocity based on PendingForces, Gravity and LinearDamping  */
	void UpdateMovementVelocity(const FVector& OldVelocity, float DeltaTime);

	/**
	 * Call after moving the updated component. Will change Velocity based on PendingForces, Gravity and LinearDamping.
	 * If Hit is a blocking hit DeltaTime will be modified using Hit.Time.
	 */
	void UpdateMovementVelocity(const FVector& OldVelocity, const FHitResult& Hit, float DeltaTime);

	/** Call after moving the updated component. Will change AngularVelocity based on PendingForces and AngularDamping.  */
	void UpdateMovementAngularVelocity(const FVector& OldAngularVelocity, float DeltaTime);

	/**
     * Call after moving the updated component. Will change AngularVelocity based on PendingForces and AngularDamping.
     * If Hit is a blocking hit DeltaTime will be modified using Hit.Time.
     */
	void UpdateMovementAngularVelocity(const FVector& OldAngularVelocity, const FHitResult& Hit, float DeltaTime);

	/** Stop all movement by setting Velocity magnitide to 0 */
	void StopAllMovementImmediately();

	/** Check if simulation has been stopped */
	bool HasStoppedSimulation() const;

	/** Get acceleration from gravity in cm/s^2 (Will apply GravityScale) */
	float GetGravityZ() const override;

	void SetLastBlockingHitResult(const FHitResult& Hit);
	void ClearLastBlockingHitResult();

	//EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining);
	//void HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta);

	USimplePhysicsRigidBodyComponent();

	const FHitResult& GetLastHitResult() const { return LastHitResult; }

	const USphereComponent* GetSphereComponent() const { return OwnerSphereComponent; }

	float GetScaledSphereRadius() const;

	float CalculateMomentOfInertia() const;

	void SetMass(float NewMass);
	float GetMass() const { return Mass; }

	void SetMomentOfInertia(float NewMomentOfInertia) { MomentOfInertia = NewMomentOfInertia; }
	float GetMomentOfInertia() const;

	void ApplyRotationDelta(const FRotator& RotationDelta);

	void AddAngularVelocity(const FVector& AngularVelocityToAdd);
	void AddVelocity(const FVector& VelocityToAdd);

protected:

	/*UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bSimulationEnabled;*/

	/** Accumulated Force to apply next movement update */
	FVector PendingForce;

	/** Accumulated Torque to apply next movement update */
	FVector PendingTorque;

	FHitResult LastHitResult;


	/** Values loaded from SimplePhysics_Settings */
	float GravityAcceleration;

	USphereComponent* OwnerSphereComponent;

private:

	float CalculatedMomentOfInertia;


	
};
