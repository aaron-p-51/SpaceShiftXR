// Fill out your copyright notice in the Description page of Project Settings.


#include "MixedRealitySetup/SMixedRealitySetupCommands.h"
#include "MixedRealitySetup/SMixedRealityCommandIssuer.h"
#include "AndroidPermissionFunctionLibrary.h"
#include "AndroidPermissionCallbackProxy.h"
#include "Kismet/GameplayStatics.h"
#include "MRUtilityKitAnchorActorSpawner.h"
#include "MRUtilityKitRoom.h"
#include "MRUtilityKitSubsystem.h"


//
//	USMixedRealitySetupCommand
//
bool USMixedRealitySetupCommand::Initialize(ISMixedRealityCommandIssuer* Issuer)
{
	if (Issuer)
	{
		CommandIssuer = Issuer;
		auto IssuerObject = Cast<UObject>(CommandIssuer);
		WorldPtr = IssuerObject ? IssuerObject->GetWorld() : nullptr;
	}

	return CommandIssuer && WorldPtr;
}


void USMixedRealitySetupCommand::CommandComplete(bool Result)
{
	if (CommandIssuer)
	{
		CommandIssuer->MixedRealitySetupCommandComplete(this, Result);
	}
}


void USMixedRealitySetupCommand::Cleanup()
{
	if (IsValid(this))
	{
		ConditionalBeginDestroy();
	}
}


UMRUKSubsystem* USMixedRealitySetupCommand::GetMRUKSubsystem() const
{
	if (WorldPtr)
	{
		if (auto GameInstance = WorldPtr->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UMRUKSubsystem>();
		}
	}

	return nullptr;
}


AMRUKRoom* USMixedRealitySetupCommand::GetCurrentRoom() const
{
	if (auto Subsystem = GetMRUKSubsystem())
	{
		return Subsystem->GetCurrentRoom();
	}

	return nullptr;
}


void USMyTestCommand::Execute()
{
	UE_LOG(SMixedRealitySetup, Warning, TEXT("USMyTestCommand::Execute"));

	CommandComplete(true);
	
}

//
//	Begin USRequestUseSceneDataCommand
//

const FString USRequestUseSceneDataCommand::SCENE_PERMISSION = FString("com.oculus.permission.USE_SCENE");

USRequestUseSceneDataCommand* USRequestUseSceneDataCommand::MakeCommand(ISMixedRealityCommandIssuer* Issuer)
{
	auto Command = NewObject<USRequestUseSceneDataCommand>();
	if (!Command->Initialize(Issuer))
	{
		UE_LOG(SMixedRealitySetup, Error, TEXT("Unable to properly Initialize USRequestUseSceneDataCommand"));
	}
	return Command;
}


void USRequestUseSceneDataCommand::Execute()
{
	// Check if ppermission is already granted. If not granted then reqest permission and bind to OnPermissionsGrantedDelegate
	if(!HadPermissionForSceneData())
	{
		TArray<FString> Permission{ USRequestUseSceneDataCommand::SCENE_PERMISSION };
		AndroidPermissionCallbackProxy = UAndroidPermissionFunctionLibrary::AcquirePermissions(Permission);
		PermissionGrantedDelegateHandle = AndroidPermissionCallbackProxy->OnPermissionsGrantedDelegate.AddUObject(this, &USRequestUseSceneDataCommand::OnPermissionRequestComplete);
	}
	else
	{
		// Already has permission to use Scene Data
		CommandComplete(true);
	}
}


bool USRequestUseSceneDataCommand::HadPermissionForSceneData() const
{
#if PLATFORM_ANDROID
	return UAndroidPermissionFunctionLibrary::CheckPermission(USRequestUseSceneDataCommand::SCENE_PERMISSION);
#else
	return true;
#endif
}

void USRequestUseSceneDataCommand::OnPermissionRequestComplete(const TArray<FString>& Permissions, const TArray<bool>& GrantResults)
{
	// Remove USRequestUseSceneDataCommand binding to OnPermissionsGrantedDelegate
	if (AndroidPermissionCallbackProxy && PermissionGrantedDelegateHandle.IsValid())
	{
		AndroidPermissionCallbackProxy->OnPermissionsGrantedDelegate.Remove(PermissionGrantedDelegateHandle);
		PermissionGrantedDelegateHandle.Reset();
	}

	// Check to see if SCENE_PERMISSION was grated
	bool Success = false;
	for (int32 i = 0; i < Permissions.Num() && i < GrantResults.Num(); ++i)
	{
		if (GrantResults[i] && Permissions[i].Compare(SCENE_PERMISSION) == 0)
		{
			Success = true;
			break;
		}
	}

	// Inform CommandIssuer command is complete. returns true if SCENE_PERMISSION was grated
	CommandComplete(Success);
}


void USRequestUseSceneDataCommand::Cleanup()
{
	// Ensure all bindings are removed from OnPermissionsGrantedDelegate
	if (AndroidPermissionCallbackProxy && PermissionGrantedDelegateHandle.IsValid())
	{
		AndroidPermissionCallbackProxy->OnPermissionsGrantedDelegate.Remove(PermissionGrantedDelegateHandle);
		PermissionGrantedDelegateHandle.Reset();
	}

	Super::Cleanup();
}


//
// Begin USRunSceneCaptureCommand
//

USRunSceneCaptureCommand* USRunSceneCaptureCommand::MakeCommand(ISMixedRealityCommandIssuer* Issuer)
{
	auto Command = NewObject<USRunSceneCaptureCommand>();
	if (!Command->Initialize(Issuer))
	{
		UE_LOG(SMixedRealitySetup, Error, TEXT("Unable to properly Initialize USRunSceneCaptureCommand"));
	}
	return Command;
}


void USRunSceneCaptureCommand::Execute()
{
#if WITH_EDITOR
	CommandComplete(true);
	return;
#elif PLATFORM_ANDROID
	if (auto Subsystem = GetMRUKSubsystem())
	{
		Subsystem->OnCaptureComplete.AddUniqueDynamic(this, &USRunSceneCaptureCommand::OnMRUKSubsystemCaptureComplete);
		Subsystem->LaunchSceneCapture();
	}
	else
	{
		CommandComplete(false);
	}
#endif
}


void USRunSceneCaptureCommand::OnMRUKSubsystemCaptureComplete(bool Success)
{
	if (auto Subsystem = GetMRUKSubsystem())
	{
		Subsystem->OnCaptureComplete.RemoveDynamic(this, &USRunSceneCaptureCommand::OnMRUKSubsystemCaptureComplete);
	}

	CommandComplete(Success);
}


void USRunSceneCaptureCommand::Cleanup()
{
	auto Subsystem = GetMRUKSubsystem();
	if (Subsystem && Subsystem->OnCaptureComplete.IsBound())
	{
		Subsystem->OnCaptureComplete.RemoveDynamic(this, &USRunSceneCaptureCommand::OnMRUKSubsystemCaptureComplete);
	}

	Super::Cleanup();
}


//
// Begin USClearSceneCommand
//
USClearSceneCommand* USClearSceneCommand::MakeCommand(ISMixedRealityCommandIssuer* Issuer)
{
	auto Command = NewObject<USClearSceneCommand>();
	if (!Command->Initialize(Issuer))
	{
		UE_LOG(SMixedRealitySetup, Error, TEXT("Unable to properly Initialize USClearSceneCommand"));
	}

	return Command;
}


void USClearSceneCommand::Execute()
{
	if (auto Subsystem = GetMRUKSubsystem())
	{
		Subsystem->ClearScene();
		CommandComplete(true);
		return;
	}

	CommandComplete(false);
}


//
// Begin USLoadSceneFromDeviceCommand
//
USLoadSceneFromDeviceCommand* USLoadSceneFromDeviceCommand::MakeCommand(ISMixedRealityCommandIssuer* Issuer)
{
	auto Command = NewObject<USLoadSceneFromDeviceCommand>();
	if (!Command->Initialize(Issuer))
	{
		UE_LOG(SMixedRealitySetup, Error, TEXT("Unable to properly Initialize USLoadSceneFromDeviceCommand"));
	}

	return Command;
}


void USLoadSceneFromDeviceCommand::Execute()
{
#if WITH_EDITOR
	CommandComplete(false);
	return;
#elif PLATFORM_ANDROID
	if (auto Subsystem = GetMRUKSubsystem())
	{
		Subsystem->OnSceneLoaded.AddUniqueDynamic(this, &USLoadSceneFromDeviceCommand::OnSceneLoaded);
		Subsystem->LoadSceneFromDevice();
	}
	else
	{
		CommandComplete(false);
	}
#endif
}


void USLoadSceneFromDeviceCommand::OnSceneLoaded(bool Success)
{
	if (auto Subsystem = GetMRUKSubsystem())
	{
		Subsystem->OnSceneLoaded.RemoveDynamic(this, &USLoadSceneFromDeviceCommand::OnSceneLoaded);
	}

	CommandComplete(Success);
}


void USLoadSceneFromDeviceCommand::Cleanup()
{
	auto Subsystem = GetMRUKSubsystem();
	if (Subsystem && Subsystem->OnSceneLoaded.IsBound())
	{
		Subsystem->OnSceneLoaded.RemoveDynamic(this, &USLoadSceneFromDeviceCommand::OnSceneLoaded);
	}

	Super::Cleanup();
}


//
// BeginUSLoadPresetSceneCommand
//

USLoadPresetSceneCommand* USLoadPresetSceneCommand::MakeCommand(ISMixedRealityCommandIssuer* Issuer, FString* PresetRoom, TObjectPtr<AMRUKAnchorActorSpawner> Spawner)
{
	auto Command = NewObject<USLoadPresetSceneCommand>();
	const bool InitializeResult = Command->Initialize(Issuer);
	Command->PresetRoomJSON = PresetRoom;
	Command->MRUKAnchorActorSpawner = Spawner;

	if (!InitializeResult || !Command->PresetRoomJSON || !Command->MRUKAnchorActorSpawner)
	{
		UE_LOG(SMixedRealitySetup, Error, TEXT("Unable to properly Initialize USLoadPresetSceneCommand"));
	}

	return Command;
}


void USLoadPresetSceneCommand::Execute()
{
	if (!PresetRoomJSON || !MRUKAnchorActorSpawner)
	{
		CommandComplete(false);
		return;
	}

#if PLATFORM_ANDROID
	CommandComplete(false);
	return;
#elif WITH_EDITOR
	if (auto Subsystem = GetMRUKSubsystem())
	{
		Subsystem->OnSceneLoaded.AddUniqueDynamic(this, &USLoadPresetSceneCommand::OnSceneLoaded);
		Subsystem->LoadSceneFromJsonString(*PresetRoomJSON);
	}
	else
	{
		CommandComplete(false);
	}
#endif
}


void USLoadPresetSceneCommand::OnSceneLoaded(bool Success)
{
	if (auto Subsystem = GetMRUKSubsystem())
	{
		Subsystem->OnSceneLoaded.RemoveDynamic(this, &USLoadPresetSceneCommand::OnSceneLoaded);
	}

	if (Success && MRUKAnchorActorSpawner)
	{
		MRUKAnchorActorSpawner->OnActorsSpawned.AddUniqueDynamic(this, &USLoadPresetSceneCommand::OnMRUKAnchorActorsSpawned);
		MRUKAnchorActorSpawner->SpawnInterior();
	}
	else
	{
		CommandComplete(false);
	}
}


void USLoadPresetSceneCommand::OnMRUKAnchorActorsSpawned()
{
	if (MRUKAnchorActorSpawner)
	{
		MRUKAnchorActorSpawner->OnActorsSpawned.RemoveDynamic(this, &USLoadPresetSceneCommand::OnMRUKAnchorActorsSpawned);
	}

	CommandComplete(true);
}

void USLoadPresetSceneCommand::Cleanup()
{
	auto Subsystem = GetMRUKSubsystem();
	if (Subsystem && Subsystem->OnSceneLoaded.IsBound())
	{
		Subsystem->OnSceneLoaded.RemoveDynamic(this, &USLoadPresetSceneCommand::OnSceneLoaded);
	}

	if (MRUKAnchorActorSpawner && MRUKAnchorActorSpawner->OnActorsSpawned.IsBound())
	{
		MRUKAnchorActorSpawner->OnActorsSpawned.RemoveDynamic(this, &USLoadPresetSceneCommand::OnMRUKAnchorActorsSpawned);
	}

	Super::Cleanup();
}


//
// Begin USLoadGloblaMeshFromDeviceCommand
//
TObjectPtr<USLoadGloblaMeshFromDeviceCommand> USLoadGloblaMeshFromDeviceCommand::MakeCommand(ISMixedRealityCommandIssuer* Issuer, TObjectPtr<UMaterial> Material)
{
	auto Command = NewObject<USLoadGloblaMeshFromDeviceCommand>();
	const bool InitializeResult = Command->Initialize(Issuer);
	Command->GloblaMeshMaterial = Material;

	if (!InitializeResult || !Command->GloblaMeshMaterial)
	{
		UE_LOG(SMixedRealitySetup, Error, TEXT("Unable to properly Initialize USLoadGloblaMeshFromDeviceCommand"));
	}

	return Command;
}


void USLoadGloblaMeshFromDeviceCommand::Execute()
{
	if (!GloblaMeshMaterial)
	{
		CommandComplete(false);
		return;
	}

	bool Success = false;
	if (auto Subsystem = GetMRUKSubsystem())
	{
		if (Subsystem->SceneLoadStatus == EMRUKInitStatus::Complete)
		{
			if (auto CurrentRoom = Subsystem->GetCurrentRoom())
			{
				Success = CurrentRoom->LoadGlobalMeshFromDevice(GloblaMeshMaterial);
			}
		}
		else
		{
			UE_LOG(SMixedRealitySetup, Error, TEXT("USLoadGloblaMeshFromDeviceCommand can not run if MRUKSubsystem does not have scene loaded"));
		}
	}

	CommandComplete(Success);
}


//
// Begin USSetGlobalMeshVisibleCommand
//
TObjectPtr<USSetGlobalMeshVisibleCommand> USSetGlobalMeshVisibleCommand::MakeCommand(ISMixedRealityCommandIssuer* Issuer)
{
	auto Command = NewObject<USSetGlobalMeshVisibleCommand>();
	if (!Command->Initialize(Issuer))
	{
		UE_LOG(SMixedRealitySetup, Error, TEXT("Unable to properly Initialize USSetGlobalMeshVisibleCommand"));
	}
	return Command;
}


void USSetGlobalMeshVisibleCommand::Execute()
{
	bool Success = false;

	if (auto CurrentRoom = GetCurrentRoom())
	{
		if (CurrentRoom->GlobalMeshAnchor)
		{
			CurrentRoom->GlobalMeshAnchor->SetActorHiddenInGame(false);
			Success = true;
		}
	}

	CommandComplete(Success);
}


//
// Begin USSetGlobalMeshHiddenCommand
//
TObjectPtr<USSetGlobalMeshHiddenCommand> USSetGlobalMeshHiddenCommand::MakeCommand(ISMixedRealityCommandIssuer* Issuer)
{
	auto Command = NewObject<USSetGlobalMeshHiddenCommand>();
	if (!Command->Initialize(Issuer))
	{
		UE_LOG(SMixedRealitySetup, Error, TEXT("Unable to properly Initialize USSetGlobalMeshHiddenCommand"));
	}
	return Command;
}


void USSetGlobalMeshHiddenCommand::Execute()
{
	bool Success = false;

	if (auto CurrentRoom = GetCurrentRoom())
	{
		if (CurrentRoom->GlobalMeshAnchor)
		{
			CurrentRoom->GlobalMeshAnchor->SetActorHiddenInGame(true);
			Success = true;
		}
	}

	CommandComplete(Success);
}


//
// Begin USEnableGlobalCollisionCommand
//
TObjectPtr<USEnableGlobalCollisionCommand> USEnableGlobalCollisionCommand::MakeCommand(ISMixedRealityCommandIssuer* Issuer)
{
	auto Command = NewObject<USEnableGlobalCollisionCommand>();
	if (!Command->Initialize(Issuer))
	{
		UE_LOG(SMixedRealitySetup, Error, TEXT("Unable to properly Initialize USEnableGlobalCollisionCommand"));
	}
	return Command;
}


void USEnableGlobalCollisionCommand::Execute()
{
	bool Success = false;

	if (auto CurrentRoom = GetCurrentRoom())
	{
		if (CurrentRoom->GlobalMeshAnchor)
		{
			CurrentRoom->GlobalMeshAnchor->SetActorEnableCollision(true);
			Success = true;
		}
	}

	CommandComplete(Success);
}


//
// Begin USDisableGlobalCollisionCommand
//
TObjectPtr<USDisableGlobalCollisionCommand> USDisableGlobalCollisionCommand::MakeCommand(ISMixedRealityCommandIssuer* Issuer)
{
	auto Command = NewObject<USDisableGlobalCollisionCommand>();
	if (!Command->Initialize(Issuer))
	{
		UE_LOG(SMixedRealitySetup, Error, TEXT("Unable to properly Initialize USDisableGlobalCollisionCommand"));
	}
	return Command;
}


void USDisableGlobalCollisionCommand::Execute()
{
	bool Success = false;

	if (auto CurrentRoom = GetCurrentRoom())
	{
		if (CurrentRoom->GlobalMeshAnchor)
		{
			CurrentRoom->GlobalMeshAnchor->SetActorEnableCollision(false);
			Success = true;
		}
	}

	CommandComplete(Success);
}


// 
// Begin USApplyTextureToWallsCommand
//

TObjectPtr<USApplyTextureToWallsCommand> USApplyTextureToWallsCommand::MakeCommand(ISMixedRealityCommandIssuer* Issuer, FPresetRoomMaterials Materials)
{
	auto Command = NewObject<USApplyTextureToWallsCommand>();
	if (!Command->Initialize(Issuer))
	{
		UE_LOG(SMixedRealitySetup, Error, TEXT("Unable to properly Initialize USApplyTextureToWallsCommand"));
	}
	Command->PresetRoomMaterials = Materials;

	return Command;
}


void USApplyTextureToWallsCommand::Execute()
{
	if (auto Subsystem = GetMRUKSubsystem())
	{
		for (auto Room : Subsystem->Rooms)
		{
			// Set Floor Material
			if (PresetRoomMaterials.Floor && Room && Room->FloorAnchor && Room->FloorAnchor->ProceduralMeshComponent)
			{
				auto FloorMeshComp = Room->FloorAnchor->ProceduralMeshComponent;
				auto FloorDynamicMaterialInst = UMaterialInstanceDynamic::Create(PresetRoomMaterials.Floor, FloorMeshComp);
				FloorMeshComp->SetMaterial(0, FloorDynamicMaterialInst);
			}

			// Set Ceiling Material
			if (PresetRoomMaterials.Ceiling && Room && Room->CeilingAnchor && Room->CeilingAnchor->ProceduralMeshComponent)
			{
				auto CeilingMeshComp = Room->CeilingAnchor->ProceduralMeshComponent;
				auto CeilingDynamicMaterialInst = UMaterialInstanceDynamic::Create(PresetRoomMaterials.Ceiling, CeilingMeshComp);
				CeilingMeshComp->SetMaterial(0, CeilingDynamicMaterialInst);
			}

			if (PresetRoomMaterials.Walls)
			{
				for (auto Wall : Room->WallAnchors)
				{
					// Set Wall Material
					if (Wall && Wall->ProceduralMeshComponent)
					{
						auto WallMeshComp = Wall->ProceduralMeshComponent;
						auto WallDynamicMaterialInst = UMaterialInstanceDynamic::Create(PresetRoomMaterials.Walls, WallMeshComp);
						WallMeshComp->SetMaterial(0, WallDynamicMaterialInst);
					}
				}
			}
		}

		CommandComplete(true);
		return;
	}

	CommandComplete(false);
}

