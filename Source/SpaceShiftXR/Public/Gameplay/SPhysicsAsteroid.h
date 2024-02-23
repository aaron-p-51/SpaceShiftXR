// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPhysicsAsteroid.generated.h"

class USphereComponent;

UCLASS()
class SPACESHIFTXR_API ASPhysicsAsteroid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPhysicsAsteroid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LastVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Acceleration;


	bool HasHit;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// UFUNCTION()
	// void OnSphereCompHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
