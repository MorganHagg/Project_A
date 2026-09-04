// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityActor.h"
#include "Ability.h"


// Sets default values
AAbilityActor::AAbilityActor()
{
}

void AAbilityActor::BeginPlay()
{
	Super::BeginPlay();
	DurationTimer = Duration;
	Ticker.IntervalTimer = Ticker.Interval;
	IAbilityLifecycle::Execute_OnActivate(this);
}

void AAbilityActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Duration != 0.f)
	{
		DurationTimer -= DeltaTime;

		if (DurationTimer <= 0.f)
		{
			// Final tick
			IAbilityLifecycle::Execute_OnTick(this);
			Finish();
			return;
		}
	}

	if (Ticker.ShouldTick(DeltaTime))
	{
		IAbilityLifecycle::Execute_OnTick(this);
	}
}

FVector AAbilityActor::Finish()
{
	// OnEnd must only ever fire once - Super::Finish()'s guard only protects its own body,
	// not this one, so check bHasFinished (inherited, protected) here first.
	if (!bHasFinished)
	{
		IAbilityLifecycle::Execute_OnEnd(this);
	}
	return Super::Finish();
}
