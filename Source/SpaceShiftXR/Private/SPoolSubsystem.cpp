// Fill out your copyright notice in the Description page of Project Settings.


#include "SPoolSubsystem.h"

#include "SPoolable.h"



void USPoolSubsystem::CreatePool(TSubclassOf<AActor> PoolClass, int32 StartingSize, const FVector& Location, const FRotator& Rotation)
{
	if (PoolClass.Get()->ImplementsInterface(USPoolable::StaticClass()))
	{
		// Create pool if it does not exist
		FPoolArray* ObjectPool = ObjectPools.Find(PoolClass);
		if (!ObjectPool)
		{
			ObjectPools.Add(PoolClass);
			ObjectPool = ObjectPools.Find(PoolClass);
		}

		// Pool should exist here. Spawn actors and add to pool until size equals StartingSize
		if (ObjectPool)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			while (ObjectPool->Size() < StartingSize)
			{
				AActor* Actor = GetWorld()->SpawnActor<AActor>(PoolClass, Location, Rotation, SpawnParams);
				ObjectPool->Push(Actor);
			}
		}
	}
}


int32 USPoolSubsystem::GetPoolSize(TSubclassOf<AActor> PoolClass) const
{
	if (PoolClass.Get()->ImplementsInterface(USPoolable::StaticClass()))
	{
		if (const FPoolArray* ObjectPool = ObjectPools.Find(PoolClass))
		{
			return ObjectPool->Size();
		}
	}

	return 0;
}


void USPoolSubsystem::SpawnFromPool(TSubclassOf<AActor> PoolClass, const FVector& Location, const FRotator& Rotation, AActor*& SpawnedActor)
{
	SpawnedActor = SpawnFromPool<AActor>(PoolClass, Location, Rotation);
}


bool USPoolSubsystem::GetFromPool(TSubclassOf<AActor> PoolClass, AActor*& SpawnedActor)
{
	SpawnedActor = GetFromPool<AActor>(PoolClass);
	return SpawnedActor != nullptr;
}


void USPoolSubsystem::ReturnToPool(AActor* Poolable)
{
	if (!Poolable)
	{
		return;
	}

	const UClass* PoolableClass = Poolable->GetClass();

	if (PoolableClass && PoolableClass->ImplementsInterface(USPoolable::StaticClass()))
	{
		ISPoolable::Execute_OnReturnToPool(Poolable);
		FPoolArray* ObjectPool = ObjectPools.Find(PoolableClass);
		ObjectPool->Push(Poolable);
	}
	else
	{
		Poolable->Destroy();
	}
}
