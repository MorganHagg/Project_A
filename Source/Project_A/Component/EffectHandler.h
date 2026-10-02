#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Misc/GameplayEffect.h"
#include "../Misc/IntervalTicker.h"
#include "../Ability/Ability.h"
#include "EffectHandler.generated.h"

class AUnitBase;
class UStats;

// A GameplayEffect instance that is currently active on a target, with its
// own duration/interval progress. FGameplayEffect itself stays config-only.
USTRUCT()
struct FActiveGameplayEffect
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayEffect Effect;

	UPROPERTY()
	float DurationTimer = 0.f;

	UPROPERTY()
	FIntervalTicker Ticker;

	// Damage type of the ability that applied this effect, kept so every interval tick is mitigated the same way.
	UPROPERTY()
	EAbilityType AbilityType = EAbilityType::Magic;
};

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

	void UpdateEffect(float DeltaTime);
	void AddEffect(const FGameplayEffect& Effect, EAbilityType AbilityType);
	void RemoveEffect(const FGameplayEffect& Effect);
	void ApplyEffect(const FGameplayEffect& Effect, EAbilityType AbilityType);

	UPROPERTY()
	TArray<FActiveGameplayEffect> GameplayEffects;

private:
	bool IsTargetDead() const;
};
