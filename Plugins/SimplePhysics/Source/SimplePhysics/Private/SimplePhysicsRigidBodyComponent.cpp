// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplePhysicsRigidBodyComponent.h"

#include "SimplePhysics_Settings.h"
#include "SimplePhysicsSolver.h"
#include "Components/SphereComponent.h"

USimplePhysicsRigidBodyComponent::USimplePhysicsRigidBodyComponent()
{
	bUseGravity = false;
	bEnableSimulationOnRigidBodyCollision = true;
	Mass = 100.f;
	Friction = 0.2f;
	MinFrictionFraction = 0.f;
	Bounciness = 0.6f;
	LinearDamping = 0.f;
	MaxSpeed = 1000.f;
	GravityScale = 1.f;
	MomentOfInertia = 1.f;
	CalculatedMomentOfInertia = 0.1f;


	/*PreviousHitTime = 1.f;
	PreviousHitNormal = FVector::UpVector;*/
	//bBounceAngleAffectsFriction = false;
	LastHitResult.Init();

	PendingTorque = FVector::ZeroVector;
}


float USimplePhysicsRigidBodyComponent::GetScaledSphereRadius() const
{
	if (OwnerSphereComponent)
	{
		return OwnerSphereComponent->GetScaledSphereRadius();
	}

	UE_LOG(LogTemp, Warning, TEXT("I am here !!!!!"));
	return 0.f;
}


float USimplePhysicsRigidBodyComponent::CalculateMomentOfInertia() const
{
	// http://hyperphysics.phy-astr.gsu.edu/hbase/isph.html
	//UE_LOG(LogTemp, Warning, TEXT("CalculateMomentOfInertia"));
	return 0.4f * Mass * FMath::Square(GetScaledSphereRadius());
}


void USimplePhysicsRigidBodyComponent::SetMass(float NewMass)
{
	Mass = NewMass;
	//CalculatedMomentOfInertia = CalculateMomentOfInertia();
}


float USimplePhysicsRigidBodyComponent::GetMomentOfInertia() const
{
	return (MomentOfInertia <= 0.f) ? CalculateMomentOfInertia() : MomentOfInertia;
}


void USimplePhysicsRigidBodyComponent::ApplyRotationDelta(const FRotator& RotationDelta)
{
	if (RotationDelta != FRotator::ZeroRotator)
	{
		if (AActor* Owner = GetOwner())
		{
			Owner->AddActorWorldRotation(RotationDelta);
		}
	}
}

void USimplePhysicsRigidBodyComponent::AddAngularVelocity(const FVector& AngularVelocityToAdd)
{
	SetAngularVelocity(AngularVelocity + AngularVelocityToAdd);
}

void USimplePhysicsRigidBodyComponent::AddVelocity(const FVector& VelocityToAdd)
{
	SetVelocity(Velocity + VelocityToAdd);
}


void USimplePhysicsRigidBodyComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (USimplePhysics_Settings* SimplePhysicsSettings = GetMutableDefault<USimplePhysics_Settings>())
	{
		GravityAcceleration = SimplePhysicsSettings->GravityAcceleration;
	}

	if (IsValid(UpdatedComponent))
	{
		OwnerSphereComponent = Cast<USphereComponent>(UpdatedComponent);
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
	if (!IsValid(UpdatedComponent))
	{
		return;
	}

	if (auto Subsystem = GetSimplePhysicsSolver())
	{
		Subsystem->SetSimulationEnabled(this, Enabled);
		//bSimulationEnabled = Enabled;
	}
}


void USimplePhysicsRigidBodyComponent::AddForce(const FVector& Force)
{
	PendingForce += Force;
}


void USimplePhysicsRigidBodyComponent::AddTorque(const FVector& Torque)
{
	PendingTorque += Torque;
}


FVector USimplePhysicsRigidBodyComponent::ComputeAngularVelocityDelta(const FVector& InAngularVelocity, float DeltaTime) const
{
	const FVector NewAngularVelocity = ComputeAngularVelocity(InAngularVelocity, DeltaTime);
	const FVector Delta = (InAngularVelocity * DeltaTime) + (NewAngularVelocity - InAngularVelocity) * (0.5f * DeltaTime);

	return Delta;//FRotator(Delta.Y, Delta.Z, Delta.X);
}


FVector USimplePhysicsRigidBodyComponent::ComputeAngularVelocity(const FVector& InitialAngularVelocity, float DeltaTime) const
{
	const FVector AngularAcceleration = ComputeAngularAcceleration(InitialAngularVelocity, DeltaTime);
	const FVector NewAngularVelocity = InitialAngularVelocity + (AngularAcceleration * DeltaTime);

	return LimitAngularVelocity(NewAngularVelocity);
}


FVector USimplePhysicsRigidBodyComponent::ComputeAngularAcceleration(const FVector& InitialAngularVelocity, float DeltaTime) const
{
	const float CurrentMomentOfInertia = GetMomentOfInertia();
	check(CurrentMomentOfInertia > 0.f);

	FVector Torque = GetAngularDragTorque(InitialAngularVelocity);
	Torque += GetPendingTorque();

	const FVector AngularAcceleration = Torque / CurrentMomentOfInertia;
	return AngularAcceleration;
}


FVector USimplePhysicsRigidBodyComponent::GetAngularDragTorque(const FVector& InAngularVelocity) const
{
	return -AngularDamping * InAngularVelocity;
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

FVector USimplePhysicsRigidBodyComponent::ComputeAcceleration(const FVector& InitialVelocity, float DeltaTime) const
{
	check(Mass > 0.f);

	FVector Acceleration(FVector::ZeroVector);

	if (bUseGravity)
	{
		Acceleration.Z -= GetGravityZ();
	}

	FVector Force = GetLinearDragForce(InitialVelocity);
	Force += GetPendingForce();

	Acceleration += (Force / Mass);

	return Acceleration;
}


void USimplePhysicsRigidBodyComponent::SetVelocity(const FVector& NewVelocity, bool UpdateVelocity)
{
	Velocity = LimitVelocity(NewVelocity);

	if (UpdateVelocity)
	{
		UpdateComponentVelocity();
	}
}


void USimplePhysicsRigidBodyComponent::SetAngularVelocity(const FVector& NewAngularVelocity)
{
	AngularVelocity = LimitAngularVelocity(NewAngularVelocity);
}


void USimplePhysicsRigidBodyComponent::SetMovementData(const FMovementData& MovementData)
{
	SetVelocity(MovementData.LinearVelocity);
	SetAngularVelocity(MovementData.AngularVelocity);

	//UE_LOG(LogTemp, Warning, TEXT("AngularVelocity: %s"), *AngularVelocity.ToString());
}


float USimplePhysicsRigidBodyComponent::GetRestitutionCoefficient(TObjectPtr<USimplePhysicsRigidBodyComponent> OtherRidigBody) const
{
	const float OtherRigidBodyBounciness = OtherRidigBody->Bounciness;

	float RestitutionCoefficient = Bounciness;

	switch (BounceCombine)
	{
	case EBounceCombine::Maximum:
		RestitutionCoefficient = FMath::Max(Bounciness, OtherRigidBodyBounciness);
		break;

	case EBounceCombine::Minimum:
		RestitutionCoefficient = FMath::Min(Bounciness, OtherRigidBodyBounciness);
		break;

	case EBounceCombine::Average:
		RestitutionCoefficient = (Bounciness + OtherRigidBodyBounciness) / 2.f;
		break;

		// No need for EBounceCombine::Average to set RigidBodyBouceFactor to current value

	default:
		break;
	}

	return FMath::Clamp(RestitutionCoefficient, 0.f, 1.f);
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


FVector USimplePhysicsRigidBodyComponent::LimitAngularVelocity(FVector NewAngularVelocity) const
{
	const float CurrentMaxAngularVelocity = GetMaxAngularVelocity();
	if (CurrentMaxAngularVelocity > 0.f)
	{
		NewAngularVelocity = NewAngularVelocity.GetClampedToMaxSize(CurrentMaxAngularVelocity);
	}

	// TODO: Contrain rotation to plane
	return NewAngularVelocity;
}


void USimplePhysicsRigidBodyComponent::UpdateMovementVelocity(const FVector& OldVelocity, float DeltaTime)
{
	LastHitResult.Time = 1.f;
	//bIsSliding = false;

	if (Velocity == OldVelocity)
	{
		Velocity = ComputeVelocity(Velocity, DeltaTime);
	}
}


void USimplePhysicsRigidBodyComponent::UpdateMovementVelocity(const FVector& OldVelocity, const FHitResult& Hit, float DeltaTime)
{
	if (!Hit.bBlockingHit)
	{
		UpdateMovementVelocity(OldVelocity, DeltaTime);
		return;
	}

	if (Velocity == OldVelocity)
	{
		const FVector NewVelocity = ComputeVelocity(OldVelocity, DeltaTime * Hit.Time);
		Velocity = (Hit.Time > UE_KINDA_SMALL_NUMBER) ? NewVelocity : OldVelocity;
	}
}


void USimplePhysicsRigidBodyComponent::UpdateMovementAngularVelocity(const FVector& OldAngularVelocity, float DeltaTime)
{
	if (AngularVelocity == OldAngularVelocity)
	{
		AngularVelocity = ComputeAngularVelocity(AngularVelocity, DeltaTime);
	}
}


void USimplePhysicsRigidBodyComponent::UpdateMovementAngularVelocity(const FVector& OldAngularVelocity, const FHitResult& Hit, float DeltaTime)
{
	if (!Hit.bBlockingHit)
	{
		UpdateMovementAngularVelocity(OldAngularVelocity, DeltaTime);
		return;
	}

	if (AngularVelocity == OldAngularVelocity)
	{
		const FVector NewAngularVelocity = ComputeAngularVelocity(OldAngularVelocity, DeltaTime * Hit.Time);
		AngularVelocity = (Hit.Time > UE_KINDA_SMALL_NUMBER) ? NewAngularVelocity : OldAngularVelocity;	
	}
}


void USimplePhysicsRigidBodyComponent::StopAllMovementImmediately()
{
	SetVelocity(FVector::ZeroVector);
}


bool USimplePhysicsRigidBodyComponent::HasStoppedSimulation() const
{
	if ((UpdatedComponent == nullptr) || !IsActive())
	{
		return false;
	}

	if (auto Subsystem = GetSimplePhysicsSolver())
	{
		return Subsystem->IsSimulating(this);
	}

	return false;
}


float USimplePhysicsRigidBodyComponent::GetGravityZ() const
{
	return bUseGravity ? GravityAcceleration * GravityScale : 0.f;
}


void USimplePhysicsRigidBodyComponent::SetLastBlockingHitResult(const FHitResult& Hit)
{
	LastHitResult = Hit;
}


void USimplePhysicsRigidBodyComponent::ClearLastBlockingHitResult()
{
	LastHitResult.Init();
}
