// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "SimplePhysics.generated.h"

enum class EHandleBlockingHitResult
{
	Deflect,
	AdvanceNextSubstep,
	Abort,
	HitRigidBody
};


UENUM()
enum class EBounceCombine : uint8
{
	Minimum			UMETA(DisplayName = "Minimum"),
	Maximum			UMETA(DisplayName = "Maximum"),
	Average			UMETA(DisplayName = "Average"),
	Ignore			UMETA(DisplayName = "Ignore"),

	MAX				UMETA(DisplayName = "DefaultMax")
};


USTRUCT()
struct FMovementData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector LinearVelocity;

	UPROPERTY()
	FVector AngularVelocity;

	FMovementData()
		:
		LinearVelocity(FVector::ZeroVector),
		AngularVelocity(FVector::ZeroVector)
	{}

	void Set(const FVector& NewLinearVelocity, const FVector& NewAngularVelocity)
	{
		LinearVelocity = NewLinearVelocity;
		AngularVelocity = NewAngularVelocity;
	}

	void Set(const FVector& NewLinearVelocity, const FRotator& NewAngularVelocityRotator)
	{
		LinearVelocity = NewLinearVelocity;

		AngularVelocity.X = NewAngularVelocityRotator.Roll;
		AngularVelocity.Y = NewAngularVelocityRotator.Pitch;
		AngularVelocity.Z = NewAngularVelocityRotator.Yaw;
	}

	static FVector AngularVelocityFromQuat(const FQuat& Quat) 
	{
		const FRotator Rotator = Quat.Rotator();
		return FVector(Rotator.Roll, Rotator.Pitch, Rotator.Yaw);
	}


};


class FSimplePhysicsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
