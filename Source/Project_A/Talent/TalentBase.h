#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "../Ability/AbilityEventPayload.h"
#include "TalentBase.generated.h"

class APlayerUnit;

UCLASS(Blueprintable)
class PROJECT_A_API UTalentBase : public UObject
{
	GENERATED_BODY()
public:
	// The single event this talent reacts to. Hierarchical - listening to a parent tag
	// (e.g. "Ability.Fireball") also catches its children (e.g. "Ability.Fireball.Finish").
	// A talent that needs to react to more than one distinct event should be split into
	// separate talents, one per event.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	FGameplayTag ListenTag;

	// Binds this talent to its owning PlayerUnit - the sole delegation point for talents.
	UFUNCTION(BlueprintCallable)
	void BindToPlayerUnit(APlayerUnit* PlayerUnit);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSetup();

	UPROPERTY(BlueprintReadWrite)
	APlayerUnit* MyPlayerUnit;

	// Native filter: forwards to OnAbilityEvent only if EventTag matches ListenTag.
	UFUNCTION()
	void HandleAbilityEvent(FGameplayTag EventTag, FAbilityEventPayload Payload);

	UFUNCTION(BlueprintImplementableEvent)
	void OnAbilityEvent(FAbilityEventPayload Payload);
};
