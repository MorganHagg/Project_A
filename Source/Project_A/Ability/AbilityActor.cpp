// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityActor.h"
#include "AbilitySlot.h"
#include "../Unit/UnitBase.h"
#include "../Unit/PlayerUnit.h"
#include "../Unit/EnemyUnit.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
AAbilityActor::AAbilityActor()
{
}

FTransform AAbilityActor::GetSpawnTransform_Implementation()
{
	checkf(false, TEXT("%s must override GetSpawnTransform()"), *GetClass()->GetName());
	return FTransform::Identity;
}

void AAbilityActor::BeginPlay()
{
	Super::BeginPlay();
	Ticker.Start(Duration);
	MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AAbilityActor::HandleOverlap);
	IAbilityLifecycle::Execute_OnActivate(this);
}

void AAbilityActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bool bFinished = false;
	const int32 TicksDue = Ticker.Advance(DeltaTime, bFinished);
	for (int32 TickIndex = 0; TickIndex < TicksDue && !bHasFinished; ++TickIndex)
	{
		IAbilityLifecycle::Execute_OnTick(this);
	}

	// OnTick may already have finished this actor itself.
	if (bFinished && !bHasFinished)
	{
		Finish();
	}
}

void AAbilityActor::Finish()
{
	if (bHasFinished)
	{
		return;
	}

	const FAbilityEventPayload Payload = NotifyFinish();

	IAbilityLifecycle::Execute_OnEnd(this, Payload.Location);

	Destroy();
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
	else if (AAbility* Product = Cast<AAbility>(OtherActor))
	{
		Payload.OverlappedAbility = Product;
	}

	// Auto-generate from this ability's own AbilityTag unless a bespoke tag set was already set
	// (e.g. on the Blueprint's class defaults) - that override is kept for cases that need it.
	if (OverlapEventTags.IsEmpty())
	{
		OverlapEventTags = ComposeEventTags(TEXT("Overlap"));
	}

	ReportAbilityEvent(OverlapEventTags, Payload);

	OnOverlap(Payload.Target, Payload.OverlappedAbility, Payload.Location);
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
