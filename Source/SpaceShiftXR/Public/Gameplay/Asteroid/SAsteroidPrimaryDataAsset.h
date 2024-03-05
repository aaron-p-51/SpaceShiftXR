// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SAsteroidPrimaryDataAsset.generated.h"

class UStaticMesh;

/**
 * 
 */
UCLASS()
class SPACESHIFTXR_API USAsteroidPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** Static Mesh of asteroid */
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMesh> Mesh;

	/** Local offset for Mesh component in asteroid actor */
	UPROPERTY(EditAnywhere)
	FTransform MeshLocalTransform;

	/** Collision radius of sphere collision component in asteroid actor */
	UPROPERTY(EditAnywhere)
	float CollisionRadius;

	/** Deflaut Mass of the asteroid. Will scale acording to MinScale and MaxScale */
	UPROPERTY(EditAnywhere)
	float DefaultMass;

	/** Minimim world scale of the asteroid actor */
	UPROPERTY(EditAnywhere)
	float MinScale;

	/** Maximim world scale of the asteroid actor*/
	UPROPERTY(EditAnywhere)
	float MaxScale;

	/** Heath of acteroid, used to set amount of damage it can take before being destroyed*/
	UPROPERTY(EditAnywhere)
	float Health;

	/** Number of fragments the asteroid actor will break into */
	UPROPERTY(EditAnywhere)
	int32 FragmentCount;

	/** Minimim impulse magnitude of the fragments when breaking away from asteroid */
	UPROPERTY(EditAnywhere)
	float MinFragmentImpulseMagnitude;

	/** Maximum impulse magnitude of the fragments when breaking away from asteroid */
	UPROPERTY(EditAnywhere)
	float MaxFragmentImpulseMagnitude;

	/** Data assets for fragment asteroids */
	UPROPERTY(EditAnywhere)
	TObjectPtr<USAsteroidPrimaryDataAsset> AsteroidFragments;
	
};
