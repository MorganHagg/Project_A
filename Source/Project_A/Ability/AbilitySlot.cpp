#include "AbilitySlot.h"
#include "../Unit/UnitBase.h"
#include "../Unit/PlayerUnit.h"
#include "../Component/AttributeComponent.h"
#include "Ability.h"
#include "Projectile.h"
#include "AbilityActor.h"

UAbilitySlot::UAbilitySlot()
{

}

void UAbilitySlot::SetupAbility(AUnitBase* NewCaster)
{
	MyCaster = NewCaster;
	World = MyCaster->GetWorld();

	// Checks
	check(MyCaster);
	check(World);
}

void UAbilitySlot::ActivateAbility(AAbility* InModifiedAbility)
{
	// Checked here, not only in UAbilitySystem - AControllerBase::ApplyModify activates slots directly.
	if (IsCasterDead())
	{
		return;
	}

	// cooldown/cost gating: deferred, out of scope for now - always proceeds when told to.
	bHasEnded = false;

	if (bActivateOnRelease)
	{
		SpawnedAbility.Reset();
		return;
	}

	SpawnedAbility = SpawnAbility(0.f, InModifiedAbility);
}

void UAbilitySlot::StartCharging()
{
	bIsCharging = true;
	ChargeStartTime = World->GetTimeSeconds();
}

void UAbilitySlot::ReleaseAbility()
{
	const bool bFire = bActivateOnRelease && bIsCharging;
	const float ChargeTime = GetChargeTime();

	EndAbility();

	// Spawned after EndAbility and not tracked as SpawnedAbility - a fired shot is on its own, and
	// isn't finished by a later end of this slot.
	if (bFire && !IsCasterDead())
	{
		SpawnAbility(ChargeTime);
	}
}

float UAbilitySlot::GetChargeTime() const
{
	if (!bIsCharging || !World)
	{
		return 0.f;
	}

	const float Elapsed = World->GetTimeSeconds() - ChargeStartTime;
	return MaxChargeTime > 0.f ? FMath::Min(Elapsed, MaxChargeTime) : Elapsed;
}

AAbility* UAbilitySlot::SpawnAbility(float ChargeTime, AAbility* InModifiedAbility)
{
	if (!ProductClass)
	{
		return nullptr;
	}

	AAbility* Spawned = World->SpawnActorDeferred<AAbility>(ProductClass, FTransform::Identity);
	if (!Spawned)
	{
		return nullptr;
	}

	Spawned->SetMyAbility(this);
	Spawned->SetMyCaster(MyCaster);
	Spawned->ChargeTime = ChargeTime;
	Spawned->ModifiedAbility = InModifiedAbility;

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
	return Spawned;
}


void UAbilitySlot::EndAbility()
{
	if (bHasEnded) return;
	bHasEnded = true;

	const float FinalChargeTime = GetChargeTime();
	bIsCharging = false;

	// Reset either way - an Ability that keeps living (e.g. a finished-growing wall) is no longer this
	// slot's to end.
	AAbility* Ability = SpawnedAbility.Get();
	SpawnedAbility.Reset();
	if (Ability)
	{
		// Before OnHoldEnded, so it can read the final value.
		Ability->ChargeTime = FinalChargeTime;
		if (Ability->Implements<UAbilityLifecycle>())
		{
			IAbilityLifecycle::Execute_OnHoldEnded(Ability);
		}
	}

	const FVector EndLocation = MyCaster ? MyCaster->GetActorLocation() : FVector::ZeroVector;
	IAbilityLifecycle::Execute_OnEnd(this, EndLocation);
}

bool UAbilitySlot::IsCasterDead() const
{
	return MyCaster && MyCaster->AttributeComponent && MyCaster->AttributeComponent->IsDead();
}

void UAbilitySlot::KillAbility()
{
	MyCaster = nullptr;
}

void UAbilitySlot::ReportAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload)
{
	if (APlayerUnit* PlayerUnit = Cast<APlayerUnit>(MyCaster))
	{
		PlayerUnit->BroadcastAbilityEvent(EventTags, Payload);
	}
}
