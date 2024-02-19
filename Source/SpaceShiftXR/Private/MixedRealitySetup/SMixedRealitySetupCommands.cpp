// Fill out your copyright notice in the Description page of Project Settings.


#include "MixedRealitySetup/SMixedRealitySetupCommands.h"
#include "MixedRealitySetup/SMixedRealityCommandIssuer.h"
#include "AndroidPermissionFunctionLibrary.h"
#include "AndroidPermissionCallbackProxy.h"

void USMixedRealitySetupCommand::CommandComplete(bool Result)
{
	if (CommandIssuer)
	{
		CommandIssuer->MixedRealitySetupCommandComplete(this, Result);
	}
}


void USMyTestCommand::Execute()
{
	UE_LOG(LogTemp, Warning, TEXT("USMyTestCommand::Execute"));

	CommandComplete(true);
	
}



//
//	Begin USRequestUseSceneDataCommand
//

const FString USRequestUseSceneDataCommand::SCENE_PERMISSION = FString("com.oculus.permission.USE_SCENE");

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

void USRequestUseSceneDataCommand::BeginDestroy()
{
	// Ensure all bindings are removed from OnPermissionsGrantedDelegate
	if (AndroidPermissionCallbackProxy && PermissionGrantedDelegateHandle.IsValid())
	{
		AndroidPermissionCallbackProxy->OnPermissionsGrantedDelegate.Remove(PermissionGrantedDelegateHandle);
		PermissionGrantedDelegateHandle.Reset();
	}

	Super::BeginDestroy();
}


