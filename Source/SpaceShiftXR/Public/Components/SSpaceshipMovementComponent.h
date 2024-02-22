// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SSpaceshipMovementComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPACESHIFTXR_API USSpaceshipMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void SetThrustInput(float Value);

	UFUNCTION(BlueprintCallable)
	void SetYokeInput(const FVector2D Value);


public:
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float PitchSpeed = 100.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float YawSpeed = 100.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float Damping = 0.4f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float MaxAngularVelocity = 10.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float Mass = 10.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float MaxTrustForce = 300.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float DragCoefficient = 1.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float PitchMoment = 1000.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float MaxPitchForce = 20.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float YawMoment = 1000.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float MaxYawForce = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	TObjectPtr<UCurveFloat> PitchDampingCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	TObjectPtr<UCurveFloat> YawDampingCurve;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float MomentOfInertia = 1000.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float MaxAngularVelocityPitch = 80.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float MaxAngularVelocityYaw = 80.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float MaxPitchAngle = 85.f;




public:

	// Begin UActorComponent Interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// End UActorComponent Interface

	FVector GetForceFromThrust(float ThrustInput) const;
	FVector GetDragForce() const;

	FVector ComputeMoveDelta(const FVector& InVelocity, float DeltaTime) const;
	FVector ComputeVelocity(const FVector& InitialVelocity, float DeltaTime) const;
	FVector ComputeAcceleration() const;

	FVector GetTorqueFromYokeInput(const FVector2D& YokeInput) const;
	FVector GetTorqueResistance() const;

	FVector ComputeAngularAcceleration() const;
	FVector ComputeAngularVelocity(const FVector& InitialAngularVelocity, float DeltaTime) const;
	FVector ClampAngularVelocity(const FVector& NewAngularVelocity) const;
	FQuat ComputeRotationDelta(const FVector& InAngularVelocity, float DeltaTime) const;
	FQuat ComputeNewRotation(const FQuat& RotationDelta);
	FQuat ClampMaxPitch(const FQuat& InRotation) const;
	FQuat ClampRotationLimits(const FQuat& InRotation);

	UFUNCTION(BlueprintCallable)
	void PlaceUnderManualControl();

	UFUNCTION(BlueprintCallable)
	void RestoreControl(bool RestVelocity, bool ResetAngularVelocity);

protected:

	void RotateVelocityToOwnerForwardVector(float DeltaTime);

private:

	float Thrust = 0.f;
	FVector2D Yoke = FVector2D::ZeroVector;

	FVector ForceAppliedThisFrame = FVector::ZeroVector;
	FVector TorqueAppliedThisFrame = FVector::ZeroVector;

	FVector AngularVelocity = FVector::ZeroVector;

	FQuat RotationThisFrame;

	bool bUnderManualControl;

	FVector VelocityBeforeManualControl;
	FVector AngularVelocityBeforeManualControl;

};
