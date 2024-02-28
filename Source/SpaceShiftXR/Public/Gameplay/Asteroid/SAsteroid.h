// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAsteroid.generated.h"


class USphereComponent;
class USAsteroidPrimaryDataAsset;


UCLASS()
class SPACESHIFTXR_API ASAsteroid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASAsteroid();

	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	TObjectPtr<USAsteroidPrimaryDataAsset> DataAsset;

	void InitializeAsFragment(bool Value);

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	virtual void PostInitializeComponents() override;

	FVector GetRandomPointInUnitSphere() const;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FVector> FragmentSpawnPositions;

	void GenerateFragmentSpawnLocations();

	bool ValidFragmentSpawnLocalPosition(const FVector& Value) const;

	bool bIsFramgnet;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
