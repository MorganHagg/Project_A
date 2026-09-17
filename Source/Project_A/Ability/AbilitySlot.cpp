#include "AbilitySlot.h"
#include "../Unit/UnitBase.h"
#include "../Unit/PlayerUnit.h"
#include "Ability.h"
#include "Projectile.h"
#include "AbilityActor.h"

UAbilitySlot::UAbilitySlot()
{

}

float UAbilitySlot::GetCoolDown()
{
	return CoolDown;
}

float UAbilitySlot::GetCost()
{
	return Cost;
}

float UAbilitySlot::GetMagnitude()
{
	return MagnitudeMultiplier;
}

void UAbilitySlot::SetupAbility(AUnitBase* NewCaster)
{
	MyCaster = NewCaster;
	World = MyCaster->GetWorld();

	// Checks
	check(MyCaster);
	check(World);
}

void UAbilitySlot::ActivateAbility()
{
	// cooldown/cost gating: deferred, out of scope for now - always proceeds when told to.
	bHasEnded = false;

	if (!ProductClass)
	{
		return;
	}

	AAbility* Spawned = World->SpawnActorDeferred<AAbility>(ProductClass, FTransform::Identity);
	if (!Spawned)
	{
		return;
	}

	Spawned->SetMyAbility(this);
	Spawned->SetMyCaster(MyCaster);

	// Queried on the still-deferred instance - MyCaster is already set, AbilityTag isn't yet
	// (composed in BeginPlay, below, since only OnActivate/Cast reporting needs it). GetSpawnTransform
	// is declared separately on AProjectile/AAbilityActor rather than once on AAbility - see the
	// note in Ability.h - so dispatch by concrete type here instead of a single virtual call.
	FTransform SpawnTransform = FTransform::Identity;
	if (AProjectile* Projectile = Cast<AProjectile>(Spawned))
	{
		SpawnTransform = Projectile->GetSpawnTransform();
	}
	else if (AAbilityActor* AbilityActor = Cast<AAbilityActor>(Spawned))
	{
		SpawnTransform = AbilityActor->GetSpawnTransform();
	}

	Spawned->FinishSpawning(SpawnTransform);
}

void UAbilitySlot::TickAbility(float DeltaTime)
{
	if (Ticker.ShouldTick(DeltaTime))
		IAbilityLifecycle::Execute_OnTick(this);
}


void UAbilitySlot::EndAbility()
{
	if (bHasEnded) return;
	bHasEnded = true;

	const FVector EndLocation = MyCaster ? MyCaster->GetActorLocation() : FVector::ZeroVector;
	IAbilityLifecycle::Execute_OnEnd(this, EndLocation);
}

void UAbilitySlot::KillAbility()
{
	MyCaster = nullptr;
}

void UAbilitySlot::ReportAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload)
{
	Payload.Ability = this;
	if (APlayerUnit* PlayerUnit = Cast<APlayerUnit>(MyCaster))
	{
		PlayerUnit->BroadcastAbilityEvent(EventTags, Payload);
	}
}
