// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplePhysicsSolver.h"

#include "SimplePhysics.h"
#include "SimplePhysics_Settings.h"
#include "SimplePhysicsRigidBodyComponent.h"


const float USimplePhysicsSolver::MIN_TICK_TIME = 1e-6f;

USimplePhysicsSolver::USimplePhysicsSolver()
{
	GravityAcceleration = 980.f;
	MaxSimulationIterations = 3;
}


void USimplePhysicsSolver::PostInitialize()
{
	Super::PostInitialize();

	if (USimplePhysics_Settings* SimplePhysicsSettings = GetMutableDefault<USimplePhysics_Settings>())
	{
		GravityAcceleration = SimplePhysicsSettings->GravityAcceleration;
		MaxSimulationIterations = SimplePhysicsSettings->MaxSimulationIterations;
	}
}


void USimplePhysicsSolver::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	QUICK_SCOPE_CYCLE_COUNTER(STAT_SimplePhysicsSolver_TickComponent);
	ValidateRigidBodyTick(DeltaTime);
}

void USimplePhysicsSolver::ValidateRigidBodyTick(float DeltaTime)
{
	InvalidRigidBodies.Empty();
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
			//RigidBody->PreviousHitTime = 1.f;
			//RigidBody->bIsSliding = false;

			//if (RigidBody->Velocity == OldVelocity)
			//{
			//	RigidBody->Velocity = RigidBody->ComputeVelocity(RigidBody->Velocity, TimeTick);
			//}
		}
		else
		{
			if (RigidBody->Velocity == OldVelocity)
			{
				RigidBody->Velocity = (Hit.Time > UE_KINDA_SMALL_NUMBER) ? RigidBody->ComputeVelocity(OldVelocity, TimeTick * Hit.Time) : OldVelocity;
			}

			NumImpacts++;
			float SubTickTimeRemaining = TimeTick * (1.f - Hit.Time);
			const EHandleBlockingHitResult HandleBlockingHitResult = RigidBody->HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
			if (HandleBlockingHitResult == EHandleBlockingHitResult::Abort)
			{
				break;
			}
			else if (HandleBlockingHitResult == EHandleBlockingHitResult::Deflect)
			{
				NumBounces++;
				//HandleDeflection()
				RigidBody->PreviousHitTime = Hit.Time;
				RigidBody->PreviousHitNormal = RigidBody->ConstrainNormalToPlane(Hit.Normal);
			}
			else
			{
				checkNoEntry();
			}

			if (SubTickTimeRemaining >= MIN_TICK_TIME)
			{
				RemainingTime += SubTickTimeRemaining;
				if (NumImpacts <= RigidBody->BounceAdditionalIterations)
				{
					Iterations--;
				}
			}
		}

		RigidBody->UpdateComponentVelocity();
	}


}


void USimplePhysicsSolver::SetSimulationEnabled(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, bool Enabled)
{
	if (Enabled)
	{
		SimulatedRigidBodies.AddUnique(RigidBody);
	}
	else
	{
		SimulatedRigidBodies.Remove(RigidBody);
	}
}


//FVector USimplePhysicsSolver::ComputeMoveDelta(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, const FVector& InVelocity, float DeltaTime) const
//{
//	// Velocity Verlet integration (http://en.wikipedia.org/wiki/Verlet_integration#Velocity_Verlet)
//	// The addition of p0 is done outside this method, we are just computing the delta.
//	// p = p0 + v0*t + 1/2*a*t^2
//
//	// We use ComputeVelocity() here to infer the acceleration, to make it easier to apply custom velocities.
//	// p = p0 + v0*t + 1/2*((v1-v0)/t)*t^2
//	// p = p0 + v0*t + 1/2*((v1-v0))*t
//
//	const FVector NewVelocity = ComputeVelocity(RigidBody, InVelocity, DeltaTime);
//	const FVector Delta = (InVelocity * DeltaTime) + (NewVelocity - InVelocity) * (0.5f * DeltaTime);
//	return Delta;
//}


//FVector USimplePhysicsSolver::ComputeVelocity(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, const FVector& InitialVelocity, float DeltaTime) const
//{
//	// v = v0 + a*t
//	const FVector Acceleration = ComputeAcceleration(RigidBody, InitialVelocity, DeltaTime);
//	FVector NewVelocity = InitialVelocity + (Acceleration * DeltaTime);
//
//	return NewVelocity; /* LimitVelocity(NewVelocity); */
//}


//FVector USimplePhysicsSolver::ComputeAcceleration(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, const FVector& InitialVelocity, float DeltaTime) const
//{
//	if (!RigidBody)
//	{
//		return FVector::ZeroVector;
//	}
//
//	FVector Acceleration(FVector::ZeroVector);
//	
//	if (RigidBody->UseGravity)
//	{
//		Acceleration += (GetGravityZ() * RigidBody->GravityScale);
//	}
//
//	if (RigidBody->HasPendingForce())
//	{
//		check(RigidBody->Mass != 0.f)
//		Acceleration += RigidBody->GetPendingForce() / RigidBody->Mass;
//	}
//
//	return Acceleration;
//}

//FVector USimplePhysicsSolver::GetGravityZ() const
//{
//	return FVector(0.f, 0.f, GravityAcceleration);
//}


bool USimplePhysicsSolver::IsTickable() const
{
	return SimulatedRigidBodies.Num() > 0;
}



