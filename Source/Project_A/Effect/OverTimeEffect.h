#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "../Misc/AttributeEffect.h"
#include "../Misc/IntervalTicker.h"
#include "OverTimeEffect.generated.h"

class AUnitBase;
class UEffectComponent;
class UGrantedOverTimeEffect;

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
// One application of an over-time effect (a burn, a buff) on one target. Owned and ticked by the
// target's UEffectComponent; configured by the caster's UGrantedOverTimeEffect. Generic by default: it
// applies the granted effect's Effect once on application (Interval 0) or on every tick (Interval > 0).
// Optional Blueprint subclasses add behaviour on top through the On* hooks and ApplyEffect. Stat
// changes made through ApplyEffect are undone automatically however the effect stops - a designer
// never has to remove them by hand.
// ============================================================================

UCLASS(Blueprintable)
class PROJECT_A_API UOverTimeEffect : public UObject
{
	GENERATED_BODY()

public:
	// Through the owning UEffectComponent, so world-context Blueprint nodes work inside effects.
	virtual UWorld* GetWorld() const override;

	// -- Context --

	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	UGrantedOverTimeEffect* MyGrantedEffect = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	AUnitBase* MyTarget = nullptr;

	// May be null - the caster can be gone while its effect is still running.
	UFUNCTION(BlueprintPure, Category = "Effect")
	AUnitBase* GetCaster() const;

	UFUNCTION(BlueprintPure, Category = "Effect")
	FGameplayTag GetEffectTag() const;

	// -- Applying --

	// Applies one instant change to MyTarget. Add/Subtract on Health damages (mitigated by
	// Effect.DamageType) or heals and stays. Every other combination is a stat change (see
	// ModifyStat), undone automatically when this effect stops. Returns what was applied - for
	// damage/heal, the final amount (see UEffectComponent::ApplyEffect).
	UFUNCTION(BlueprintCallable, Category = "Effect")
	FAttributeEffect ApplyEffect(const FAttributeEffect& Effect);

	// True once a stat change has been made that RevertModifyStat will undo.
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	bool bModifiedStat = false;

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

	// Starts this application: reads Duration/Interval from GrantedEffect, applies its Effect if
	// Interval is 0, then runs OnApplied.
	void Begin(UGrantedOverTimeEffect* GrantedEffect, AUnitBase* Target);

	// Fires the ticks due this frame. Returns true once the duration has run out - the handler
	// then ends it.
	bool Advance(float DeltaTime);

	// Runs the hook for Reason, then undoes every stat change, then reports. Once only.
	void Resolve(EOverTimeEffectEnd Reason);

	bool IsResolved() const { return bResolved; }

protected:
	// Applies a stat change and records it in ModifiedStats for RevertModifyStat.
	FAttributeEffect ModifyStat(const FAttributeEffect& Effect);

	// Undoes every recorded stat change, newest first.
	void RevertModifyStat();

private:
	UEffectComponent* GetHandler() const;

	// Applies MyGrantedEffect's Effect through ApplyEffect and returns what it applied (Magnitude 0 = nothing).
	// Runs outside the hooks so an override can't skip it.
	FAttributeEffect ApplyGrantedEffect();

	// Reports {EffectTag, Event.<EventName>} to the caster's PlayerUnit, for talents. AppliedEffect is
	// the granted Effect this event applied, if any.
	void ReportEvent(const TCHAR* EventName, const FAttributeEffect& AppliedEffect = FAttributeEffect());

	UPROPERTY()
	TArray<FAttributeEffect> ModifiedStats;

	// Weak: the caster can die or be destroyed while this effect runs.
	TWeakObjectPtr<AUnitBase> MyCaster;

	FIntervalTicker Ticker;

	bool bResolved = false;
};
