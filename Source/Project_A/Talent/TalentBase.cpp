#include "TalentBase.h"

#include "Project_A/Unit/PlayerUnit.h"
#include "Project_A/Component/AbilitySystem.h"
#include "Project_A/Ability/AbilitySlot.h"
#include "Project_A/Effect/OverTimeEffectSlot.h"

void UTalentBase::BindToPlayerUnit(APlayerUnit* PlayerUnit)
{
	MyPlayerUnit = PlayerUnit;
	if (!PlayerUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to bind Talent to a null PlayerUnit"));
		return;
	}

	PlayerUnit->OnAbilityEvent.AddDynamic(this, &UTalentBase::HandleAbilityEvent);

	OnSetup();
}

void UTalentBase::HandleAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload)
{
	// RequiredTags.IsEmpty() is guarded explicitly - FGameplayTagContainer::HasAll() considers an
	// empty requirement trivially satisfied, which would otherwise make an unconfigured talent
	// fire on every event instead of staying inert.
	if (!RequiredTags.IsEmpty() && EventTags.HasAll(RequiredTags))
	{
		OnAbilityEvent(Payload);
	}
}

UAbilitySlot* UTalentBase::GetAbilitySlot(FGameplayTag AbilityTag) const
{
	if (!MyPlayerUnit)
	{
		UE_LOG(LogTemp, Error, TEXT("UTalentBase::GetAbilitySlot called before BindToPlayerUnit."));
		return nullptr;
	}

	UAbilitySystem* AbilitySystem = MyPlayerUnit->FindComponentByClass<UAbilitySystem>();
	if (!AbilitySystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UTalentBase::GetAbilitySlot - %s has no AbilitySystem component."), *MyPlayerUnit->GetName());
		return nullptr;
	}

	return AbilitySystem->FindSlot(AbilityTag);
}

UOverTimeEffectSlot* UTalentBase::GetEffectSlot(FGameplayTag EffectTag) const
{
	if (!MyPlayerUnit)
	{
		UE_LOG(LogTemp, Error, TEXT("UTalentBase::GetEffectSlot called before BindToPlayerUnit."));
		return nullptr;
	}

	UAbilitySystem* AbilitySystem = MyPlayerUnit->FindComponentByClass<UAbilitySystem>();
	if (!AbilitySystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UTalentBase::GetEffectSlot - %s has no AbilitySystem component."), *MyPlayerUnit->GetName());
		return nullptr;
	}

	return AbilitySystem->FindEffectSlot(EffectTag);
}