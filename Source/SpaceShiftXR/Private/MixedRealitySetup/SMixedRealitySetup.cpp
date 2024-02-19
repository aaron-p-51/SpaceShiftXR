// Fill out your copyright notice in the Description page of Project Settings.


#include "MixedRealitySetup/SMixedRealitySetup.h"

DEFINE_LOG_CATEGORY(SMixedRealitySetup)

// Sets default values
ASMixedRealitySetup::ASMixedRealitySetup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ASMixedRealitySetup::MixedRealitySetupCommandComplete(USMixedRealitySetupCommand* Command, bool Result)
{
	UE_LOG(SMixedRealitySetup, Log, TEXT("Command Complete: %s"), *Command->GetName());
}

// Called when the game starts or when spawned
void ASMixedRealitySetup::BeginPlay()
{
	Super::BeginPlay();

	auto Command = USMyTestCommand::MakeCommand(this);
	Command->Execute();
	
}

// Called every frame
void ASMixedRealitySetup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

