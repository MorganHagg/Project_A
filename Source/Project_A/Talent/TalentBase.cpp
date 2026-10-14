#include "TalentBase.h"

#include "Project_A/Unit/PlayerUnit.h"
#include "Project_A/Component/AbilityComponent.h"
#include "Project_A/Ability/GrantedAbility.h"

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

UGrantedAbility* UTalentBase::GetGrantedAbility(FGameplayTag AbilityTag) const
{
	if (!MyPlayerUnit)
	{
		UE_LOG(LogTemp, Error, TEXT("UTalentBase::GetGrantedAbility called before BindToPlayerUnit."));
		return nullptr;
	}

	UAbilityComponent* AbilityComponent = MyPlayerUnit->FindComponentByClass<UAbilityComponent>();
	if (!AbilityComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("UTalentBase::GetGrantedAbility - %s has no AbilityComponent."), *MyPlayerUnit->GetName());
		return nullptr;
	}

	return AbilityComponent->FindGrantedAbility(AbilityTag);
}