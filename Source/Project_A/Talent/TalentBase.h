#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "../Ability/AbilityEventPayload.h"
#include "TalentBase.generated.h"

class APlayerUnit;
class UOverTimeEffectSlot;

UCLASS(Blueprintable)
class PROJECT_A_API UTalentBase : public UObject
{
	GENERATED_BODY()
public:
	// The set of facet tags this talent requires to all be present on a reported event (AND, not
	// OR - see HandleAbilityEvent). Events report independent facets: an ability identity tag
	// (e.g. "Ability.Fireball") plus one or more event-category tags (e.g. "Event.TargetHit",
	// "Event.Crit"). {Event.Crit} reacts to a crit from any ability; {Ability.Fireball, Event.Crit}
	// reacts only to Fireball's crits. Left empty, the talent never fires - it isn't a wildcard.
	// A talent that needs to react to more than one distinct combination should be split into
	// separate talents, one per combination.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent", meta = (Categories = "Ability,Effect,Event"))
	FGameplayTagContainer RequiredTags;

	// Binds this talent to its owning PlayerUnit - the sole delegation point for talents.
	UFUNCTION(BlueprintCallable)
	void BindToPlayerUnit(APlayerUnit* PlayerUnit);

	UFUNCTION(BlueprintImplementableEvent, Category = "Talent")
	void OnSetup();

	UPROPERTY(BlueprintReadWrite)
	APlayerUnit* MyPlayerUnit;

	// Native filter: forwards to OnAbilityEvent only if EventTags has every tag in RequiredTags.
	UFUNCTION()
	void HandleAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload);

	UFUNCTION(BlueprintImplementableEvent, Category = "Talent")
	void OnAbilityEvent(FAbilityEventPayload Payload);

	// Finds MyPlayerUnit's granted ability slot by identity tag (e.g. "Ability.Fireball"), so the
	// talent can change its values (Cost, CoolDown, MagnitudeMultiplier). Logs an error and returns
	// nullptr if MyPlayerUnit isn't bound yet, has no AbilitySystem, or has no slot with that tag.
	UFUNCTION(BlueprintCallable, Category = "Talent")
	UAbilitySlot* GetAbilitySlot(UPARAM(meta = (Categories = "Ability")) FGameplayTag AbilityTag) const;

	// Finds MyPlayerUnit's granted over-time effect slot by identity tag (e.g. "Effect.Burn"), so
	// the talent can change its values (Duration, Interval, StackLimit, bMultipleCaster). Logs an
	// error and returns nullptr like GetAbilitySlot.
	UFUNCTION(BlueprintCallable, Category = "Talent")
	UOverTimeEffectSlot* GetEffectSlot(UPARAM(meta = (Categories = "Effect")) FGameplayTag EffectTag) const;
};
