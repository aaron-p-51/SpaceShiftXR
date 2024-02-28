// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Asteroid/SAsteroid.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/Asteroid/SAsteroidPrimaryDataAsset.h"


// Sets default values
ASAsteroid::ASAsteroid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SetRootComponent(SphereComp);
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(GetRootComponent());
}

void ASAsteroid::InitializeAsFragment(bool Value)
{
	bIsFramgnet = Value;
}


void ASAsteroid::PostInitializeComponents()
{
	Super::PostInitializeComponents();


	if (DataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASAsteroid::PostInitializeComponents 2"));
		if (MeshComp)
		{
			MeshComp->SetStaticMesh(DataAsset->Mesh);
			MeshComp->SetRelativeTransform(DataAsset->MeshLocalTransform);
			MeshComp->SetVisibility(true, true);
		}

		if (SphereComp)
		{
			SphereComp->SetSphereRadius(DataAsset->CollisionRadius);
		}

		GenerateFragmentSpawnLocations();
	}
	UE_LOG(LogTemp, Warning, TEXT("ASAsteroid::PostInitializeComponents"));
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

bool ASAsteroid::ValidFragmentSpawnLocalPosition(const FVector& Value) const
{
	for (const auto Position : FragmentSpawnPositions)
	{
		const float AngleBetween = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Position, Value)));
		if (AngleBetween < 60.f)
		{
			return false;
		}
	}

	return true;
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

// Called when the game starts or when spawned
void ASAsteroid::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("ASAsteroid::BeginPlay"));
	
}



// Called every frame
void ASAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

