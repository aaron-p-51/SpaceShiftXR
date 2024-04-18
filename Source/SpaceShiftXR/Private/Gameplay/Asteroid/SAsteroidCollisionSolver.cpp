// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Asteroid/SAsteroidCollisionSolver.h"
#include "Gameplay/Asteroid/SAsteroidMovementComponent.h"
#include "Gameplay/Asteroid/SAsteroid.h"

// Sets default values
USAsteroidCollisionSolver::USAsteroidCollisionSolver()
{


}

void USAsteroidCollisionSolver::RegisterAsteroidMovementComponent(USAsteroidMovementComponent* MovementComp)
{
	if (MovementComp)
	{
		AstroidMovementComponents.Add(MovementComp);
	}
}

void USAsteroidCollisionSolver::UnRegisterAsteroidMovementComponent(USAsteroidMovementComponent* MovementComp)
{
	if (MovementComp)
	{
		AstroidMovementComponents.Remove(MovementComp);
	}
}


// Called every frame
void USAsteroidCollisionSolver::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp, Warning, TEXT("I have %d Asteroids registered"), AstroidMovementComponents.Num());

	// Process collisions with room collisions
	TickHandleAstroidMovement(DeltaTime);


}

void USAsteroidCollisionSolver::TickHandleAstroidMovement(float DeltaTime)
{
	CollisionResultMap.Empty();
	TArray<USAsteroidMovementComponent*> MovementComponentsToRemove;

	for (int32 i = 0; i < AstroidMovementComponents.Num(); ++i)
	{
		USAsteroidMovementComponent* CurrentMoveComp = AstroidMovementComponents[i];
		if (!CurrentMoveComp || !IsValid(CurrentMoveComp->UpdatedComponent))
		{
			MovementComponentsToRemove.Add(CurrentMoveComp);
			continue;
		}

		const float MinTickTime = 1e-6f;
		float RemainingTime = DeltaTime;
		int32 NumImpacts = 0;
		int32 NumBounces = 0;
		int32 LoopCount = 0;
		int32 Iterations = 0;
		FHitResult Hit(1.f);
		while ((RemainingTime >= MinTickTime) && IsValid(CurrentMoveComp->UpdatedComponent) && (Iterations < MaxSimulationIterations))
		{
			LoopCount++;
			Iterations++;

			const float InitialTimeRemaining = RemainingTime;
			const float TimeTick = RemainingTime;
			RemainingTime -= TimeTick;

			Hit.Time = 1.f;
			const FVector OldVelocity = CurrentMoveComp->Velocity;
			const FVector MoveDelta = ComputeMoveDelta(CurrentMoveComp, OldVelocity, TimeTick);

			// Should bounce
			CurrentMoveComp->SafeMoveUpdatedComponent(MoveDelta, CurrentMoveComp->UpdatedComponent->GetComponentRotation(), true, Hit);
		
			if (!Hit.bBlockingHit)
			{
				CurrentMoveComp->PreviousHitTime = 1.f;
				CurrentMoveComp->bIsSliding = false;

				if (CurrentMoveComp->Velocity == OldVelocity)
				{
					CurrentMoveComp->Velocity = ComputeVelocity(CurrentMoveComp, CurrentMoveComp->Velocity, TimeTick);
				}
			}
			else
			{
				if (CurrentMoveComp->Velocity == OldVelocity)
				{
					CurrentMoveComp->Velocity = (Hit.Time > UE_KINDA_SMALL_NUMBER) ? ComputeVelocity(CurrentMoveComp, OldVelocity, TimeTick * Hit.Time) : OldVelocity;
				}

				NumImpacts++;
				float SubTickTimeRemaining = TimeTick * (1.f - Hit.Time);
				const EHandleBlockingHitResult HandleBlockingHitResult = HandleBlockingHit(CurrentMoveComp, Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
				if (HandleBlockingHitResult == EHandleBlockingHitResult::Abort)
				{
					break;
				}
				else if (HandleBlockingHitResult == EHandleBlockingHitResult::Deflect)
				{
					NumBounces++;
					HandleDeflection(CurrentMoveComp, Hit, OldVelocity, NumBounces, SubTickTimeRemaining);
					CurrentMoveComp->PreviousHitTime = Hit.Time;
					CurrentMoveComp->PreviousHitNormal = CurrentMoveComp->ConstrainNormalToPlane(Hit.Normal);
				}
				else
				{
					checkNoEntry();
				}


				if (SubTickTimeRemaining >= MinTickTime)
				{
					RemainingTime += SubTickTimeRemaining;

					if (NumImpacts <= CurrentMoveComp->BounceAdditionalIterations)
					{
						Iterations--;
					}
				}
			}
		}

		CurrentMoveComp->UpdateComponentVelocity();
	}

	for (auto MoveComps : MovementComponentsToRemove)
	{
		AstroidMovementComponents.Remove(MoveComps);
	}

}

FVector USAsteroidCollisionSolver::ComputeMoveDelta(USAsteroidMovementComponent* MovementComp, const FVector& InVelocity, float DeltaTime) const
{
	// Velocity Verlet integration (http://en.wikipedia.org/wiki/Verlet_integration#Velocity_Verlet)
// The addition of p0 is done outside this method, we are just computing the delta.
// p = p0 + v0*t + 1/2*a*t^2

// We use ComputeVelocity() here to infer the acceleration, to make it easier to apply custom velocities.
// p = p0 + v0*t + 1/2*((v1-v0)/t)*t^2
// p = p0 + v0*t + 1/2*((v1-v0))*t

	const FVector NewVelocity = ComputeVelocity(MovementComp, InVelocity, DeltaTime);
	const FVector Delta = (InVelocity * DeltaTime) + (NewVelocity - InVelocity) * (0.5f * DeltaTime);
	return Delta;
}

FVector USAsteroidCollisionSolver::ComputeBounceResult(USAsteroidMovementComponent* MovementComp, const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	// Check if we hit an asteroid
	if (ASAsteroid* HitAsteroid = Cast<ASAsteroid>(Hit.GetActor()))
	{
		// If we hit an asteroid then the other asteroid may have already computed our resulting velocity
		if (CollisionResultMap.Contains(MovementComp))
		{
			const FVector V1Final = CollisionResultMap[MovementComp];
			UE_LOG(LogTemp, Warning, TEXT("I hit an asteroid, has entry in map, V1:%s"), *V1Final.ToString());
			return V1Final;
		}

		if (USAsteroidMovementComponent* OtherMovementComp = HitAsteroid->GetComponentByClass<USAsteroidMovementComponent>())
		{
			FVector V1Final;
			FVector V2Final;
			ComputeAsteroidCollisionVelocities(MovementComp, OtherMovementComp, V1Final, V2Final);
			CollisionResultMap.Emplace(MovementComp, V1Final);
			CollisionResultMap.Emplace(OtherMovementComp, V2Final);


			UE_LOG(LogTemp, Warning, TEXT("I hit an asteroid, no entry in map, V1:%s, V2:%s"), *V1Final.ToString(), *V2Final.ToString());


			OtherMovementComp->Velocity = V2Final;
			OtherMovementComp->UpdateComponentVelocity();

			return V1Final;
		}
	}
	

	FVector TempVelocity = MovementComp->Velocity;
	const FVector Normal = MovementComp->ConstrainNormalToPlane(Hit.Normal);
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
		TempVelocity = LimitVelocity(MovementComp, TempVelocity);
	}

	return TempVelocity;
}

FVector USAsteroidCollisionSolver::ComputeVelocity(USAsteroidMovementComponent* MovementComp, const FVector& InitialVelocity, float DeltaTime) const
{
	// v = v0 + a*t
	const FVector Acceleration = ComputeAcceleration(MovementComp, InitialVelocity, DeltaTime);
	FVector NewVelocity = InitialVelocity + (Acceleration * DeltaTime);

	return LimitVelocity(MovementComp, NewVelocity);
}

FVector USAsteroidCollisionSolver::ComputeAcceleration(USAsteroidMovementComponent* MovementComp, const FVector& InVelocity, float DeltaTime) const
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

FVector USAsteroidCollisionSolver::LimitVelocity(USAsteroidMovementComponent* MovementComp, FVector NewVelocity) const
{
	if (!MovementComp)
	{
		return NewVelocity;
	}

	const float CurrentMaxSpeed = MovementComp->MaxSpeed;
	if (CurrentMaxSpeed > 0.f)
	{
		NewVelocity = NewVelocity.GetClampedToMaxSize(CurrentMaxSpeed);
	}

	return MovementComp->ConstrainDirectionToPlane(NewVelocity);
}

USAsteroidCollisionSolver::EHandleBlockingHitResult USAsteroidCollisionSolver::HandleBlockingHit(USAsteroidMovementComponent* MovementComp, const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	AActor* ActorOwner = MovementComp->UpdatedComponent ? MovementComp->UpdatedComponent->GetOwner() : NULL;
	if (/*!CheckStillInWorld() ||*/ !IsValid(ActorOwner))
	{
		return EHandleBlockingHitResult::Abort;
	}

	HandleImpact(MovementComp, Hit, TimeTick, MoveDelta);

	if (!IsValid(ActorOwner) /* || HasStoppedSimulation() */)
	{
		return EHandleBlockingHitResult::Abort;
	}

	if (Hit.bStartPenetrating)
	{
		UE_LOG(LogTemp, Warning, TEXT("Asteroid %s is stuck inside %s.%s with velocity %s!"), *GetNameSafe(ActorOwner), *Hit.HitObjectHandle.GetName(), *GetNameSafe(Hit.GetComponent()), *MovementComp->Velocity.ToString());
		return EHandleBlockingHitResult::Abort;
	}

	SubTickTimeRemaining = TimeTick * (1.f - Hit.Time);
	return EHandleBlockingHitResult::Deflect;
}

void USAsteroidCollisionSolver::HandleImpact(USAsteroidMovementComponent* MovementComp, const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	bool bStopSimulating = false;

	if (true /*bShouldBounce*/)
	{
		const FVector OldVelocity = MovementComp->Velocity;
		MovementComp->Velocity = ComputeBounceResult(MovementComp, Hit, TimeSlice, MoveDelta);

		// Trigger bounce events
		//OnProjectileBounce.Broadcast(Hit, OldVelocity);

		// Event may modify velocity or threshold, so check velocity threshold now.
		MovementComp->Velocity = LimitVelocity(MovementComp, MovementComp->Velocity);
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
		StopSimulating(MovementComp, Hit);
	}
}

void USAsteroidCollisionSolver::StopSimulating(USAsteroidMovementComponent* MovementComp, const FHitResult& HitResult)
{
	MovementComp->Velocity = FVector::ZeroVector;
	MovementComp->UpdateComponentVelocity();
	MovementComp->SetUpdatedComponent(NULL);
}

void USAsteroidCollisionSolver::ComputeAsteroidCollisionVelocities(USAsteroidMovementComponent* MovementComp1, USAsteroidMovementComponent* MovementComp2, FVector& V1Final, FVector& V2Final)
{
	if (!MovementComp1 || !MovementComp2)
	{
		return;
	}

	const float Mass1 = MovementComp1->Mass;
	const float Mass2 = MovementComp2->Mass;

	const FVector Velocity1Initial = MovementComp1->Velocity;
	const FVector Velocity2Initial = MovementComp2->Velocity;

	V1Final = ((Mass1 - Mass2) * Velocity1Initial + (1.f + 0.3f) * Mass2 * Velocity2Initial) / (Mass1 + Mass2);
	V2Final = ((1.f + 0.3) * Mass1 * Velocity1Initial + (Mass2 - Mass1) * Velocity2Initial) / (Mass1 + Mass2);
}

bool USAsteroidCollisionSolver::HandleDeflection(USAsteroidMovementComponent* MovementComp, FHitResult& Hit, const FVector& OldVelocity, const uint32 NumBounces, float& SubTickTimeRemaining)
{
	const FVector Normal = MovementComp->ConstrainNormalToPlane(Hit.Normal);

	// Multiple hits within very short time period?
	const bool bMultiHit = (MovementComp->PreviousHitTime < 1.f && Hit.Time <= UE_KINDA_SMALL_NUMBER);

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


TStatId USAsteroidCollisionSolver::GetStatId() const
{
	return TStatId();
}

