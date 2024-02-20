// Fill out your copyright notice in the Description page of Project Settings.


#include "MixedRealitySetup/SMixedRealitySetup.h"

#include "MixedRealitySetup/SMixedRealitySetupCommands.h"


// Sets default values
ASMixedRealitySetup::ASMixedRealitySetup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}



void ASMixedRealitySetup::BuildCommandQueue()
{
	Commands.Empty();

	TArray<ESetupCommand> BuildCommands;
#if PLATFORM_ANDROID
	BuildCommands = AndroidCommands;
#elif WITH_EDITOR
	BuildCommands = EditorCommands;
#endif

	for (auto SetupCommand : BuildCommands)
	{
		if (SetupCommand == ESetupCommand::ESCRequestUseSceneData)
		{
			auto Command = USRequestUseSceneDataCommand::MakeCommand(this);
			Commands.Enqueue(Command);
		}
		else if (SetupCommand == ESetupCommand::ESCRunSceneCapture)
		{
			auto Command = USRunSceneCaptureCommand::MakeCommand(this);
			Commands.Enqueue(Command);
		}
	}
}

void ASMixedRealitySetup::RunNextSetupCommand()
{
	if (!Commands.IsEmpty())
	{
		TObjectPtr<USMixedRealitySetupCommand> NextCommand = nullptr;
		if (Commands.Dequeue(NextCommand))
		{
			NextCommand->Execute();
		}
	}
}





void ASMixedRealitySetup::MixedRealitySetupCommandComplete(USMixedRealitySetupCommand* Command, bool Result)
{
	if (!Command)
	{
		UE_LOG(SMixedRealitySetup, Error, TEXT("Unknown Command Completed"));
		return;
	}

	if (Result)
	{
		UE_LOG(SMixedRealitySetup, Log, TEXT("%s Success"), *Command->GetName());
	}
	else
	{
		UE_LOG(SMixedRealitySetup, Log, TEXT("%s Failed"), *Command->GetName());
		//SetupState = ESetupState::ESS_Failed;
	}

	Command->Cleanup();

	if (!Commands.IsEmpty())
	{
		RunNextSetupCommand();
	}
	else
	{
		CompleteSetup();
	}
}

void ASMixedRealitySetup::CompleteSetup()
{
	UE_LOG(SMixedRealitySetup, Log, TEXT("CompleteSetup"));
}

// Called when the game starts or when spawned
void ASMixedRealitySetup::BeginPlay()
{
	Super::BeginPlay();

	//auto Command = USMyTestCommand::MakeCommand(this);
	//Command->Execute();
	

}

void ASMixedRealitySetup::Run()
{
	BuildCommandQueue();

	RunNextSetupCommand();
}

// Called every frame
void ASMixedRealitySetup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

