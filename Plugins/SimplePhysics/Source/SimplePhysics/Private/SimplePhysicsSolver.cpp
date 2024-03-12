// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplePhysicsSolver.h"

#include "SimplePhysics.h"
#include "SimplePhysics_Settings.h"
#include "SimplePhysicsRigidBodyComponent.h"


const float USimplePhysicsSolver::MIN_TICK_TIME = 1e-6f;

USimplePhysicsSolver::USimplePhysicsSolver()
{
	MaxSimulationIterations = 3;
	MinimumSimulationVelocity = 0.01f;
}


void USimplePhysicsSolver::PostInitialize()
{
	Super::PostInitialize();

	if (USimplePhysics_Settings* SimplePhysicsSettings = GetMutableDefault<USimplePhysics_Settings>())
	{
		MaxSimulationIterations = SimplePhysicsSettings->MaxSimulationIterations;
		MinimumSimulationVelocity = SimplePhysicsSettings->MinimumSimulationVelocity;
	}
}

void USimplePhysicsSolver::SetSimulationEnabled(AActor* Actor, bool Enabled)
{
	if (Actor)
	{
		if (USimplePhysicsRigidBodyComponent* RigidBodyComponent = Actor->GetComponentByClass<USimplePhysicsRigidBodyComponent>())
		{
			SetSimulationEnabled(RigidBodyComponent, Enabled);
		}
	}
}


void USimplePhysicsSolver::SetSimulationEnabled(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, bool Enabled)
{
	if (Enabled)
	{
		AddRigidBodies.AddUnique(RigidBody);
	}
	else
	{
		InvalidRigidBodies.AddUnique(RigidBody);
	}
}


bool USimplePhysicsSolver::IsTickable() const
{
	return SimulatedRigidBodies.Num() > 0 || AddRigidBodies.Num() > 0 || InvalidRigidBodies.Num() > 0;
}


void USimplePhysicsSolver::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RegisterRigidBodies();

	QUICK_SCOPE_CYCLE_COUNTER(STAT_SimplePhysicsSolver_TickComponent);
	ValidateRigidBodyTick(DeltaTime);
}


void USimplePhysicsSolver::RegisterRigidBodies()
{
	for (auto RigidBody : AddRigidBodies)
	{
		SimulatedRigidBodies.AddUnique(RigidBody);
	}

	for (auto RigidBody : InvalidRigidBodies)
	{
		SimulatedRigidBodies.Remove(RigidBody);
	}

	AddRigidBodies.Empty();
	InvalidRigidBodies.Empty();
}


void USimplePhysicsSolver::ValidateRigidBodyTick(float DeltaTime)
{
	InvalidRigidBodies.Empty();
	RigidCollisionResultMap.Empty();

	UE_LOG(LogTemp, Warning, TEXT("Simulating Rigid Bodies:%d"), SimulatedRigidBodies.Num());

	for (auto RigidBody : SimulatedRigidBodies)
	{
		if (!RigidBody || !IsValid(RigidBody->UpdatedComponent))
		{
			InvalidRigidBodies.Add(RigidBody);
			continue;
		}

		const bool ValidTick = TickRigidBody(RigidBody, DeltaTime);
		
		if (!ValidTick)
		{
			InvalidRigidBodies.Add(RigidBody);
		}
	}
}


bool USimplePhysicsSolver::TickRigidBody(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, float DeltaTime)
{
	CSV_SCOPED_TIMING_STAT_EXCLUSIVE(TickRigidBody);

	if (RigidBody->HasStoppedSimulation() || RigidBody->ShouldSkipUpdate(DeltaTime))
	{
		return false;
	}

	if (!IsValid(RigidBody->UpdatedComponent))
	{
		return false;
	}

	if (RigidBody->UpdatedComponent->IsSimulatingPhysics())
	{
		return false;
	}

	ApplyRigidBodyMovement(RigidBody, DeltaTime);
	return true;
}


void USimplePhysicsSolver::ApplyRigidBodyMovement(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, float DeltaTime)
{
	float RemainingTime = DeltaTime;
	int32 NumImpacts = 0;
	int32 NumBounces = 0;
	int32 LoopCount = 0;
	int32 Iterations = 0;
	FHitResult Hit(1.f);
	

	while (RigidBody->IsSimulationEnabled() && RemainingTime >= MIN_TICK_TIME && (Iterations < MaxSimulationIterations) &&  !RigidBody->HasStoppedSimulation())
	{
		LoopCount++;
		Iterations++;

		// subdivide long ticks to more closely follow parabolic trajectory
		const float InitialTimeRemaining = RemainingTime;

		// TODO: Restructure allow for smaller ticks (simulate small movement for each object, then simulate again
		//const float TimeTick = ShouldUseSubStepping() ? GetSimulationTimeStep(RemainingTime, Iterations) : RemainingTime;
		const float TimeTick = RemainingTime;
		RemainingTime -= TimeTick;

		// Initial move state
		Hit.Time = 1.f;
		const FVector OldVelocity = RigidBody->Velocity;
		const FVector MoveDelta = RigidBody->ComputeMoveDelta(OldVelocity, TimeTick);

		// Handle Rotation here

		RigidBody->SafeMoveUpdatedComponent(MoveDelta, RigidBody->UpdatedComponent->GetComponentRotation(), true, Hit);

		// If we hit a trigger that destroyed us, abort.
		if (!IsValid(RigidBody) || !IsValid(RigidBody->UpdatedComponent) || !IsValid(RigidBody->UpdatedComponent->GetOwner()))
		{
			return;
		}

		if (!Hit.bBlockingHit)
		{
			RigidBody->SetMovementVelocityFromNoHit(OldVelocity, TimeTick);
		}
		else
		{
			if (RigidBody->Velocity == OldVelocity)
			{
				const FVector NewVelocity = RigidBody->ComputeVelocity(OldVelocity, TimeTick * Hit.Time);
				RigidBody->Velocity = (Hit.Time > UE_KINDA_SMALL_NUMBER) ? NewVelocity : OldVelocity;
			}

			NumImpacts++;
			float SubTickTimeRemaining = TimeTick * (1.f - Hit.Time);

			if (ShouldAbort(RigidBody, Hit))
			{
				break;
			}

			bool StillSimulating = true;
			if (USimplePhysicsRigidBodyComponent* OtherHitRigidBody = GetOtherHitRigidBody(Hit))
			{
				HandleRigidBodyCollision(RigidBody, OtherHitRigidBody);
			}
			else
			{
				StillSimulating = HandleImpact(RigidBody, Hit, TimeTick, MoveDelta);
			}

			if (!StillSimulating || ShouldAbort(RigidBody, Hit))
			{
				break;
			}

			RigidBody->PreviousHitTime = Hit.Time;
			RigidBody->PreviousHitNormal = RigidBody->ConstrainNormalToPlane(Hit.Normal);
			RigidBody->LimitVelocityFromCurrent();


			if (SubTickTimeRemaining >= MIN_TICK_TIME)
			{
				RemainingTime += SubTickTimeRemaining;
			}
		}

	

		if (IsBelowSimulationVelocity(RigidBody))
		{
			UE_LOG(LogTemp, Warning, TEXT("IsBelowSimulationVelocity"));
			StopSimulating(RigidBody);
		}

		RigidBody->UpdateComponentVelocity();
	}
}


bool USimplePhysicsSolver::HandleImpact(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	const FVector OldVelocity = RigidBody->Velocity;

	FVector NewVelocity = ComputeBounceResult(RigidBody, Hit, TimeSlice, MoveDelta);

	// Broadcast
	RigidBody->OnRigidBodyBounceDelegate.Broadcast(Hit, OldVelocity, NewVelocity);

	RigidBody->SetVelocity(NewVelocity);
	if (IsBelowSimulationVelocity(RigidBody))
	{
		StopSimulating(RigidBody);
		return false;
	}


	return true;
}

FVector USimplePhysicsSolver::ComputeBounceResult(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	FVector TempVelocity = RigidBody->Velocity;
	const FVector Normal = RigidBody->ConstrainNormalToPlane(Hit.Normal);
	const float VelocityDotNormal = FVector::DotProduct(TempVelocity, Normal);

	// Only if velocity is opposed by normal or parellel
	if (VelocityDotNormal <= 0.f)
	{
		// Project velocity onto normal in reflected direction
		const FVector ProjectedNormal = Normal * -VelocityDotNormal;

		// Point velocity in direction parallel to surface
		TempVelocity += ProjectedNormal;

		const bool SouldScaleFriction = RigidBody->bIsSliding || RigidBody->bBounceAngleAffectsFriction;
		if (SouldScaleFriction)
		{
			// Get how much parrel the bounce angle is to the surface. The closer to parrel the more friction to apply
			const float Friction = FMath::Clamp(FMath::Abs(VelocityDotNormal / TempVelocity.Size()), RigidBody->MinFrictionFraction, 1.f) * RigidBody->Friction;

			// Only tangential velocity should be affected by friction.
			TempVelocity *= FMath::Clamp(1.f - Friction, 0.f, 1.f);
		}

		// Coefficient of restitution only applies perpendicular to impact.
		TempVelocity += (ProjectedNormal * FMath::Max(RigidBody->Bounciness, 0.f));

		// Bounciness could cause us to exceed max speed.
		TempVelocity = RigidBody->LimitVelocity(TempVelocity);

	}

	return TempVelocity;
}


bool USimplePhysicsSolver::IsBelowSimulationVelocity(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody) const
{
	return RigidBody->Velocity.SizeSquared() < FMath::Square(MinimumSimulationVelocity);
}


void USimplePhysicsSolver::StopSimulating(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody)
{
	InvalidRigidBodies.Add(RigidBody);
	RigidBody->Velocity = FVector::ZeroVector;
	RigidBody->ClearPendingForce();
	RigidBody->UpdateComponentVelocity();
	RigidBody->OnSimulationStopDelegate.Broadcast();
}




bool USimplePhysicsSolver::ShouldAbort(USimplePhysicsRigidBodyComponent* RigidBody, const FHitResult& Hit) const
{
	if (!IsValid(RigidBody))
	{
		return true;
	}

	AActor* ActorOwner = RigidBody->UpdatedComponent ? RigidBody->UpdatedComponent->GetOwner() : nullptr;
	if (!IsValid(ActorOwner))
	{
		return true;
	}

	if (Hit.bStartPenetrating)
	{
		UE_LOG(LogTemp, Warning, TEXT("Asteroid %s is stuck inside %s.%s with velocity %s!"), *GetNameSafe(ActorOwner), *Hit.HitObjectHandle.GetName(), *GetNameSafe(Hit.GetComponent()), *RigidBody->Velocity.ToString());
		return true;
	}

	return false;
}


TObjectPtr<USimplePhysicsRigidBodyComponent> USimplePhysicsSolver::GetOtherHitRigidBody(const FHitResult& Hit) const
{
	if (AActor* OtherActor = Hit.GetActor())
	{
		return OtherActor->GetComponentByClass<USimplePhysicsRigidBodyComponent>();
	}

	return nullptr;
}






void USimplePhysicsSolver::HandleRigidBodyCollision(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, TObjectPtr<USimplePhysicsRigidBodyComponent> OtherRigidBody)
{
	UE_LOG(LogTemp, Warning, TEXT("HandleRigidBodyCollision"));

	if (RigidCollisionResultMap.Contains(RigidBody))
	{
		RigidBody->SetVelocity(RigidCollisionResultMap[RigidBody]);
	}
	else
	{
		FVector V1Final, V2Final;
		if (ComputeRigidBodyCollision(RigidBody, OtherRigidBody, V1Final, V2Final))
		{
			RigidCollisionResultMap.Emplace(RigidBody, V1Final);
			RigidCollisionResultMap.Emplace(OtherRigidBody, V2Final);

			RigidBody->SetVelocity(V1Final);

			if (OtherRigidBody->bEnableSimulationOnRigidBodyCollision)
			{
				OtherRigidBody->SetVelocity(V2Final);
				OtherRigidBody->SetSimulationEnabled(true);
			}
		}
	}
}


bool USimplePhysicsSolver::ComputeRigidBodyCollision(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody1, TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody2, FVector& V1, FVector& V2) const
{
	if (!RigidBody1 || !RigidBody2)
	{
		return false;
	}

	const float Mass1 = RigidBody1->Mass;
	const float Mass2 = RigidBody2->Mass;

	const FVector Velocity1Initial = RigidBody1->Velocity;
	const FVector Velocity2Initial = RigidBody2->Velocity;

	const float V1CoefficientOfRestitution = GetBounceFactor(RigidBody1, RigidBody2);
	const float V2CoefficientOfRestitution = GetBounceFactor(RigidBody2, RigidBody1);

	V1 = ((Mass1 - Mass2) * Velocity1Initial + (1.f + V1CoefficientOfRestitution) * Mass2 * Velocity2Initial) / (Mass1 + Mass2);
	V2 = ((1.f + V2CoefficientOfRestitution) * Mass1 * Velocity1Initial + (Mass2 - Mass1) * Velocity2Initial) / (Mass1 + Mass2);

	return true;
}


float USimplePhysicsSolver::GetBounceFactor(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, TObjectPtr<USimplePhysicsRigidBodyComponent> OtherRidigBody) const
{
	const float DefaultRigidBodyBounceFactor = RigidBody->Bounciness;
	const float DefaultOtherRigidBodyBounceFactor = OtherRidigBody->Bounciness;

	float RigidBodyBouceFactor = DefaultRigidBodyBounceFactor;

	switch (RigidBody->BounceCombine)
	{
		case EBounceCombine::Maximum:
			RigidBodyBouceFactor = FMath::Max(DefaultRigidBodyBounceFactor, DefaultOtherRigidBodyBounceFactor);
			break;

		case EBounceCombine::Minimum:
			RigidBodyBouceFactor = FMath::Max(DefaultRigidBodyBounceFactor, DefaultOtherRigidBodyBounceFactor);
			break;

		case EBounceCombine::Average:
			RigidBodyBouceFactor = (DefaultRigidBodyBounceFactor + DefaultOtherRigidBodyBounceFactor) / 2.f;
			break;

		// No need for EBounceCombine::Average to set RigidBodyBouceFactor to current value

		default:
			break;
	}

	return FMath::Clamp(RigidBodyBouceFactor, 0.f, 1.f);
}



