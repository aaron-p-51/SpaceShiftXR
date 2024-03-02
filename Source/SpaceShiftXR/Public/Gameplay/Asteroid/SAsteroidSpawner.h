// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAsteroidSpawner.generated.h"

class USAsteroidPrimaryDataAsset;
class ASAsteroid;
class ASAsteroidFragment;
class UMRUKSubsystem;
class AMRUKRoom;
class ASMixedRealitySetup;
class USPoolSubsystem;

UCLASS()
class SPACESHIFTXR_API ASAsteroidSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASAsteroidSpawner();

	void GetSpawnLocations(TArray<FVector>& Locations) const;
	bool PositionOverlap(const FVector& Position, const TArray<FVector>& OtherPositions, float Distance) const;

	UMRUKSubsystem* GetMRUKSubsystem() const;
	AMRUKRoom* GetCurrentRoom() const;

	void SpawnAsteroids();
	

	USPoolSubsystem* GetPoolSubsystem() const;

	void CreateAsteroidObjectPool();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



	UPROPERTY(EditAnywhere)
	TObjectPtr<ASMixedRealitySetup> MixedRealitySetup;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USAsteroidPrimaryDataAsset> AsteroidDataAsset;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ASAsteroid> AsteroidClass;

	UPROPERTY(EditAnywhere)
	int32 MaxSpawnAttempts = 500;

	UPROPERTY(EditAnywhere)
	int32 MaxAsteroids;

	UPROPERTY(EditAnywhere)
	int32 SpawnSeperationDistance;

	UPROPERTY(EditAnywhere)
	bool bStartSpawnOnStart;

	UPROPERTY(EditAnywhere)
	int32 InitialAsteroidPoolSize = 60;
	
	TArray<TObjectPtr<ASAsteroid>> Asteroids;



private:

	void SpawnAsteroidsInternal();


	UFUNCTION()
	void OnMixedRealitySetupComplete(bool Result);
};
