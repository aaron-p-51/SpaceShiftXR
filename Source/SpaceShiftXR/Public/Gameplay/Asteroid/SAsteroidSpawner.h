// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAsteroidSpawner.generated.h"

class USAsteroidPrimaryDataAsset;
class ASAsteroid;
class UMRUKSubsystem;
class AMRUKRoom;
class ASMixedRealitySetup;

UCLASS()
class SPACESHIFTXR_API ASAsteroidSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASAsteroidSpawner();

	int32 GetSpawnLocations(TArray<FVector>& Locations) const;
	bool PositionOverlap(const FVector& Position, const TArray<FVector>& OtherPositions, float Distance) const;

	UMRUKSubsystem* GetMRUKSubsystem() const;
	AMRUKRoom* GetCurrentRoom() const;

	void SpawnAsteroids();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



	UPROPERTY(EditAnywhere)
	TObjectPtr<ASMixedRealitySetup> MixedRealitySetup;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USAsteroidPrimaryDataAsset> AsteroidDataAsset;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ASAsteroid> AsteroidClass;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxSpawnAttempts = 500;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxAsteroids;

	UPROPERTY(EditDefaultsOnly)
	int32 SpawnSeperationDistance;

	UPROPERTY(EditDefaultsOnly)
	bool bStartSpawnOnStart;

	
	TArray<TObjectPtr<ASAsteroid>> Asteroids;

	TArray<TObjectPtr<ASAsteroid>> AsteroidFragmentPool;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	void SpawnAsteroidsInternal();

	UFUNCTION()
	void OnMixedRealitySetupComplete(bool Result);

	

};
