// Fill out your copyright notice in the Description page of Project Settings.


#include "MixedRealitySetup/SMixedRealitySetupCommands.h"
#include "MixedRealitySetup/SMixedRealityCommandIssuer.h"



void USMyTestCommand::Execute()
{
	UE_LOG(LogTemp, Warning, TEXT("USMyTestCommand::Execute"));

	if (CommandIssuer)
	{
		CommandIssuer->MixedRealitySetupCommandComplete(this, true);
	}
}
