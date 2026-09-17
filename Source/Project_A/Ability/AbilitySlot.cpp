#include "AbilitySlot.h"
#include "../Unit/UnitBase.h"
#include "../Unit/PlayerUnit.h"
#include "../Unit/EnemyUnit.h"
#include "Gameframework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "Ability.h"
#include "Projectile.h"
#include "AbilityActor.h"
#include "../Misc/GameplayEffect.h"
#include "../Component/EffectHandler.h"

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

	FAbilityEventPayload FinishPayload;
	FinishPayload.Location = EndLocation;
	ReportAbilityEvent(ComposeEventTags(TEXT("Finish")), FinishPayload);
}

void UAbilitySlot::KillAbility()
{
	MyCaster = nullptr;
}

// ============================================================================
// Execute library
// ============================================================================
bool UAbilitySlot::MatchesSelection(const AUnitBase* Unit, ETargetSelection TargetSelection)
{
	switch (TargetSelection)
	{
	case ETargetSelection::PlayerUnit:
		return Unit->IsA<APlayerUnit>();
	case ETargetSelection::EnemyUnit:
		return Unit->IsA<AEnemyUnit>();
	case ETargetSelection::All:
	default:
		return true;
	}
}

AUnitBase* UAbilitySlot::Target_Single(ETargetSelection TargetSelection, FVector Location)
{
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(TargetAcceptanceRadius);
	FCollisionQueryParams Params;

	AUnitBase* Target = nullptr;
	float ClosestDistSq = 0.f;

	if (World && World->OverlapMultiByChannel(Overlaps, Location, FQuat::Identity, ECC_Pawn, Sphere, Params))
	{
		for (FOverlapResult& Overlap : Overlaps)
		{
			AUnitBase* Unit = Cast<AUnitBase>(Overlap.GetActor());
			if (!Unit || !MatchesSelection(Unit, TargetSelection))
			{
				continue;
			}

			const float DistSq = FVector::DistSquared(Unit->GetActorLocation(), Location);
			if (!Target || DistSq < ClosestDistSq)
			{
				Target = Unit;
				ClosestDistSq = DistSq;
			}
		}
	}

	return Target;
}

void UAbilitySlot::ApplyEffect(AUnitBase* Target, const FGameplayEffect& Effect)
{
	if (UEffectHandler* EffectHandler = Target->FindComponentByClass<UEffectHandler>())
	{
		EffectHandler->AddEffect(Effect);
	}
}

TArray<AUnitBase*> UAbilitySlot::Target_AOE(ETargetSelection TargetSelection, FVector Location, float Radius)
{
	TArray<AUnitBase*> FoundUnits;
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;

	if (World && World->OverlapMultiByChannel(Overlaps, Location, FQuat::Identity, ECC_Pawn, Sphere, Params))
	{
		for (FOverlapResult& Overlap : Overlaps)
		{
			AUnitBase* Unit = Cast<AUnitBase>(Overlap.GetActor());
			if (Unit && MatchesSelection(Unit, TargetSelection))
			{
				FoundUnits.Add(Unit);
			}
		}
	}

	return FoundUnits;
}

void UAbilitySlot::ReportAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload)
{
	Payload.Ability = this;
	if (APlayerUnit* PlayerUnit = Cast<APlayerUnit>(MyCaster))
	{
		PlayerUnit->BroadcastAbilityEvent(EventTags, Payload);
	}
}

FGameplayTagContainer UAbilitySlot::ComposeEventTags(const TCHAR* Suffix) const
{
	if (!AbilityTag.IsValid())
	{
		return FGameplayTagContainer();
	}

	FGameplayTagContainer Tags;
	Tags.AddTag(AbilityTag);

	const FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(FName(*(FString(TEXT("Event.")) + Suffix)));
	if (EventTag.IsValid())
	{
		Tags.AddTag(EventTag);
	}

	return Tags;
}