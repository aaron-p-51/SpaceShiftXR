// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Asteroid/SAsteroid.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/Asteroid/SAsteroidPrimaryDataAsset.h"
#include "Gameplay/Asteroid/SAsteroidSpawner.h"
#include "Gameplay/Asteroid/SAsteroidMovementComponent.h"
#include "SPoolSubsystem.h"


// Sets default values
ASAsteroid::ASAsteroid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SetRootComponent(SphereComp);
	SphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
	SphereComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	SphereComp->SetGenerateOverlapEvents(false);
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(GetRootComponent());
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComp->SetGenerateOverlapEvents(false);

	MovementComp = CreateDefaultSubobject<USAsteroidMovementComponent>(TEXT("AsteroidMovementComp"));
}


// Called when the game starts or when spawned
void ASAsteroid::BeginPlay()
{
	Super::BeginPlay();

	Mass = DefaultMass;

	if (DataAsset)
	{
		InitializeAsteroid(DataAsset);
	}
	

}


void ASAsteroid::InitializeAsteroid(TObjectPtr<USAsteroidPrimaryDataAsset> AsteroidConfig)
{
	UE_LOG(LogTemp, Warning, TEXT("InitializeAsteroid"));
	if (!AsteroidConfig)
	{
		if (auto Subsystem = GetPoolSubsystem())
		{
			Subsystem->ReturnToPool(this);
		}
		else
		{
			Destroy();
		}

		return;
	}

	DataAsset = AsteroidConfig;

	const float Size = FMath::RandRange(DataAsset->MinScale, DataAsset->MaxScale);
	SetActorScale3D(FVector(Size, Size, Size));

	const float CenterScale = (DataAsset->MinScale + DataAsset->MaxScale) / 2.f;
	Mass = DataAsset->DefaultMass * Size / CenterScale;

	UE_LOG(LogTemp, Warning, TEXT("Size: %f, CenterScale: %f, DefaultMass: %f, Mass: %f"), Size, CenterScale, DataAsset->DefaultMass, Mass);


	if (MeshComp)
	{
		MeshComp->SetStaticMesh(DataAsset->Mesh);
		MeshComp->SetRelativeTransform(DataAsset->MeshLocalTransform);
		MeshComp->SetVisibility(true, true);
	}

	if (SphereComp)
	{
		SphereComp->SetSphereRadius(DataAsset->CollisionRadius);
		SphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SphereComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	}

	Health = DataAsset->Health;

	bHasFragments = (DataAsset->FragmentCount > 0) && (DataAsset->AsteroidFragments != nullptr);

	if (bHasFragments)
	{
		GenerateFragmentSpawnLocations();
	}
}


void ASAsteroid::GenerateFragmentSpawnLocations()
{
	if (!DataAsset || !SphereComp)
	{
		return;
	}

	while (FragmentSpawnPositions.Num() < DataAsset->FragmentCount)
	{
		const FVector Location = GetRandomPointInUnitSphere();
		if (ValidFragmentSpawnLocalPosition(Location))
		{
			FragmentSpawnPositions.Add(Location);
		}
	}

	for (auto& Position : FragmentSpawnPositions)
	{
		Position *= SphereComp->GetScaledSphereRadius();
	}
}


FVector ASAsteroid::GetRandomPointInUnitSphere() const
{
	const float Theta = FMath::FRandRange(0.0f, PI * 2.0f);
	const float Phi = FMath::Acos(FMath::FRandRange(-1.0f, 1.0f));

	const float X = FMath::Sin(Phi) * FMath::Cos(Theta);
	const float Y = FMath::Sin(Phi) * FMath::Sin(Theta);
	const float Z = FMath::Cos(Phi);

	return FVector(X, Y, Z);
}


bool ASAsteroid::ValidFragmentSpawnLocalPosition(const FVector& Value) const
{
	for (const auto& Position : FragmentSpawnPositions)
	{
		const float AngleBetween = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Position, Value)));
		if (AngleBetween < 60.f)
		{
			return false;
		}
	}

	return true;
}


const TArray<FVector>& ASAsteroid::GetFragmentSpawnPositions() const
{
	return FragmentSpawnPositions;
}


void ASAsteroid::OnSpawnFromPool()
{
	
}


void ASAsteroid::OnReturnToPool()
{
	if (MeshComp)
	{
		MeshComp->SetVisibility(false, true);
	}

	if (SphereComp)
	{
		SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComp->SetGenerateOverlapEvents(false);
	}
}


void ASAsteroid::AsteroidHit(AActor* OtherActor)
{
	Health -= 1.f;
	UE_LOG(LogTemp, Warning, TEXT("AsteroidHit"));
	if (Health <= 0.f)
	{
		AsteroidDestroyed();
	}
}


void ASAsteroid::AsteroidDestroyed()
{
	UE_LOG(LogTemp, Warning, TEXT("AsteroidDestroyed"));
	if (MeshComp)
	{
		MeshComp->SetVisibility(false, true);
	}

	if (SphereComp)
	{
		SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComp->SetGenerateOverlapEvents(false);
	}

	if (auto Subsystem = GetPoolSubsystem())
	{
		if (bHasFragments)
		{
			for (const auto& Position : FragmentSpawnPositions)
			{
				const FVector WorldPosition = Position + GetActorLocation();
				const FRotator RandomRotation(FMath::RandRange(-180.f, 180.f), FMath::RandRange(-180.f, 180.f), FMath::RandRange(-180.f, 180.f));
				if (ASAsteroid* Asteroid = Subsystem->SpawnFromPool<ASAsteroid>(ASAsteroid::StaticClass(), WorldPosition, RandomRotation))
				{
					Asteroid->InitializeAsteroid(DataAsset->AsteroidFragments);
				}
			}	
		}
		
		Subsystem->ReturnToPool(this);
	}
	else
	{
		Destroy();
	}
}


USPoolSubsystem* ASAsteroid::GetPoolSubsystem() const
{
	if (const UWorld* World = GetWorld())
	{
		return World->GetSubsystem<USPoolSubsystem>();
	}

	return nullptr;
}



