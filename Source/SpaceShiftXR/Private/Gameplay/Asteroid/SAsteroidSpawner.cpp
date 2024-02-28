// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Asteroid/SAsteroidSpawner.h"

#include "Kismet/GameplayStatics.h"
#include "Gameplay/Asteroid/SAsteroid.h"
#include "Gameplay/Asteroid/SAsteroidPrimaryDataAsset.h"
#include "MRUtilityKitSubsystem.h"
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

	if (bStartSpawnOnStart)
	{
		SpawnAsteroids();
	}
}


int32 ASAsteroidSpawner::GetSpawnLocations(TArray<FVector>& Locations) const
{
	int32 FoundLocations = 0;
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
					FoundLocations++;

					if (FoundLocations >= MaxAsteroids)
					{
						break;
					}
				}
			}
		}
	}

	return FoundLocations;
}


bool ASAsteroidSpawner::PositionOverlap(const FVector& Position, const TArray<FVector>& OtherPositions, float Distance) const
{
	const float DistanceSqr = Distance * Distance;
	for (const auto OtherPosition : OtherPositions)
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
	const int32 ValidSpawnLocations = GetSpawnLocations(SpawnLocations);

	UE_LOG(LogTemp, Warning, TEXT("SpawnAsteroidsInternal, %d valid positions"), ValidSpawnLocations);


	for (int32 i = 0; i < ValidSpawnLocations; ++i)
	{
		const FRotator Rotation(FMath::RandRange(-180.f, 180.f), FMath::RandRange(-180.f, 180.f), FMath::RandRange(-180.f, 180.f));
		const float ScaleFactor = FMath::RandRange(AsteroidDataAsset->MinScale, AsteroidDataAsset->MaxScale);
		const FVector Scale(ScaleFactor, ScaleFactor, ScaleFactor);

		const FTransform SpawnTransform(Rotation, SpawnLocations[i], Scale);
		auto Asteroid = GetWorld()->SpawnActorDeferred<ASAsteroid>(AsteroidClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (Asteroid)
		{
			Asteroid->DataAsset = AsteroidDataAsset;
			Asteroid->InitializeAsFragment(false);
			Asteroid->FinishSpawning(SpawnTransform);
			Asteroids.Add(Asteroid);
		}
	}
}


AMRUKRoom* ASAsteroidSpawner::GetCurrentRoom() const
{
	if (auto Subsystem = GetMRUKSubsystem())
	{
		return Subsystem->GetCurrentRoom();
	}

	return nullptr;
}


// Called every frame
void ASAsteroidSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



