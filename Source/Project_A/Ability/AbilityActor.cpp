// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityActor.h"
#include "Ability.h"


// Sets default values
AAbilityActor::AAbilityActor()
{
	PrimaryActorTick.bCanEverTick = true;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	// No mass / no physics - query-only collision, overlap rather than block.
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	MeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	MeshComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	MeshComponent->SetGenerateOverlapEvents(true);
}

void AAbilityActor::BeginPlay()
{
	Super::BeginPlay();
	DurationTimer = Duration;
	Ticker.IntervalTimer = Ticker.Interval;
	IAbilityLifecycle::Execute_OnActivate(this);
}

void AAbilityActor::SetMyAbility(UAbility* Ability)
{
	MyAbility = Ability;
}

void AAbilityActor::SetMyCaster(ACharacter* Caster)
{
	MyCaster = Caster;
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
			EndAbility();
			return;
		}
	}

	if (Ticker.ShouldTick(DeltaTime))
	{
		IAbilityLifecycle::Execute_OnTick(this);
	}
}

void AAbilityActor::EndAbility()
{
	if (bHasEnded) return;
	bHasEnded = true;

	IAbilityLifecycle::Execute_OnEnd(this);
	Destroy();
}

void AAbilityActor::OnHit(AUnitBase* HitUnit)
{
	if (MyAbility)
	{
		MyAbility->DelegateOnHit(HitUnit);
	}
}

void AAbilityActor::OnOverlap(AActor* HitActor)
{
	if (MyAbility)
	{
		MyAbility->DelegateOnOverlap(HitActor);
	}
}