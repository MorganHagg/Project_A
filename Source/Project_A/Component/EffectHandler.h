#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "../Misc/GameplayEffect.h"
#include "../Effect/OverTimeEffect.h"
#include "EffectHandler.generated.h"

class AUnitBase;
class UOverTimeEffectSlot;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_A_API UEffectHandler : public UActorComponent
{
	GENERATED_BODY()

public:
	UEffectHandler();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	AUnitBase* MyTarget;

	// Applies one instant change to MyTarget and returns what was applied: for damage/heal, Magnitude
	// is the final amount (after mitigation and incoming modifiers). Ignored once dead (Magnitude 0).
	FGameplayEffect ApplyEffect(const FGameplayEffect& Effect);

	// Applies a new instance of Slot's effect, following Slot's StackLimit / bMultipleCaster.
	// Ignored once dead.
	void AddOverTimeEffect(UOverTimeEffectSlot* Slot);

	// Cleanses one effect instance (OnRemoved).
	UFUNCTION(BlueprintCallable, Category = "Effect")
	void RemoveOverTimeEffect(UOverTimeEffect* Effect);

	// Cleanses every active instance tagged EffectTag (OnRemoved).
	UFUNCTION(BlueprintCallable, Category = "Effect")
	void RemoveOverTimeEffectsByTag(UPARAM(meta = (Categories = "Effect")) FGameplayTag EffectTag);

	// Passes Amount through every active effect's ModifyIncomingDamage / ModifyIncomingHeal, oldest
	// first, and returns the result. Called by AUnitBase::ReceiveDamage / ReceiveHeal.
	float ModifyIncomingDamage(float Amount, EAbilityType AbilityType);
	float ModifyIncomingHeal(float Amount);

	// Called by AUnitBase::HandleDeath - ends every active effect (OnUnitDeath) and stops ticking.
	void HandleUnitDeath();

	// Active over-time effects in application order (oldest first).
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	TArray<UOverTimeEffect*> OverTimeEffects;

private:
	void ResolveAndRemove(UOverTimeEffect* Effect, EOverTimeEffectEnd Reason);

	bool IsTargetDead() const;
};
