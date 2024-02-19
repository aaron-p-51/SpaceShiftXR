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

	/** Execute command */
	virtual void Execute() PURE_VIRTUAL(USMixedRealitySetupCommand::Execute);

protected:

	ISMixedRealityCommandIssuer* CommandIssuer;

	void CommandComplete(bool Result);
	
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


UCLASS()
class SPACESHIFTXR_API USRequestUseSceneDataCommand : public USMixedRealitySetupCommand
{
	GENERATED_BODY()

public:

	/** Create a command of type USRequestUseSceneDataCommand */
	static USRequestUseSceneDataCommand* MakeCommand(ISMixedRealityCommandIssuer* Issuer)
	{
		USRequestUseSceneDataCommand* Command = NewObject<USRequestUseSceneDataCommand>();
		Command->CommandIssuer = Issuer;
		return Command;
	}

	/** Execute USRequestUseSceneDataCommand command */
	virtual void Execute() override;

	/** Check if app has permission to use scene data */
	bool HadPermissionForSceneData() const;

	/** Callback after requesting android permissions */
	UFUNCTION()
	void OnPermissionRequestComplete(const TArray<FString>& Permissions, const TArray<bool>& GrantResults);

	/** Callback proxy for OnPermissionsGranted delegate */
	class UAndroidPermissionCallbackProxy* AndroidPermissionCallbackProxy;

	/** Delegaet for Permission Request binding */
	class FDelegateHandle PermissionGrantedDelegateHandle;

	/** UObject override for object destruction */
	virtual void BeginDestroy() override;

	/** Scene permission request parameter */
	static const FString SCENE_PERMISSION;

};