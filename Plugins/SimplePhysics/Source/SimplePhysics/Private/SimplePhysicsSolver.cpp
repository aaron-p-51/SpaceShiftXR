// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplePhysicsSolver.h"

#include "SimplePhysics.h"
#include "SimplePhysics_Settings.h"
#include "SimplePhysicsRigidBodyComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"


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


bool USimplePhysicsSolver::IsSimulating(const USimplePhysicsRigidBodyComponent* RigidBody) const
{
	return SimulatedRigidBodies.Contains(RigidBody) || AddRigidBodies.Contains(RigidBody);
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

	// Ensure SimulatedRigidBodies is current before applying movement logic. 
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

	//UE_LOG(LogTemp, Warning, TEXT("Simulating Rigid Bodies:%d"), SimulatedRigidBodies.Num());

	// Process movement for all SimulatedRigidBodies. If during the movement process the RigidBody
	// becomes invalid add it to the InvalidRigidBodies list. These Rigidbodies will then be removed at the
	// start of the next frame when RegisterRigidBodies() is called
	for (auto RigidBody : SimulatedRigidBodies)
	{
		if (!RigidBody || !IsValid(RigidBody->UpdatedComponent))
		{
			InvalidRigidBodies.Add(RigidBody);
			continue;
		}

		if (CanSimulateRigidBodyMovement(RigidBody, DeltaTime))
		{
			ApplyRigidBodyMovement(RigidBody, DeltaTime);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Simulating Rigid Bodies:%d"), SimulatedRigidBodies.Num());
			InvalidRigidBodies.Add(RigidBody);
		}
	}
}


bool USimplePhysicsSolver::CanSimulateRigidBodyMovement(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, float DeltaTime) const
{
	return RigidBody && IsValid(RigidBody->UpdatedComponent) &&
		   !RigidBody->ShouldSkipUpdate(DeltaTime) &&
		   !RigidBody->UpdatedComponent->IsSimulatingPhysics();
}


void USimplePhysicsSolver::ApplyRigidBodyMovement(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, float DeltaTime)
{
	float RemainingTime = DeltaTime;
	//int32 NumImpacts = 0;
	int32 NumBounces = 0;
	int32 LoopCount = 0;
	int32 Iterations = 0;
	FHitResult Hit(1.f);
	
	//RigidBody->AngularVelocity -= RigidBody->AngularDamping * RigidBody->AngularVelocity * DeltaTime;
	FRotator DeltaRotation = RigidBody->AngularVelocity * DeltaTime;
	if (AActor* Owner = RigidBody->GetOwner())
	{
		Owner->AddActorWorldRotation(DeltaRotation);
	}

	//if (auto Sphere = Cast<USphereComponent>(RigidBody->UpdatedComponent))
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("I have a sphere"));
	//}


	while (RemainingTime >= MIN_TICK_TIME && (Iterations < MaxSimulationIterations) && RigidBody->UpdatedComponent && RigidBody->IsActive())
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

			//NumImpacts++;
			float SubTickTimeRemaining = TimeTick * (1.f - Hit.Time);

			if (ShouldAbort(RigidBody, Hit))
			{
				break;
			}

			if (USimplePhysicsRigidBodyComponent* OtherHitRigidBody = GetOtherHitRigidBody(Hit))
			{
				HandleRigidBodyCollision(RigidBody, OtherHitRigidBody, Hit);
			}
			else
			{
				HandleImpact(RigidBody, Hit, TimeTick, MoveDelta);
			}

			if (ShouldAbort(RigidBody, Hit))
			{
				break;
			}

			// Add Handle Deflection similar the projectile movement here if needed

			if (IsBelowSimulationVelocity(RigidBody))
			{
				UE_LOG(LogTemp, Warning, TEXT("IsBelowSimulationVelocity"));
				StopSimulating(RigidBody);
				break;
			}

			RigidBody->SetLastBlockingHitResult(Hit);
			RigidBody->LimitVelocityFromCurrent();


			if (SubTickTimeRemaining >= MIN_TICK_TIME)
			{
				RemainingTime += SubTickTimeRemaining;
			}
		}

		RigidBody->UpdateComponentVelocity();
	}
}


void USimplePhysicsSolver::HandleImpact(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	const FVector OldVelocity = RigidBody->Velocity;
	FVector NewVelocity = ComputeBounceResult(RigidBody, Hit, TimeSlice, MoveDelta);

	RigidBody->OnRigidBodyBounceDelegate.Broadcast(Hit, OldVelocity, NewVelocity);

	RigidBody->SetVelocity(NewVelocity);
}


FVector USimplePhysicsSolver::ComputeBounceResult(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	FVector TempVelocity = RigidBody->Velocity;
	FVector VelocityNormal = TempVelocity.GetSafeNormal();
	const FVector Normal = RigidBody->ConstrainNormalToPlane(Hit.Normal);
	const float VelocityDotNormal = FVector::DotProduct(TempVelocity, Normal);

	// Only if velocity is opposed by normal or parellel
	if (VelocityDotNormal <= 0.f)
	{
		// Project velocity onto normal in reflected direction
		const FVector ProjectedNormal = Normal * -VelocityDotNormal;

		// Point velocity in direction parallel to surface
		TempVelocity += ProjectedNormal;

		const bool SouldScaleFriction = /* RigidBody->bIsSliding || */ RigidBody->bBounceAngleAffectsFriction;
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

		// Compute rotation
		FVector LevelArm = Hit.ImpactPoint - RigidBody->UpdatedComponent->GetComponentLocation();
		FVector AxisOfRotation = FVector::CrossProduct(  VelocityNormal, Hit.Normal).GetSafeNormal() * RigidBody->TempScale;
		UE_LOG(LogTemp, Warning, TEXT("Axis:%s"), *AxisOfRotation.ToString());

		FQuat RotationDelta = FQuat(AxisOfRotation, RigidBody->TempScale);
		RigidBody->AngularVelocity = RotationDelta.Rotator();
		//FRotator NewAngularVelocity;
		//NewAngularVelocity.Roll = AxisOfRotation.X;
		//NewAngularVelocity.Pitch = AxisOfRotation.Y;
		//NewAngularVelocity.Yaw = AxisOfRotation.Z;

		//RigidBody->AngularVelocity = NewAngularVelocity;
		
	}

	return TempVelocity;
}


bool USimplePhysicsSolver::IsBelowSimulationVelocity(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody) const
{
	return RigidBody->Velocity.SizeSquared() < FMath::Square(MinimumSimulationVelocity);
}


void USimplePhysicsSolver::StopSimulating(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody)
{
	if (!RigidBody->HasPendingForce())
	{
		InvalidRigidBodies.Add(RigidBody);
		RigidBody->SetVelocity(FVector::ZeroVector);
		RigidBody->OnSimulationStopDelegate.Broadcast();
	}
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


void USimplePhysicsSolver::HandleRigidBodyCollision(TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody, TObjectPtr<USimplePhysicsRigidBodyComponent> OtherRigidBody, const FHitResult& Hit)
{
	if (RigidCollisionResultMap.Contains(RigidBody))
	{
		RigidBody->SetVelocity(RigidCollisionResultMap[RigidBody]);
	}
	else
	{
		const bool OtherRigidBodySimulating = SimulatedRigidBodies.Contains(OtherRigidBody);
		if (OtherRigidBodySimulating || OtherRigidBody->bEnableSimulationOnRigidBodyCollision)
		{
			FVector V1Final, V2Final;
			FRotator AV1Final, AV2Final;
			if (ComputeRigidBodyCollision(Hit, RigidBody, OtherRigidBody, V1Final, V2Final, AV1Final, AV2Final))
			{
				RigidCollisionResultMap.Emplace(RigidBody, V1Final);
				RigidCollisionResultMap.Emplace(OtherRigidBody, V2Final);

				RigidBody->SetVelocity(V1Final);
				OtherRigidBody->SetVelocity(V2Final);

				RigidBody->AngularVelocity = AV1Final;
				OtherRigidBody->AngularVelocity = AV2Final;


				if (!OtherRigidBodySimulating)
				{
					OtherRigidBody->SetSimulationEnabled(true);
				}
			}
		}
		else
		{
			HandleImpact(RigidBody, Hit, 0.f, FVector());
		}
	}
}


bool USimplePhysicsSolver::ComputeRigidBodyCollision(const FHitResult& Hit, TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody1, TObjectPtr<USimplePhysicsRigidBodyComponent> RigidBody2, FVector& Velocity1, FVector& Velocity2, FRotator& AngularVelocity1, FRotator& AngularVelocity2) const
{
	DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 10, 26, FColor(181, 0, 0), true, -1, 0, 2);

	if (!RigidBody1 || !RigidBody2)
	{
		return false;
	}

	const float Mass1 = RigidBody1->Mass;
	const float Mass2 = RigidBody2->Mass;

	check(Mass1 > 0.f && Mass2 > 0.f);

	Velocity1 = RigidBody1->Velocity;
	Velocity2 = RigidBody2->Velocity;

	// Calculate relative velocity
	const FVector RelativeVelocity = Velocity2 - Velocity1;

	// Calculate collision normal
	const FVector CollisionNormal = (RigidBody2->UpdatedComponent->GetComponentLocation() - RigidBody1->UpdatedComponent->GetComponentLocation()).GetSafeNormal();

	// Calculate impulse along the normal
	float Impulse = (2.0f * Mass1 * Mass2) / (Mass1 + Mass2) * FVector::DotProduct(RelativeVelocity, CollisionNormal);

	// Update velocities
	const float V1RestitutionCoefficient = RigidBody1->GetRestitutionCoefficient(RigidBody2);//		GetRestitutionCoefficient(RigidBody1, RigidBody2);
	const float V2RestitutionCoefficient = RigidBody2->GetRestitutionCoefficient(RigidBody1);
	Velocity1 += (V1RestitutionCoefficient * Impulse / Mass1) * CollisionNormal;
	Velocity2 -= (V2RestitutionCoefficient * Impulse / Mass2) * CollisionNormal;

	USphereComponent* RigidBody1SphereComponent = Cast<USphereComponent>(RigidBody1->UpdatedComponent);
	USphereComponent* RigidBody2SphereComponent = Cast<USphereComponent>(RigidBody2->UpdatedComponent);

	if (!RigidBody1SphereComponent || !RigidBody2SphereComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Faile"));
		return false;
	}

	const FVector LeverArm1 = Hit.ImpactPoint - RigidBody1SphereComponent->GetComponentLocation();
	const FVector LeverArm2 = Hit.ImpactPoint - RigidBody2SphereComponent->GetComponentLocation();

	FVector AngularDirection1 = FVector::CrossProduct(LeverArm1, CollisionNormal).GetSafeNormal();
	FVector AngularDirection2 = FVector::CrossProduct(LeverArm2, CollisionNormal).GetSafeNormal();

	float AngularImpulseMagnitude1 = FVector::DotProduct(LeverArm1, Impulse * CollisionNormal) / RigidBody1->MomentOfInertia;
	float AngularImpulseMagnitude2 = FVector::DotProduct(LeverArm2, Impulse * CollisionNormal);

	

	FVector AV1 = FVector(RigidBody1->AngularVelocity.Roll, RigidBody1->AngularVelocity.Pitch, RigidBody1->AngularVelocity.Yaw);
	AV1 += (AngularImpulseMagnitude1 / Mass1) * AngularDirection1;










	/*const FVector Radius1 = RigidBody1SphereComponent->GetScaledSphereRadius() * CollisionNormal;
	const FVector Radius2 = RigidBody2SphereComponent->GetScaledSphereRadius() * CollisionNormal;*/

	//const FVector AngularImpulse = FVector::CrossProduct(CollisionNormal, Impulse * CollisionNormal);

	//UE_LOG(LogTemp, Warning, TEXT("AngularImpulse:%s"), *AngularImpulse.ToString());

	//FVector AV1 = FVector(RigidBody1->AngularVelocity.Roll, RigidBody1->AngularVelocity.Pitch, RigidBody1->AngularVelocity.Yaw);

	////AngularVelocity1 = (RigidBody1->AngularVelocity.Vector() + (AngularImpulse / RigidBody1->MomentOfInertia)).Rotation();
	//AV1 += AngularImpulse / RigidBody1->MomentOfInertia;
	AngularVelocity1.Roll = AV1.X;
	AngularVelocity1.Pitch = AV1.Y;
	AngularVelocity1.Yaw = AV1.Z;


	//AngularVelocity2 = (RigidBody2->AngularVelocity.Vector() - (AngularImpulse / RigidBody2->MomentOfInertia)).Rotation();

	//UE_LOG(LogTemp, Warning, TEXT("AV1:%s, AV2:%s"), *AngularVelocity1.ToString(), *AngularVelocity2.ToString());
	
	return true;
}
