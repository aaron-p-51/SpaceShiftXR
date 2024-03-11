// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

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

class FSimplePhysicsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
