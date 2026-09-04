#pragma once
#include "CoreMinimal.h"
#include "../Misc/GameplayEffect.h"
#include "AbilityEventPayload.generated.h"

class UAbility;
class AUnitBase;
class AAbilityProduct;

// Common payload sent with every ability-side delegation (AbilityActor/Projectile -> Ability -> PlayerUnit -> Talent).
// Each event only populates the fields relevant to it; the rest are left at their default (e.g. Target = nullptr).
USTRUCT(BlueprintType)
struct FAbilityEventPayload
{
	GENERATED_BODY()

	// The ability that reported this event. Always stamped by ReportAbilityEvent regardless of
	// what the caller passed in, so it's read-only in Blueprint - setting it manually would be
	// pointless, since it's overwritten before the payload goes anywhere.
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	UAbility* Ability = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	AUnitBase* Target = nullptr;

	// The AbilityActor/Projectile that reported this event, if any. Populated for events sourced
	// from either producer so a listening talent can manipulate it directly (e.g. redirect a
	// projectile, or a pet's attack/death) - Cast to AProjectile/AAbilityActor for the concrete API.
	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	AAbilityProduct* AbilityProduct = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	FGameplayEffect AppliedEffect;

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	float Magnitude = 0.f;
};

// Per-instance hook, separate from the tag-based ReportAbilityEvent chain. Lets code/Blueprint
// that already holds a reference to one specific AbilityProduct (AProjectile, AAbilityActor)
// attach extra behavior to just that instance without going through the tag system. Declared
// once on AAbilityProduct as OnHit/OnFinish, inherited by both concrete types.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityEventDelegate, FAbilityEventPayload, Payload);
