// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Asteroid/SAsteroidMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Asteroid/SAsteroidCollisionSolver.h"

USAsteroidMovementComponent::USAsteroidMovementComponent()
{
	PreviousHitTime = 1.f;
	bIsSliding = false;
	PreviousHitNormal = FVector::UpVector;
	BounceAdditionalIterations = 1;
	MaxSimulationIterations = 4;
}



void USAsteroidMovementComponent::BeginPlay()
{
	//if (auto FoundActor = UGameplayStatics::GetActorOfClass(this, ASAsteroidCollisionSolver::StaticClass()))
	//{
	//	AsteroidCollisionSolver = Cast<ASAsteroidCollisionSolver>(FoundActor);
	//}

	//if (AsteroidCollisionSolver)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Found Collision Solver"));
	//}
}

USAsteroidCollisionSolver* USAsteroidMovementComponent::GetAsteroidCollisionSolver() const
{
	if (const UWorld* World = GetWorld())
	{
		return World->GetSubsystem<USAsteroidCollisionSolver>();
	}
	return nullptr;
}


void USAsteroidMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	////return;

	//if (!IsValid(UpdatedComponent) || !bEnableMovement)
	//{
	//	return;
	//}

	//const float MinTickTime = 1e-6f;
	//float RemainingTime = DeltaTime;
	//int32 NumImpacts = 0;
	//int32 NumBounces = 0;
	//int32 LoopCount = 0;
	//int32 Iterations = 0;
	//FHitResult Hit(1.f);
	//while (bEnableMovement && (RemainingTime >= MinTickTime) && IsValid(UpdatedComponent) && (Iterations < MaxSimulationIterations))
	//{

	//	LoopCount++;
	//	Iterations++;

	//	const float InitialTimeRemaining = RemainingTime;
	//	const float TimeTick = RemainingTime;
	//	RemainingTime -= TimeTick;

	//	Hit.Time = 1.f;
	//	const FVector OldVelocity = Velocity;
	//	const FVector MoveDelta = ComputeMoveDelta(OldVelocity, TimeTick);

	//	// Should bounce
	//	SafeMoveUpdatedComponent(MoveDelta, UpdatedComponent->GetComponentRotation(), true, Hit);

	//	if (!Hit.bBlockingHit)
	//	{
	//		PreviousHitTime = 1.f;
	//		bIsSliding = false;

	//		if (Velocity == OldVelocity)
	//		{
	//			Velocity = ComputeVelocity(Velocity, TimeTick);
	//		}
	//	}
	//	else
	//	{
	//		// Only calculate new velocity if events didn't change it during the movement update.
	//		if (Velocity == OldVelocity)
	//		{
	//			// re-calculate end velocity for partial time
	//			Velocity = (Hit.Time > UE_KINDA_SMALL_NUMBER) ? ComputeVelocity(OldVelocity, TimeTick * Hit.Time) : OldVelocity;
	//		}

	//		NumImpacts++;
	//		float SubTickTimeRemaining = TimeTick * (1.f - Hit.Time);
	//		const EHandleBlockingHitResult HandleBlockingHitResult = HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	//		if (HandleBlockingHitResult == EHandleBlockingHitResult::Abort)
	//		{
	//			break;
	//		}
	//		else if (HandleBlockingHitResult == EHandleBlockingHitResult::Deflect)
	//		{
	//			NumBounces++;
	//			HandleDeflection(Hit, OldVelocity, NumBounces, SubTickTimeRemaining);
	//			PreviousHitTime = Hit.Time;
	//			PreviousHitNormal = ConstrainNormalToPlane(Hit.Normal);
	//		}
	//		else
	//		{
	//			checkNoEntry();
	//		}

	//		if (SubTickTimeRemaining >= MinTickTime)
	//		{
	//			RemainingTime += SubTickTimeRemaining;

	//			if (NumImpacts <= BounceAdditionalIterations)
	//			{
	//				Iterations--;
	//			}
	//		}

	//	}

	//}

	//if (LoopCount >= 2)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Loop Count: %d"), LoopCount);
	//}


	//UpdateComponentVelocity();
}

USAsteroidMovementComponent::EHandleBlockingHitResult USAsteroidMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	AActor* ActorOwner = UpdatedComponent ? UpdatedComponent->GetOwner() : NULL;
	if (/*!CheckStillInWorld() ||*/ !IsValid(ActorOwner))
	{
		return EHandleBlockingHitResult::Abort;
	}

	HandleImpact(Hit, TimeTick, MoveDelta);

	if (!IsValid(ActorOwner) /* || HasStoppedSimulation() */ )
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


void USAsteroidMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	bool bStopSimulating = false;

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
		StopSimulating(Hit);
	}
}

FVector USAsteroidMovementComponent::ComputeBounceResult(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
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

void USAsteroidMovementComponent::StopSimulating(const FHitResult& HitResult)
{
	Velocity = FVector::ZeroVector;
	UpdateComponentVelocity();
	SetUpdatedComponent(NULL);
}

void USAsteroidMovementComponent::SetMovementEnabled(bool Enabled)
{
	if (bEnableMovement == Enabled)
	{
		return;
	}

	if (auto Subsystem = GetAsteroidCollisionSolver())
	{
		if (Enabled)
		{
			Subsystem->RegisterAsteroidMovementComponent(this);
		}
		else
		{
			Subsystem->UnRegisterAsteroidMovementComponent(this);
		}

		bEnableMovement = Enabled;
		
	}
}

FVector USAsteroidMovementComponent::ComputeMoveDelta(const FVector& InVelocity, float DeltaTime) const
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

FVector USAsteroidMovementComponent::ComputeVelocity(FVector InitialVelocity, float DeltaTime) const
{
	// v = v0 + a*t
	const FVector Acceleration = ComputeAcceleration(InitialVelocity, DeltaTime);
	FVector NewVelocity = InitialVelocity + (Acceleration * DeltaTime);

	return LimitVelocity(NewVelocity);
}

FVector USAsteroidMovementComponent::ComputeAcceleration(const FVector& InVelocity, float DeltaTime) const
{
	FVector Acceleration(FVector::ZeroVector);

	//Acceleration.Z += GetGravityZ();

	//Acceleration += PendingForceThisUpdate;

	//if (bIsHomingProjectile && HomingTargetComponent.IsValid())
	//{
	//	Acceleration += ComputeHomingAcceleration(InVelocity, DeltaTime);
	//}

	return Acceleration;
}

FVector USAsteroidMovementComponent::LimitVelocity(FVector NewVelocity) const
{
	const float CurrentMaxSpeed = MaxSpeed;
	if (CurrentMaxSpeed > 0.f)
	{
		NewVelocity = NewVelocity.GetClampedToMaxSize(CurrentMaxSpeed);
	}

	return ConstrainDirectionToPlane(NewVelocity);
}

bool USAsteroidMovementComponent::HandleDeflection(FHitResult& Hit, const FVector& OldVelocity, const uint32 NumBounces, float& SubTickTimeRemaining)
{
	const FVector Normal = ConstrainNormalToPlane(Hit.Normal);

	// Multiple hits within very short time period?
	const bool bMultiHit = (PreviousHitTime < 1.f && Hit.Time <= UE_KINDA_SMALL_NUMBER);

	// if velocity still into wall (after HandleBlockingHit() had a chance to adjust), slide along wall
	const float DotTolerance = 0.01f;
	/*bIsSliding = (bMultiHit && FVector::Coincident(PreviousHitNormal, Normal)) ||
		((Velocity.GetSafeNormal() | Normal) <= DotTolerance);*/

	//if (bIsSliding)
	//{
	//	if (bMultiHit && (PreviousHitNormal | Normal) <= 0.f)
	//	{
	//		//90 degree or less corner, so use cross product for direction
	//		FVector NewDir = (Normal ^ PreviousHitNormal);
	//		NewDir = NewDir.GetSafeNormal();
	//		Velocity = Velocity.ProjectOnToNormal(NewDir);
	//		if ((OldVelocity | Velocity) < 0.f)
	//		{
	//			Velocity *= -1.f;
	//		}
	//		Velocity = ConstrainDirectionToPlane(Velocity);
	//	}
	//	else
	//	{
	//		//adjust to move along new wall
	//		Velocity = ComputeSlideVector(Velocity, 1.f, Normal, Hit);
	//	}

	//	// Check min velocity.
	//	if (IsVelocityUnderSimulationThreshold())
	//	{
	//		StopSimulating(Hit);
	//		return false;
	//	}

	//	// Velocity is now parallel to the impact surface.
	//	if (SubTickTimeRemaining > UE_KINDA_SMALL_NUMBER)
	//	{
	//		if (!HandleSliding(Hit, SubTickTimeRemaining))
	//		{
	//			return false;
	//		}
	//	}
	//}

	return true;
}

