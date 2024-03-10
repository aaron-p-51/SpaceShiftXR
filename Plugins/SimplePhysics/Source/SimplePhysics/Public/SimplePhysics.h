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

class FSimplePhysicsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
