// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SPoolable.h"
#include "SPoolSubsystem.generated.h"


/**
 * Wrapper for pooled objects
 */
USTRUCT()
struct FPoolArray
{
	GENERATED_BODY()

	/** Object pool for individual object type*/
	UPROPERTY()
	TArray<AActor*> ObjectPool;

	/** Check if the object pool is empty */
	FORCEINLINE bool IsEmpty() const { return ObjectPool.IsEmpty(); }

	/** Remove return and object from the pool */
	FORCEINLINE AActor* Pop(bool bAllowShrinking = false) { return ObjectPool.Pop(bAllowShrinking); }

	/** Add an object back to the pool */
	FORCEINLINE void Push(AActor* Actor) { ObjectPool.Push(Actor); }

	/** Return the number of objects in the pool */
	FORCEINLINE int32 Size() const { return ObjectPool.Num(); }
};


/**
 * 
 */
UCLASS()
class SPACESHIFTXR_API USPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	/**
	 * Create an object pool with a preset size. A pool is exists for the pooled object and has less than preset size the pool with create objects to reach preset size.
	 * If the pool already exists and is larger than StartingSize the pool will not shrink.
	 * @param	PoolClass			Class to of objects to pool
	 * @param	StartingSize		Preset pool size
	 * @param	Location			Location to spawn new pooled objects
	 * @param	Rotation			Rotation to spawn new pooled objects
	 */
	UFUNCTION(BlueprintCallable, Category = "Pool Subsystem")
	void CreatePool(TSubclassOf<AActor> PoolClass, int32 StartingSize, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator);

	/** Get the current size of object pool */
	UFUNCTION(BlueprintCallable, Category = "Pool Subsystem")
	int32 GetPoolSize(TSubclassOf<AActor> PoolClass) const;

	/**
	 * Spawn from object pool. If the pool is empty then a new object will be created. Will call ISPoolable::OnSpawnFromPool() for the returned actor
	 * @param	PoolClass			Class of the pooled object
	 * @param	Location			Location for the object
	 * @param	Rotation			Rotation for the object
	 * @param	SpawnedActor		Out param reference for actor returned from the pool
	 */
	UFUNCTION(BlueprintCallable, Category = "Pool Subsystem", meta = (DetermineOutputType = "PoolClass", DynamicOutputParam = "SpawnedActor"))
	void SpawnFromPool(TSubclassOf<AActor> PoolClass, const FVector& Location, const FRotator& Rotation, AActor*& SpawnedActor);

	/**
	 * Spawn from object pool. If the pool is empty then a new object will be created. Will call ISPoolable::OnSpawnFromPool() for the returned actor
	 * @param	PoolClass			Class of the pooled object
	 * @param	Location			Location for the object
	 * @param	Rotation			Rotation for the object
	 *
	 * @return object from pool / newly spanwed object if pool was empty
	 */
	template<typename T>
	T* SpawnFromPool(TSubclassOf<AActor> PoolClass, const FVector& Location, const FRotator& Rotation);

	/**
	 * Get an object from object pool if pool for the PoolClass actor exists. Will not call ISPoolable::OnSpawnFromPool() for the returned actor
	 * @param	PoolClass			Class of the pooled object
	 * @param	SpawnedActor		Out param reference for the actor returned from pool will be null of pool does not exist
	 * 
	 * @return true it pool exists and successfully set SpawnedActor 
	 */
	UFUNCTION(BlueprintCallable, Category = "Pool Subsystem", meta = (DetermineOutputType = "PoolClass", DynamicOutputParam = "SpawnedActor"))
	bool GetFromPool(TSubclassOf<AActor> PoolClass, AActor*& SpawnedActor);

	/**
	 * Get an object from object pool if pool for the PoolClass actor exists. Will not call ISPoolable::OnSpawnFromPool() for the returned actor
	 * @param	PoolClass			Class of the pooled object
	 * 
	 * @return object from pool will return nullptr if pool does not exist
	 */
	template<typename T>
	T* GetFromPool(TSubclassOf<AActor> PoolClass);

	/** Return actor to its object pool */
	UFUNCTION(BlueprintCallable, Category = "Pool Subsystem")
	void ReturnToPool(AActor* Poolable);

private:
	
	/** Object pool for each pooled object */
	UPROPERTY()
	TMap<UClass*, FPoolArray> ObjectPools;
};


template<typename T>
T* USPoolSubsystem::SpawnFromPool(TSubclassOf<AActor> PoolClass, const FVector& Location, const FRotator& Rotation)
{
	T* PooledActor = nullptr;

	if (PoolClass.Get()->ImplementsInterface(USPoolable::StaticClass()))
	{
		FPoolArray& ObjectPool = ObjectPools.FindOrAdd(PoolClass);

		// If pool is emplty or just created then spawn new object. Spawned objects will be added back to pool assuming they call ReturnToPool
		// when no longer in use
		if (ObjectPool.IsEmpty())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			PooledActor = GetWorld()->SpawnActor<T>(PoolClass, Location, Rotation, SpawnParams);
		}
		else
		{
			PooledActor = CastChecked<T>(ObjectPool.Pop());
			PooledActor->SetActorLocationAndRotation(Location, Rotation);
		}

		ISPoolable::Execute_OnSpawnFromPool(PooledActor);
	}
	return PooledActor;
}


template<typename T>
T* USPoolSubsystem::GetFromPool(TSubclassOf<AActor> PoolClass)
{
	T* PooledActor = nullptr;

	if (PoolClass.Get()->ImplementsInterface(USPoolable::StaticClass()))
	{
		if (FPoolArray* ObjectPool = ObjectPools.Find(PoolClass))
		{
			PooledActor = CastChecked<T>(ObjectPool->Pop());
		}
	}

	return PooledActor;
}
