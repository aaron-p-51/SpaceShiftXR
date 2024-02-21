// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SSpaceShipMovementComponent.h"



void USSpaceshipMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bUnderManualControl)
	{
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("Thrust: %f, Yoke: %s"), Thrust, *Yoke.ToString());

	ForceAppliedThisFrame = FVector::ZeroVector;
	ForceAppliedThisFrame += GetForceFromThrust(Thrust);
	ForceAppliedThisFrame += GetDragForce();

	TorqueAppliedThisFrame = FVector::ZeroVector;
	TorqueAppliedThisFrame += GetTorqueFromYokeInput(Yoke);
	TorqueAppliedThisFrame += GetTorqueResistance();

	AngularVelocity = ComputeAngularVelocity(AngularVelocity, DeltaTime);
	//RotateVelocityToOwnerForwardVector(DeltaTime);

	FQuat RotationDelta = ComputeRotationDelta(AngularVelocity, DeltaTime);
	RotationThisFrame = ComputeNewRotation(RotationDelta);
	RotationThisFrame = ClampRotationLimits(RotationThisFrame);

	const FVector OldVelocity = Velocity;
	const FVector MoveDelta = ComputeMoveDelta(OldVelocity, DeltaTime);

	FHitResult Hit;
	SafeMoveUpdatedComponent(MoveDelta, RotationThisFrame, false, Hit);

	if (Velocity == OldVelocity)
	{
		Velocity = ComputeVelocity(Velocity, DeltaTime);
	}


	UpdateComponentVelocity();
}


void USSpaceshipMovementComponent::SetThrustInput(float Value)
{
	Thrust = FMath::Clamp(Value, -1.f, 1.f);
}


void USSpaceshipMovementComponent::SetYokeInput(const FVector2D Value)
{
	Yoke.X = FMath::Clamp(Value.X, -1.f, 1.f);
	Yoke.Y = FMath::Clamp(Value.Y, -1.f, 1.f);
}

FVector USSpaceshipMovementComponent::GetForceFromThrust(float ThrustInput) const
{
	if (UpdatedComponent)
	{
		return UpdatedComponent->GetForwardVector() * MaxTrustForce * FMath::Clamp(ThrustInput, -1.f, 1.f);
	}

	return FVector::ZeroVector;
}


FVector USSpaceshipMovementComponent::GetTorqueFromYokeInput(const FVector2D& YokeInput) const
{
	const float YokeYaw = FMath::Clamp(YokeInput.X, -1.f, 1.f);
	const float YokePitch = FMath::Clamp(YokeInput.Y, -1.f, 1.f);

	return FVector(YokePitch * PitchMoment * MaxPitchForce, 0.f, YokeYaw * YawMoment * MaxYawForce);
}


FVector USSpaceshipMovementComponent::GetTorqueResistance() const
{
	float PitchDampingCoefficient = 0.f;
	if (PitchDampingCurve)
	{
		PitchDampingCoefficient = PitchDampingCurve->GetFloatValue(FMath::Abs(AngularVelocity.X));
	}

	float YawDampingCoefficient = 0.f;
	if (YawDampingCurve)
	{
		YawDampingCoefficient = YawDampingCurve->GetFloatValue(FMath::Abs(AngularVelocity.Z));
	}

	const float PitchDamping = -FMath::Sign(AngularVelocity.X) * PitchDampingCoefficient;
	const float YawDamping = -FMath::Sign(AngularVelocity.Z) * YawDampingCoefficient;

	return FVector(PitchDamping, 0.f, YawDamping);
}


FVector USSpaceshipMovementComponent::ComputeAngularAcceleration() const
{
	check(MomentOfInertia != 0.f);

	return TorqueAppliedThisFrame / MomentOfInertia;
}

FVector USSpaceshipMovementComponent::ComputeAngularVelocity(const FVector& InitialAngularVelocity, float DeltaTime) const
{
	const FVector NewAngularVelocity = InitialAngularVelocity + (ComputeAngularAcceleration() * DeltaTime);
	return ClampAngularVelocity(NewAngularVelocity);
}

FVector USSpaceshipMovementComponent::ClampAngularVelocity(const FVector& NewAngularVelocity) const
{
	const float AngularVelocityPitch = FMath::Clamp(NewAngularVelocity.X, -MaxAngularVelocityPitch, MaxAngularVelocityPitch);
	const float AngularVelocityYaw = FMath::Clamp(NewAngularVelocity.Z, -MaxAngularVelocityYaw, MaxAngularVelocityYaw);

	return FVector(AngularVelocityPitch, 0.f, AngularVelocityYaw);
}

FQuat USSpaceshipMovementComponent::ComputeRotationDelta(const FVector& InAngularVelocity, float DeltaTime) const
{
	const FVector NewAngularVelocity = ComputeAngularVelocity(InAngularVelocity, DeltaTime);
	const FVector RotationAngleOffset = NewAngularVelocity * DeltaTime;

	return FQuat(FVector::RightVector, FMath::DegreesToRadians(RotationAngleOffset.X)) * FQuat(FVector::UpVector, FMath::DegreesToRadians(RotationAngleOffset.Z));

}


FQuat USSpaceshipMovementComponent::ComputeNewRotation(const FQuat& RotationDelta)
{
	if (!UpdatedComponent)
	{
		return GetOwner() ? GetOwner()->GetActorQuat() : FQuat();
	}

	const FQuat CurrentRotation = UpdatedComponent->GetComponentQuat();
	FQuat NewRotation = CurrentRotation * RotationDelta;
	NewRotation.Normalize();

	return NewRotation;
}


FQuat USSpaceshipMovementComponent::ClampMaxPitch(const FQuat& InRotation) const
{
	FRotator RotationRotator = InRotation.Rotator();
	if (FMath::Abs(RotationRotator.Pitch) < MaxPitchAngle)
	{
		return InRotation;
	}

	RotationRotator.Pitch = FMath::Clamp(RotationRotator.Pitch, -MaxPitchAngle, MaxPitchAngle);
	return RotationRotator.Quaternion();
}


FQuat USSpaceshipMovementComponent::ClampRotationLimits(const FQuat& InRotation)
{
	FRotator Rotation = InRotation.Rotator();
	Rotation.Roll = 0.f;

	const float PitchMagnitude = FMath::Abs(Rotation.Pitch);

	if (PitchMagnitude > MaxPitchAngle)
	{
		AngularVelocity.X = 0.f;
		Rotation.Pitch = FMath::Clamp(Rotation.Pitch, -MaxPitchAngle, MaxPitchAngle);
	}

	return Rotation.Quaternion();
}


void USSpaceshipMovementComponent::PlaceUnderManualControl()
{
	bUnderManualControl = true;

	VelocityBeforeManualControl = Velocity;
	AngularVelocityBeforeManualControl = AngularVelocity;

	UpdateComponentVelocity();
}


void USSpaceshipMovementComponent::RestoreControl(bool RestVelocity, bool ResetAngularVelocity)
{
	bUnderManualControl = false;

	Velocity = RestVelocity ? VelocityBeforeManualControl : FVector::ZeroVector;
	AngularVelocity = ResetAngularVelocity ? AngularVelocityBeforeManualControl : FVector::ZeroVector;

	UpdateComponentVelocity();
}


void USSpaceshipMovementComponent::RotateVelocityToOwnerForwardVector(float DeltaTime)
{
	//if (bIsSliding || /*RotateVelocityStrength == 0.f || */ !UpdatedComponent || !StreamlineCurve)
	//{
	//	return;
	//}

	//FVector VelocityNormal = Velocity.GetSafeNormal();
	//FVector Forward = UpdatedComponent->GetForwardVector();

	//const float Dot = FVector::DotProduct(VelocityNormal, Forward);
	//const float Angle = FMath::RadiansToDegrees(FMath::Acos(Dot));
	////UE_LOG(LogTemp, Warning, TEXT("Angle: %f"), Angle);

	//const float RotateVelocityStrength = StreamlineCurve->GetFloatValue(Angle);

	//FVector RotatedVelocity = FVector::SlerpNormals(VelocityNormal, Forward, RotateVelocityStrength * DeltaTime);

	//Velocity = RotatedVelocity * Velocity.Size();
	//UpdateComponentVelocity();
}

FVector USSpaceshipMovementComponent::GetDragForce() const
{
	return 0.5f * -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector USSpaceshipMovementComponent::ComputeMoveDelta(const FVector& InVelocity, float DeltaTime) const
{
	const FVector NewVelocity = ComputeVelocity(InVelocity, DeltaTime);
	return ((InVelocity * DeltaTime) + (ComputeAcceleration() * (0.5f * DeltaTime * DeltaTime))) * 100.f;

}

FVector USSpaceshipMovementComponent::ComputeVelocity(const FVector& InitialVelocity, float DeltaTime) const
{
	return InitialVelocity + (ComputeAcceleration() * DeltaTime);
}

FVector USSpaceshipMovementComponent::ComputeAcceleration() const
{
	check(Mass != 0.f);
	return ForceAppliedThisFrame / Mass;
}



