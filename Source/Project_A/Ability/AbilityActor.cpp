// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityActor.h"
#include "Ability.h"
#include "../Unit/UnitBase.h"
#include "../Unit/PlayerUnit.h"
#include "../Unit/EnemyUnit.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
AAbilityActor::AAbilityActor()
{
}

void AAbilityActor::BeginPlay()
{
	Super::BeginPlay();
	DurationTimer = Duration;
	Ticker.IntervalTimer = Ticker.Interval;
	MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AAbilityActor::HandleOverlap);
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

void AAbilityActor::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FAbilityEventPayload Payload;
	Payload.Location = bFromSweep ? FVector(SweepResult.ImpactPoint) : GetActorLocation();

	if (AUnitBase* Unit = Cast<AUnitBase>(OtherActor))
	{
		Payload.Target = Unit;
	}
	else if (AAbilityProduct* Product = Cast<AAbilityProduct>(OtherActor))
	{
		Payload.OverlappedProduct = Product;
	}

	// Auto-generate from the owning Ability's AbilityTag unless a bespoke tag was already set
	// (e.g. on the Blueprint's class defaults) - that override is kept for cases that need it.
	if (!OverlapEventTag.IsValid() && MyAbility)
	{
		OverlapEventTag = MyAbility->ComposeEventTag(TEXT("Overlap"));
	}

	ReportAbilityEvent(OverlapEventTag, Payload);
}

TArray<AUnitBase*> AAbilityActor::GetOverlappingUnits(bool bIsHelpful)
{
	TArray<AUnitBase*> Units;

	TArray<AActor*> OverlappingActors;
	MeshComponent->GetOverlappingActors(OverlappingActors, AUnitBase::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		if (bIsHelpful)
		{
			if (APlayerUnit* PlayerUnit = Cast<APlayerUnit>(Actor))
			{
				Units.Add(PlayerUnit);
			}
		}
		else if (AEnemyUnit* EnemyUnit = Cast<AEnemyUnit>(Actor))
		{
			Units.Add(EnemyUnit);
		}
	}

	return Units;
}
