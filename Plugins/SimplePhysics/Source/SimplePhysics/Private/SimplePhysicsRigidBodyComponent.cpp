// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplePhysicsRigidBodyComponent.h"

#include "SimplePhysics_Settings.h"
#include "SimplePhysicsSolver.h"

USimplePhysicsRigidBodyComponent::USimplePhysicsRigidBodyComponent()
{
	bUseGravity = false;
	Mass = 100.f;
	Friction = 0.2f;
	MinFrictionFraction = 0.f;
	Bounciness = 0.6f;
	LinearDamping = 0.f;
	MaxSpeed = 1000.f;
}

void USimplePhysicsRigidBodyComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (USimplePhysics_Settings* SimplePhysicsSettings = GetMutableDefault<USimplePhysics_Settings>())
	{
		GravityAcceleration = SimplePhysicsSettings->GravityAcceleration;
		
	}
}


USimplePhysicsSolver* USimplePhysicsRigidBodyComponent::GetSimplePhysicsSolver() const
{
	if (const UWorld* World = GetWorld())
	{
		return World->GetSubsystem<USimplePhysicsSolver>();
	}
	return nullptr;
}


void USimplePhysicsRigidBodyComponent::SetSimulationEnabled(bool Enabled)
{
	if (!IsValid(UpdatedComponent) || bSimulationEnabled == Enabled)
	{
		return;
	}

	if (auto Subsystem = GetSimplePhysicsSolver())
	{
		Subsystem->SetSimulationEnabled(this, Enabled);
		bSimulationEnabled = Enabled;
	}
}



void USimplePhysicsRigidBodyComponent::AddForce(const FVector& Force)
{
	if (bSimulationEnabled)
	{
		PendingForce += Force;
	}
}


FVector USimplePhysicsRigidBodyComponent::GetLinearDragForce(const FVector& InVelocity) const
{
	return 0.5f * -InVelocity.GetSafeNormal() * InVelocity.SizeSquared() * LinearDamping;
}


FVector USimplePhysicsRigidBodyComponent::ComputeMoveDelta(const FVector& InVelocity, float DeltaTime) const
{
	// Velocity Verlet integration (http://en.wikipedia.org/wiki/Verlet_integration#Velocity_Verlet)
    // The addition of p0 is done outside this method, we are just computing the delta.
    // p = p0 + v0*t + 1/2*a*t^2
    
    // We use ComputeVelocity() here to infer the acceleration, to make it easier to apply custom velocities.
    // p = p0 + v0*t + 1/2*((v1-v0)/t)*t^2
    // p = p0 + v0*t + 1/2*((v1-v0))*t

	const FVector NewVelocity = ComputeVelocity(InVelocity, DeltaTime);
	const FVector Delta = (InVelocity * DeltaTime) + (NewVelocity - InVelocity) * (0.5f * DeltaTime);
	return Delta;
}




FVector USimplePhysicsRigidBodyComponent::ComputeVelocity(const FVector& InitialVelocity, float DeltaTime) const
{
	// v = v0 + a*t
	const FVector Acceleration = ComputeAcceleration(InitialVelocity, DeltaTime);
	const FVector NewVelocity = InitialVelocity + (Acceleration * DeltaTime);

	return LimitVelocity(NewVelocity);
}


void USimplePhysicsRigidBodyComponent::SetVelocity(const FVector& NewVelocity, bool UpdateVelocity)
{
	Velocity = LimitVelocity(NewVelocity);

	if (UpdateVelocity)
	{
		UpdateComponentVelocity();
	}
}


FVector USimplePhysicsRigidBodyComponent::LimitVelocity(FVector NewVelocity) const
{
	const float CurrentMaxSpeed = GetMaxSpeed();
	if (CurrentMaxSpeed > 0.f)
	{
		NewVelocity = NewVelocity.GetClampedToMaxSize(CurrentMaxSpeed);
	}

	return ConstrainDirectionToPlane(NewVelocity);
}


FVector USimplePhysicsRigidBodyComponent::LimitVelocityFromCurrent()
{
	const float CurrentMaxSpeed = GetMaxSpeed();
	if (CurrentMaxSpeed > 0.f)
	{
		Velocity = Velocity.GetClampedToMaxSize(CurrentMaxSpeed);
	}

	return ConstrainDirectionToPlane(Velocity);
}


FVector USimplePhysicsRigidBodyComponent::ComputeAcceleration(const FVector& InitialVelocity, float DeltaTime) const
{
	check(Mass != 0.f);

	FVector Acceleration(FVector::ZeroVector);
	
	if (bUseGravity)
	{
		Acceleration.Z += GetGravityZ();
	}

	FVector Force = GetLinearDragForce(InitialVelocity);
	Force += GetPendingForce();

	Acceleration += (Force / Mass);

	return Acceleration;
}


void USimplePhysicsRigidBodyComponent::SetMovementVelocityFromNoHit(const FVector& OldVelocity, float DeltaTime)
{
	PreviousHitTime = 1.f;
	bIsSliding = false;

	if (Velocity == OldVelocity)
	{
		Velocity = ComputeVelocity(Velocity, DeltaTime);
	}
}


void USimplePhysicsRigidBodyComponent::StopAllMovementImmediately()
{
	SetVelocity(FVector::ZeroVector);
}


bool USimplePhysicsRigidBodyComponent::HasStoppedSimulation() const
{
	return ((UpdatedComponent == nullptr) || (bSimulationEnabled == false) || (IsActive() == false));
}

float USimplePhysicsRigidBodyComponent::GetGravityZ() const
{
	return bUseGravity ? GravityAcceleration * GravityScale : 0.f;
}

EHandleBlockingHitResult USimplePhysicsRigidBodyComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	AActor* ActorOwner = UpdatedComponent ? UpdatedComponent->GetOwner() : nullptr;
	if (!IsValid(ActorOwner))
	{
		return EHandleBlockingHitResult::Abort;
	}

	HandleImpact(Hit, TimeTick, MoveDelta);

	if (!IsValid(ActorOwner))
	{
		return EHandleBlockingHitResult::Abort;
	}

	if (Hit.bStartPenetrating)
	{
		UE_LOG(LogTemp, Warning, TEXT("Asteroid %s is stuck inside %s.%s with velocity %s!"), *GetNameSafe(ActorOwner), *Hit.HitObjectHandle.GetName(), *GetNameSafe(Hit.GetComponent()), *Velocity.ToString());
		return EHandleBlockingHitResult::Abort;
	}

	SubTickTimeRemaining = TimeTick * (1.f - Hit.Time);
	return EHandleBlockingHitResult::Deflect;
}


void USimplePhysicsRigidBodyComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	const FVector NormalT = ConstrainNormalToPlane(FVector(-1.f, 0.f, 0.f));
	UE_LOG(LogTemp, Warning, TEXT("Test Normal: %s"), *NormalT.ToString());

	bool bStopSimulating = false;

	//const FVector OldVelocity = Velocity;
	//Velocity = ComputeBounceResult(Hit, TimeSlice, MoveDelta);

	//// Broadcast Event here

	//// Event may cause properties to change, ensure velocity is within valid range
	//Velocity = LimitVelocityFromCurrent();



	if (true /*bShouldBounce*/)
	{
		const FVector OldVelocity = Velocity;
		Velocity = ComputeBounceResult(Hit, TimeSlice, MoveDelta);

		// Trigger bounce events
		//OnProjectileBounce.Broadcast(Hit, OldVelocity);

		// Event may modify velocity or threshold, so check velocity threshold now.
		Velocity = LimitVelocity(Velocity);
		/*if (IsVelocityUnderSimulationThreshold())
		{
			bStopSimulating = true;
		}*/
	}
	else
	{
		bStopSimulating = true;
	}


	if (bStopSimulating)
	{
		SetSimulationEnabled(false);
	}
}




FVector USimplePhysicsRigidBodyComponent::ComputeBounceResult(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//// Check if we hit an asteroid
	//if (ASAsteroid* HitAsteroid = Cast<ASAsteroid>(Hit.GetActor()))
	//{
	//	// If we hit an asteroid then the other asteroid may have already computed our resulting velocity
	//	if (CollisionResultMap.Contains(MovementComp))
	//	{
	//		const FVector V1Final = CollisionResultMap[MovementComp];
	//		UE_LOG(LogTemp, Warning, TEXT("I hit an asteroid, has entry in map, V1:%s"), *V1Final.ToString());
	//		return V1Final;
	//	}

	//	if (USAsteroidMovementComponent* OtherMovementComp = HitAsteroid->GetComponentByClass<USAsteroidMovementComponent>())
	//	{
	//		FVector V1Final;
	//		FVector V2Final;
	//		ComputeAsteroidCollisionVelocities(MovementComp, OtherMovementComp, V1Final, V2Final);
	//		CollisionResultMap.Emplace(MovementComp, V1Final);
	//		CollisionResultMap.Emplace(OtherMovementComp, V2Final);


	//		UE_LOG(LogTemp, Warning, TEXT("I hit an asteroid, no entry in map, V1:%s, V2:%s"), *V1Final.ToString(), *V2Final.ToString());


	//		OtherMovementComp->Velocity = V2Final;
	//		OtherMovementComp->UpdateComponentVelocity();

	//		return V1Final;
	//	}
	//}


	FVector TempVelocity = Velocity;
	const FVector Normal = ConstrainNormalToPlane(Hit.Normal);
	const float VDotNormal = (TempVelocity | Normal);

	// Only if velocity is opposed by normal or parallel
	if (VDotNormal <= 0.f)
	{
		// Project velocity onto normal in reflected direction.
		const FVector ProjectedNormal = Normal * -VDotNormal;

		// Point velocity in direction parallel to surface
		TempVelocity += ProjectedNormal;

		// Only tangential velocity should be affected by friction.
		//const float ScaledFriction = (bBounceAngleAffectsFriction || bIsSliding) ? FMath::Clamp(-VDotNormal / TempVelocity.Size(), MinFrictionFraction, 1.f) * Friction : Friction;
		//TempVelocity *= FMath::Clamp(1.f - /*(ScaledFriction*/, 0.f, 1.f);

		// Coefficient of restitution only applies perpendicular to impact.
		TempVelocity += (ProjectedNormal * FMath::Max(/* Bounciness*/ 1.f, 0.f));

		// Bounciness could cause us to exceed max speed.
		TempVelocity = LimitVelocity(TempVelocity);
	}

	return TempVelocity;
}