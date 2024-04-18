// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Asteroid/SAsteroidSpawner.h"

#include "Kismet/GameplayStatics.h"
#include "Gameplay/Asteroid/SAsteroidPrimaryDataAsset.h"
#include "Gameplay/Asteroid/SAsteroid.h"
#include "MRUtilityKitSubsystem.h"
#include "SPoolSubsystem.h"
#include "MixedRealitySetup/SMixedRealitySetup.h"

// Sets default values
ASAsteroidSpawner::ASAsteroidSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}


// Called when the game starts or when spawned
void ASAsteroidSpawner::BeginPlay()
{
	Super::BeginPlay();

	CreateAsteroidObjectPool();

	if (bStartSpawnOnStart)
	{
		SpawnAsteroids();
	}
}


void ASAsteroidSpawner::CreateAsteroidObjectPool()
{
	if (auto Subsystem = GetPoolSubsystem())
	{
		Subsystem->CreatePool(AsteroidClass, InitialAsteroidPoolSize);
	}
}


void ASAsteroidSpawner::SpawnAsteroids()
{
	UE_LOG(LogTemp, Warning, TEXT("SpawnAsteroids"));
	if (MixedRealitySetup)
	{
		if (MixedRealitySetup->GetSetupState() == ESetupState::ESS_Complete)
		{
			SpawnAsteroidsInternal();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SpawnAsteroids, adding dynamic"));
			MixedRealitySetup->OnSetupComplete.AddDynamic(this, &ASAsteroidSpawner::OnMixedRealitySetupComplete);
		}
	}
}


void ASAsteroidSpawner::OnMixedRealitySetupComplete(bool Result)
{
	UE_LOG(LogTemp, Warning, TEXT("OnMixedRealitySetupComplete"));
	MixedRealitySetup->OnSetupComplete.RemoveDynamic(this, &ASAsteroidSpawner::OnMixedRealitySetupComplete);
	if (Result)
	{
		SpawnAsteroidsInternal();
	}
}


void ASAsteroidSpawner::SpawnAsteroidsInternal()
{
	TArray<FVector> SpawnLocations;
	GetSpawnLocations(SpawnLocations);

	if (auto Subsystem = GetPoolSubsystem())
	{
		for (const auto& Location : SpawnLocations)
		{
			const FRotator RandomRotation(FMath::RandRange(-180.f, 180.f), FMath::RandRange(-180.f, 180.f), FMath::RandRange(-180.f, 180.f));
			if (ASAsteroid* Asteroid = Subsystem->SpawnFromPool<ASAsteroid>(AsteroidClass, Location, RandomRotation))
			{
				Asteroid->InitializeAsteroid(AsteroidDataAsset);
			}
		}
	}
}


void ASAsteroidSpawner::GetSpawnLocations(TArray<FVector>& Locations) const
{
	if (auto CurrentRoom = GetCurrentRoom())
	{
		for (int32 i = 0; i < MaxSpawnAttempts; ++i)
		{
			FVector PossiblePosition;
			if (CurrentRoom->GenerateRandomPositionInRoom(PossiblePosition, SpawnSeperationDistance, true))
			{
				if (!PositionOverlap(PossiblePosition, Locations, SpawnSeperationDistance))
				{
					Locations.Add(PossiblePosition);
					if (Locations.Num() >= MaxAsteroids)
					{
						return;
					}
				}
			}
		}
	}
}


bool ASAsteroidSpawner::PositionOverlap(const FVector& Position, const TArray<FVector>& OtherPositions, float Distance) const
{
	const float DistanceSqr = Distance * Distance;
	for (const auto& OtherPosition : OtherPositions)
	{
		if (FVector::DistSquared(OtherPosition, Position) < DistanceSqr)
		{
			return true;
		}
	}

	return false;
}


UMRUKSubsystem* ASAsteroidSpawner::GetMRUKSubsystem() const
{
	if (auto GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UMRUKSubsystem>();
	}

	return nullptr;
}


AMRUKRoom* ASAsteroidSpawner::GetCurrentRoom() const
{
	if (auto Subsystem = GetMRUKSubsystem())
	{
		return Subsystem->GetCurrentRoom();
	}

	return nullptr;
}


USPoolSubsystem* ASAsteroidSpawner::GetPoolSubsystem() const
{
	if (const UWorld* World = GetWorld())
	{
		return World->GetSubsystem<USPoolSubsystem>();
	}

	return nullptr;
}
