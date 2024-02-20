// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SMixedRealitySetupCommands.generated.h"

class ISMixedRealityCommandIssuer;
class UMRUKSubsystem;
class AMRUKRoom;
class AMRUKAnchorActorSpawner;

/**
 * 
 */
UCLASS()
class SPACESHIFTXR_API USMixedRealitySetupCommand : public UObject
{
	GENERATED_BODY()

public:

	/** Execute command */
	virtual void Execute() PURE_VIRTUAL(USMixedRealitySetupCommand::Execute);

	/** Unbind and remaining delegates and delete command UObject */
	virtual void Cleanup();

	/** Get the Mixed Reality Utility Kit GameInstance subsystem */
	UMRUKSubsystem* GetMRUKSubsystem() const;

	/** Get Mixed Reality Utility Kit Room Actor */
	AMRUKRoom* GetCurrentRoom() const;

protected:

	/** Set member variables for command */
	bool Initialize(ISMixedRealityCommandIssuer* Issuer);

	/** Object to created command. This object will be informed when command is complete via ISMixedRealityCommandIssuer interface */
	ISMixedRealityCommandIssuer* CommandIssuer = nullptr;

	/** World context pointer */
	TObjectPtr<UWorld> WorldPtr = nullptr;

	/** Notifies CommandIssuer command is complete with success result*/
	void CommandComplete(bool Result);
	


	
};

UCLASS()
class SPACESHIFTXR_API USMyTestCommand : public USMixedRealitySetupCommand
{
	GENERATED_BODY()

public:

	static USMyTestCommand* MakeCommand(ISMixedRealityCommandIssuer* Issuer, TObjectPtr<UWorld> WorldContextPtr)
	{
		USMyTestCommand* Command = NewObject<USMyTestCommand>();
		Command->Initialize(Issuer);
		return Command;
	}

	virtual void Execute() override;
};


UCLASS()
class SPACESHIFTXR_API USRequestUseSceneDataCommand : public USMixedRealitySetupCommand
{
	GENERATED_BODY()

public:

	/** Create a command of type USRequestUseSceneDataCommand */
	static USRequestUseSceneDataCommand* MakeCommand(ISMixedRealityCommandIssuer* Issuer);

	/** Scene permission request parameter */
	static const FString SCENE_PERMISSION;

	/** Execute USRequestUseSceneDataCommand command */
	virtual void Execute() override;

	/** Remove bound delegates and delete command  */
	virtual void Cleanup() override;

private:

	/** Check if app has permission to use scene data */
	bool HadPermissionForSceneData() const;

	/** Callback after requesting android permissions */
	UFUNCTION()
	void OnPermissionRequestComplete(const TArray<FString>& Permissions, const TArray<bool>& GrantResults);

	/** Callback proxy for OnPermissionsGranted delegate */
	class UAndroidPermissionCallbackProxy* AndroidPermissionCallbackProxy;

	/** Delegaet for Permission Request binding */
	class FDelegateHandle PermissionGrantedDelegateHandle;
};


UCLASS()
class SPACESHIFTXR_API USRunSceneCaptureCommand : public USMixedRealitySetupCommand
{
	GENERATED_BODY()

public:

	/** Create a command of type USRunSceneCaptureCommand */
	static USRunSceneCaptureCommand* MakeCommand(ISMixedRealityCommandIssuer* Issuer);

	/** Execute USRunSceneCaptureCommand command */
	virtual void Execute() override;

	/** Remove bound delegates and delete command  */
	virtual void Cleanup() override;

private:

	/** Callback when scene capture process is complete */
	UFUNCTION()
	void OnMRUKSubsystemCaptureComplete(bool Success);

};


UCLASS()
class SPACESHIFTXR_API USClearSceneCommand : public USMixedRealitySetupCommand
{
	GENERATED_BODY()

public:

	/** Create a command of type USClearSceneCommand */
	static USClearSceneCommand* MakeCommand(ISMixedRealityCommandIssuer* Issuer);

	/** Execute USClearSceneCommand */
	virtual void Execute() override;
};


UCLASS()
class SPACESHIFTXR_API USLoadSceneFromDeviceCommand : public USMixedRealitySetupCommand
{
	GENERATED_BODY()

public:

	/** Create a command of type USLoadSceneFromDeviceCommand */
	static USLoadSceneFromDeviceCommand* MakeCommand(ISMixedRealityCommandIssuer* Issuer);

	/** Execute USLoadSceneFromDeviceCommand */
	virtual void Execute() override;

	/** Remove bound delegates and delete command  */
	virtual void Cleanup() override;

private:

	/** Callback when scene load is complete */
	UFUNCTION()
	void OnSceneLoaded(bool Success);
};


UCLASS()
class SPACESHIFTXR_API USLoadPresetSceneCommand : public USMixedRealitySetupCommand
{
	GENERATED_BODY()

public:

	/** Create a command of type USLoadPresetSceneCommand */
	static USLoadPresetSceneCommand* MakeCommand(ISMixedRealityCommandIssuer* Issuer, FString* PresetRoom, TObjectPtr<AMRUKAnchorActorSpawner> Spawner);

	/** Execute USLoadPresetSceneCommand */
	virtual void Execute() override;

	/** Remove bound delegates and delete command  */
	virtual void Cleanup() override;

private:

	/** Callback when scene load is complete */
	UFUNCTION()
	void OnSceneLoaded(bool Success);

	/** Callback when AMRUKAnchorActorSpawner has spawned actors into the scene */
	UFUNCTION()
	void OnMRUKAnchorActorsSpawned();

	/** Pointer to the AMRUKAnchorActorSpawner in the scene */
	TObjectPtr<AMRUKAnchorActorSpawner> MRUKAnchorActorSpawner = nullptr;

	/** Preset room configuration to load */
	FString* PresetRoomJSON = nullptr;
};


UCLASS()
class SPACESHIFTXR_API USLoadGloblaMeshFromDeviceCommand : public USMixedRealitySetupCommand
{
	GENERATED_BODY()

public:

	/** Create a command of type USLoadGloblaMeshFromDeviceCommand */
	static TObjectPtr<USLoadGloblaMeshFromDeviceCommand> MakeCommand(ISMixedRealityCommandIssuer* Issuer, TObjectPtr<UMaterial> Material);

	/** Execute USLoadGloblaMeshFromDeviceCommand */
	virtual void Execute() override;

private:
	
	/** Material to apply to global scene mesh */
	TObjectPtr<UMaterial> GloblaMeshMaterial;

};


UCLASS()
class SPACESHIFTXR_API USSetGlobalMeshVisibleCommand : public USMixedRealitySetupCommand
{
	GENERATED_BODY()

public:

	/** Create a command of type USSetGlobalMeshVisibleCommand */
	static TObjectPtr<USSetGlobalMeshVisibleCommand> MakeCommand(ISMixedRealityCommandIssuer* Issuer, bool Visible);

	/** Execute USSetGlobalMeshVisibleCommand */
	virtual void Execute() override;

private:

	/** Global Mesh Anchor visibility */
	bool bEnableVisibility;
};
	