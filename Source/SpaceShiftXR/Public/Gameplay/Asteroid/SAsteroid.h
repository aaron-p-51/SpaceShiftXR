// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPoolable.h"
#include "SAsteroid.generated.h"


class USphereComponent;
class USAsteroidPrimaryDataAsset;
class ASAsteroidSpawner;
class USPoolSubsystem;
class USAsteroidMovementComponent;

UCLASS()
class SPACESHIFTXR_API ASAsteroid : public AActor, public ISPoolable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASAsteroid();

	friend class USAsteroidMovementComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	TObjectPtr<USAsteroidPrimaryDataAsset> DataAsset;

	UPROPERTY(EditDefaultsOnly)
	float DefaultMass = 100.f;

	void InitializeAsteroid(TObjectPtr<USAsteroidPrimaryDataAsset> AsteroidConfig);


	bool ValidFragmentSpawnLocalPosition(const FVector& Value) const;

	UFUNCTION(BlueprintCallable)
	const TArray<FVector>& GetFragmentSpawnPositions() const;

	void OnSpawnFromPool();

	void OnReturnToPool();


	USPoolSubsystem* GetPoolSubsystem() const;

	float GetMass() const { return Mass; }

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USAsteroidMovementComponent> MovementComp;

	
	

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	

	

	virtual void AsteroidDestroyed();

	bool bHasFragments;

	float Health;

	float Mass;

private:

	
	TArray<FVector> FragmentSpawnPositions;

	

	FVector GetRandomPointInUnitSphere() const;

	void GenerateFragmentSpawnLocations();

	

public:	
	//// Called every frame
	//virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void AsteroidHit(AActor* OtherActor);
};
