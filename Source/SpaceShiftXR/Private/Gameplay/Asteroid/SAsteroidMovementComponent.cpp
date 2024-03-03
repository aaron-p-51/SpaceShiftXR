// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Asteroid/SAsteroidMovementComponent.h"

#include "Gameplay/Asteroid/SAsteroid.h"


// Sets default values for this component's properties
USAsteroidMovementComponent::USAsteroidMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	//bSimulationEnabled = true;
	//MaxSimulationIterations = 4;
}


// Called when the game starts
void USAsteroidMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

FVector USAsteroidMovementComponent::ComputeBounceResult(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	return Super::ComputeBounceResult(Hit, TimeSlice, MoveDelta);

	//auto OtherAsteroid = Cast<ASAsteroid>(Hit.GetActor());
	//if (OtherAsteroid)
	//{
	//	const FVector OtherAsteroidVelocity = OtherAsteroid->MovementComp->Velocity;
	//	const FVector RelativeVelocity = Velocity - OtherAsteroidVelocity;
	//	const float OtherMass = OtherAsteroid->Mass;

	//	auto AstroidOwner = Cast<ASAsteroid>(GetOwner());
	//	const FVector FinalVelocity = (RelativeVelocity * (AstroidOwner->Mass - OtherMass) + 2.f * OtherMass * OtherAsteroidVelocity) / (AstroidOwner->Mass + OtherAsteroid->Mass);
	//	
	//	FVector OtherAsteroidFinalVelocity =  (2.f * AstroidOwner->Mass * Velocity - (AstroidOwner->Mass - OtherMass) * OtherAsteroidVelocity) / (AstroidOwner->Mass + OtherAsteroid->Mass);

	//	/*OtherAsteroid->MovementComp->Velocity = OtherAsteroidVelocity;
	//	OtherAsteroid->MovementComp->UpdateComponentVelocity();*/

	//	return FinalVelocity;

	//}
	//else
	//{

	//	
	//}

	

}


void USAsteroidMovementComponent::SetInitialForce(const FVector& Force)
{
	AddForce(Force);
	//bSimulationEnabled = true;

}



// Called every frame
void USAsteroidMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// TODO: Add checking here for if we shoudl updat (Check Projectile Movement)
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Get force to make velocity to to 0
	//if (Velocity.SquaredLength() > 0.1f)
	//{
	//	FVector Drag = 0.5f * -Velocity.GetSafeNormal() * /*Velocity.SizeSquared() * */ DragCoefficient;
	//	AddForce(Drag);
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Velocity lower"));
	//	//bSimulationEnabled = false;
	//	Velocity = FVector::ZeroVector;
	//	UpdateComponentVelocity();
	//}

	//UE_LOG(LogTemp, Warning, TEXT("Velocity Sqrd: %f"), Velocity.SizeSquared());




	//if (!IsValid(UpdatedComponent) || !bSimulationEnabled)
	//{
	//	return;
	//}

	//AActor* ActorOwner = UpdatedComponent->GetOwner();
	//if (!ActorOwner)
	//{
	//	return;
	//}

	//if (UpdatedComponent->IsSimulatingPhysics())
	//{
	//	return;
	//}

	//float RemainingTime = DeltaTime;
	//int32 NumImpacts = 0;
	//int32 NumBounces = 0;
	//int32 LoopCount = 0;
	//int32 Iterations = 0;
	//FHitResult Hit(1.f);

	//while (bSimulationEnabled && RemainingTime >= MIN_TICK_TIME && (Iterations < MaxSimulationIterations) && IsValid(ActorOwner))
	//{
	//	LoopCount++;
	//	Iterations++;

	//	const float InitialTimeRemaining = RemainingTime;
	//	const float TimeTick = RemainingTime;
	//	RemainingTime -= TimeTick;

	//	// Initial move state
	//	Hit.Time = 1.f;
	//	const FVector OldVelocity = Velocity;
	//	const FVector MoveDelta = ComputeMoveDelta(OldVelocity, TimeTick);
	//}
}


//FVector USAsteroidMovementComponent::ComputeMoveDelta(const FVector& InVelocity, float DeltaTime) const
//{
//	// Velocity Verlet integration (http://en.wikipedia.org/wiki/Verlet_integration#Velocity_Verlet)
//	// The addition of p0 is done outside this method, we are just computing the delta.
//	// p = p0 + v0*t + 1/2*a*t^2
//
//	// We use ComputeVelocity() here to infer the acceleration, to make it easier to apply custom velocities.
//	// p = p0 + v0*t + 1/2*((v1-v0)/t)*t^2
//	// p = p0 + v0*t + 1/2*((v1-v0))*t
//
//	const FVector NewVelocity = ComputeVelocity(InVelocity, DeltaTime);
//	const FVector Delta = (InVelocity * DeltaTime) + (NewVelocity - InVelocity) * (0.5f * DeltaTime);
//	return Delta;
//}

