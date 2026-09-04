#pragma once
#include "CoreMinimal.h"
#include "../Misc/GameplayEffect.h"
#include "AbilityEventPayload.generated.h"

class UAbility;
class AUnitBase;
class AProjectile;

// Common payload sent with every ability-side delegation (AbilityActor/Projectile -> Ability -> PlayerUnit -> Talent).
// Each event only populates the fields relevant to it; the rest are left at their default (e.g. Target = nullptr).
USTRUCT(BlueprintType)
struct FAbilityEventPayload
{
	GENERATED_BODY()

	// The ability that reported this event. Always populated (overwritten by
	// UAbility::ReportAbilityEvent regardless of what the caller passed in).
	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	UAbility* Ability = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	AUnitBase* Target = nullptr;

	// The projectile that reported this event, if any. Populated for projectile-sourced
	// events so a listening talent can manipulate it directly (e.g. redirect, destroy).
	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	AProjectile* Projectile = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	FGameplayEffect AppliedEffect;

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	float Magnitude = 0.f;
};
