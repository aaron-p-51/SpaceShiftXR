// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMixedRealityCommandIssuer.h"
#include "SMixedRealitySetupTypes.h"
#include "SMixedRealitySetup.generated.h"



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

	UFUNCTION(BlueprintCallable)
	void Run();

	virtual void MixedRealitySetupCommandComplete(USMixedRealitySetupCommand* Command, bool Result) override;

	void BuildCommandQueue();

private:

	TQueue<TObjectPtr<USMixedRealitySetupCommand>> Commands;

	void RunNextSetupCommand();

	void CompleteSetup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
