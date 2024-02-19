// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SMixedRealitySetupCommands.generated.h"

class ISMixedRealityCommandIssuer;

/**
 * 
 */
UCLASS()
class SPACESHIFTXR_API USMixedRealitySetupCommand : public UObject
{
	GENERATED_BODY()

public:

	virtual void Execute() PURE_VIRTUAL(USMixedRealitySetupCommand::Execute);

protected:

	ISMixedRealityCommandIssuer* CommandIssuer;
	
};

UCLASS()
class SPACESHIFTXR_API USMyTestCommand : public USMixedRealitySetupCommand
{
	GENERATED_BODY()

public:

	static USMyTestCommand* MakeCommand(ISMixedRealityCommandIssuer* Issuer)
	{
		USMyTestCommand* Command = NewObject<USMyTestCommand>();
		Command->CommandIssuer = Issuer;
		return Command;
	}

	virtual void Execute() override;
};
