#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "../Ability/Ability.h"
#include "OverTimeEffectSlot.generated.h"

class AUnitBase;
class UOverTimeEffect;
class UOverTimeEffectDataAsset;

// ============================================================================
// UOverTimeEffectSlot
// A caster's granted over-time effect: the configuration every application of it uses. Lives in
// the caster's UAbilitySystem::GrantedOverTimeEffects, built from a UOverTimeEffectDataAsset.
// Talents find it by tag (UTalentBase::GetEffectSlot) and change its values directly.
// ============================================================================

UCLASS(BlueprintType)
class PROJECT_A_API UOverTimeEffectSlot : public UObject
{
	GENERATED_BODY()

public:
	// Copies Data's values and composes EffectTag from its EffectName.
	void SetupSlot(const UOverTimeEffectDataAsset* Data, AUnitBase* Caster);

	// Builds the identity tag "Effect.<InEffectName>".
	static FGameplayTag ComposeEffectTag(FName InEffectName, bool bErrorIfNotFound = true);

	// Applies a new instance of EffectClass to Target, through Target's UEffectHandler.
	UFUNCTION(BlueprintCallable, Category = "Effect")
	void ApplyOverTimeEffect(AUnitBase* Target);

	// -- Identity / context --

	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	FGameplayTag EffectTag;

	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UOverTimeEffect> EffectClass;

	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	AUnitBase* MyCaster = nullptr;

	// -- Tunables (see UOverTimeEffectDataAsset) --

	UPROPERTY(BlueprintReadWrite, Category = "Effect")
	float Duration = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Effect")
	float Interval = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Effect")
	int32 StackLimit = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Effect")
	bool bMultipleCaster = true;

	UPROPERTY(BlueprintReadWrite, Category = "Effect")
	FGameplayEffect Effect;

	UPROPERTY(BlueprintReadWrite, Category = "Effect")
	EAbilityType AbilityType = EAbilityType::Magic;
};
