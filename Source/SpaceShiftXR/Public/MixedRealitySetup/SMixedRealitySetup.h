// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMixedRealityCommandIssuer.h"
#include "SMixedRealitySetupTypes.h"
#include "SMixedRealitySetup.generated.h"

class AMRUKAnchorActorSpawner;

UCLASS()
class SPACESHIFTXR_API ASMixedRealitySetup : public AActor, public ISMixedRealityCommandIssuer
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASMixedRealitySetup();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Android Configuration")
	TArray<ESetupCommand> AndroidCommands;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editor Configuration")
	TArray<ESetupCommand> EditorCommands;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editor Configuration")
	EPresetRoom PresetRoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AMRUKAnchorActorSpawner> MRUKAnchorActorSpawner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterial> GlobalMeshMaterial;

	UFUNCTION(BlueprintCallable)
	void Run();

	virtual void MixedRealitySetupCommandComplete(USMixedRealitySetupCommand* Command, bool Result) override;



	void BuildPresetRoomMap(TMap<EPresetRoom, FName>& PresetRoomMap) const;


	UFUNCTION(BlueprintImplementableEvent, DisplayName = "FindRoomConfig")
	void BP_FindRoomConfig(FName RoomName);

	UFUNCTION(BlueprintCallable)
	void SetPresetRoomConfig(FString RoomConfig);

	ESetupState GetSetupState() const { return SetupState; }

private:

	TQueue<TObjectPtr<USMixedRealitySetupCommand>> Commands;

	FString RoomConfigJSON;

	void RunNextSetupCommand();

	void CompleteSetup();

	void BeginSetup();

	ESetupState SetupState;

	bool bSetupInProgress;

	void BuildCommandQueue();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
