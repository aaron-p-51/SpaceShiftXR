// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/SPhysicsAsteroid.h"

#include "Components/SphereComponent.h"

// Sets default values
ASPhysicsAsteroid::ASPhysicsAsteroid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SetRootComponent(SphereComp);

}

// Called when the game starts or when spawned
void ASPhysicsAsteroid::BeginPlay()
{
	Super::BeginPlay();


	//SphereComp->OnComponentHit.AddDynamic(this, &ASPhysicsAsteroid::OnSphereCompHit);
}

// Called every frame
void ASPhysicsAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DeltaTime != 0.f)
	{
		Acceleration = (GetVelocity() - LastVelocity) / DeltaTime;
	}

}

//void ASPhysicsAsteroid::OnSphereCompHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
//{
//	if (SphereComp && SphereComp->IsSimulatingPhysics())
//	{
//		
//		if (ASPhysicsAsteroid* OtherAsteroid = Cast<ASPhysicsAsteroid>(OtherActor))
//		{
//			if (HasHit && OtherAsteroid->HasHit) return;
//
//			UE_LOG(LogTemp, Warning, TEXT("OnSphereCompHit"));
//			OtherAsteroid->SphereComp->SetSimulatePhysics(true);
//
//			const float ThisAsteroidMass = SphereComp->GetMass();
//			const float OtherAsteroidMass = OtherAsteroid->SphereComp->GetMass();
//
//			const FVector ThisAsteroidVelocity = GetVelocity();
//			const FVector OtherAsteroidVelocity = OtherAsteroid->GetVelocity();
//
//			const FVector ThisAsteroidNewVelocity = ((ThisAsteroidMass - OtherAsteroidMass) * ThisAsteroidVelocity + 2 * OtherAsteroidMass * OtherAsteroidVelocity) / (ThisAsteroidMass + OtherAsteroidMass);
//			const FVector OtherAsteroidNewVelocity = (2 * OtherAsteroidMass * OtherAsteroidVelocity + (ThisAsteroidMass - OtherAsteroidMass) * ThisAsteroidVelocity) / (ThisAsteroidMass + OtherAsteroidMass);
//		
//			SphereComp->ComponentVelocity = ThisAsteroidVelocity;
//			OtherAsteroid->SphereComp->ComponentVelocity = OtherAsteroidNewVelocity;
//
//			HasHit = true;
//			OtherAsteroid->HasHit = true;
//		
//		}
//	}
//}

