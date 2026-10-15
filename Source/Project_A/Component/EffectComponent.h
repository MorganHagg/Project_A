#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "../Misc/AttributeEffect.h"
#include "../Effect/OverTimeEffect.h"
#include "EffectComponent.generated.h"

class AUnitBase;

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

	// Applies Effect to MyTarget and returns what was applied. Instant: applied now, Magnitude scaled by
	// its Source's magnitude multiplier; for damage/heal the returned Magnitude is the final amount
	// (after mitigation and incoming modifiers). Timed: a new UOverTimeEffect, following its Source's
	// StackLimit / bMultipleCaster; returns what it applied on application (Magnitude 0 if it ticks
	// instead, or was ignored). Ignored once dead (Magnitude 0).
	FAttributeEffect ApplyEffect(const FAttributeEffect& Effect);

	// Applies Effect to MyTarget now, as given - ignores Duration, Interval and Source. Used by
	// ApplyEffect and by UOverTimeEffect for each of its applications.
	FAttributeEffect ApplyInstantEffect(const FAttributeEffect& Effect);

	// Applies a stat change owned by Source, kept until RemoveStatModifiers(Source). Returns Effect,
	// or Magnitude 0 if it was ignored because MyTarget is dead.
	FAttributeEffect ApplyStatModifier(const UObject* Source, const FAttributeEffect& Effect);

	// Removes every stat change Source applied through ApplyStatModifier.
	void RemoveStatModifiers(const UObject* Source);

	// Cleanses one effect instance (OnRemoved).
	UFUNCTION(BlueprintCallable, Category = "Effect")
	void RemoveOverTimeEffect(UOverTimeEffect* Effect);

	// Cleanses every active instance tagged EffectTag (OnRemoved).
	UFUNCTION(BlueprintCallable, Category = "Effect")
	void RemoveOverTimeEffectsByTag(UPARAM(meta = (Categories = "Ability")) FGameplayTag EffectTag);

	// Passes Amount through every active effect's ModifyIncomingDamage / ModifyIncomingHeal, oldest
	// first, and returns the result. Called by AUnitBase::ReceiveDamage / ReceiveHeal.
	float ModifyIncomingDamage(float Amount, EDamageType DamageType);
	float ModifyIncomingHeal(float Amount);

	// Called by AUnitBase::HandleDeath - ends every active effect (OnUnitDeath) and stops ticking.
	void HandleUnitDeath();

	// Active over-time effects in application order (oldest first).
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	TArray<UOverTimeEffect*> OverTimeEffects;

private:
	// The timed half of ApplyEffect: applies a new UOverTimeEffect for Effect.
	FAttributeEffect AddOverTimeEffect(const FAttributeEffect& Effect);

	void ResolveAndRemove(UOverTimeEffect* Effect, EOverTimeEffectEnd Reason);

	bool IsTargetDead() const;
};
