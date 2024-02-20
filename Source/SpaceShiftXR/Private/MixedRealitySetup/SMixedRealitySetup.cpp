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
			Commands.Enqueue(USRequestUseSceneDataCommand::MakeCommand(this));
		}
		else if (SetupCommand == ESetupCommand::ESCRunSceneCapture)
		{
			Commands.Enqueue(USRunSceneCaptureCommand::MakeCommand(this));
		}
		else if (SetupCommand == ESetupCommand::ESCClearScene)
		{
			Commands.Enqueue(USClearSceneCommand::MakeCommand(this));
		}
		else if (SetupCommand == ESetupCommand::ESCLoadSceneFromDevice)
		{
			Commands.Enqueue(USLoadSceneFromDeviceCommand::MakeCommand(this));
		}
		else if (SetupCommand == ESetupCommand::ESCLoadGlobalMeshFromDevice)
		{
			Commands.Enqueue(USLoadGloblaMeshFromDeviceCommand::MakeCommand(this, GlobalMeshMaterial));
		}
		else if (SetupCommand == ESetupCommand::ESCSetGlobalMeshHidden)
		{
			Commands.Enqueue(USSetGlobalMeshVisibleCommand::MakeCommand(this, false));
		}
		else if (SetupCommand == ESetupCommand::ESCSetGlobalMeshVisible)
		{
			Commands.Enqueue(USSetGlobalMeshVisibleCommand::MakeCommand(this, true));
		}
#if WITH_EDITOR
		else if (SetupCommand == ESetupCommand::ESCLoadPresetScene)
		{
			Commands.Enqueue(USLoadPresetSceneCommand::MakeCommand(this, &RoomConfigJSON, MRUKAnchorActorSpawner));
		}
#endif
		
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
		SetupState = ESetupState::ESS_Failed;
		return;
	}

	if (Result)
	{
		UE_LOG(SMixedRealitySetup, Log, TEXT("%s Success"), *Command->GetName());
	}
	else
	{
		UE_LOG(SMixedRealitySetup, Error, TEXT("%s Failed"), *Command->GetName());
		SetupState = ESetupState::ESS_Failed;
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
	SetupState = (SetupState == ESetupState::ESS_Running) ? ESetupState::ESS_Complete : SetupState;
	bSetupInProgress = false;

	UE_LOG(SMixedRealitySetup, Log, TEXT("Setup Complete Status: %s"), *UEnum::GetValueAsString(SetupState));
}

void ASMixedRealitySetup::BeginSetup()
{
	BuildCommandQueue();
	RunNextSetupCommand();
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
	if (bSetupInProgress)
	{
		return;
	}

	bSetupInProgress = true;
	SetupState = ESetupState::ESS_Running;


#if PLATFORM_ANDROID
	BeginSetup();
	return;
#elif WITH_EDITOR

	if (EditorCommands.Contains(ESetupCommand::ESCLoadPresetScene))
	{
		TMap<EPresetRoom, FName> PresetRoomMap;
		BuildPresetRoomMap(PresetRoomMap);
		FName* RoomName = PresetRoomMap.Find(PresetRoom);
		if (RoomName)
		{
			BP_FindRoomConfig(*RoomName);
		}
	}
	else
	{
		BeginSetup();
	}

#endif
}




// Called every frame
void ASMixedRealitySetup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASMixedRealitySetup::BuildPresetRoomMap(TMap<EPresetRoom, FName>& PresetRoomMap) const
{
	PresetRoomMap.Add(EPresetRoom::EPR_Bedroom00, FName("Bedroom00"));
	PresetRoomMap.Add(EPresetRoom::EPR_Bedroom01, FName("Bedroom01"));
	PresetRoomMap.Add(EPresetRoom::EPR_Bedroom02, FName("Bedroom02"));
	PresetRoomMap.Add(EPresetRoom::EPR_Bedroom03, FName("Bedroom03"));
	PresetRoomMap.Add(EPresetRoom::EPR_Bedroom04, FName("Bedroom04"));
	PresetRoomMap.Add(EPresetRoom::EPR_Bedroom05, FName("Bedroom05"));
	PresetRoomMap.Add(EPresetRoom::EPR_Bedroom06, FName("Bedroom06"));
	PresetRoomMap.Add(EPresetRoom::EPR_Bedroom07, FName("Bedroom07"));

	PresetRoomMap.Add(EPresetRoom::EPR_LivingRoom00, FName("LivingRoom00"));
	PresetRoomMap.Add(EPresetRoom::EPR_LivingRoom01, FName("LivingRoom01"));
	PresetRoomMap.Add(EPresetRoom::EPR_LivingRoom02, FName("LivingRoom02"));
	PresetRoomMap.Add(EPresetRoom::EPR_LivingRoom03, FName("LivingRoom03"));
	PresetRoomMap.Add(EPresetRoom::EPR_LivingRoom04, FName("LivingRoom04"));
	PresetRoomMap.Add(EPresetRoom::EPR_LivingRoom05, FName("LivingRoom05"));
	PresetRoomMap.Add(EPresetRoom::EPR_LivingRoom06, FName("LivingRoom06"));
	PresetRoomMap.Add(EPresetRoom::EPR_LivingRoom07, FName("LivingRoom07"));

	PresetRoomMap.Add(EPresetRoom::EPR_Office00, FName("Office00"));
	PresetRoomMap.Add(EPresetRoom::EPR_Office01, FName("Office01"));
	PresetRoomMap.Add(EPresetRoom::EPR_Office02, FName("Office02"));
	PresetRoomMap.Add(EPresetRoom::EPR_Office03, FName("Office03"));
	PresetRoomMap.Add(EPresetRoom::EPR_Office04, FName("Office04"));
	PresetRoomMap.Add(EPresetRoom::EPR_Office05, FName("Office05"));
	PresetRoomMap.Add(EPresetRoom::EPR_Office06, FName("Office06"));
	PresetRoomMap.Add(EPresetRoom::EPR_Office07, FName("Office07"));
	PresetRoomMap.Add(EPresetRoom::EPR_Office08, FName("Office08"));
	PresetRoomMap.Add(EPresetRoom::EPR_Office09, FName("Office09"));

	PresetRoomMap.Add(EPresetRoom::ESR_TrippingHazard, FName("TrippingHazard"));
	PresetRoomMap.Add(EPresetRoom::ESR_ExtraWide, FName("ExtraWide"));
	PresetRoomMap.Add(EPresetRoom::ESR_RecRoom, FName("RecRoom"));
	PresetRoomMap.Add(EPresetRoom::ESR_EmptySplit, FName("EmptySplit"));
	PresetRoomMap.Add(EPresetRoom::ESR_HardAngles,FName("HardAngles"));
	PresetRoomMap.Add(EPresetRoom::ESR_Triangular,	FName("Triangular"));
	PresetRoomMap.Add(EPresetRoom::ESR_Cluttered,FName("Cluttered"));
}

void ASMixedRealitySetup::SetPresetRoomConfig(FString RoomConfig)
{
	RoomConfigJSON = RoomConfig;

	if (bSetupInProgress)
	{
		BeginSetup();
	}
}

