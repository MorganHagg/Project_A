#pragma once
#include "CoreMinimal.h"
#include "../Misc/GameplayEffect.h"
#include "AbilityEventPayload.generated.h"

class AUnitBase;
class AAbility;
class UOverTimeEffect;

// Common payload sent with every ability-side delegation (AbilityActor/Projectile -> Ability -> PlayerUnit -> Talent).
// Each event only populates the fields relevant to it; the rest are left at their default (e.g. Target = nullptr).
USTRUCT(BlueprintType)
struct FAbilityEventPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	AUnitBase* Target = nullptr;

	// The Ability (AbilityActor/Projectile) that reported this event, if any. Populated for events
	// sourced from either so a listening talent can manipulate it directly (e.g. redirect a
	// projectile, or a pet's attack/death) - Cast to AProjectile/AAbilityActor for the concrete API.
	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	AAbility* Ability = nullptr;

	// The other Ability involved when this event came from an overlap (e.g. a Projectile that
	// overlapped this AbilityActor). Ability above always refers to the reporting instance itself -
	// this is the other one. Populated by AAbilityActor::HandleOverlap.
	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	AAbility* OverlappedAbility = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	FGameplayEffect AppliedEffect;

	// The over-time effect instance that reported this event (Effect.* events), if any.
	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	UOverTimeEffect* OverTimeEffect = nullptr;
};

// Per-instance hook, separate from the tag-based ReportAbilityEvent chain. Lets code/Blueprint
// that already holds a reference to one specific AbilityProduct (AProjectile, AAbilityActor)
// attach extra behavior to just that instance without going through the tag system. Declared
// once on AAbility as OnHit/OnFinish, inherited by both concrete types.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityEventDelegate, FAbilityEventPayload, Payload);
