#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "../Misc/AttributeEffect.h"
#include "../Effect/OverTimeEffect.h"
#include "EffectComponent.generated.h"

class AUnitBase;
class UGrantedAbility;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_A_API UEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEffectComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	AUnitBase* MyTarget;

	// Applies Effect to MyTarget and returns what was applied. Instant: see ApplyInstantEffect. Timed: a
	// new UOverTimeEffect, following its Source's StackLimit / bMultipleCaster; returns what it applied
	// on application (Magnitude 0 if it ticks instead, or was ignored). Ignored once dead (Magnitude 0).
	FAttributeEffect ApplyEffect(const FAttributeEffect& Effect);

	// Applies Effect to MyTarget now - ignores Duration and Interval. Used by ApplyEffect and by
	// UOverTimeEffect for each of its applications. Magnitude is scaled by its Source's magnitude
	// multiplier; damage also gets stat scaling, crit and mitigation (see ApplyDamage). For damage/heal
	// the returned Magnitude is the final amount and bCrit is set if it crit.
	FAttributeEffect ApplyInstantEffect(const FAttributeEffect& Effect);

	// Applies a stat change owned by Source, kept until RemoveStatModifiers(Source). Magnitude is scaled
	// by Effect.Source's magnitude multiplier. Returns what was applied, or Magnitude 0 if it was
	// ignored because MyTarget is dead.
	FAttributeEffect ApplyStatModifier(const UObject* Source, const FAttributeEffect& Effect);

	// Removes every stat change Source applied through ApplyStatModifier.
	void RemoveStatModifiers(const UObject* Source);

	// Applies Passive's Effect to MyTarget as a permanent over-time effect of its EffectClass - Duration
	// is ignored, so it lasts until removed or MyTarget dies. Applied once on application, or on every
	// tick with an Interval. Reports Event.Applied.
	void ApplyPassive(UGrantedAbility* Passive);

	// Cleanses one effect instance (OnRemoved).
	UFUNCTION(BlueprintCallable, Category = "Effect")
	void RemoveOverTimeEffect(UOverTimeEffect* Effect);

	// Cleanses every active instance tagged EffectTag (OnRemoved).
	UFUNCTION(BlueprintCallable, Category = "Effect")
	void RemoveOverTimeEffectsByTag(UPARAM(meta = (Categories = "Ability")) FGameplayTag EffectTag);

	// Called by AUnitBase::HandleDeath - ends every active effect (OnUnitDeath) and stops ticking.
	void HandleUnitDeath();

	// Active over-time effects in application order (oldest first).
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	TArray<UOverTimeEffect*> OverTimeEffects;

private:
	// Damage pipeline: CalculateOutgoingDamage (caster side), then MitigateDamage (MyTarget's side),
	// then AUnitBase::ReceiveDamage. Effect.Magnitude is the unscaled damage.
	FAttributeEffect ApplyDamage(const FAttributeEffect& Effect);

	// Heal: magnitude multiplier, then ModifyIncomingHeal, then AUnitBase::ReceiveHeal. Heals don't
	// scale with stats and can't crit.
	FAttributeEffect ApplyHeal(const FAttributeEffect& Effect);

	// (Magnitude + Source's Coefficient x caster's stat) x Source's magnitude multiplier, then the crit
	// roll (caster's Crit as a percent chance, x CritMultiplier). The stat is Intellect for Magic,
	// Strength for Physical, none for True. Read live from Source and its caster; with no Source there's
	// no scaling or multiplier, and with no valid caster no stat scaling or crit.
	float CalculateOutgoingDamage(const FAttributeEffect& Effect, bool& bOutCrit) const;

	// Reduces Damage by MyTarget's mitigation stat for DamageType (MagicResist for Magic, Armour for
	// Physical, none for True) using Mit / (Mit + MitigationConstant), then ModifyIncomingDamage, then
	// clamps at 0 and rounds to whole damage.
	float MitigateDamage(float Damage, EDamageType DamageType);

	// Passes Amount through every active effect's ModifyIncomingDamage / ModifyIncomingHeal, oldest
	// first, and returns the result.
	float ModifyIncomingDamage(float Amount, EDamageType DamageType);
	float ModifyIncomingHeal(float Amount);

	// Effect.Source's magnitude multiplier, or 1 with no Source.
	static float GetMagnitudeMultiplier(const FAttributeEffect& Effect);

	// K in the mitigation formula - the stat value that gives 50% reduction.
	static constexpr float MitigationConstant = 400.f;

	static constexpr float CritMultiplier = 2.f;

	// The timed half of ApplyEffect: applies a new UOverTimeEffect for Effect.
	FAttributeEffect AddOverTimeEffect(const FAttributeEffect& Effect);

	void ResolveAndRemove(UOverTimeEffect* Effect, EOverTimeEffectEnd Reason);

	bool IsTargetDead() const;
};
