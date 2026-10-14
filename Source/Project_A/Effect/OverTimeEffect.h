#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "../Misc/AttributeEffect.h"
#include "../Misc/IntervalTicker.h"
#include "OverTimeEffect.generated.h"

class AUnitBase;
class UEffectComponent;
class UGrantedAbility;

// How an over-time effect stopped: Ended = its duration ran out (or it was pushed out by a newer
// stack), Removed = cleansed, UnitDeath = its target died.
enum class EOverTimeEffectEnd : uint8
{
	Ended,
	Removed,
	UnitDeath
};

// ============================================================================
// UOverTimeEffect
// One application of a timed effect (a burn, a buff) on one target. Owned and ticked by the
// target's UEffectComponent; created from a timed FAttributeEffect, usually one fetched from the
// caster's granted ability (AAbility::GetEffect). Generic by default: it applies that effect once on
// application (Interval 0) or on every tick (Interval > 0). The effect is snapshotted when applied;
// only its granted ability's magnitude multiplier is read live, on every application. Optional
// Blueprint subclasses add behaviour on top through the On* hooks and ApplyEffect. Stat changes made
// through ApplyEffect, and magnitude modifiers made through ModifyAbilityMagnitude, are undone
// automatically however the effect stops - a designer never has to remove them by hand.
// ============================================================================

UCLASS(Blueprintable)
class PROJECT_A_API UOverTimeEffect : public UObject
{
	GENERATED_BODY()

public:
	// Through the owning UEffectComponent, so world-context Blueprint nodes work inside effects.
	virtual UWorld* GetWorld() const override;

	// -- Context --

	// The granted ability this effect came from (its Source). Null for an effect made inline.
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	UGrantedAbility* MyGrantedAbility = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	AUnitBase* MyTarget = nullptr;

	// May be null - the caster can be gone while its effect is still running.
	UFUNCTION(BlueprintPure, Category = "Effect")
	AUnitBase* GetCaster() const;

	UFUNCTION(BlueprintPure, Category = "Effect")
	FGameplayTag GetEffectTag() const;

	// -- Applying --

	// Applies one change to MyTarget now - Duration and Interval are ignored. Add/Subtract on Health
	// damages (mitigated by Effect.DamageType) or heals and stays. Every other combination is a stat
	// change (see ModifyStat), undone automatically when this effect stops. Returns what was applied -
	// for damage/heal, the final amount (see UEffectComponent::ApplyInstantEffect).
	UFUNCTION(BlueprintCallable, Category = "Effect")
	FAttributeEffect ApplyEffect(const FAttributeEffect& Effect);

	// Adds Percent (whole numbers, 20 = +20%) to the magnitude of the caster's granted ability tagged
	// AbilityTag - every application of its effect, including timed effects already running. Undone
	// automatically when this effect stops. Logs an error if the caster wasn't granted it.
	UFUNCTION(BlueprintCallable, Category = "Effect")
	void ModifyAbilityMagnitude(UPARAM(meta = (Categories = "Ability")) FGameplayTag AbilityTag, float Percent);

	// -- Hooks --

	UFUNCTION(BlueprintImplementableEvent, Category = "Effect")
	void OnApplied();

	UFUNCTION(BlueprintImplementableEvent, Category = "Effect")
	void OnTick();

	// Duration ran out, or a newer stack pushed this one out.
	UFUNCTION(BlueprintImplementableEvent, Category = "Effect")
	void OnEnded();

	// Cleansed.
	UFUNCTION(BlueprintImplementableEvent, Category = "Effect")
	void OnRemoved();

	UFUNCTION(BlueprintImplementableEvent, Category = "Effect")
	void OnUnitDeath();

	// -- Incoming damage/heal --

	// Called before MyTarget takes damage (already mitigated by DamageType); return the amount it
	// should take instead. Default: unchanged. The result is clamped at 0 by AUnitBase.
	UFUNCTION(BlueprintNativeEvent, Category = "Effect")
	float ModifyIncomingDamage(float Amount, EDamageType DamageType);

	// Called before MyTarget is healed; return the amount it should heal instead. Default:
	// unchanged. The result is clamped at 0 by AUnitBase.
	UFUNCTION(BlueprintNativeEvent, Category = "Effect")
	float ModifyIncomingHeal(float Amount);

	// -- Driven by UEffectComponent --

	// Starts this application: snapshots Effect, starts its Duration/Interval, applies it if Interval
	// is 0, then runs OnApplied. Returns what was applied (Magnitude 0 = nothing yet).
	FAttributeEffect Begin(const FAttributeEffect& Effect, AUnitBase* Target);

	// Fires the ticks due this frame. Returns true once the duration has run out - the handler
	// then ends it.
	bool Advance(float DeltaTime);

	// Runs the hook for Reason, then undoes every stat change, then reports. Once only.
	void Resolve(EOverTimeEffectEnd Reason);

	bool IsResolved() const { return bResolved; }

protected:
	// Applies a stat change as a modifier owned by this effect, removed when it resolves.
	FAttributeEffect ModifyStat(const FAttributeEffect& Effect);

private:
	UEffectComponent* GetHandler() const;

	// Applies SnapshotEffect, scaled by MyGrantedAbility's current magnitude multiplier, through
	// ApplyEffect and returns what it applied (Magnitude 0 = nothing). Runs outside the hooks so an
	// override can't skip it.
	FAttributeEffect ApplySnapshotEffect();

	// Reports {EffectTag, Event.<EventName>} to the caster's PlayerUnit, for talents. AppliedEffect is
	// the snapshot effect this event applied, if any.
	void ReportEvent(const TCHAR* EventName, const FAttributeEffect& AppliedEffect = FAttributeEffect());

	// The effect as it was when applied (see Begin).
	UPROPERTY()
	FAttributeEffect SnapshotEffect;

	// Granted abilities this effect added a magnitude modifier to (ModifyAbilityMagnitude), for removal.
	TArray<TWeakObjectPtr<UGrantedAbility>> ModifiedAbilities;

	// Weak: the caster can die or be destroyed while this effect runs.
	TWeakObjectPtr<AUnitBase> MyCaster;

	FIntervalTicker Ticker;

	bool bResolved = false;
};
