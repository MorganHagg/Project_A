#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "../Misc/GameplayEffect.h"
#include "../Misc/IntervalTicker.h"
#include "../Ability/Ability.h"
#include "OverTimeEffect.generated.h"

class AUnitBase;
class UEffectHandler;
class UOverTimeEffectSlot;

// How an over-time effect stopped: Ended = its duration ran out (or it was pushed out by a newer
// stack), Removed = cleansed, UnitDeath = its target died.
enum class EOverTimeEffectEnd : uint8
{
	Ended,
	Removed,
	UnitDeath
};

// One stat change made through UOverTimeEffect::ApplyEffect, kept so it can be undone.
USTRUCT()
struct FModifiedStat
{
	GENERATED_BODY()

	UPROPERTY()
	EEffectOperation Operation = EEffectOperation::Add;

	UPROPERTY()
	EAttributeType Attribute = EAttributeType::Health;

	UPROPERTY()
	float Amount = 0.f;

	UPROPERTY()
	EAbilityType AbilityType = EAbilityType::Magic;
};

// ============================================================================
// UOverTimeEffect
// One application of an over-time effect (a burn, a buff) on one target. Owned and ticked by the
// target's UEffectHandler; configured by the caster's UOverTimeEffectSlot. Blueprint subclasses
// describe what the effect does through the On* hooks and ApplyEffect. Stat changes made through
// ApplyEffect are undone automatically however the effect stops - a designer never has to
// remove them by hand.
// ============================================================================

UCLASS(Abstract, Blueprintable)
class PROJECT_A_API UOverTimeEffect : public UObject
{
	GENERATED_BODY()

public:
	// Through the owning UEffectHandler, so world-context Blueprint nodes work inside effects.
	virtual UWorld* GetWorld() const override;

	// -- Context --

	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	UOverTimeEffectSlot* MySlot = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	AUnitBase* MyTarget = nullptr;

	// May be null - the caster can be gone while its effect is still running.
	UFUNCTION(BlueprintPure, Category = "Effect")
	AUnitBase* GetCaster() const;

	UFUNCTION(BlueprintPure, Category = "Effect")
	FGameplayTag GetEffectTag() const;

	// -- Applying --

	// Applies one instant change to MyTarget. Add/Subtract on Health damages (mitigated by
	// AbilityType) or heals and stays. Every other combination is a stat change (see ModifyStat),
	// undone automatically when this effect stops.
	UFUNCTION(BlueprintCallable, Category = "Effect")
	void ApplyEffect(EEffectOperation Operation, EAttributeType Attribute, float Amount, EAbilityType AbilityType);

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

	// -- Driven by UEffectHandler --

	// Starts this application: reads Duration/Interval from Slot, then runs OnApplied.
	void Begin(UOverTimeEffectSlot* Slot, AUnitBase* Target);

	// Fires the ticks due this frame. Returns true once the duration has run out - the handler
	// then ends it.
	bool Advance(float DeltaTime);

	// Runs the hook for Reason, then undoes every stat change, then reports. Once only.
	void Resolve(EOverTimeEffectEnd Reason);

	bool IsResolved() const { return bResolved; }

protected:
	// Applies a stat change and records it in ModifiedStats for RevertModifyStat.
	void ModifyStat(EEffectOperation Operation, EAttributeType Attribute, float Amount, EAbilityType AbilityType);

	// Undoes every recorded stat change, newest first.
	void RevertModifyStat();

private:
	UEffectHandler* GetHandler() const;

	// Reports {EffectTag, Event.<EventName>} to the caster's PlayerUnit, for talents.
	void ReportEvent(const TCHAR* EventName);

	UPROPERTY()
	TArray<FModifiedStat> ModifiedStats;

	// Weak: the caster can die or be destroyed while this effect runs.
	TWeakObjectPtr<AUnitBase> MyCaster;

	FIntervalTicker Ticker;

	bool bResolved = false;
};
