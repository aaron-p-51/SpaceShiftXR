// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SPoolable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USPoolable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SPACESHIFTXR_API ISPoolable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poolable")
	void OnSpawnFromPool();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poolable")
	void OnReturnToPool();
};
